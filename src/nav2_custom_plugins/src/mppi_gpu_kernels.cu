/**
 * @file mppi_gpu_kernels.cu
 * @brief MPPI GPU 内核 — 采样、轨迹滚动、三组件代价计算与全时域加权求和
 *
 * 代价函数采用三组件解耦架构:
 *
 *   total_cost = obstacle_weight × obstacle_cost
 *              + heading_weight   × heading_cost
 *              + time_weight      × time_cost
 *
 * 每个组件内部无额外可调权重，三个权重独立调节，消除数据竞争。
 *
 * 障碍物代价: 足迹碰撞检测 (norm^4 幂律)
 * 朝向代价:   混合朝向对齐 (路径方向 → 目标姿态) + 终端朝向
 * 耗时代价:   速度奖励 + 终端距离
 *
 * 已删除的代价项 (及其理由):
 *   - cross-track error:  与避障对抗，全局路径仅提供走廊不要求精确跟随
 *   - per-step goal dist: 终端距离已覆盖，逐步惩罚抑制前期绕障
 *   - path length:        与 speed reward 互斥
 *   - proximity reward:   终端距离已覆盖
 *   - cost_vy_threshold:  全向底盘横向移动是特性
 *   - vel_direction_weight: 朝向代价已处理对齐
 *   - base_speed_floor:   采样侧 hack，代价函数不应干预采样
 */

#include <cuda_runtime.h>
#include <cfloat>

#include "mppi_gpu_rewards.cuh"

#ifndef CUDART_PI_F
#define CUDART_PI_F 3.14159265358979323846f
#endif

// ═══════════════════════════════════════════════════════════════════════════
// 常量
// ═══════════════════════════════════════════════════════════════════════════

/// 朝向退火距离: 在此距离内朝向偏好从路径方向过渡到目标姿态
#define HEADING_ANNEAL_SCALE 0.8f

// ═══════════════════════════════════════════════════════════════════════════
// 内核 1: MPPI 轨迹采样与代价计算
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief MPPI 采样与代价计算内核 — 每个 CUDA 线程独立处理一条轨迹
 *
 * 网格映射: 一维 grid，每个线程处理一条完整轨迹 (s ∈ [0, num_samples))
 *
 * 每条轨迹的处理流程:
 *   for t = 0 .. horizon-1:
 *     ├─ 时变噪声缩放 (时间+空间混合衰减 × 宏观距离缩放)
 *     ├─ 采样控制量 (base + 噪声 + guidance 混合, 纯旋转, 转弯增强)
 *     ├─ RK2 运动学积分
 *     ├─ obstacle_cost += compute_obstacle_cost(...)      // 组件1
 *     ├─ heading_cost   += compute_heading_cost_step(...)  // 组件2 (逐步)
 *     └─ time_cost      += compute_speed_reward(...)       // 组件3 (逐步)
 *   终端:
 *     ├─ heading_cost   += compute_heading_cost_terminal(...) // 组件2 (终端)
 *     └─ time_cost      += compute_terminal_distance_cost(...)// 组件3 (终端)
 *   组合:
 *     costs[s] = w_obs×obs + w_head×head + w_time×time
 */
__global__ void mppi_sample_kernel(
    const float* __restrict__ noise_vx,
    const float* __restrict__ noise_vy,
    const float* __restrict__ noise_w,

    const float* __restrict__ base_vx,
    const float* __restrict__ base_vy,
    const float* __restrict__ base_w,

    float current_x, float current_y, float current_theta,
    float target_x, float target_y,

    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,

    float dt, float min_v, float max_v, float max_vy, float max_w,

    // ── 三组件代价权重 (仅此三个可调) ──
    float obstacle_weight,
    float heading_weight,
    float time_weight,

    // ── 路径引导 (采样侧, 非代价) ──
    float path_dir_x, float path_dir_y,
    float guidance_weight,

    // ── 噪声参数 (采样侧) ──
    float cross_track_noise_scale,
    float noise_decay_rate,
    float exploration_range_scale,
    float spatial_decay_weight,
    float noise_scale_floor_vx,
    float noise_scale_floor_vy,
    float noise_scale_floor_w,
    float pure_rotation_ratio,
    int pure_rotation_steps,
    float pure_rotation_w_boost,

    // ── 转弯横向增强 (采样侧) ──
    float lateral_guidance_scale,
    float path_turn_angle,
    float turn_lateral_boost,
    float turn_lateral_max_boost,

    // ── 碰撞检测参数 ──
    float fp_front, float fp_back, float fp_left, float fp_right,
    float footprint_sample_spacing,
    float rear_obstacle_cost,

    // ── 行为参数 ──
    float lookahead_theta,
    float cost_discount,           // 代价时序折扣 γ, 近期权重=1, 远期=γ^t

    // ── 分层规划 ──
    float final_goal_x, float final_goal_y, float final_goal_yaw,
    int global_horizon, int num_global_trajs,

    int num_samples, int horizon,

    // ── 输出 ──
    float* __restrict__ costs,
    float* __restrict__ sampled_vx,
    float* __restrict__ sampled_vy,
    float* __restrict__ sampled_w,
    float* __restrict__ traj_x,
    float* __restrict__ traj_y)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  if (s >= num_samples) return;

  // ════════════════════════════════════════════════════════════════
  // 阶段 0: 分层规划 — 全局轨迹 vs 局部轨迹
  // ════════════════════════════════════════════════════════════════
  bool is_global = (s < num_global_trajs);
  float traj_target_x  = is_global ? final_goal_x  : target_x;
  float traj_target_y  = is_global ? final_goal_y  : target_y;
  int   traj_horizon   = is_global ? global_horizon : horizon;
  float traj_lookahead_theta = is_global ? final_goal_yaw : lookahead_theta;

  // ════════════════════════════════════════════════════════════════
  // 阶段 1: 初始化 — 坐标变换与转弯增强预计算
  // ════════════════════════════════════════════════════════════════

  float cos_rot = cosf(-current_theta);
  float sin_rot = sinf(-current_theta);

  // 前瞻点方向 (机器人坐标系) — 统一的采样/代价方向基准
  float dx_target = target_x - current_x;
  float dy_target = target_y - current_y;
  float target_x_robot = dx_target * cos_rot - dy_target * sin_rot;
  float target_y_robot = dx_target * sin_rot + dy_target * cos_rot;
  float lh_dist_init = sqrtf(target_x_robot * target_x_robot + target_y_robot * target_y_robot);
  float lh_vx_robot = (lh_dist_init > 1e-6f) ? (target_x_robot / lh_dist_init) : 1.0f;
  float lh_vy_robot = (lh_dist_init > 1e-6f) ? (target_y_robot / lh_dist_init) : 0.0f;
  float path_angle_error = atan2f(lh_vy_robot, lh_vx_robot);  // 机器人→前瞻点角度偏差

  // ── 转弯横向增强: 路径弯曲度 + 目标侧偏角 ──
  float turn_sharpness = fabsf(path_turn_angle) / (CUDART_PI_F / 3.0f);
  turn_sharpness = fminf(1.0f, turn_sharpness);

  float target_angle_abs = fabsf(atan2f(target_y_robot, target_x_robot));
  float target_side_ratio = fminf(1.0f, target_angle_abs / (CUDART_PI_F / 2.0f));

  float combined_lateral_boost = turn_sharpness * turn_lateral_boost
                               + target_side_ratio * turn_lateral_boost * 0.4f;
  float turn_lateral_mult = fminf(turn_lateral_max_boost, 1.0f + combined_lateral_boost);

  float combined_noise_boost = turn_sharpness * turn_lateral_boost * 0.4f
                             + target_side_ratio * turn_lateral_boost * 0.15f;
  float turn_noise_mult = 1.0f + combined_noise_boost;

  // ── 朝向退火预计算 ──
  float path_angle = atan2f(path_dir_y, path_dir_x);
  float lookahead_heading_error = normalize_angle(traj_lookahead_theta - current_theta);
  float dist_to_lookahead_init = sqrtf(
      (traj_target_x - current_x) * (traj_target_x - current_x)
    + (traj_target_y - current_y) * (traj_target_y - current_y));
  float t_omega = fminf(1.0f, dist_to_lookahead_init / HEADING_ANNEAL_SCALE);
  float alpha_omega = (1.0f + cosf(CUDART_PI_F * t_omega)) * 0.5f;
  float diff_omega = normalize_angle(lookahead_heading_error - path_angle_error);
  float blended_omega_error = path_angle_error + alpha_omega * diff_omega;

  // ── 轨迹初始状态 ──
  float x = current_x;
  float y = current_y;
  float theta = current_theta;

  // ════════════════════════════════════════════════════════════════
  // 三组件代价累加器
  // ════════════════════════════════════════════════════════════════
  float obstacle_cost_acc = 0.0f;
  float heading_cost_acc   = 0.0f;
  float time_cost_acc      = 0.0f;

  float cumulative_dist = 0.0f;
  float max_travel_dist = max_v * dt * horizon;

  // ════════════════════════════════════════════════════════════════
  // 阶段 2: 逐步前向预测
  // ════════════════════════════════════════════════════════════════
  for (int t = 0; t < horizon; ++t) {
    int idx = s * horizon + t;

    // ── 2.1 读取 base 控制量 ──
    float base_vx_t = base_vx[t];
    float base_vy_t = base_vy[t];
    float base_w_t = base_w[t];

    float base_speed = hypotf(base_vx_t, base_vy_t);

    // ── 2.2 时变噪声缩放 ──
    float progress_t = static_cast<float>(t) / fmaxf(1.0f, static_cast<float>(horizon - 1));
    float spatial_progress = fminf(1.0f, cumulative_dist / fmaxf(0.01f, max_travel_dist));
    float effective_progress = (1.0f - spatial_decay_weight) * progress_t
                             + spatial_decay_weight * spatial_progress;

    float noise_scale_t = (1.0f - noise_decay_rate * effective_progress)
                        * exploration_range_scale;
    float noise_scale_vx = fmaxf(noise_scale_floor_vx, noise_scale_t);
    float noise_scale_vy = fmaxf(noise_scale_floor_vy, noise_scale_t);
    float noise_scale_w  = fmaxf(noise_scale_floor_w,  noise_scale_t);

    // ── 2.3 控制量采样 ──
    float noise_vx_s = noise_vx[idx] * noise_scale_vx;
    float noise_vy_s = noise_vy[idx] * noise_scale_vy;
    float noise_w_s  = noise_w[idx] * noise_scale_w;

    // guidance 参考速度用 max_v (与 base 状态解耦, 防止低 base → 低 guidance → 更低 base 死循环)
    float guide_ref_v = fmaxf(base_speed, max_v * 0.5f);

    float vx = (1.0f - guidance_weight) * (base_vx_t + noise_vx_s)
             + guidance_weight * lh_vx_robot * guide_ref_v;

    float vy = (1.0f - guidance_weight) *
                   (base_vy_t + noise_vy_s * cross_track_noise_scale * turn_noise_mult)
             + guidance_weight * lh_vy_robot * guide_ref_v
                   * lateral_guidance_scale * turn_lateral_mult;

    float omega = base_w_t + noise_w_s
                + blended_omega_error * (0.5f / dt) * guidance_weight;

    // ── 纯角速度采样 ──
    if (static_cast<float>(s) < pure_rotation_ratio * static_cast<float>(num_samples)
        && t < pure_rotation_steps) {
      vx = 0.0f;
      vy = 0.0f;
      omega = base_w_t + noise_w_s * pure_rotation_w_boost
            + blended_omega_error * (0.5f / dt) * fmaxf(guidance_weight, 0.3f);
    }

    // ── 控制量钳制 ──
    vx    = fminf(max_v,  fmaxf(min_v,   vx));
    vy    = fminf(max_vy, fmaxf(-max_vy, vy));
    omega = fminf(max_w,  fmaxf(-max_w,  omega));

    // ── 2.4 平滑目标到达阻尼 ──
    // ── 2.5 存储控制量 ──
    sampled_vx[idx] = vx;
    sampled_vy[idx] = vy;
    sampled_w[idx]  = omega;

    // ════════════════════════════════════════════════════════════
    // 2.6 RK2 运动学积分
    // ════════════════════════════════════════════════════════════
    float half_dt = 0.5f * dt;
    float theta_mid = theta + omega * half_dt;

    float dx2 = vx * cosf(theta_mid) - vy * sinf(theta_mid);
    float dy2 = vx * sinf(theta_mid) + vy * cosf(theta_mid);

    float new_x = x + dx2 * dt;
    float new_y = y + dy2 * dt;
    float new_theta = theta + omega * dt;

    x = new_x;
    y = new_y;
    theta = new_theta;

    if (traj_x != nullptr && traj_y != nullptr) {
      traj_x[idx] = x;
      traj_y[idx] = y;
    }

    // ════════════════════════════════════════════════════════════
    // 2.7 三组件代价计算 (带时序折扣 γ^t)
    // ════════════════════════════════════════════════════════════
    float discount_t = powf(cost_discount, static_cast<float>(t));

    // 组件1: 障碍物代价
    obstacle_cost_acc += discount_t * compute_obstacle_cost(
        x, y, cosf(theta), sinf(theta),
        costmap, costmap_w, costmap_h,
        costmap_res, costmap_origin_x, costmap_origin_y,
        fp_front, fp_back, fp_left, fp_right,
        footprint_sample_spacing, rear_obstacle_cost, vx);

    // 机器人→目标点方向 (世界坐标系), 共享给朝向代价和速度奖励
    float dx_lh = traj_target_x - x;
    float dy_lh = traj_target_y - y;
    float dist_to_lookahead = sqrtf(dx_lh * dx_lh + dy_lh * dy_lh);
    float inv_dist = (dist_to_lookahead > 1e-6f) ? (1.0f / dist_to_lookahead) : 1.0f;
    float dir_x_w = dx_lh * inv_dist;  // 世界系单位向量
    float dir_y_w = dy_lh * inv_dist;
    // 旋转到机器人坐标系
    float target_vx_r = dir_x_w * cos_rot - dir_y_w * sin_rot;
    float target_vy_r = dir_x_w * sin_rot + dir_y_w * cos_rot;
    float dir_to_target = atan2f(dy_lh, dx_lh);

    // 组件2: 朝向代价 (逐步)
    //   朝向参考 = 机器人→目标点方向 (始终对准要去的地方)
    //   距终点 ≤ 0.5m: 余弦退火混入 goal_yaw (对齐目标姿态)
    {
      float dist_to_final = sqrtf(
          (final_goal_x - x) * (final_goal_x - x)
        + (final_goal_y - y) * (final_goal_y - y));
      float heading_target = (dist_to_final < 0.5f) ? traj_lookahead_theta : dir_to_target;
      heading_cost_acc += discount_t * compute_heading_cost_step(
          theta, dir_to_target, heading_target,
          dist_to_lookahead, HEADING_ANNEAL_SCALE, horizon);
    }

    // 组件3: 耗时代价 (逐步: 速度奖励 = 合成速度·前瞻点方向)
    time_cost_acc += discount_t * compute_speed_reward(
        vx, vy, target_vx_r, target_vy_r, horizon);
    // 平滑正则化: 轻量约束, 防止输出跳变但允许加速
    {
      float dvx = vx - base_vx_t, dvy = vy - base_vy_t, dw = omega - base_w_t;
      time_cost_acc += discount_t * 0.3f * (dvx*dvx + dvy*dvy + dw*dw);
    }
    // omega 惩罚: 轻量抑制不必要的转向
    time_cost_acc += discount_t * 0.15f * omega * omega;
    // 超速正则化
    {
      float spd = hypotf(vx, vy);
      float desired = 0.7f * max_v;
      if (spd > desired) {
        float excess = spd - desired;
        time_cost_acc += discount_t * 0.3f * excess * excess;
      }
    }
    // 横向漂移惩罚
    {
      float lateral = -vx * lh_vy_robot + vy * lh_vx_robot;
      time_cost_acc += discount_t * 0.3f * lateral * lateral;
    }

    // ── 累计行进距离 ──
    cumulative_dist += hypotf(x - (x - dx2 * dt), y - (y - dy2 * dt));
  }

  // ════════════════════════════════════════════════════════════════
  // 阶段 3: 终端代价 (权重 = γ^H)
  // ════════════════════════════════════════════════════════════════
  float discount_H = powf(cost_discount, static_cast<float>(horizon));

  // 组件2 (终端): 终点朝向目标
  heading_cost_acc += discount_H * compute_heading_cost_terminal(
      x, y, theta, traj_target_x, traj_target_y);

  // 组件3 (终端): 终点到目标的剩余距离
  time_cost_acc += discount_H * compute_terminal_distance_cost(
      x, y, traj_target_x, traj_target_y);

  // ════════════════════════════════════════════════════════════════
  // 阶段 4: 三组件加权组合
  // ════════════════════════════════════════════════════════════════
  costs[s] = obstacle_weight * obstacle_cost_acc
           + heading_weight   * heading_cost_acc
           + time_weight      * time_cost_acc;
}

// ═══════════════════════════════════════════════════════════════════════════
// 内核 2: 全时域加权求和 (不变)
// ═══════════════════════════════════════════════════════════════════════════

__global__ void mppi_weighted_sum_kernel(
    const float* __restrict__ costs,
    const float* __restrict__ sampled_vx,
    const float* __restrict__ sampled_vy,
    const float* __restrict__ sampled_w,
    float* __restrict__ result_seq,
    float min_cost,
    float lambda,
    int num_samples,
    int horizon)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  int t = blockIdx.y;

  if (s >= num_samples || t >= horizon) return;

  int idx = s * horizon + t;
  float weight = expf(-(costs[s] - min_cost) / lambda);

  int base = t * 4;
  atomicAdd(&result_seq[base + 0], weight * sampled_vx[idx]);
  atomicAdd(&result_seq[base + 1], weight * sampled_vy[idx]);
  atomicAdd(&result_seq[base + 2], weight * sampled_w[idx]);
  atomicAdd(&result_seq[base + 3], weight);
}

// ═══════════════════════════════════════════════════════════════════════════
// Host 端包装函数
// ═══════════════════════════════════════════════════════════════════════════

extern "C" {

int mppi_gpu_sample_and_cost(
    const float* noise_vx, const float* noise_vy, const float* noise_w,
    const float* base_vx, const float* base_vy, const float* base_w,
    float current_x, float current_y, float current_theta,
    float target_x, float target_y,
    const unsigned char* costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float dt, float min_v, float max_v, float max_vy, float max_w,
    float obstacle_weight,
    float heading_weight,
    float time_weight,
    float path_dir_x, float path_dir_y,
    float guidance_weight,
    float cross_track_noise_scale,
    float noise_decay_rate,
    float exploration_range_scale,
    float spatial_decay_weight,
    float noise_scale_floor_vx,
    float noise_scale_floor_vy,
    float noise_scale_floor_w,
    float pure_rotation_ratio,
    int pure_rotation_steps,
    float pure_rotation_w_boost,
    float lateral_guidance_scale,
    float path_turn_angle,
    float turn_lateral_boost,
    float turn_lateral_max_boost,
    float lookahead_theta,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float footprint_sample_spacing,
    float rear_obstacle_cost,
    float cost_discount,
    float final_goal_x, float final_goal_y, float final_goal_yaw,
    int global_horizon, int num_global_trajs,
    int num_samples, int horizon,
    float* d_noise_vx, float* d_noise_vy, float* d_noise_w,
    float* d_base_vx, float* d_base_vy, float* d_base_w,
    unsigned char* d_costmap,
    float* d_path_x, float* d_path_y,
    float* d_costs, float* d_sampled_vx, float* d_sampled_vy, float* d_sampled_w,
    float* d_traj_x, float* d_traj_y,
    cudaStream_t stream)
{
  size_t noise_bytes = num_samples * horizon * sizeof(float);
  size_t base_bytes = horizon * sizeof(float);
  size_t costmap_bytes = costmap_w * costmap_h * sizeof(unsigned char);

  cudaError_t err;

  err = cudaMemcpyAsync(d_noise_vx, noise_vx, noise_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 1;
  err = cudaMemcpyAsync(d_noise_vy, noise_vy, noise_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 2;
  err = cudaMemcpyAsync(d_noise_w, noise_w, noise_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 3;

  err = cudaMemcpyAsync(d_base_vx, base_vx, base_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 4;
  err = cudaMemcpyAsync(d_base_vy, base_vy, base_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 5;
  err = cudaMemcpyAsync(d_base_w, base_w, base_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 6;

  err = cudaMemcpyAsync(d_costmap, costmap, costmap_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 7;

  // 全局路径点不再上传 (cross-track error 已移除)

  int threads_per_block = 256;
  int blocks = (num_samples + threads_per_block - 1) / threads_per_block;

  mppi_sample_kernel<<<blocks, threads_per_block, 0, stream>>>(
      d_noise_vx, d_noise_vy, d_noise_w,
      d_base_vx, d_base_vy, d_base_w,
      current_x, current_y, current_theta,
      target_x, target_y,
      d_costmap,
      costmap_w, costmap_h,
      costmap_res, costmap_origin_x, costmap_origin_y,
      dt, min_v, max_v, max_vy, max_w,
      obstacle_weight,
      heading_weight,
      time_weight,
      path_dir_x, path_dir_y,
      guidance_weight,
      cross_track_noise_scale,
      noise_decay_rate,
      exploration_range_scale,
      spatial_decay_weight,
      noise_scale_floor_vx,
      noise_scale_floor_vy,
      noise_scale_floor_w,
      pure_rotation_ratio,
      pure_rotation_steps,
      pure_rotation_w_boost,
      lateral_guidance_scale,
      path_turn_angle,
      turn_lateral_boost,
      turn_lateral_max_boost,
      lookahead_theta,
      fp_front, fp_back, fp_left, fp_right,
      footprint_sample_spacing,
      rear_obstacle_cost,
      cost_discount,
      final_goal_x, final_goal_y, final_goal_yaw,
      global_horizon, num_global_trajs,
      num_samples, horizon,
      d_costs, d_sampled_vx, d_sampled_vy, d_sampled_w,
      d_traj_x, d_traj_y);

  err = cudaGetLastError();
  if (err != cudaSuccess) return 8;

  return 0;
}

int mppi_gpu_weighted_sum(
    const float* d_costs,
    const float* d_sampled_vx,
    const float* d_sampled_vy,
    const float* d_sampled_w,
    float* d_result_seq,
    float min_cost,
    float lambda,
    int num_samples,
    int horizon,
    cudaStream_t stream)
{
  cudaError_t err = cudaMemsetAsync(d_result_seq, 0,
                                     horizon * 4 * sizeof(float), stream);
  if (err != cudaSuccess) return 1;

  int threads_per_block = 256;
  int blocks_per_t = (num_samples + threads_per_block - 1) / threads_per_block;

  dim3 grid(blocks_per_t, horizon);
  mppi_weighted_sum_kernel<<<grid, threads_per_block, 0, stream>>>(
      d_costs, d_sampled_vx, d_sampled_vy, d_sampled_w,
      d_result_seq,
      min_cost, lambda,
      num_samples, horizon);

  err = cudaGetLastError();
  if (err != cudaSuccess) return 2;

  return 0;
}

}  // extern "C"
