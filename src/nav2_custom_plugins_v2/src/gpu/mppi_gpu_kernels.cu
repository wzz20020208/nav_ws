/**
 * @file mppi_gpu_kernels.cu
 * @brief MPPI GPU 代价计算 kernel — N 条轨迹并行, CriticManager 评估
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 并行: N 线程, grid = ceil(N/256), block = 256
 * 每线程 H 步串行: 读 (x,y,θ,vx,vy) → CriticManager.evaluate → 累加
 * 输出: d_costs[N]
 *
 * 数据来源 (CPU 已上传):
 *   traj_x/y/theta [N×H]  ← batch_rollout 轨迹位姿
 *   sampled_vx/vy/delta  ← batch_rollout 采样控制量
 *   costmap               ← GPUUploader::uploadCostmap
 *   path_x/y              ← GPUUploader::uploadPath
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#include <cuda_runtime.h>
#include <cfloat>
#include <stdexcept>
#include <string>

#include "nav2_custom_plugins_v2/gpu/gpu_engine.hpp"   // buf::, GPUEngine
#include "critic_manager.cuh"                          // CriticManager (cost/)
#include "mppi_gpu_common.cuh"                         // costmap_bilinear, normalize_angle (gpu/)

// ═══════════════════════════════════════════════════════════════════════════
// 代价评估 kernel
// ═══════════════════════════════════════════════════════════════════════════
//
// 每线程一条轨迹, 串行 H 步调用 CriticManager::evaluate().
// 循环后加终点距离代价 (TerminalDistCritic, per-trajectory 而非 per-step).

__global__ void cost_eval_kernel(
    const float *__restrict__ traj_x,        // [N×H]
    const float *__restrict__ traj_y,        // [N×H]
    const float *__restrict__ traj_theta,    // [N×H]
    const float *__restrict__ sampled_vx,    // [N×H]
    const float *__restrict__ sampled_vy,    // [N×H]
    const float *__restrict__ sampled_omega, // [N×H]
    const float *__restrict__ base_vx,       // [H] warm-start base
    const float *__restrict__ base_vy,       // [H]
    const float *__restrict__ base_omega,    // [H]
    const CostmapInfo cmap,
    const Footprint   fp,
    const PathInfo    path,
    const GoalInfo    goal,
    const CriticParams critic_params,
    float cost_scale,
    int N, int H,
    float *__restrict__ d_costs)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  if (s >= N) return;

  CriticManager mgr;
  mgr.init(critic_params);

  float total = 0.0f;
  float x = 0.0f, y = 0.0f;

  // 预计算路径切线方向 (前瞻点处), 供越界检测
  float cos_tan = cosf(path.path_tangent);
  float sin_tan = sinf(path.path_tangent);

  for (int t = 0; t < H; ++t) {
    int idx = s * H + t;

    x            = traj_x[idx];
    y            = traj_y[idx];
    float theta  = traj_theta[idx];
    float vx     = sampled_vx[idx];
    float vy     = sampled_vy[idx];
    float omega  = sampled_omega[idx];

    float cos_t = cosf(theta);
    float sin_t = sinf(theta);

    total += mgr.evaluate(x, y, cos_t, sin_t, theta, vx, vy, omega,
                          cmap, fp, path, goal,
                          t, base_vx, base_vy, base_omega);

    // ── 前瞻点越界惩罚 (安全网: rollout 层已将到达后的 vx/vy 置零) ──
    float dx_lh = x - goal.lookahead_x;
    float dy_lh = y - goal.lookahead_y;
    float along = dx_lh * cos_tan + dy_lh * sin_tan;  // 沿路径方向的越界距离
    if (along > 0.0f) {
      total += goal.lookahead_overshoot_weight * along * along;
    }
  }

  // ── 终点距离代价 (1:1 THEMIS: 加到 prog_acc, 权重由 speed_ratio 统一处理) ──
  float term_dx = goal.goal_x - x;
  float term_dy = goal.goal_y - y;
  total += sqrtf(term_dx * term_dx + term_dy * term_dy);  // 不加权重, 归类到总代价

  // ── 1:1 THEMIS: cost_scale × total / H ──
  float inv_h = 1.0f / static_cast<float>(H);
  d_costs[s] = cost_scale * total * inv_h;
}

// ═══════════════════════════════════════════════════════════════════════════
// GPUEngine::launchCostKernel — host 端启动 + 解析 buffer 指针
// ═══════════════════════════════════════════════════════════════════════════

namespace nav2_custom_plugins_v2
{

void GPUEngine::launchCostKernel(
    const CostmapInfo &cmap, const Footprint &fp,
    const PathInfo &path, const GoalInfo &goal,
    const CriticParams &critic_params,
    float cost_scale, int N, int H, cudaStream_t stream)
{
  auto *d_traj_x     = static_cast<const float *>(getDevicePtr(buf::traj_x));
  auto *d_traj_y     = static_cast<const float *>(getDevicePtr(buf::traj_y));
  auto *d_traj_theta = static_cast<const float *>(getDevicePtr(buf::traj_theta));
  auto *d_sampled_vx    = static_cast<const float *>(getDevicePtr(buf::sampled_vx));
  auto *d_sampled_vy    = static_cast<const float *>(getDevicePtr(buf::sampled_vy));
  auto *d_sampled_omega = static_cast<const float *>(getDevicePtr(buf::sampled_omega));
  auto *d_base_vx    = static_cast<const float *>(getDevicePtr(buf::base_vx));
  auto *d_base_vy    = static_cast<const float *>(getDevicePtr(buf::base_vy));
  auto *d_base_omega = static_cast<const float *>(getDevicePtr(buf::base_w));
  auto *d_costs      = static_cast<float *>(getDevicePtr(buf::costs));

  int blocks = (N + 255) / 256;
  cost_eval_kernel<<<blocks, 256, 0, stream>>>(
      d_traj_x, d_traj_y, d_traj_theta,
      d_sampled_vx, d_sampled_vy, d_sampled_omega,
      d_base_vx, d_base_vy, d_base_omega,
      cmap, fp, path, goal, critic_params, cost_scale, N, H, d_costs);

  cudaError_t e = cudaGetLastError();
  if (e != cudaSuccess) {
    throw std::runtime_error(
        std::string("launchCostKernel: ") + cudaGetErrorString(e));
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// 加权求和 kernel — Softmax 加权 N 条轨迹 → 最优控制序列
// ═══════════════════════════════════════════════════════════════════════════
//
// 并行: 2D grid dim3(bpt, H), x=轨迹分块, y=时间步
//   w_s = exp(-(cost[s] - min_cost) / lambda)
//   out[t,c] = Σ_s w_s * sampled[s,t,c]  (c∈{vx,vy,delta})
//   out[t,3] = Σ_s w_s (归一化分母)
// atomicAdd: 多线程竞争同一时间步的 result[t,*]

__global__ void weighted_sum_kernel(
    const float *__restrict__ d_costs,         // [N]
    const float *__restrict__ d_sampled_vx,    // [N×H]
    const float *__restrict__ d_sampled_vy,    // [N×H]
    const float *__restrict__ d_sampled_omega, // [N×H]
    float *__restrict__ d_result_seq,          // [H×4] 输出
    float min_cost, float lambda, int N, int H)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  int t = blockIdx.y;
  if (s >= N || t >= H) return;

  float w = expf(-(d_costs[s] - min_cost) / lambda);
  int base = t * 4;
  atomicAdd(&d_result_seq[base + 0], w * d_sampled_vx[s * H + t]);
  atomicAdd(&d_result_seq[base + 1], w * d_sampled_vy[s * H + t]);
  atomicAdd(&d_result_seq[base + 2], w * d_sampled_omega[s * H + t]);
  atomicAdd(&d_result_seq[base + 3], w);
}

void GPUEngine::launchWeightedSumKernel(float min_cost, float lambda,
                                         int N, int H, cudaStream_t stream)
{
  auto *d_costs        = static_cast<const float *>(getDevicePtr(buf::costs));
  auto *d_sampled_vx   = static_cast<const float *>(getDevicePtr(buf::sampled_vx));
  auto *d_sampled_vy   = static_cast<const float *>(getDevicePtr(buf::sampled_vy));
  auto *d_sampled_omega= static_cast<const float *>(getDevicePtr(buf::sampled_omega));
  auto *d_result_seq   = static_cast<float *>(getDevicePtr(buf::result_seq));

  cudaMemsetAsync(d_result_seq, 0, H * 4 * sizeof(float), stream);

  int bpt = (N + 255) / 256;
  weighted_sum_kernel<<<dim3(bpt, H), 256, 0, stream>>>(
      d_costs, d_sampled_vx, d_sampled_vy, d_sampled_omega,
      d_result_seq, min_cost, lambda, N, H);

  cudaError_t e = cudaGetLastError();
  if (e != cudaSuccess) {
    throw std::runtime_error(
        std::string("launchWeightedSumKernel: ") + cudaGetErrorString(e));
  }
}

}  // namespace nav2_custom_plugins_v2
