#ifndef MPPI_GPU_REWARDS_CUH_
#define MPPI_GPU_REWARDS_CUH_

/**
 * @file mppi_gpu_rewards.cuh
 * @brief MPPI 代价函数 — 三组件解耦架构
 *
 * 总代价由三个独立组件加权求和:
 *
 *   total_cost = obstacle_weight × obstacle_cost
 *              + heading_weight   × heading_cost
 *              + time_weight      × time_cost
 *
 * 每个组件内部无额外可调权重，只返回归一化的代价值。
 * 三个权重独立调节，互不干扰:
 *   - obstacle_weight ↑ → 更保守, 远离障碍物
 *   - heading_weight   ↑ → 更注重朝向对齐
 *   - time_weight      ↑ → 更快到达目标 (速度+效率)
 *
 * 组件明细:
 *   obstacle_cost = 足迹碰撞检测 (costmap 采样, norm^4)
 *   heading_cost   = 逐步混合朝向 + 终端朝向对齐
 *   time_cost      = 速度奖励 + 终端距离惩罚
 */

#include "mppi_gpu_common.cuh"

#ifndef CUDART_PI_F
#define CUDART_PI_F 3.14159265358979323846f
#endif

// ═══════════════════════════════════════════════════════════════════════════
// 组件 1: 障碍物代价 (Obstacle Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算障碍物碰撞代价 — 足迹密集网格采样 + norm^4 幂律
 *
 * 对碰撞箱足迹的每个采样点查询代价地图（双线性插值），
 * 取 (costmap_val / 255)^4 作为该点的碰撞代价，对所有点取平均。
 *
 * 后方隐形障碍 (运动方向感知):
 *   前进 (vx >= 0): 后方正在被离开 → 不施加代价地板
 *   后退 (vx <  0): 后方正在被进入 → 抬升后方足迹点代价至 rear_obstacle_cost
 *   rear_obstacle_cost = 0 时禁用。
 *
 * 公式:
 *   obstacle_cost = Σ max(cell_valᵢ, floorᵢ) / 255)^4 / total_fp
 *
 * @param  rear_obstacle_cost  后方隐形障碍代价地板 (0~255), 0=禁用
 * @return                     障碍物代价 ∈ [0, 1]
 */
__device__ float compute_obstacle_cost(
    float x, float y,
    float cos_theta, float sin_theta,
    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float fp_sample_spacing, float rear_obstacle_cost, float vx)
{
  if (costmap == nullptr || costmap_w <= 0 || costmap_h <= 0) {
    return 0.0f;
  }

  float fp_len_x = fp_front + fp_back;
  float fp_len_y = fp_left + fp_right;

  int nx = static_cast<int>(ceilf(fp_len_x / fp_sample_spacing)) + 1;
  int ny = static_cast<int>(ceilf(fp_len_y / fp_sample_spacing)) + 1;
  nx = (nx < 2) ? 2 : ((nx > 10) ? 10 : nx);
  ny = (ny < 2) ? 2 : ((ny > 10) ? 10 : ny);

  int total_fp = nx * ny;
  float cost_acc = 0.0f;
  float step_x = (nx > 1) ? fp_len_x / static_cast<float>(nx - 1) : 0.0f;
  float step_y = (ny > 1) ? fp_len_y / static_cast<float>(ny - 1) : 0.0f;

  for (int iy = 0; iy < ny; ++iy) {
    float fp_ly = -fp_right + static_cast<float>(iy) * step_y;
    for (int ix = 0; ix < nx; ++ix) {
      float fp_lx = -fp_back + static_cast<float>(ix) * step_x;

      float fp_wx = x + fp_lx * cos_theta - fp_ly * sin_theta;
      float fp_wy = y + fp_lx * sin_theta + fp_ly * cos_theta;

      float cell_val = costmap_bilinear(fp_wx, fp_wy, costmap,
                                        costmap_w, costmap_h,
                                        costmap_res, costmap_origin_x, costmap_origin_y);

      // 后方隐形障碍地板 (仅后退时对后方足迹点生效)
      if (fp_lx < 0.0f && vx < 0.0f && rear_obstacle_cost > 0.0f) {
        cell_val = fmaxf(cell_val, rear_obstacle_cost);
      }

      if (cell_val >= 1.0f) {
        float norm = cell_val / 255.0f;
        float norm2 = norm * norm;
        cost_acc += norm2 * norm2;  // norm^4
      }
    }
  }

  return cost_acc / static_cast<float>(total_fp);
}

// ═══════════════════════════════════════════════════════════════════════════
// 组件 2: 朝向代价 (Heading Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算混合朝向代价 (逐步) — 距离越近，朝向偏好从路径方向过渡到目标姿态
 *
 * 余弦退火混合:
 *   alpha = (1 + cos(π·t)) / 2,  t = clamp(dist / anneal_scale, 0, 1)
 *   blended = lerp(path_angle, goal_yaw, alpha)
 *   cost = (heading_err)² / horizon
 *
 * 退火效果: 远离时跟随路径方向，靠近时过渡到目标姿态朝向
 *
 * @return  朝向代价 (已按 horizon 归一化)
 */
__device__ inline float compute_heading_cost_step(
    float theta, float path_angle, float goal_yaw,
    float dist_to_goal, float anneal_scale, int horizon)
{
  float t = dist_to_goal / anneal_scale;
  t = fmaxf(0.0f, fminf(1.0f, t));

  float alpha = (1.0f + cosf(CUDART_PI_F * t)) * 0.5f;

  float diff = normalize_angle(goal_yaw - path_angle);
  float blended_heading = path_angle + alpha * diff;

  float heading_err = normalize_angle(theta - blended_heading);

  // 距离衰减: 远处朝向不重要, 近处才要求对准
  // dist=0 → 1.0, dist=0.5m → 0.5, dist=1.0m → 0.33, dist=2.0m → 0.2
  float dist_decay = 1.0f / (1.0f + dist_to_goal * 2.0f);

  return heading_err * heading_err * dist_decay / static_cast<float>(horizon);
}

/**
 * @brief 计算终端朝向代价 — 鼓励轨迹终点朝向目标
 *
 * 距离门控: 越近越重要
 *   goal_proximity = 1 / (1 + dist · 1.6)
 *
 * @return  终端朝向代价
 */
__device__ inline float compute_heading_cost_terminal(
    float x_final, float y_final, float theta_final,
    float target_x, float target_y)
{
  float dx = target_x - x_final;
  float dy = target_y - y_final;
  float dist_to_goal = sqrtf(dx * dx + dy * dy);

  float goal_proximity = 1.0f / (1.0f + dist_to_goal * 1.6f);

  float heading_to_goal = atan2f(dy, dx);
  float heading_err = normalize_angle(heading_to_goal - theta_final);

  return heading_err * heading_err * 0.8f * goal_proximity;
}

// ═══════════════════════════════════════════════════════════════════════════
// 组件 3: 耗时/效率代价 (Time Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算速度奖励 (逐步) — 鼓励沿路径方向快速前进
 *
 * speed_along_path = vx · path_vx + vy · path_vy  (点积)
 * 只奖励正向分量，不惩罚后退。
 *
 * @return  负值 = 奖励 (cost 减少)
 */
__device__ inline float compute_speed_reward(
    float vx, float vy,
    float path_vx_robot, float path_vy_robot,
    int horizon)
{
  (void)horizon;
  float speed_along_path = vx * path_vx_robot + vy * path_vy_robot;
  // 正向→奖励(负代价), 反向→惩罚(正代价), 零→无影响
  return -speed_along_path;
}

/**
 * @brief 计算终端距离代价 — 轨迹终点到目标点的剩余距离
 *
 * 标准 MPPI 终端代价: 防止选择在时域内看起来便宜但终点远离目标的短视轨迹。
 *
 * @return  终端距离代价 = dist_to_goal
 */
__device__ inline float compute_terminal_distance_cost(
    float x_final, float y_final,
    float target_x, float target_y)
{
  float dx = target_x - x_final;
  float dy = target_y - y_final;
  return sqrtf(dx * dx + dy * dy);
}

#endif  // MPPI_GPU_REWARDS_CUH_
