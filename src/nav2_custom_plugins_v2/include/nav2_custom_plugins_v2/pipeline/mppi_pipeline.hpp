#ifndef NAV2_CUSTOM_PLUGINS_V2__MPPI_PIPELINE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MPPI_PIPELINE_HPP_

/**
 * @file mppi_pipeline.hpp
 * @brief MPPI 管线: CPU rollout 结果 → GPU 上传 → 计算 → 下载 → CPU 后处理
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 本类在整条 MPPI 数据流中的位置
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   CPU:
 *     NoiseGenerator::generate(N, H, lookahead_yaw)  ← 偏置噪声
 *     batch_rollout(start, base_seq, noise, params)   ← 采样 + 运动学积分
 *              │
 *              ▼  BatchTrajectories {x, y, vx, vy, delta}  ← N×H 展平数组
 *   Pipeline:                                                      ← 本类
 *     uploadRollout(traj, N, H, stream)            ← 轨迹坐标 + 控制量 → GPU
 *     uploadBase(base_seq, H, stream)               ← warm-start 基序列 → GPU
 *              │
 *              ▼  GPU 显存: traj_x, traj_y, sampled_vx/vy/delta, base_vx/vy/delta
 *   GPU:
 *     launchCostKernel(...)                          ← 代价评估 kernel (后续)
 *              │
 *              ▼  d_costs_[N], d_result_seq_[H×4]
 *   CPU:
 *     downloadCosts(...)  → 找 min_cost
 *     launchWeightedSum(...) → downloadOptimalSequence(...)
 *     velocity_postprocessor.process(...)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 不拥有 GPUEngine / GPUUploader — 调用者在外部管理其生命周期
 * ═══════════════════════════════════════════════════════════════════════════════
 */

#include <cuda_runtime.h>
#include <vector>

// 全局命名空间 (完整定义在 core/mppi_core.hpp)
struct CostmapInfo;
struct Footprint;
struct PathInfo;
struct GoalInfo;
struct CriticParams;

namespace nav2_custom_plugins_v2
{

// 前向声明
struct MPPIParams;
struct ControlSequence;
struct BatchTrajectories;
class GPUEngine;
class GPUUploader;

class MPPIPipeline {
public:
  /// @param engine   GPUEngine 实例 (需已构造)
  /// @param uploader GPUUploader 实例 (需已调用 registerAll)
  /// @param params   MPPI 参数 (dt, max_v, max_w, ...)
  MPPIPipeline(GPUEngine &engine, GPUUploader &uploader,
               const MPPIParams &params);

  /// 上传批量 rollout 结果 → GPU
  ///
  /// 上传 5 个展平数组到 GPU 对应 buffer:
  ///   traj.x/y       → buf::traj_x / buf::traj_y       (轨迹坐标, 代价计算用)
  ///   traj.vx/vy/delta → buf::sampled_vx/vy/delta      (控制量, 速度代价用)
  ///
  /// 前置条件:
  ///   - batch_rollout() 已完成
  ///   - GPUUploader::registerAll() 已调用 (buffer 已分配)
  ///
  /// @param traj   批量 rollout 结果 (N×H 展平数组)
  /// @param N      轨迹数
  /// @param H      每轨迹步数
  /// @param stream CUDA 流 (异步上传, 同流内保序)
  void uploadRollout(const BatchTrajectories &traj, int N, int H,
                     cudaStream_t stream);

  /// 上传 warm-start 基序列 → GPU
  ///
  /// 上传 base_vx/vy/delta 到 GPU buffer, 作为本帧采样的基线。
  /// 基序列来源: 上一帧最优控制量左移一位, 尾部衰减。
  ///
  /// @param base   H 步基控制序列
  /// @param H      horizon 步数
  /// @param stream CUDA 流
  void uploadBase(const ControlSequence &base, int H, cudaStream_t stream);

  /// 启动代价评估 kernel + 下载结果
  /// @param critic_params 大类权重等代价参数 (从 MPPIParams 映射)
  /// @return N 条轨迹的总代价
  std::vector<float> launchCost(const struct CostmapInfo &cmap,
                                const struct Footprint &fp,
                                const struct PathInfo &path,
                                const struct GoalInfo &goal,
                                const struct CriticParams &critic_params,
                                int N, int H, cudaStream_t stream);

  /// 加权求和 + 下载最优控制序列
  /// @return [H×4] 加权平均控制量 (vx_sum, vy_sum, omega_sum, weight_sum per step)
  std::vector<float> launchWeightedSum(float min_cost, float lambda,
                                       int N, int H, cudaStream_t stream);

private:
  GPUEngine      &engine_;     ///< 持有引用, 不拥有 — GPUEngine::upload(name, data, stream)
  GPUUploader    &uploader_;   ///< 持有引用, 不拥有 — GPUUploader (后续 download 等)
  const MPPIParams &params_;   ///< MPPI 参数 (dt, max_v, max_w, ...)

  /// double→float 转换缓冲区 (base.vx/vy/omega 是 std::vector<double>)
  std::vector<float> float_fbuf_;
  std::vector<float> float_fbuf2_;
  std::vector<float> float_fbuf3_;
};

}  // namespace nav2_custom_plugins_v2

#endif
