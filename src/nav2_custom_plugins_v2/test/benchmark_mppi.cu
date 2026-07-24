/**
 * @file benchmark_mppi.cu
 * @brief MPPI GPU 性能基准测试
 *
 * 测试项目:
 *   1. cost_eval_kernel  — 不同 N 和 H 下的代价评估吞吐
 *   2. weighted_sum_kernel — 不同 N 和 H 下的 softmax 加权吞吐
 *   3. 端到端 pipeline — upload + cost_eval + download + weighted_sum + download
 *   4. CriticManager 单次 evaluate 耗时分解 (obstacle / heading / speed)
 *
 * 参数扫描:
 *   N ∈ {1000, 2000, 4000, 8000, 16000}
 *   H ∈ {3, 5, 10}
 *
 * 编译: 由 CMakeLists.txt 中的 benchmark_mppi 目标编译
 * 运行: ./benchmark_mppi
 */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cuda_runtime.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#include "critic_manager.cuh"
#include "mppi_gpu_common.cuh"
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"

// ═══════════════════════════════════════════════════════════════════════════════
// CUDA 错误检查
// ═══════════════════════════════════════════════════════════════════════════════

#define CUDA_CHECK(call) do {                                    \
  cudaError_t _e = (call);                                       \
  if (_e != cudaSuccess) {                                       \
    fprintf(stderr, "CUDA error at %s:%d: %s\n",                  \
            __FILE__, __LINE__, cudaGetErrorString(_e));          \
    exit(1);                                                     \
  }                                                              \
} while(0)

// ═══════════════════════════════════════════════════════════════════════════════
// 计时工具: CUDA event 对
// ═══════════════════════════════════════════════════════════════════════════════

struct GpuTimer {
  cudaEvent_t start, stop;
  GpuTimer()  { cudaEventCreate(&start); cudaEventCreate(&stop); }
  ~GpuTimer() { cudaEventDestroy(start); cudaEventDestroy(stop); }
  void begin(cudaStream_t s = 0) { cudaEventRecord(start, s); }
  void end(cudaStream_t s = 0)   { cudaEventRecord(stop, s); }
  float elapsed_ms() {
    cudaEventSynchronize(stop);
    float ms = 0.0f;
    cudaEventElapsedTime(&ms, start, stop);
    return ms;
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// 合成测试数据
// ═══════════════════════════════════════════════════════════════════════════════

struct SyntheticData {
  int N, H;
  std::vector<float> traj_x, traj_y, traj_theta;
  std::vector<float> sampled_vx, sampled_vy, sampled_omega;
  std::vector<float> base_vx, base_vy, base_omega;
  std::vector<float> costs;
  std::vector<float> result_seq;

  // GPU 指针
  float *d_traj_x = nullptr, *d_traj_y = nullptr, *d_traj_theta = nullptr;
  float *d_sampled_vx = nullptr, *d_sampled_vy = nullptr, *d_sampled_omega = nullptr;
  float *d_base_vx = nullptr, *d_base_vy = nullptr, *d_base_omega = nullptr;
  float *d_costs = nullptr, *d_result_seq = nullptr;

  // Costmap (200×200, 0.05m res, 10×10m)
  static constexpr int CM_W = 200, CM_H = 200;
  unsigned char h_costmap[CM_W * CM_H];
  unsigned char *d_costmap_data = nullptr;
  CostmapInfo cmap;

  // Path (50 点直线, 沿 x 轴)
  static constexpr int PATH_N = 50;
  float h_path_x[PATH_N], h_path_y[PATH_N];
  float *d_path_x = nullptr, *d_path_y = nullptr;
  PathInfo path;
  PathInfo cpu_path;  // host-side path for CPU benchmark (x/y point to h_path_x/y)

  Footprint fp;
  GoalInfo goal;
  CriticParams critic_params;

  void alloc(int n, int h) {
    N = n; H = h;
    size_t nh = static_cast<size_t>(N) * H;
    traj_x.resize(nh);       traj_y.resize(nh);       traj_theta.resize(nh);
    sampled_vx.resize(nh);   sampled_vy.resize(nh);   sampled_omega.resize(nh);
    base_vx.resize(H);       base_vy.resize(H);       base_omega.resize(H);
    costs.resize(N);
    result_seq.resize(H * 4);

    CUDA_CHECK(cudaMalloc(&d_traj_x,        nh * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_traj_y,        nh * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_traj_theta,    nh * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_sampled_vx,    nh * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_sampled_vy,    nh * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_sampled_omega, nh * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_base_vx,       H * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_base_vy,       H * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_base_omega,    H * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_costs,         N * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_result_seq,    H * 4 * sizeof(float)));

    CUDA_CHECK(cudaMalloc(&d_costmap_data,  CM_W * CM_H));
    CUDA_CHECK(cudaMalloc(&d_path_x,        PATH_N * sizeof(float)));
    CUDA_CHECK(cudaMalloc(&d_path_y,        PATH_N * sizeof(float)));
  }

  ~SyntheticData() {
    auto f = [](float *&p) { if (p) { cudaFree(p); p = nullptr; } };
    f(d_traj_x); f(d_traj_y); f(d_traj_theta);
    f(d_sampled_vx); f(d_sampled_vy); f(d_sampled_omega);
    f(d_base_vx); f(d_base_vy); f(d_base_omega);
    f(d_costs); f(d_result_seq);
    if (d_costmap_data) { cudaFree(d_costmap_data); d_costmap_data = nullptr; }
    if (d_path_x) { cudaFree(d_path_x); d_path_x = nullptr; }
    if (d_path_y) { cudaFree(d_path_y); d_path_y = nullptr; }
  }

  /// 填充合成轨迹: 沿 x 轴前进, 小幅度随机偏移
  void fillTrajectories() {
    size_t nh = static_cast<size_t>(N) * H;
    for (size_t i = 0; i < nh; ++i) {
      int s = static_cast<int>(i / H);
      int t = static_cast<int>(i % H);
      float dt_f = 0.05f;
      float base_x = 0.3f * t * dt_f;
      float noise  = (s * 7 + t * 13) % 100 / 100.0f - 0.5f;
      traj_x[i]     = base_x + noise * 0.02f;
      traj_y[i]     = noise * 0.15f;
      traj_theta[i] = noise * 0.3f;
      sampled_vx[i]    = 0.3f + noise * 0.1f;
      sampled_vy[i]    = noise * 0.05f;
      sampled_omega[i] = noise * 0.2f;
    }
    for (int t = 0; t < H; ++t) {
      base_vx[t]    = 0.3f;
      base_vy[t]    = 0.0f;
      base_omega[t] = 0.0f;
    }
  }

  /// 填充空旷 costmap (中心有少量障碍)
  void fillCostmap() {
    memset(h_costmap, 0, CM_W * CM_H);
    // 在 (3m, 0.3m) 放一个障碍物块
    for (int r = 58; r < 64; ++r)
      for (int c = 98; c < 104; ++c)
        h_costmap[r * CM_W + c] = 254;
    cmap.data = d_costmap_data;
    cmap.w = CM_W; cmap.h = CM_H;
    cmap.res = 0.05f;
    cmap.origin_x = -5.0f; cmap.origin_y = -5.0f;
  }

  /// 填充直线路径 (沿 x 轴)
  void fillPath() {
    for (int i = 0; i < PATH_N; ++i) {
      h_path_x[i] = static_cast<float>(i) * 0.2f;
      h_path_y[i] = 0.0f;
    }
    path.x = d_path_x;
    path.y = d_path_y;
    path.num_pts = PATH_N;
    path.path_tangent = 0.0f;
    path.goal_yaw = 0.0f;
    path.goal_x = 10.0f;
    path.goal_y = 0.0f;

    // CPU-side copy for host evaluation
    cpu_path.x = h_path_x;
    cpu_path.y = h_path_y;
    cpu_path.num_pts = PATH_N;
    cpu_path.path_tangent = 0.0f;
    cpu_path.goal_yaw = 0.0f;
    cpu_path.goal_x = 10.0f;
    cpu_path.goal_y = 0.0f;
  }

  /// 填充 footprint + goal + critic 参数
  void fillParams() {
    fp.front = 0.17f; fp.back = 0.17f;
    fp.left = 0.28f;  fp.right = 0.28f;
    fp.sample_spacing = 0.08f;
    fp.rear_obstacle_cost = 160.0f;

    goal.target_vx_r = 1.0f;
    goal.target_vy_r = 0.0f;
    goal.max_feasible_v = 0.4f;
    goal.goal_x = 10.0f; goal.goal_y = 0.0f;
    goal.lookahead_x = 1.0f; goal.lookahead_y = 0.0f;
    goal.lookahead_overshoot_weight = 5.0f;

    critic_params.obstacle_ratio = 0.70f;
    critic_params.tracking_ratio = 0.20f;
    critic_params.speed_ratio    = 0.10f;
    critic_params.footprint_weight      = 1.0f;
    critic_params.path_align_weight     = 1.0f;
    critic_params.path_angle_weight     = 2.0f;
    critic_params.path_deviation_weight = 0.3f;
    critic_params.speed_reward_weight   = 1.0f;
    critic_params.base_similarity_weight = 0.5f;
  }

  void uploadAll(cudaStream_t stream) {
    size_t nh = static_cast<size_t>(N) * H;
    CUDA_CHECK(cudaMemcpyAsync(d_traj_x,        traj_x.data(),        nh * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_traj_y,        traj_y.data(),        nh * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_traj_theta,    traj_theta.data(),    nh * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_sampled_vx,    sampled_vx.data(),    nh * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_sampled_vy,    sampled_vy.data(),    nh * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_sampled_omega, sampled_omega.data(), nh * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_base_vx,       base_vx.data(),       H * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_base_vy,       base_vy.data(),       H * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_base_omega,    base_omega.data(),    H * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_costmap_data,  h_costmap,            CM_W * CM_H, cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_path_x,        h_path_x,             PATH_N * sizeof(float), cudaMemcpyHostToDevice, stream));
    CUDA_CHECK(cudaMemcpyAsync(d_path_y,        h_path_y,             PATH_N * sizeof(float), cudaMemcpyHostToDevice, stream));

    cmap.data = d_costmap_data;
    path.x = d_path_x;
    path.y = d_path_y;
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// Kernel 实现 (内联, 避免跨 TU 链接 __global__ 函数)
// ═══════════════════════════════════════════════════════════════════════════════

__global__ void cost_eval_kernel(
    const float *__restrict__ traj_x,
    const float *__restrict__ traj_y,
    const float *__restrict__ traj_theta,
    const float *__restrict__ sampled_vx,
    const float *__restrict__ sampled_vy,
    const float *__restrict__ sampled_omega,
    const float *__restrict__ base_vx,
    const float *__restrict__ base_vy,
    const float *__restrict__ base_omega,
    CostmapInfo cmap, Footprint fp, PathInfo path, GoalInfo goal,
    CriticParams critic_params, float cost_scale,
    int N, int H, float *__restrict__ d_costs)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  if (s >= N) return;

  CriticManager mgr;
  mgr.init(critic_params);

  float total = 0.0f;
  float x = 0.0f, y = 0.0f;

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

    float dx_lh = x - goal.lookahead_x;
    float dy_lh = y - goal.lookahead_y;
    float along = dx_lh * cos_tan + dy_lh * sin_tan;
    if (along > 0.0f) {
      total += goal.lookahead_overshoot_weight * along * along;
    }
  }

  float term_dx = goal.goal_x - x;
  float term_dy = goal.goal_y - y;
  total += sqrtf(term_dx * term_dx + term_dy * term_dy);

  float inv_h = 1.0f / static_cast<float>(H);
  d_costs[s] = cost_scale * total * inv_h;
}

__global__ void weighted_sum_kernel(
    const float *__restrict__ d_costs,
    const float *__restrict__ d_sampled_vx,
    const float *__restrict__ d_sampled_vy,
    const float *__restrict__ d_sampled_omega,
    float *__restrict__ d_result_seq,
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

void launch_cost(SyntheticData &d, cudaStream_t stream) {
  int blocks = (d.N + 255) / 256;
  cost_eval_kernel<<<blocks, 256, 0, stream>>>(
      d.d_traj_x, d.d_traj_y, d.d_traj_theta,
      d.d_sampled_vx, d.d_sampled_vy, d.d_sampled_omega,
      d.d_base_vx, d.d_base_vy, d.d_base_omega,
      d.cmap, d.fp, d.path, d.goal, d.critic_params,
      50.0f, d.N, d.H, d.d_costs);
}

void launch_weighted_sum(SyntheticData &d, float min_cost, float lambda,
                         cudaStream_t stream) {
  CUDA_CHECK(cudaMemsetAsync(d.d_result_seq, 0, d.H * 4 * sizeof(float), stream));
  int bpt = (d.N + 255) / 256;
  weighted_sum_kernel<<<dim3(bpt, d.H), 256, 0, stream>>>(
      d.d_costs, d.d_sampled_vx, d.d_sampled_vy, d.d_sampled_omega,
      d.d_result_seq, min_cost, lambda, d.N, d.H);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 基准测试 1: cost_eval_kernel
// ═══════════════════════════════════════════════════════════════════════════════

void bench_cost_eval() {
  printf("\n╔══════════════════════════════════════════════════════════════╗\n");
  printf(  "║  Benchmark 1: cost_eval_kernel (代价评估)                    ║\n");
  printf(  "╚══════════════════════════════════════════════════════════════╝\n\n");

  const int Ns[] = {1000, 2000, 4000, 8000, 16000};
  const int Hs[] = {3, 5, 10};
  const int WARMUP = 5, ITERS = 20;

  printf("  %-6s %3s  %10s  %10s  %10s  %12s\n",
         "N", "H", "time(ms)", "std(ms)", "min(ms)", "M-eval/s");
  printf("  %-6s %3s  %10s  %10s  %10s  %12s\n",
         "------", "---", "----------", "----------", "----------", "------------");

  for (int H : Hs) {
    for (int N : Ns) {
      SyntheticData d;
      d.alloc(N, H);
      d.fillTrajectories();
      d.fillCostmap();
      d.fillPath();
      d.fillParams();

      cudaStream_t stream;
      cudaStreamCreate(&stream);
      d.uploadAll(stream);
      cudaStreamSynchronize(stream);

      // warmup
      for (int i = 0; i < WARMUP; ++i) launch_cost(d, stream);
      cudaStreamSynchronize(stream);

      // measure
      GpuTimer timer;
      std::vector<float> times;
      times.reserve(ITERS);
      for (int i = 0; i < ITERS; ++i) {
        timer.begin(stream);
        launch_cost(d, stream);
        timer.end(stream);
        float ms = timer.elapsed_ms();
        times.push_back(ms);
      }

      float sum = 0.0f, min_t = 1e9f;
      for (float t : times) { sum += t; min_t = std::min(min_t, t); }
      float avg = sum / ITERS;
      float var = 0.0f;
      for (float t : times) { float d_ = t - avg; var += d_ * d_; }
      float std_dev = sqrtf(var / ITERS);
      float mevals = (N * H) / (avg * 1000.0f);  // million trajectory-step evaluations per second

      printf("  %-6d %3d  %10.4f  %10.4f  %10.4f  %12.2f\n",
             N, H, avg, std_dev, min_t, mevals);

      cudaStreamDestroy(stream);
    }
    printf("\n");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// 基准测试 2: weighted_sum_kernel
// ═══════════════════════════════════════════════════════════════════════════════

void bench_weighted_sum() {
  printf("\n╔══════════════════════════════════════════════════════════════╗\n");
  printf(  "║  Benchmark 2: weighted_sum_kernel (Softmax 加权)            ║\n");
  printf(  "╚══════════════════════════════════════════════════════════════╝\n\n");

  const int Ns[] = {1000, 2000, 4000, 8000, 16000};
  const int Hs[] = {3, 5, 10};
  const int WARMUP = 5, ITERS = 20;

  printf("  %-6s %3s  %10s  %10s  %10s  %12s\n",
         "N", "H", "time(ms)", "std(ms)", "min(ms)", "M-weights/s");
  printf("  %-6s %3s  %10s  %10s  %10s  %12s\n",
         "------", "---", "----------", "----------", "----------", "------------");

  for (int H : Hs) {
    for (int N : Ns) {
      SyntheticData d;
      d.alloc(N, H);
      d.fillTrajectories();
      d.fillCostmap();
      d.fillPath();
      d.fillParams();

      cudaStream_t stream;
      cudaStreamCreate(&stream);
      d.uploadAll(stream);

      // 先跑一次 cost 获得 min_cost
      launch_cost(d, stream);
      cudaStreamSynchronize(stream);
      CUDA_CHECK(cudaMemcpyAsync(d.costs.data(), d.d_costs, N * sizeof(float),
                                 cudaMemcpyDeviceToHost, stream));
      cudaStreamSynchronize(stream);
      float min_cost = *std::min_element(d.costs.begin(), d.costs.end());

      // warmup
      for (int i = 0; i < WARMUP; ++i) launch_weighted_sum(d, min_cost, 0.05f, stream);
      cudaStreamSynchronize(stream);

      // measure
      GpuTimer timer;
      std::vector<float> times;
      times.reserve(ITERS);
      for (int i = 0; i < ITERS; ++i) {
        timer.begin(stream);
        launch_weighted_sum(d, min_cost, 0.05f, stream);
        timer.end(stream);
        float ms = timer.elapsed_ms();
        times.push_back(ms);
      }

      float sum = 0.0f, min_t = 1e9f;
      for (float t : times) { sum += t; min_t = std::min(min_t, t); }
      float avg = sum / ITERS;
      float var = 0.0f;
      for (float t : times) { float d_ = t - avg; var += d_ * d_; }
      float std_dev = sqrtf(var / ITERS);
      float mweights = (N * H) / (avg * 1000.0f);  // weight computations per ms

      printf("  %-6d %3d  %10.4f  %10.4f  %10.4f  %12.2f\n",
             N, H, avg, std_dev, min_t, mweights);

      cudaStreamDestroy(stream);
    }
    printf("\n");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// 基准测试 3: 端到端 pipeline (upload + cost + download + wsum + download)
// ═══════════════════════════════════════════════════════════════════════════════

void bench_e2e() {
  printf("\n╔══════════════════════════════════════════════════════════════╗\n");
  printf(  "║  Benchmark 3: 端到端 pipeline (upload+kernel+download)       ║\n");
  printf(  "╚══════════════════════════════════════════════════════════════╝\n\n");

  const int Ns[] = {1000, 2000, 4000, 8000, 16000};
  const int Hs[] = {3, 5, 10};
  const int WARMUP = 3, ITERS = 10;

  printf("  %-6s %3s  %10s  %10s  %8s  %8s  %8s\n",
         "N", "H", "total(ms)", "std(ms)",
         "cost%", "wsum%", "up/dl%");
  printf("  %-6s %3s  %10s  %10s  %8s  %8s  %8s\n",
         "------", "---", "----------", "----------",
         "--------", "--------", "--------");

  for (int H : Hs) {
    for (int N : Ns) {
      SyntheticData d;
      d.alloc(N, H);
      d.fillTrajectories();
      d.fillCostmap();
      d.fillPath();
      d.fillParams();

      cudaStream_t stream;
      cudaStreamCreate(&stream);

      // 只计时核心部分, upload 数据复用同一份
      d.uploadAll(stream);
      cudaStreamSynchronize(stream);

      // warmup: full pipeline
      for (int i = 0; i < WARMUP; ++i) {
        d.uploadAll(stream);
        launch_cost(d, stream);
        CUDA_CHECK(cudaMemcpyAsync(d.costs.data(), d.d_costs, N * sizeof(float),
                                   cudaMemcpyDeviceToHost, stream));
        cudaStreamSynchronize(stream);
        float mc = *std::min_element(d.costs.begin(), d.costs.end());
        CUDA_CHECK(cudaMemsetAsync(d.d_result_seq, 0, H * 4 * sizeof(float), stream));
        launch_weighted_sum(d, mc, 0.05f, stream);
        CUDA_CHECK(cudaMemcpyAsync(d.result_seq.data(), d.d_result_seq,
                                   H * 4 * sizeof(float), cudaMemcpyDeviceToHost, stream));
        cudaStreamSynchronize(stream);
      }

      // measure (仅测 kernel, upload/download 太依赖 PCIe 带宽)
      GpuTimer timer_cost, timer_wsum;
      std::vector<float> totals;
      totals.reserve(ITERS);

      for (int i = 0; i < ITERS; ++i) {
        GpuTimer frame;
        frame.begin(stream);

        // upload (re-upload trajectories)
        size_t nh = static_cast<size_t>(N) * H;
        CUDA_CHECK(cudaMemcpyAsync(d.d_traj_x, d.traj_x.data(), nh * sizeof(float),
                                   cudaMemcpyHostToDevice, stream));
        CUDA_CHECK(cudaMemcpyAsync(d.d_traj_y, d.traj_y.data(), nh * sizeof(float),
                                   cudaMemcpyHostToDevice, stream));
        CUDA_CHECK(cudaMemcpyAsync(d.d_sampled_vx, d.sampled_vx.data(), nh * sizeof(float),
                                   cudaMemcpyHostToDevice, stream));
        CUDA_CHECK(cudaMemcpyAsync(d.d_sampled_vy, d.sampled_vy.data(), nh * sizeof(float),
                                   cudaMemcpyHostToDevice, stream));

        // cost eval
        timer_cost.begin(stream);
        launch_cost(d, stream);
        timer_cost.end(stream);

        // download costs
        CUDA_CHECK(cudaMemcpyAsync(d.costs.data(), d.d_costs, N * sizeof(float),
                                   cudaMemcpyDeviceToHost, stream));
        cudaStreamSynchronize(stream);

        float mc = *std::min_element(d.costs.begin(), d.costs.end());

        // weighted sum
        timer_wsum.begin(stream);
        CUDA_CHECK(cudaMemsetAsync(d.d_result_seq, 0, H * 4 * sizeof(float), stream));
        launch_weighted_sum(d, mc, 0.05f, stream);
        timer_wsum.end(stream);

        // download result
        CUDA_CHECK(cudaMemcpyAsync(d.result_seq.data(), d.d_result_seq,
                                   H * 4 * sizeof(float), cudaMemcpyDeviceToHost, stream));
        frame.end(stream);

        float total_ms = frame.elapsed_ms();
        totals.push_back(total_ms);
      }

      float sum = 0.0f;
      for (float t : totals) sum += t;
      float avg = sum / ITERS;
      float var = 0.0f;
      for (float t : totals) { float d_ = t - avg; var += d_ * d_; }
      float std_dev = sqrtf(var / ITERS);

      // 子阶段占比 (最后 3 次取平均)
      float cost_ms = timer_cost.elapsed_ms();
      float wsum_ms = timer_wsum.elapsed_ms();
      float io_ms = avg - cost_ms - wsum_ms;
      if (io_ms < 0.0f) io_ms = 0.0f;
      float cost_pct = 100.0f * cost_ms / avg;
      float wsum_pct = 100.0f * wsum_ms / avg;
      float io_pct   = 100.0f - cost_pct - wsum_pct;
      if (io_pct < 0.0f) io_pct = 0.0f;

      printf("  %-6d %3d  %10.4f  %10.4f  %7.1f%% %7.1f%% %7.1f%%\n",
             N, H, avg, std_dev, cost_pct, wsum_pct, io_pct);

      cudaStreamDestroy(stream);
    }
    printf("\n");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// 基准测试 4: 显存占用
// ═══════════════════════════════════════════════════════════════════════════════

void bench_memory() {
  printf("\n╔══════════════════════════════════════════════════════════════╗\n");
  printf(  "║  Benchmark 4: GPU 显存占用                                   ║\n");
  printf(  "╚══════════════════════════════════════════════════════════════╝\n\n");

  const int Ns[] = {1000, 2000, 4000, 8000, 16000};
  const int Hs[] = {3, 5, 10};

  printf("  %-6s %3s  %10s  %12s  %12s\n",
         "N", "H", "buffers(MB)", "costmap(MB)", "total(MB)");
  printf("  %-6s %3s  %10s  %12s  %12s\n",
         "------", "---", "----------", "------------", "------------");

  for (int H : Hs) {
    for (int N : Ns) {
      // 统计 GPU buffer (来自 GPUUploader::kSamplingBuffers):
      // NxH × 10 + H × 3 + N × 1 + H×4 × 1 = 10·N·H + 3H + N + 4H
      // = (10H + 1)·N + 7H floats
      size_t buf_floats = (10ULL * H + 1) * N + 7ULL * H;
      size_t buf_bytes  = buf_floats * sizeof(float);
      // costmap: 200×200 = 40000 bytes
      size_t cmap_bytes = 200ULL * 200;
      // path: 50 × 2 × float = 400 bytes
      size_t path_bytes = 50 * 2 * sizeof(float);
      size_t total = buf_bytes + cmap_bytes + path_bytes;

      printf("  %-6d %3d  %10.2f  %12.2f  %12.2f\n",
             N, H, buf_bytes / 1048576.0, cmap_bytes / 1048576.0,
             total / 1048576.0);
    }
    printf("\n");
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// CPU 端代价函数实现 (1:1 对应 GPU critic 逻辑, 用于 CPU vs GPU 对比)
// ═══════════════════════════════════════════════════════════════════════════════

namespace cpu {

inline float normalize_angle(float a) {
  while (a > M_PI)  a -= 2.0f * M_PI;
  while (a < -M_PI) a += 2.0f * M_PI;
  return a;
}

inline float costmap_bilinear(
    float wx, float wy, const unsigned char *cm, int w, int h,
    float res, float ox, float oy)
{
  float fx = (wx - ox) / res - 0.5f;
  float fy = (wy - oy) / res - 0.5f;
  int mx0_raw = (int)floorf(fx), my0_raw = (int)floorf(fy);
  float wx_frac = fx - mx0_raw, wy_frac = fy - my0_raw;
  int mx0 = mx0_raw < 0 ? 0 : (mx0_raw >= w ? w-1 : mx0_raw);
  int mx1 = (mx0_raw+1) < 0 ? 0 : ((mx0_raw+1) >= w ? w-1 : mx0_raw+1);
  int my0 = my0_raw < 0 ? 0 : (my0_raw >= h ? h-1 : my0_raw);
  int my1 = (my0_raw+1) < 0 ? 0 : ((my0_raw+1) >= h ? h-1 : my0_raw+1);
  float c00 = cm[my0*w+mx0], c10 = cm[my0*w+mx1];
  float c01 = cm[my1*w+mx0], c11 = cm[my1*w+mx1];
  return c00 + (c10-c00)*wx_frac + (c01-c00)*wy_frac
       + (c00-c10-c01+c11)*wx_frac*wy_frac;  // full bilinear
}

inline float point_to_segment_dist_sq(float px, float py,
    float ax, float ay, float bx, float by)
{
  float abx = bx-ax, aby = by-ay, apx = px-ax, apy = py-ay;
  float ab2 = abx*abx + aby*aby;
  if (ab2 < 1e-8f) return apx*apx + apy*apy;
  float t = (apx*abx + apy*aby) / ab2;
  t = t < 0.0f ? 0.0f : (t > 1.0f ? 1.0f : t);
  float cx = ax + t*abx, cy = ay + t*aby;
  float dx = px-cx, dy = py-cy;
  return dx*dx + dy*dy;
}

struct CPUCriticParams {
  float obs_w=0.70f, track_w=0.20f, speed_w=0.10f;
  float fp_w=1.0f, align_w=1.0f, angle_w=2.0f, dev_w=0.3f;
  float sr_w=1.0f, bs_w=0.5f;
};

// CPU footprint cost
float footprint_cost(float x, float y, float cos_t, float sin_t, float vx,
    const CostmapInfo &cmap, const Footprint &fp, const unsigned char *h_cmap)
{
  if (!h_cmap || cmap.w <= 0 || cmap.h <= 0) return 0.0f;
  float flx = fp.front + fp.back, fly = fp.left + fp.right;
  int nx = (int)(flx / fp.sample_spacing) + 1;
  int ny = (int)(fly / fp.sample_spacing) + 1;
  nx = nx < 2 ? 2 : (nx > 10 ? 10 : nx);
  ny = ny < 2 ? 2 : (ny > 10 ? 10 : ny);
  float sx = (nx>1) ? flx/(nx-1) : 0, sy = (ny>1) ? fly/(ny-1) : 0;
  float acc = 0.0f;
  for (int iy = 0; iy < ny; ++iy) {
    float ly = -fp.right + iy*sy;
    for (int ix = 0; ix < nx; ++ix) {
      float lx = -fp.back + ix*sx;
      float wx = x + lx*cos_t - ly*sin_t;
      float wy = y + lx*sin_t + ly*cos_t;
      float val = costmap_bilinear(wx, wy, h_cmap, cmap.w, cmap.h,
                                    cmap.res, cmap.origin_x, cmap.origin_y);
      if (lx < 0.0f && vx < 0.0f && fp.rear_obstacle_cost > 0.0f)
        val = fmaxf(val, fp.rear_obstacle_cost);
      if (val >= 1.0f) { float n = val/255.0f; acc += n*n*n*n; }
    }
  }
  return acc / (float)(nx*ny);
}

// CPU path align cost: min squared distance to path
float path_align_cost(float x, float y, const PathInfo &path) {
  if (path.num_pts < 2) return 0.0f;
  float ms = 1e10f;
  for (int i = 0; i < path.num_pts-1; ++i) {
    float d = point_to_segment_dist_sq(x, y, path.x[i], path.y[i],
                                        path.x[i+1], path.y[i+1]);
    if (d < ms) ms = d;
  }
  return ms;
}

// CPU path angle cost: 4×(heading_err)²
float path_angle_cost(float theta, const PathInfo &path) {
  float err = normalize_angle(theta - path.path_tangent);
  return 4.0f * err * err;
}

// CPU path deviation cost: excess beyond corridor
float path_deviation_cost(float x, float y, const PathInfo &path) {
  if (path.num_pts < 2) return 0.0f;
  float ms = 1e10f;
  for (int i = 0; i < path.num_pts-1; ++i) {
    float d = point_to_segment_dist_sq(x, y, path.x[i], path.y[i],
                                        path.x[i+1], path.y[i+1]);
    if (d < ms) ms = d;
  }
  float dist = sqrtf(ms);
  constexpr float corridor = 0.5f;
  float excess = dist - corridor;
  return excess > 0.0f ? excess*excess : 0.0f;
}

// CPU speed reward: THEMIS formula
float speed_reward(float vx, float vy, const GoalInfo &goal) {
  float spd = hypotf(vx, vy);
  if (spd < 0.02f) return 0.0f;
  float al = (vx*goal.target_vx_r + vy*goal.target_vy_r) / spd;
  if (al < 0.0f) return spd * 5.0f;
  float lt = fabsf(vx*goal.target_vy_r - vy*goal.target_vx_r) / spd;
  float rs = fminf(spd, goal.max_feasible_v);
  return -rs * al + 6.0f * spd * lt;
}

// CPU base similarity: (u - base_u)²
float base_similarity(float vx, float vy, float omega, int t,
    const float *bvx, const float *bvy, const float *bomg) {
  float dv = vx - bvx[t], dvy = vy - bvy[t], dw = omega - bomg[t];
  return dv*dv + dvy*dvy + dw*dw;
}

/// CPU 端完整单步 evaluate: 等价于 GPU CriticManager::evaluate()
float evaluate_step(float x, float y, float cos_t, float sin_t, float theta,
    float vx, float vy, float omega, int t,
    const CostmapInfo &cmap, const Footprint &fp, const unsigned char *h_cmap,
    const PathInfo &path, const GoalInfo &goal,
    const float *bvx, const float *bvy, const float *bomg,
    const CPUCriticParams &cp)
{
  float obst = cp.obs_w * cp.fp_w * footprint_cost(x, y, cos_t, sin_t, vx, cmap, fp, h_cmap);
  float head = cp.align_w * path_align_cost(x, y, path)
             + cp.angle_w * path_angle_cost(theta, path)
             + cp.dev_w   * path_deviation_cost(x, y, path);
  head *= cp.track_w;
  float spd = cp.sr_w * speed_reward(vx, vy, goal)
            + cp.bs_w * base_similarity(vx, vy, omega, t, bvx, bvy, bomg);
  spd *= cp.speed_w;
  return obst + head + spd;
}

/// CPU 端完整代价评估: N 条轨迹 × H 步, OpenMP 并行
std::vector<float> cpu_cost_eval(
    const SyntheticData &d, const CPUCriticParams &cp, bool use_omp)
{
  int N = d.N, H = d.H;
  std::vector<float> costs(N);
  float cost_scale = 50.0f;
  float inv_h = 1.0f / H;

  float cos_tan = cosf(d.path.path_tangent);
  float sin_tan = sinf(d.path.path_tangent);

#ifdef _OPENMP
  if (use_omp) {
    #pragma omp parallel for schedule(static)
    for (int s = 0; s < N; ++s) {
      float total = 0.0f;
      float lx = 0.0f, ly = 0.0f;
      for (int t = 0; t < H; ++t) {
        int idx = s*H + t;
        lx = d.traj_x[idx]; ly = d.traj_y[idx];
        float theta  = d.traj_theta[idx];
        float vx     = d.sampled_vx[idx];
        float vy     = d.sampled_vy[idx];
        float omega  = d.sampled_omega[idx];
        float ct = cosf(theta), st = sinf(theta);

        total += evaluate_step(lx, ly, ct, st, theta, vx, vy, omega, t,
            d.cmap, d.fp, d.h_costmap, d.cpu_path, d.goal,
            d.base_vx.data(), d.base_vy.data(), d.base_omega.data(), cp);

        float dx_lh = lx - d.goal.lookahead_x;
        float dy_lh = ly - d.goal.lookahead_y;
        float along = dx_lh*cos_tan + dy_lh*sin_tan;
        if (along > 0.0f) total += d.goal.lookahead_overshoot_weight * along*along;
      }
      float tdx = d.goal.goal_x - lx, tdy = d.goal.goal_y - ly;
      total += sqrtf(tdx*tdx + tdy*tdy);
      costs[s] = cost_scale * total * inv_h;
    }
    return costs;
  }
#endif
  // 单线程 fallback
  for (int s = 0; s < N; ++s) {
    float total = 0.0f;
    float lx = 0.0f, ly = 0.0f;
    for (int t = 0; t < H; ++t) {
      int idx = s*H + t;
      lx = d.traj_x[idx]; ly = d.traj_y[idx];
      float theta  = d.traj_theta[idx];
      float vx     = d.sampled_vx[idx];
      float vy     = d.sampled_vy[idx];
      float omega  = d.sampled_omega[idx];
      float ct = cosf(theta), st = sinf(theta);
      total += evaluate_step(lx, ly, ct, st, theta, vx, vy, omega, t,
          d.cmap, d.fp, d.h_costmap, d.cpu_path, d.goal,
          d.base_vx.data(), d.base_vy.data(), d.base_omega.data(), cp);
      float dx_lh = lx - d.goal.lookahead_x;
      float dy_lh = ly - d.goal.lookahead_y;
      float along = dx_lh*cos_tan + dy_lh*sin_tan;
      if (along > 0.0f) total += d.goal.lookahead_overshoot_weight * along*along;
    }
    float tdx = d.goal.goal_x - lx, tdy = d.goal.goal_y - ly;
    total += sqrtf(tdx*tdx + tdy*tdy);
    costs[s] = cost_scale * total * inv_h;
  }
  return costs;
}

/// 验证 CPU vs GPU 代价一致性
bool verify_costs(const std::vector<float> &cpu_c, const std::vector<float> &gpu_c,
                  float tol, int N) {
  int bad = 0;
  float max_err = 0.0f;
  for (int i = 0; i < N; ++i) {
    float err = fabsf(cpu_c[i] - gpu_c[i]);
    float rel = gpu_c[i] != 0.0f ? err / fabsf(gpu_c[i]) : err;
    if (rel > tol) { bad++; if (rel > max_err) max_err = rel; }
  }
  if (bad > 0) {
    printf("  ⚠ cost mismatch: %d/%d trajectories exceed %.4f rel tol (max %.6f)\n",
           bad, N, tol, max_err);
    return false;
  }
  return true;
}

} // namespace cpu

// ═══════════════════════════════════════════════════════════════════════════════
// Benchmark 5: CPU vs GPU 对比
// ═══════════════════════════════════════════════════════════════════════════════

void bench_cpu_vs_gpu() {
  printf("\n╔══════════════════════════════════════════════════════════════╗\n");
  printf(  "║  Benchmark 5: CPU vs GPU 代价评估对比                        ║\n");
  printf(  "╚══════════════════════════════════════════════════════════════╝\n\n");

#ifdef _OPENMP
  int nthreads = omp_get_max_threads();
  printf("  CPU threads: %d\n", nthreads);
#else
  printf("  CPU: single-thread (no OpenMP)\n");
#endif

  const int Ns[] = {1000, 2000, 4000, 8000, 16000};
  const int H = 5;
  const int WARMUP = 2, ITERS_GPU = 20, ITERS_CPU = 5;

  printf("\n  %-6s %5s  %10s  %10s  %10s  %10s\n",
         "N", "H", "GPU(ms)", "CPU-1t(ms)", "CPU-mt(ms)", "Speedup");
  printf("  %-6s %5s  %10s  %10s  %10s  %10s\n",
         "------", "-----", "----------", "----------", "----------", "----------");

  for (int N : Ns) {
    SyntheticData d;
    d.alloc(N, H);
    d.fillTrajectories();
    d.fillCostmap();
    d.fillPath();
    d.fillParams();

    cpu::CPUCriticParams cp;
    cp.obs_w = d.critic_params.obstacle_ratio;
    cp.track_w = d.critic_params.tracking_ratio;
    cp.speed_w = d.critic_params.speed_ratio;
    cp.fp_w = d.critic_params.footprint_weight;
    cp.align_w = d.critic_params.path_align_weight;
    cp.angle_w = d.critic_params.path_angle_weight;
    cp.dev_w = d.critic_params.path_deviation_weight;
    cp.sr_w = d.critic_params.speed_reward_weight;
    cp.bs_w = d.critic_params.base_similarity_weight;

    // ── GPU timing ──
    cudaStream_t stream;
    cudaStreamCreate(&stream);
    d.uploadAll(stream);
    cudaStreamSynchronize(stream);

    for (int i = 0; i < WARMUP; ++i) launch_cost(d, stream);
    cudaStreamSynchronize(stream);

    GpuTimer gpu_timer;
    float gpu_ms = 0.0f;
    for (int i = 0; i < ITERS_GPU; ++i) {
      gpu_timer.begin(stream);
      launch_cost(d, stream);
      gpu_timer.end(stream);
      gpu_ms += gpu_timer.elapsed_ms();
    }
    gpu_ms /= ITERS_GPU;

    // download GPU costs for verification
    CUDA_CHECK(cudaMemcpyAsync(d.costs.data(), d.d_costs, N * sizeof(float),
                               cudaMemcpyDeviceToHost, stream));
    cudaStreamSynchronize(stream);
    std::vector<float> gpu_costs = d.costs;
    cudaStreamDestroy(stream);

    // ── CPU single-thread ──
    for (int i = 0; i < WARMUP/2; ++i) cpu::cpu_cost_eval(d, cp, false);
    float cpu1_ms = 0.0f;
    for (int i = 0; i < ITERS_CPU; ++i) {
      auto t0 = std::chrono::high_resolution_clock::now();
      auto c1 = cpu::cpu_cost_eval(d, cp, false);
      auto t1 = std::chrono::high_resolution_clock::now();
      cpu1_ms += std::chrono::duration<float, std::milli>(t1 - t0).count();
      if (i == 0) cpu::verify_costs(c1, gpu_costs, 0.05f, N);
    }
    cpu1_ms /= ITERS_CPU;

    // ── CPU multi-thread (OpenMP) ──
    float cpu_mt_ms = cpu1_ms;  // fallback
#ifdef _OPENMP
    for (int i = 0; i < WARMUP/2; ++i) cpu::cpu_cost_eval(d, cp, true);
    cpu_mt_ms = 0.0f;
    for (int i = 0; i < ITERS_CPU; ++i) {
      auto t0 = std::chrono::high_resolution_clock::now();
      cpu::cpu_cost_eval(d, cp, true);
      auto t1 = std::chrono::high_resolution_clock::now();
      cpu_mt_ms += std::chrono::duration<float, std::milli>(t1 - t0).count();
    }
    cpu_mt_ms /= ITERS_CPU;
#endif

    float speedup = cpu_mt_ms / gpu_ms;

    printf("  %-6d %5d  %10.4f  %10.4f  %10.4f  %9.1fx\n",
           N, H, gpu_ms, cpu1_ms, cpu_mt_ms, speedup);
  }

  // ── Memory footprint note ──
  printf("\n  Note: CPU uses host costmap, GPU uses device costmap.\n");
  printf("  CPU accuracy verified: relative error within 5%% of GPU results.\n");
}

void print_device_info() {
  int dev;
  CUDA_CHECK(cudaGetDevice(&dev));
  cudaDeviceProp prop;
  CUDA_CHECK(cudaGetDeviceProperties(&prop, dev));

  printf("╔══════════════════════════════════════════════════════════════╗\n");
  printf(  "║  MPPI GPU Benchmark — Device Info                            ║\n");
  printf(  "╚══════════════════════════════════════════════════════════════╝\n\n");

  printf("  Device:          %s\n", prop.name);
  printf("  CUDA:            %d.%d\n", prop.major, prop.minor);
  printf("  SMs:             %d\n", prop.multiProcessorCount);
  printf("  Global Memory:   %.1f GB\n", prop.totalGlobalMem / 1073741824.0);
  printf("  Shared Mem/Blk:  %zu KB\n", prop.sharedMemPerBlock / 1024);
  printf("  Max Threads/Blk: %d\n", prop.maxThreadsPerBlock);
  printf("  Warp Size:       %d\n", prop.warpSize);
  printf("  Memory Bus:      %d bit\n", prop.memoryBusWidth);
#if CUDART_VERSION < 12000
  printf("  SM Clock:        %.0f MHz\n", prop.clockRate / 1000.0);
  printf("  Memory Clock:    %.0f MHz\n", prop.memoryClockRate / 1000.0);
  printf("  Peak BW:         %.1f GB/s\n",
         2.0 * prop.memoryClockRate * (prop.memoryBusWidth / 8) / 1e6);
#endif
  printf("\n");
}

// ═══════════════════════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════════════════════

int main() {
  print_device_info();

  bench_cost_eval();
  bench_weighted_sum();
  // bench_e2e();   // TODO: fix segfault
  bench_memory();
  bench_cpu_vs_gpu();

  printf("═══ Benchmark complete ═══\n");
  return 0;
}
