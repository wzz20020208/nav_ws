/**
 * @file gpu_uploader.cpp
 * @brief GPUUploader 实现 — 注册 + 上传 MPPI 所需的全部 GPU 数据
 */

#include "nav2_custom_plugins_v2/gpu/gpu_uploader.hpp"
#include "nav2_custom_plugins_v2/gpu/gpu_engine.hpp"

namespace nav2_custom_plugins_v2
{

GPUUploader::GPUUploader(GPUEngine &engine)
  : engine_(engine)
{}

// ═══════════════════════════════════════════════════════════════════════════
// Buffer 注册配置表
// ═══════════════════════════════════════════════════════════════════════════

/// 尺寸模式: 各 buffer 的字节数仅取决于 N 和 H
enum class BufSize { NxH, H, N, Hx4 };

namespace {
  /// 新增 buffer 只需在此表加一行, 无需修改 registerAll 函数体
  constexpr struct {
    const char *name;
    BufSize     size;
  } kSamplingBuffers[] = {
    // ── 噪声 (CPU 预生成, 每帧上传) ──
    {buf::noise_vx,    BufSize::NxH},
    {buf::noise_vy,    BufSize::NxH},
    {buf::noise_w, BufSize::NxH},
    // ── 基序列 (warm-start, 每帧移位后上传) ──
    {buf::base_vx,     BufSize::H},
    {buf::base_vy,     BufSize::H},
    {buf::base_w,  BufSize::H},
    // ── 采样控制量 (kernel 输出) ──
    {buf::sampled_vx,    BufSize::NxH},
    {buf::sampled_vy,    BufSize::NxH},
    {buf::sampled_omega, BufSize::NxH},
    // ── 代价 (kernel 输出) ──
    {buf::costs,       BufSize::N},
    // ── 加权结果序列 (kernel 输出) ──
    {buf::result_seq,  BufSize::Hx4},
    // ── 轨迹世界坐标 (kernel 输出, 可视化) ──
    {buf::traj_x,      BufSize::NxH},
    {buf::traj_y,      BufSize::NxH},
    {buf::traj_theta,  BufSize::NxH},
  };

  constexpr size_t toBytes(BufSize s, int N, int H)
  {
    switch (s) {
      case BufSize::NxH: return static_cast<size_t>(N) * H * sizeof(float);
      case BufSize::H:   return static_cast<size_t>(H) * sizeof(float);
      case BufSize::N:   return static_cast<size_t>(N) * sizeof(float);
      case BufSize::Hx4: return static_cast<size_t>(H) * 4 * sizeof(float);
    }
    return 0;
  }
}  // namespace

// ═══════════════════════════════════════════════════════════════════════════
// Buffer 注册
// ═══════════════════════════════════════════════════════════════════════════

void GPUUploader::registerAll(int N, int H)
{
  for (auto &e : kSamplingBuffers) {
    engine_.registerBuffer(e.name, toBytes(e.size, N, H));
  }
}

void GPUUploader::registerCostmap(int w, int h)
{
  engine_.registerBuffer(buf::costmap, static_cast<size_t>(w) * h);
}

void GPUUploader::registerPath(int n)
{
  engine_.registerBuffer(buf::path_x, static_cast<size_t>(n) * sizeof(float));
  engine_.registerBuffer(buf::path_y, static_cast<size_t>(n) * sizeof(float));
}

// ═══════════════════════════════════════════════════════════════════════════
// 上传
// ═══════════════════════════════════════════════════════════════════════════

void GPUUploader::uploadNoise(const float *vx, const float *vy,
                              const float *omega,
                              int N, int H, cudaStream_t stream)
{
  engine_.upload(buf::noise_vx,   vx,    stream);
  engine_.upload(buf::noise_vy,   vy,    stream);
  engine_.upload(buf::noise_w, omega, stream);
  (void)N;
  (void)H;
}

void GPUUploader::uploadBase(const float *vx, const float *vy,
                             const float *omega,
                             int H, cudaStream_t stream)
{
  engine_.upload(buf::base_vx,    vx,    stream);
  engine_.upload(buf::base_vy,    vy,    stream);
  engine_.upload(buf::base_w, omega, stream);
  (void)H;
}

void GPUUploader::uploadCostmap(const unsigned char *data, int w, int h,
                                cudaStream_t stream)
{
  registerCostmap(w, h);
  engine_.upload(buf::costmap, data, stream);
}

void GPUUploader::uploadPath(const float *x, const float *y, int n,
                             cudaStream_t stream)
{
  if (n <= 0) return;
  registerPath(n);
  engine_.upload(buf::path_x, x, stream);
  engine_.upload(buf::path_y, y, stream);
}

}  // namespace nav2_custom_plugins_v2
