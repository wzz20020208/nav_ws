#ifndef NAV2_CUSTOM_PLUGINS_V2__GPU_UPLOADER_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__GPU_UPLOADER_HPP_

/**
 * @file gpu_uploader.hpp
 * @brief GPUUploader — 统一管理 MPPI 所需数据的 GPU 上传
 *
 * 不拥有 GPUEngine, 只持有引用。调用者在外部管理 GPUEngine 生命周期。
 *
 * 使用方式:
 *   GPUEngine engine;
 *   GPUUploader uploader(engine);
 *   uploader.registerAll(N, H);          // 注册全部 buffer
 *   uploader.uploadNoise(vx,vy,delta, N,H, stream);
 *   uploader.uploadCostmap(data, w, h, stream);
 */

#include <cuda_runtime.h>

namespace nav2_custom_plugins_v2
{

class GPUEngine;

class GPUUploader
{
public:
  explicit GPUUploader(GPUEngine &engine);

  // ══════════════════════════════════════════════════════════
  // Buffer 注册
  // ══════════════════════════════════════════════════════════

  /// 注册采样相关 buffer (噪声+base+kernel 输出, 不含路径和 costmap)
  /// 已注册的同名 buffer 不会重复分配
  void registerAll(int N, int H);

  /// 注册 costmap buffer (尺寸变化时需重新注册)
  void registerCostmap(int w, int h);

  /// 注册路径 buffer (路径点数量变化时需重新注册)
  void registerPath(int n);

  // ══════════════════════════════════════════════════════════
  // 上传
  // ══════════════════════════════════════════════════════════

  /// 上传噪声序列 (NLN 混合采样结果, 每条轨迹每步一组噪声)
  void uploadNoise(const float *vx, const float *vy, const float *omega,
                   int N, int H, cudaStream_t stream);

  /// 上传 warm-start 基控制序列
  void uploadBase(const float *vx, const float *vy, const float *omega,
                  int H, cudaStream_t stream);

  /// 上传代价地图 (需先 registerCostmap, 或直接传尺寸触发自动注册)
  void uploadCostmap(const unsigned char *data, int w, int h,
                     cudaStream_t stream);

  /// 上传全局路径点
  void uploadPath(const float *x, const float *y, int n,
                  cudaStream_t stream);

private:
  GPUEngine &engine_;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__GPU_UPLOADER_HPP_
