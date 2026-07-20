#ifndef MPPI_GPU_REWARDS_CUH_
#define MPPI_GPU_REWARDS_CUH_

/**
 * @file mppi_gpu_rewards.cuh
 * @brief MPPI 代价函数 — 移植自 nav2_mppi_controller 官方 critics
 *
 * 三组件架构:
 *   total = obstacle_weight  × obstacle_cost
 *         + tracking_weight  × tracking_cost
 *         + progress_weight  × progress_cost
 *
 * 各组件内部源自 nav2 对应 critic:
 *   obstacle_cost  → ObstaclesCritic  (碰撞检测)
 *   tracking_cost  → PathAlignCritic + PathAngleCritic + GoalAngleCritic
 *   progress_cost  → GoalCritic + PreferForwardCritic
 */

#include "mppi_gpu_common.cuh"

#ifndef CUDART_PI_F
#define CUDART_PI_F 3.14159265358979323846f
#endif

// nav2 默认阈值
#define GOAL_ANGLE_THRESHOLD  0.5f   // 距终点此距离内激活 GoalAngle
#define PATH_ANGLE_THRESHOLD  0.262f // 朝向偏差超此值(15°)触发 PathAngle 惩罚
#define HEADING_ANNEAL_DIST   0.5f   // 余弦退火距离

// ═══════════════════════════════════════════════════════════════════════════
// 组件 1: obstacle_cost (ObstaclesCritic)
// ═══════════════════════════════════════════════════════════════════════════

/// 足迹碰撞检测: 密集采样 + 双线性插值 + norm^4
/// nav2 ObstaclesCritic 等效: costmap 代价 → 幂律映射
__device__ float compute_obstacle_cost(
    float x, float y, float cos_theta, float sin_theta,
    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float fp_sample_spacing, float rear_obstacle_cost, float vx)
{
  if (costmap == nullptr || costmap_w <= 0 || costmap_h <= 0) return 0.0f;

  float flx = fp_front + fp_back, fly = fp_left + fp_right;
  int nx = (static_cast<int>(ceilf(flx / fp_sample_spacing)) + 1);
  int ny = (static_cast<int>(ceilf(fly / fp_sample_spacing)) + 1);
  nx = (nx < 2) ? 2 : ((nx > 10) ? 10 : nx);
  ny = (ny < 2) ? 2 : ((ny > 10) ? 10 : ny);
  float sx = (nx > 1) ? flx / (nx - 1) : 0.0f, sy = (ny > 1) ? fly / (ny - 1) : 0.0f;
  float acc = 0.0f;

  for (int iy = 0; iy < ny; ++iy) {
    float ly = -fp_right + iy * sy;
    for (int ix = 0; ix < nx; ++ix) {
      float lx = -fp_back + ix * sx;
      float wx = x + lx * cos_theta - ly * sin_theta;
      float wy = y + lx * sin_theta + ly * cos_theta;
      float val = costmap_bilinear(wx, wy, costmap, costmap_w, costmap_h,
                                   costmap_res, costmap_origin_x, costmap_origin_y);
      // 后方隐形障碍 (nav2: 无此机制, 我们保留)
      if (lx < 0.0f && vx < 0.0f && rear_obstacle_cost > 0.0f)
        val = fmaxf(val, rear_obstacle_cost);
      if (val >= 1.0f) { float n = val / 255.0f; acc += n * n * n * n; }
    }
  }
  return acc / static_cast<float>(nx * ny);
}

// ═══════════════════════════════════════════════════════════════════════════
// 组件 2: tracking_cost (PathAlignCritic + PathAngleCritic + GoalAngleCritic)
// ═══════════════════════════════════════════════════════════════════════════

/// PathAlignCritic: 轨迹点到全局路径的最短距离平方
/// 注意: 不再内部除以 horizon, 统一在代价组合处做 per-step 归一化
__device__ float compute_path_align_cost(
    float x, float y,
    const float* __restrict__ path_x, const float* __restrict__ path_y,
    int num_path_pts, int horizon)
{
  float min_sq = FLT_MAX;
  if (num_path_pts < 2 || path_x == nullptr) return 0.0f;
  for (int p = 0; p < num_path_pts - 1; ++p) {
    float d = point_to_segment_dist_sq(x, y, path_x[p], path_y[p], path_x[p+1], path_y[p+1]);
    if (d < min_sq) min_sq = d;
  }
  (void)horizon;  // 保留参数兼容性, 归一化移至代价组合
  return min_sq;
}

/// PathAngleCritic: 朝向与路径切线偏差惩罚 + 余弦退火到 goal_yaw
/// 180° 对称: 框体机器人 θ 与 θ+π 等价，始终选旋转量 ≤90° 的朝向
/// 注意: 不再内部除以 horizon, 统一在代价组合处做 per-step 归一化
__device__ float compute_path_angle_cost(
    float theta, float path_tangent, float goal_yaw,
    float dist_to_final, int horizon)
{
  (void)horizon;  // 保留参数兼容性, 归一化移至代价组合
  // GoalAngleCritic 区域: 距终点 < GOAL_ANGLE_THRESHOLD
  if (dist_to_final < GOAL_ANGLE_THRESHOLD) {
    float t = fminf(1.0f, dist_to_final / HEADING_ANNEAL_DIST);
    float alpha = (1.0f + cosf(CUDART_PI_F * t)) * 0.5f;
    float diff = sym_angle_diff(goal_yaw, path_tangent);
    float target = path_tangent + alpha * diff;
    float err = sym_angle_diff(theta, target);
    return 4.0f * err * err;
  }

  // PathAngleCritic 区域: 180° 对称 — 连续惩罚×4, 角度越大代价越高
  float err = sym_angle_diff(theta, path_tangent);
  return 4.0f * err * err;
}

// ═══════════════════════════════════════════════════════════════════════════
// 组件 3: progress_cost (GoalCritic + PreferForwardCritic)
// ═══════════════════════════════════════════════════════════════════════════

/// PreferForwardCritic: 匹配速度方向与"当前位置→前瞻点"方向
///   速度方向朝向前瞻点 → 奖励 (速度越快奖励越大)
///   速度方向背离前瞻点 → 惩罚 (5倍)
///   中间角度: 奖励对齐分量, 惩罚侧向分量, 防止推头
__device__ float compute_speed_reward(
    float vx, float vy, float target_vx_r, float target_vy_r)
{
  float speed = hypotf(vx, vy);
  if (speed < 1e-6f) return 0.0f;

  // 速度矢量角度 vs 前瞻点方向角度 (均在机器人坐标系)
  float vel_angle   = atan2f(vy, vx);
  float target_angle = atan2f(target_vy_r, target_vx_r);
  float angle_err = vel_angle - target_angle;
  while (angle_err > CUDART_PI_F)  angle_err -= 2.0f * CUDART_PI_F;
  while (angle_err < -CUDART_PI_F) angle_err += 2.0f * CUDART_PI_F;

  float alignment = cosf(angle_err);  // 1=完美对齐, 0=垂直, -1=完全背离

  // 背离前瞻点 (>90°): 5倍重罚, 防止反向
  if (alignment < 0.0f) return speed * 5.0f;

  // 奖励对齐分量, 重罚正交分量 (2× 增强横向抑制)
  return -speed * alignment + 2.0f * speed * fabsf(sinf(angle_err));
}

/// GoalCritic: 终端距离代价
__device__ float compute_terminal_dist_cost(float x, float y, float gx, float gy)
{
  float dx = gx - x, dy = gy - y;
  return sqrtf(dx * dx + dy * dy);
}

#endif
