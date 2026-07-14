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
 * 各组件内部:
 *   obstacle_cost  → ObstaclesCritic  (碰撞检测)
 *   tracking_cost  → PathAlignCritic + PathAngleCritic
 *                     PathAngle 直接使用 waypoint SE2 推荐朝向 (劣弧 ≤90°)
 *   progress_cost  → GoalCritic + PreferForwardCritic
 */

#include "mppi_gpu_common.cuh"

#ifndef CUDART_PI_F
#define CUDART_PI_F 3.14159265358979323846f
#endif

// nav2 默认阈值
#define PATH_ANGLE_THRESHOLD  0.262f // 朝向偏差超此值(15°)触发 PathAngle 惩罚

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

/// PathAngleCritic: 朝向与 SE2 推荐朝向偏差惩罚
///   直接使用 waypoint SE2 推荐朝向，无退火过渡
///   注意: 推荐朝向有前后分别, 不做 180° 对称 (由 heading 状态机自行决定旋转方向)
__device__ float compute_path_angle_cost(
    float theta, float rec_yaw, float goal_yaw,
    float dist_to_final, int horizon)
{
  (void)goal_yaw;
  (void)dist_to_final;
  (void)horizon;
  float err = normalize_angle(theta - rec_yaw);
  return 4.0f * err * err;
}

/// PathDeviationCritic: 偏离路径的陡增惩罚 (独立于 PathAlign, 单独的调节旋钮)
///   corridor 半宽内不罚; 超出后按"超出量²"惩罚, 形成一堵软墙把机器人挡回路径。
///   min_sq 复用 PathAlign 已算好的最近距离平方, 不再额外遍历路径。
__device__ float compute_path_deviation_cost(float min_sq, float corridor, float weight)
{
  if (weight <= 0.0f) return 0.0f;
  float dist   = sqrtf(min_sq);
  float excess = dist - corridor;
  if (excess <= 0.0f) return 0.0f;
  return weight * excess * excess;
}

// ═══════════════════════════════════════════════════════════════════════════
// 组件 3: progress_cost (GoalCritic + PreferForwardCritic)
// ═══════════════════════════════════════════════════════════════════════════

/// PreferForwardCritic: 速度方向必须对准前瞻点方向
///   使用角度误差线性惩罚 (替代弱梯度的 cosine alignment)
///   0° 偏差 → 全额奖励, ~19° → 中性, >19° → 递增惩罚
///   >90° 背离 → 重度惩罚
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

  float abs_err = fabsf(angle_err);

  // 背离前瞻点 (>90°): 重度惩罚, 禁止倒退
  if (abs_err > CUDART_PI_F / 2.0f)
    return speed * (5.0f + 3.0f * (abs_err - CUDART_PI_F / 2.0f));

  // 方向偏移线性惩罚: 0° → -speed, ~19° → 0, 更大偏差 → 正惩罚
  // 梯度 = 3*speed/rad, 远强于 cos 在 0° 处的零梯度
  return speed * (3.0f * abs_err - 1.0f);
}

/// GoalCritic: 终端距离代价
__device__ float compute_terminal_dist_cost(float x, float y, float gx, float gy)
{
  float dx = gx - x, dy = gy - y;
  return sqrtf(dx * dx + dy * dy);
}

#endif
