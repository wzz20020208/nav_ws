/**
 * @file mppi_gpu_kernels.cu
 * @brief MPPI GPU 内核 — 归一化三组件代价架构
 *
 *   total = cost_scale × ( obs_ratio × obst_acc / horizon
 *                         + trk_ratio × track_acc / horizon
 *                         + spd_ratio × prog_acc / horizon )
 *
 *   默认占比: 障碍物 40% / 跟踪 30% / 速度 30%
 */

#include <cuda_runtime.h>
#include <cfloat>
#include "mppi_gpu_rewards.cuh"

#ifndef CUDART_PI_F
#define CUDART_PI_F 3.14159265358979323846f
#endif

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
    // ── 三组件归一化权重 (占比, 推荐 0.4+0.3+0.3) + 全局缩放 ──
    float cost_scale, float obstacle_ratio, float tracking_ratio, float speed_ratio,
    // ── 独立偏离路径代价 (软墙, 不受 cost_scale 缩放) ──
    float path_deviation_weight, float path_corridor,
    // ── 路径引导 ──
    float path_dir_x, float path_dir_y,
    // ── 噪声采样 ──
    float noise_decay_rate,
    float exploration_range_scale, float spatial_decay_weight,
    float noise_scale_floor_vx, float noise_scale_floor_vy, float noise_scale_floor_w,
    float pure_rotation_ratio, int pure_rotation_steps, float pure_rotation_w_boost,
    // ── 转弯增强 ──
    float lateral_guidance_scale, float path_turn_angle,
    float turn_lateral_boost, float turn_lateral_max_boost,
    // ── 碰撞检测 ──
    float fp_front, float fp_back, float fp_left, float fp_right,
    float footprint_sample_spacing, float rear_obstacle_cost,
    // ── 全局路径 (cross-track) ──
    const float* __restrict__ path_x, const float* __restrict__ path_y,
    int num_path_pts,
    // ── 朝向余弦退火 ──
    float goal_yaw,
    // ── 分层规划 ──
    float final_goal_x, float final_goal_y,
    int global_horizon, int num_global_trajs,
    int num_samples, int horizon,
    float* __restrict__ costs,
    float* __restrict__ sampled_vx, float* __restrict__ sampled_vy, float* __restrict__ sampled_w,
    float* __restrict__ traj_x, float* __restrict__ traj_y)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  if (s >= num_samples) return;

  // 统一 vx/vy 机制后不再使用的参数 (保留签名兼容性)
  (void)path_turn_angle; (void)turn_lateral_boost; (void)turn_lateral_max_boost;

  bool is_global = (s < num_global_trajs);
  float traj_target_x = is_global ? final_goal_x : target_x;
  float traj_target_y = is_global ? final_goal_y : target_y;
  int   traj_horizon  = is_global ? global_horizon : horizon;

  // ── 坐标系变换 ──
  float cos_rot = cosf(-current_theta), sin_rot = sinf(-current_theta);
  float path_tangent = atan2f(path_dir_y, path_dir_x);  // 世界系路径切线

  // ── 前瞻点方向 (机器人坐标系), 用于速度奖励和采样偏置 ──
  float dx_t = target_x - current_x, dy_t = target_y - current_y;
  float tx_r = dx_t * cos_rot - dy_t * sin_rot, ty_r = dx_t * sin_rot + dy_t * cos_rot;
  float lh_dist = hypotf(tx_r, ty_r);
  float lh_vx_r = tx_r / fmaxf(lh_dist, 1e-6f);
  float lh_vy_r = ty_r / fmaxf(lh_dist, 1e-6f);

  // ── 初始状态 ──
  float x = current_x, y = current_y, theta = current_theta;
  float obst_acc = 0.0f, track_acc = 0.0f, prog_acc = 0.0f, dev_acc = 0.0f;
  float cum_dist = 0.0f, max_travel = max_v * dt * horizon;

  for (int t = 0; t < horizon; ++t) {
    int idx = s * horizon + t;

    // 噪声缩放
    float prog_t = static_cast<float>(t) / fmaxf(1.0f, static_cast<float>(horizon - 1));
    float sp_prog = fminf(1.0f, cum_dist / fmaxf(0.01f, max_travel));
    float eff_prog = (1.0f - spatial_decay_weight) * prog_t + spatial_decay_weight * sp_prog;
    float ns_t = fmaxf(noise_scale_floor_vx,
        (1.0f - noise_decay_rate * eff_prog) * exploration_range_scale);
    float ns_vy = fmaxf(noise_scale_floor_vy, ns_t);
    float ns_w  = fmaxf(noise_scale_floor_w,  ns_t);

    // 采样 — vx/vy 混合: base+noise 与 lookahead 方向插值, 打破纯前向偏置
    // lateral_guidance_scale: 0=纯base+noise, 1=完全锚定lookahead
    float bvx = base_vx[t], bvy = base_vy[t], bw = base_w[t];
    // 参考速度取 max_v 的 30% 为下限, 避免 base≈0 时 guidance 退化为 0
    float ref_spd = fmaxf(hypotf(bvx, bvy), max_v * 0.3f);
    float g = lateral_guidance_scale;
    float vx = (1.0f - g) * (bvx + noise_vx[idx] * ns_t) + g * lh_vx_r * ref_spd;
    float vy = (1.0f - g) * (bvy + noise_vy[idx] * ns_vy) + g * lh_vy_r * ref_spd;
    float omega = bw + noise_w[idx] * ns_w;

    if (static_cast<float>(s) < pure_rotation_ratio * num_samples && t < pure_rotation_steps) {
      vx = 0.0f; vy = 0.0f;
      omega = bw + noise_w[idx] * ns_w * pure_rotation_w_boost;
    }

    vx = fminf(max_v, fmaxf(min_v, vx));
    vy = fminf(max_vy, fmaxf(-max_vy, vy));
    omega = fminf(max_w, fmaxf(-max_w, omega));

    sampled_vx[idx] = vx; sampled_vy[idx] = vy; sampled_w[idx] = omega;

    // RK2 积分
    float half_dt = 0.5f * dt;
    float theta_mid = theta + omega * half_dt;
    float dx2 = vx * cosf(theta_mid) - vy * sinf(theta_mid);
    float dy2 = vx * sinf(theta_mid) + vy * cosf(theta_mid);
    x += dx2 * dt; y += dy2 * dt; theta += omega * dt;
    if (traj_x) { traj_x[idx] = x; traj_y[idx] = y; }

    // ══ 三组件代价 (nav2 critics) ══

    // 1. ObstaclesCritic
    obst_acc += compute_obstacle_cost(x, y, cosf(theta), sinf(theta),
        costmap, costmap_w, costmap_h, costmap_res, costmap_origin_x, costmap_origin_y,
        fp_front, fp_back, fp_left, fp_right, footprint_sample_spacing,
        rear_obstacle_cost, vx);

    // 2. PathAlignCritic + PathAngleCritic → tracking
    float dist_to_final = sqrtf((final_goal_x - x) * (final_goal_x - x)
                              + (final_goal_y - y) * (final_goal_y - y));
    float min_path_sq = compute_path_align_cost(x, y, path_x, path_y, num_path_pts, horizon);
    track_acc += min_path_sq
               + compute_path_angle_cost(theta, path_tangent, goal_yaw,
                                         dist_to_final, horizon);

    // 2b. PathDeviationCritic → 独立偏离代价 (软墙, 复用 min_path_sq 不再遍历)
    dev_acc += compute_path_deviation_cost(min_path_sq, path_corridor, path_deviation_weight);

    // 3. PreferForwardCritic → progress (使用前瞻点方向, 非路径切线)
    prog_acc += compute_speed_reward(vx, vy, lh_vx_r, lh_vy_r);

    cum_dist += hypotf(dx2 * dt, dy2 * dt);
  }

  // GoalCritic: 终端距离
  prog_acc += compute_terminal_dist_cost(x, y, final_goal_x, final_goal_y);

  // ── 归一化代价: per-step 均值 × 占比权重 × 全局缩放 ──
  //   偏离代价 dev_acc 独立于 cost_scale, 作为绝对软墙叠加
  float inv_h = 1.0f / static_cast<float>(horizon);
  costs[s] = cost_scale * (obstacle_ratio * obst_acc * inv_h
                         + tracking_ratio * track_acc * inv_h
                         + speed_ratio     * prog_acc * inv_h)
           + dev_acc * inv_h;
}

// ═══════════════════════════════════════════════════════════════════════════
// 加权求和 (不变)
// ═══════════════════════════════════════════════════════════════════════════

__global__ void mppi_weighted_sum_kernel(
    const float* __restrict__ costs,
    const float* __restrict__ sampled_vx,
    const float* __restrict__ sampled_vy,
    const float* __restrict__ sampled_w,
    float* __restrict__ result_seq, float min_cost, float lambda,
    int num_samples, int horizon)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x, t = blockIdx.y;
  if (s >= num_samples || t >= horizon) return;
  float w = expf(-(costs[s] - min_cost) / lambda);
  int base = t * 4;
  atomicAdd(&result_seq[base + 0], w * sampled_vx[s * horizon + t]);
  atomicAdd(&result_seq[base + 1], w * sampled_vy[s * horizon + t]);
  atomicAdd(&result_seq[base + 2], w * sampled_w[s * horizon + t]);
  atomicAdd(&result_seq[base + 3], w);
}

// ═══════════════════════════════════════════════════════════════════════════
// Host wrapper
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
    float cost_scale, float obstacle_ratio, float tracking_ratio, float speed_ratio,
    float path_deviation_weight, float path_corridor,
    float path_dir_x, float path_dir_y,
    float noise_decay_rate,
    float exploration_range_scale, float spatial_decay_weight,
    float noise_scale_floor_vx, float noise_scale_floor_vy, float noise_scale_floor_w,
    float pure_rotation_ratio, int pure_rotation_steps, float pure_rotation_w_boost,
    float lateral_guidance_scale, float path_turn_angle,
    float turn_lateral_boost, float turn_lateral_max_boost,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float footprint_sample_spacing, float rear_obstacle_cost,
    const float* path_x, const float* path_y, int num_path_pts,
    float goal_yaw,
    float final_goal_x, float final_goal_y,
    int global_horizon, int num_global_trajs,
    int num_samples, int horizon,
    float* d_noise_vx, float* d_noise_vy, float* d_noise_w,
    float* d_base_vx, float* d_base_vy, float* d_base_w,
    unsigned char* d_costmap, float* d_path_x, float* d_path_y,
    float* d_costs, float* d_sampled_vx, float* d_sampled_vy, float* d_sampled_w,
    float* d_traj_x, float* d_traj_y,
    cudaStream_t stream)
{
  size_t nbytes = num_samples * horizon * sizeof(float);
  cudaError_t e;
  e = cudaMemcpyAsync(d_noise_vx, noise_vx, nbytes, cudaMemcpyHostToDevice, stream); if (e) return 1;
  e = cudaMemcpyAsync(d_noise_vy, noise_vy, nbytes, cudaMemcpyHostToDevice, stream); if (e) return 2;
  e = cudaMemcpyAsync(d_noise_w,  noise_w,  nbytes, cudaMemcpyHostToDevice, stream); if (e) return 3;
  e = cudaMemcpyAsync(d_base_vx, base_vx, horizon * sizeof(float), cudaMemcpyHostToDevice, stream); if (e) return 4;
  e = cudaMemcpyAsync(d_base_vy, base_vy, horizon * sizeof(float), cudaMemcpyHostToDevice, stream); if (e) return 5;
  e = cudaMemcpyAsync(d_base_w,  base_w,  horizon * sizeof(float), cudaMemcpyHostToDevice, stream); if (e) return 6;
  e = cudaMemcpyAsync(d_costmap, costmap, costmap_w * costmap_h, cudaMemcpyHostToDevice, stream); if (e) return 7;
  if (num_path_pts > 0 && path_x) {
    size_t pb = num_path_pts * sizeof(float);
    e = cudaMemcpyAsync(d_path_x, path_x, pb, cudaMemcpyHostToDevice, stream); if (e) return 9;
    e = cudaMemcpyAsync(d_path_y, path_y, pb, cudaMemcpyHostToDevice, stream); if (e) return 10;
  }

  int blocks = (num_samples + 255) / 256;
  mppi_sample_kernel<<<blocks, 256, 0, stream>>>(
      d_noise_vx, d_noise_vy, d_noise_w, d_base_vx, d_base_vy, d_base_w,
      current_x, current_y, current_theta, target_x, target_y,
      d_costmap, costmap_w, costmap_h, costmap_res, costmap_origin_x, costmap_origin_y,
      dt, min_v, max_v, max_vy, max_w,
      cost_scale, obstacle_ratio, tracking_ratio, speed_ratio,
      path_deviation_weight, path_corridor,
      path_dir_x, path_dir_y,
      noise_decay_rate,
      exploration_range_scale, spatial_decay_weight,
      noise_scale_floor_vx, noise_scale_floor_vy, noise_scale_floor_w,
      pure_rotation_ratio, pure_rotation_steps, pure_rotation_w_boost,
      lateral_guidance_scale, path_turn_angle, turn_lateral_boost, turn_lateral_max_boost,
      fp_front, fp_back, fp_left, fp_right,
      footprint_sample_spacing, rear_obstacle_cost,
      d_path_x, d_path_y, num_path_pts, goal_yaw,
      final_goal_x, final_goal_y, global_horizon, num_global_trajs,
      num_samples, horizon,
      d_costs, d_sampled_vx, d_sampled_vy, d_sampled_w, d_traj_x, d_traj_y);
  e = cudaGetLastError();
  return e != cudaSuccess ? 8 : 0;
}

int mppi_gpu_weighted_sum(
    const float* d_costs,
    const float* d_sampled_vx, const float* d_sampled_vy, const float* d_sampled_w,
    float* d_result_seq, float min_cost, float lambda,
    int num_samples, int horizon, cudaStream_t stream)
{
  cudaMemsetAsync(d_result_seq, 0, horizon * 4 * sizeof(float), stream);
  int bpt = (num_samples + 255) / 256;
  mppi_weighted_sum_kernel<<<dim3(bpt, horizon), 256, 0, stream>>>(
      d_costs, d_sampled_vx, d_sampled_vy, d_sampled_w,
      d_result_seq, min_cost, lambda, num_samples, horizon);
  return cudaGetLastError() != cudaSuccess ? 1 : 0;
}

}  // extern "C"
