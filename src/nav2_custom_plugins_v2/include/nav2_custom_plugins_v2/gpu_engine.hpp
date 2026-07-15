#ifndef NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_

#include <vector>
#include <cuda_runtime.h>
#include "nav2_custom_plugins_v2/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// GPU 引擎: 封装所有 CUDA 交互
class GPUEngine {
public:
  GPUEngine(int N, int H);
  ~GPUEngine();

  GPUEngine(const GPUEngine&) = delete;
  GPUEngine& operator=(const GPUEngine&) = delete;

  // ── 数据上传 ──
  void uploadNoise(const std::vector<float>& noise_vx,
                   const std::vector<float>& noise_vy,
                   const std::vector<float>& noise_delta,
                   cudaStream_t stream);
  void uploadBaseSequence(const ControlSequence& base, cudaStream_t stream);
  void uploadCostmap(const unsigned char* data, int w, int h, cudaStream_t stream);
  void uploadPath(const float* xs, const float* ys, int n, cudaStream_t stream);

  // ── 内核启动 ──
  int launchSampleAndCost(
      const RobotState& robot, const LookaheadPoint& lh,
      const MPPIParams& params, int N, int H, cudaStream_t stream);

  int launchWeightedSum(
      float min_cost, float lambda, int N, int H, cudaStream_t stream);

  // ── 结果下载 ──
  std::vector<float> downloadCosts(int N, cudaStream_t stream);
  ControlSequence downloadOptimalSequence(int H, cudaStream_t stream);
  std::vector<float> downloadTrajectories(int N, int H, cudaStream_t stream);

private:
  int N_, H_;

  float *d_noise_vx_ = nullptr, *d_noise_vy_ = nullptr, *d_noise_delta_ = nullptr;
  float *d_base_vx_ = nullptr, *d_base_vy_ = nullptr, *d_base_delta_ = nullptr;
  float *d_sampled_vx_ = nullptr, *d_sampled_vy_ = nullptr, *d_sampled_delta_ = nullptr;
  float *d_costs_ = nullptr, *d_result_seq_ = nullptr;
  float *d_traj_x_ = nullptr, *d_traj_y_ = nullptr;
  float *d_path_x_ = nullptr, *d_path_y_ = nullptr;
  unsigned char *d_costmap_ = nullptr;

  void allocate();
  void free();
};

}  // namespace nav2_custom_plugins_v2

#endif
