/**
 * @file mppi_pipeline.cpp
 * @brief MPPI 管线实现: 桥接 CPU 产出 (batch_rollout) → GPU 上传 (GPUEngine)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 每帧调用顺序
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ① pipeline.uploadBase(base_seq, H, stream)     ← warm-start 基线
 *   ② pipeline.uploadRollout(batch, N, H, stream)   ← N 条轨迹
 *   ③ engine.launchSampleAndCost(...)                ← GPU 代价评估 (后续)
 *   ④ engine.download("costs", ...)                  ← 下载代价
 *   ⑤ cudaStreamSynchronize(stream)                  ← 等 GPU 完成
 *   ⑥ CPU 扫 costs 找 min_cost
 *   ⑦ engine.launchWeightedSum(...)                  ← GPU 加权求和 (后续)
 *   ⑧ engine.download("result_seq", ...)             ← 下载最优序列
 *   ⑨ cudaStreamSynchronize(stream)
 *
 *   所有 upload 均为异步 (cudaMemcpyAsync), 同 stream 内保证:
 *   upload① → upload② → launch③ → download④ (顺序执行, 不交叉)
 */

#include "nav2_custom_plugins_v2/pipeline/mppi_pipeline.hpp"
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"           // BatchTrajectories, ControlSequence
#include "nav2_custom_plugins_v2/gpu/gpu_engine.hpp"      // GPUEngine::upload
#include "nav2_custom_plugins_v2/gpu/gpu_uploader.hpp"    // buf:: 常量

namespace nav2_custom_plugins_v2
{

MPPIPipeline::MPPIPipeline(GPUEngine &engine, GPUUploader &uploader,
                           const MPPIParams &params)
  : engine_(engine)
  , uploader_(uploader)
  , params_(params)
{}

// ═══════════════════════════════════════════════════════════════════════════════
// uploadRollout — 将 N 条轨迹的位姿 + 控制量批量上传到 GPU
// ═══════════════════════════════════════════════════════════════════════════════
//
// 上传 BatchTrajectories 的 5 个展平数组 → GPU 对应 buffer:
//   x     → buf::traj_x        轨迹点全局 x 坐标 [N×H], 碰撞检测 + 路径偏离代价
//   y     → buf::traj_y        轨迹点全局 y 坐标 [N×H], 同上
//   vx    → buf::sampled_vx    实际控制量 vx [N×H], 速度代价项
//   vy    → buf::sampled_vy    实际控制量 vy [N×H], 同上
//   delta → buf::sampled_delta 实际控制量 delta [N×H], 朝向代价项
//
// GPU 代价 kernel 拿 (x,y) 去 costmap 双线性插值 → 障碍物代价,
// 拿 (vx,vy,delta) 对比期望速度和朝向 → 速度/朝向代价。
//
// 上传操作是 cudaMemcpyAsync 异步的, 同 stream 内后续 kernel 启动会排队等它完成。
//
// @param traj   batch_rollout() 产出, 全部数组已在 CPU 端分配并填充
// @param N      轨迹数 (对应 num_samples)
// @param H      每轨迹步数 (对应 prediction_horizon)
// @param stream CUDA 流, 保证 upload → kernel → download 顺序

void MPPIPipeline::uploadRollout(const BatchTrajectories &traj,
                                  int N, int H, cudaStream_t stream)
{
  // 表驱动: name → data 映射, 循环上传, 新增 buffer 只需加一行
  const struct { const char *name; const float *data; } map[] = {
    {buf::traj_x,       traj.x.data()},
    {buf::traj_y,       traj.y.data()},
    {buf::traj_theta,   traj.theta.data()},
    {buf::sampled_vx,    traj.vx.data()},
    {buf::sampled_vy,    traj.vy.data()},
    {buf::sampled_delta, traj.delta.data()},
  };
  for (auto &e : map) engine_.upload(e.name, e.data, stream);

  (void)N;
  (void)H;
}

// ═══════════════════════════════════════════════════════════════════════════════
// uploadBase — 上传 warm-start 基控制序列到 GPU
// ═══════════════════════════════════════════════════════════════════════════════
//
// 基序列是 MPPI 采样的搜索中心, 本帧采样围绕它展开:
//   采样控制量 = clamp(base[t] + noise[s,t] × decay_scale, 限幅)
//
// 基序列来源: 上一帧加权平均最优控制序列 → shiftAndDecay 左移 + 尾部衰减。
// 首帧全零, 采样完全依赖噪声探索; 之后每帧更新, 提供时间连续性避免帧间跳变。
//
// 上传 3 个数组 → GPU:
//   base.vx    → buf::base_vx     [H], 采样时 buf.base_vx[t] 读取
//   base.vy    → buf::base_vy     [H]
//   base.delta → buf::base_delta  [H]
//
// @param base   H 步基控制序列 (内部为 std::vector<double>)
// @param H      horizon 步数
// @param stream CUDA 流

void MPPIPipeline::uploadBase(const ControlSequence &base, int H,
                               cudaStream_t stream)
{
  const struct { const char *name; const double *data; } map[] = {
    {buf::base_vx,    base.vx.data()},
    {buf::base_vy,    base.vy.data()},
    {buf::base_delta, base.delta.data()},
  };
  for (auto &e : map) engine_.upload(e.name, e.data, stream);
  (void)H;
}

// ═══════════════════════════════════════════════════════════════════════════════
// launchCost — 启动代价 kernel → 同步 → 下载 → 返回 N 个代价
// ═══════════════════════════════════════════════════════════════════════════════
//
// ① engine_.launchCostKernel() 异步启动
// ② engine_.download("costs")  异步下载
// ③ cudaStreamSynchronize()    阻塞等 GPU 完成
// ④ 返回 host_costs[N], CPU 端扫 min_cost → 喂给 weighted sum

std::vector<float> MPPIPipeline::launchCost(
    const CostmapInfo &cmap, const Footprint &fp,
    const PathInfo &path, const GoalInfo &goal,
    int N, int H, cudaStream_t stream)
{
  engine_.launchCostKernel(cmap, fp, path, goal,
      static_cast<float>(params_.cost_scale), N, H, stream);

  std::vector<float> costs(N);
  engine_.download(buf::costs, costs.data(), stream);
  cudaStreamSynchronize(stream);

  return costs;
}

// ═══════════════════════════════════════════════════════════════════════════════
// launchWeightedSum — 加权求和 kernel → 同步 → 下载 → 返回最优序列
// ═══════════════════════════════════════════════════════════════════════════════
//
// ① cudaMemsetAsync 清零 result_seq (在 kernel 内完成)
// ② engine_.launchWeightedSumKernel() 异步启动
// ③ engine_.download("result_seq")  异步下载
// ④ cudaStreamSynchronize()         阻塞等 GPU 完成
// ⑤ 返回 result[H×4], CPU 端: best[t] = result[t*4+0..2] / result[t*4+3]

std::vector<float> MPPIPipeline::launchWeightedSum(
    float min_cost, float lambda,
    int N, int H, cudaStream_t stream)
{
  engine_.launchWeightedSumKernel(min_cost, lambda, N, H, stream);

  const int result_size = H * 4;
  std::vector<float> result(result_size);
  engine_.download(buf::result_seq, result.data(), stream);
  cudaStreamSynchronize(stream);

  return result;
}

}  // namespace nav2_custom_plugins_v2
