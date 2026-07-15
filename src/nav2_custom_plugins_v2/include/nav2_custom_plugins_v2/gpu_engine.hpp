#ifndef NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_

#include <vector>
#include <random>
#include <cuda_runtime.h>
#include "nav2_custom_plugins_v2/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// GPU 引擎: 噪声生成 + 缓冲区管理 + 内核调用 + 结果下载
/// 封装所有 CUDA 交互, 对外暴露干净的 C++ 接口
class GPUEngine {
public:
  GPUEngine(int N, int H);
  ~GPUEngine();

  // 禁止拷贝 (管理 GPU 资源)
  GPUEngine(const GPUEngine&) = delete;
  GPUEngine& operator=(const GPUEngine&) = delete;

  // ═════════════════════════════════════════════════════════════════════
  // 噪声生成 (CPU 端, NLN 混合采样)
  // ═════════════════════════════════════════════════════════════════════

  struct NoiseBuffers {
    std::vector<float> vx;
    std::vector<float> vy;
    std::vector<float> delta;
  };

  /// 生成 N×H 噪声序列
  NoiseBuffers generateNoise(const MPPIParams& params);

  // ═════════════════════════════════════════════════════════════════════
  // 数据上传
  // ═════════════════════════════════════════════════════════════════════

  void uploadNoise(const NoiseBuffers& noise, cudaStream_t stream);
  void uploadBaseSequence(const ControlSequence& base, cudaStream_t stream);
  void uploadCostmap(const unsigned char* data, int w, int h, cudaStream_t stream);
  void uploadPath(const float* xs, const float* ys, int n, cudaStream_t stream);

  // ═════════════════════════════════════════════════════════════════════
  // 内核启动
  // ═════════════════════════════════════════════════════════════════════

  /// 启动采样+代价计算内核, 返回 0=成功
  int launchSampleAndCost(
      const RobotState& robot, const LookaheadPoint& lh,
      const MPPIParams& params,
      int N, int H, cudaStream_t stream);

  /// 启动指数加权求和内核, 返回 0=成功
  int launchWeightedSum(
      float min_cost, float lambda,
      int N, int H, cudaStream_t stream);

  // ═════════════════════════════════════════════════════════════════════
  // 结果下载
  // ═════════════════════════════════════════════════════════════════════

  /// 下载 N 条轨迹的代价
  std::vector<float> downloadCosts(int N, cudaStream_t stream);

  /// 下载 H 步最优控制序列
  ControlSequence downloadOptimalSequence(int H, cudaStream_t stream);

  /// 下载轨迹数据 (供可视化, 返回 N×H×2 的 [x,y] 交织数组)
  std::vector<float> downloadTrajectories(int N, int H, cudaStream_t stream);

  // ═════════════════════════════════════════════════════════════════════
  // 设备指针 (供内部优化器直接访问)
  // ═════════════════════════════════════════════════════════════════════

  float* deviceCosts()       { return d_costs_; }
  float* deviceSampledVx()   { return d_sampled_vx_; }
  float* deviceSampledVy()   { return d_sampled_vy_; }
  float* deviceSampledDelta(){ return d_sampled_delta_; }
  float* deviceResultSeq()   { return d_result_seq_; }

private:
  int N_, H_;

  // 噪声分布
  std::mt19937 gen_;
  std::normal_distribution<> dist_vx_, dist_vy_, dist_delta_;
  std::lognormal_distribution<> dist_ln_vx_, dist_ln_vy_, dist_ln_delta_;
  std::uniform_real_distribution<> dist_sign_{0.0, 1.0};
  std::uniform_real_distribution<> dist_mix_{0.0, 1.0};

  void initDistributions(const MPPIParams& params);

  // ── GPU 缓冲区 ──
  float *d_noise_vx_ = nullptr;
  float *d_noise_vy_ = nullptr;
  float *d_noise_delta_ = nullptr;
  float *d_base_vx_ = nullptr;
  float *d_base_vy_ = nullptr;
  float *d_base_delta_ = nullptr;
  float *d_sampled_vx_ = nullptr;
  float *d_sampled_vy_ = nullptr;
  float *d_sampled_delta_ = nullptr;
  float *d_costs_ = nullptr;
  float *d_result_seq_ = nullptr;
  float *d_traj_x_ = nullptr;
  float *d_traj_y_ = nullptr;
  float *d_path_x_ = nullptr;
  float *d_path_y_ = nullptr;
  unsigned char *d_costmap_ = nullptr;
  int costmap_w_ = 0;
  int costmap_h_ = 0;

  void allocate();
  void free();
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_
