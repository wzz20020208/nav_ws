/**
 * @file mppi_cost_gpu.cu
 * @brief GPU 版 MPPI 代价计算 — Host wrapper + 显存管理 + 分段计时
 */

#include "mppi_benchmark/mppi_cost_gpu.cuh"

extern "C" {

/// 完整 GPU benchmark: 上传 → kernel → 下载
/// @param out_upload_ms   [out] 上传耗时 (ms)
/// @param out_kernel_ms   [out] kernel 耗时 (ms)
/// @param out_download_ms [out] 下载耗时 (ms)
/// @return 总耗时 (ms) = upload + kernel + download
float run_gpu_benchmark(
    const float* h_vx, const float* h_vy, const float* h_w,
    int num_samples, int horizon,
    float start_x, float start_y, float start_theta,
    float target_x, float target_y,
    const unsigned char* h_costmap, int cm_w, int cm_h,
    float cm_res, float cm_ox, float cm_oy,
    float dt, float cost_scale, float obs_r, float track_r, float spd_r,
    float path_vx_r, float path_vy_r, float path_tangent, float goal_yaw,
    float fp_f, float fp_b, float fp_l, float fp_r,
    float fp_sp, float rear_cost,
    const float* h_path_x, const float* h_path_y, int num_path,
    float fgx, float fgy,
    float* h_costs_out,
    float* out_upload_ms, float* out_kernel_ms, float* out_download_ms)
{
  int N = num_samples, H = horizon;

  // 分配 GPU 内存
  float *d_vx=nullptr, *d_vy=nullptr, *d_w=nullptr;
  float *d_path_x=nullptr, *d_path_y=nullptr, *d_costs=nullptr;
  unsigned char* d_cm=nullptr;

  cudaMalloc(&d_vx, N * H * sizeof(float));
  cudaMalloc(&d_vy, N * H * sizeof(float));
  cudaMalloc(&d_w,  N * H * sizeof(float));
  cudaMalloc(&d_path_x, num_path * sizeof(float));
  cudaMalloc(&d_path_y, num_path * sizeof(float));
  cudaMalloc(&d_costs, N * sizeof(float));
  cudaMalloc(&d_cm, cm_w * cm_h * sizeof(unsigned char));

  cudaStream_t stream;
  cudaStreamCreate(&stream);

  // ── 分段计时 ──
  cudaEvent_t ev[4];
  for (int i = 0; i < 4; ++i) cudaEventCreate(&ev[i]);

  cudaEventRecord(ev[0], stream);  // 开始: 上传

  cudaMemcpyAsync(d_vx, h_vx, N * H * sizeof(float), cudaMemcpyHostToDevice, stream);
  cudaMemcpyAsync(d_vy, h_vy, N * H * sizeof(float), cudaMemcpyHostToDevice, stream);
  cudaMemcpyAsync(d_w,  h_w,  N * H * sizeof(float), cudaMemcpyHostToDevice, stream);
  cudaMemcpyAsync(d_cm, h_costmap, cm_w * cm_h, cudaMemcpyHostToDevice, stream);
  cudaMemcpyAsync(d_path_x, h_path_x, num_path * sizeof(float), cudaMemcpyHostToDevice, stream);
  cudaMemcpyAsync(d_path_y, h_path_y, num_path * sizeof(float), cudaMemcpyHostToDevice, stream);

  cudaEventRecord(ev[1], stream);  // 上传完成 → kernel 开始

  int block = 256;
  int grid = (N + block - 1) / block;
  mppi_cost_benchmark_kernel<<<grid, block, 0, stream>>>(
      d_vx, d_vy, d_w,
      start_x, start_y, start_theta,
      target_x, target_y,
      d_cm, cm_w, cm_h, cm_res, cm_ox, cm_oy,
      dt, H,
      cost_scale, obs_r, track_r, spd_r,
      path_vx_r, path_vy_r, path_tangent, goal_yaw,
      fp_f, fp_b, fp_l, fp_r, fp_sp, rear_cost,
      d_path_x, d_path_y, num_path,
      fgx, fgy,
      N, d_costs);

  cudaEventRecord(ev[2], stream);  // kernel 完成 → 下载开始

  cudaMemcpyAsync(h_costs_out, d_costs, N * sizeof(float), cudaMemcpyDeviceToHost, stream);

  cudaEventRecord(ev[3], stream);  // 下载完成
  cudaStreamSynchronize(stream);

  float upload_ms=0, kernel_ms=0, download_ms=0, total_ms=0;
  cudaEventElapsedTime(&total_ms,    ev[0], ev[3]);
  cudaEventElapsedTime(&upload_ms,   ev[0], ev[1]);
  cudaEventElapsedTime(&kernel_ms,   ev[1], ev[2]);
  cudaEventElapsedTime(&download_ms, ev[2], ev[3]);

  if (out_upload_ms)   *out_upload_ms   = upload_ms;
  if (out_kernel_ms)   *out_kernel_ms   = kernel_ms;
  if (out_download_ms) *out_download_ms = download_ms;

  // 清理
  for (int i = 0; i < 4; ++i) cudaEventDestroy(ev[i]);
  cudaStreamDestroy(stream);
  cudaFree(d_vx); cudaFree(d_vy); cudaFree(d_w);
  cudaFree(d_path_x); cudaFree(d_path_y); cudaFree(d_costs); cudaFree(d_cm);

  return total_ms;
}

}  // extern "C"
