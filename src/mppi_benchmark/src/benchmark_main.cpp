/**
 * @file benchmark_main.cpp
 * @brief MPPI 代价计算 GPU vs CPU 延迟对比
 *
 * 生成随机数据, 分别用 GPU 和 CPU 计算 N 条轨迹的代价,
 * 对比耗时并验证结果一致性。
 *
 * 用法:
 *   ./mppi_benchmark [N_samples] [horizon] [warmup_runs] [bench_runs]
 *   默认: N=3000, H=12, warmup=5, bench=50
 */

#include <iostream>
#include <iomanip>
#include <vector>
#include <random>
#include <cmath>
#include <cstring>
#include <algorithm>
#include <numeric>

// GPU host wrapper
extern "C" {
float run_gpu_benchmark(
    const float* h_vx, const float* h_vy, const float* h_w,
    int num_samples, int horizon,
    float start_x, float start_y, float start_theta,
    float target_x, float target_y,
    const unsigned char* h_costmap, int cm_w, int cm_h,
    float cm_res, float cm_ox, float cm_oy,
    float dt, float obs_w, float track_w, float prog_w,
    float path_vx_r, float path_vy_r, float path_tangent, float goal_yaw,
    float fp_f, float fp_b, float fp_l, float fp_r,
    float fp_sp, float rear_cost,
    const float* h_path_x, const float* h_path_y, int num_path,
    float fgx, float fgy,
    float* h_costs_out,
    float* out_upload_ms, float* out_kernel_ms, float* out_download_ms);
}

// CPU
#include "mppi_benchmark/mppi_cost_cpu.hpp"

// ════════════════════════════════════════════
// 随机数据生成
// ════════════════════════════════════════════

struct BenchData {
  int N, H;
  std::vector<float> vx, vy, omega;
  float start_x, start_y, start_theta;
  float target_x, target_y;
  std::vector<unsigned char> costmap;
  int cm_w, cm_h;
  float cm_res, cm_ox, cm_oy;
  float dt;
  float obs_w, track_w, prog_w;
  float path_vx_r, path_vy_r, path_tangent, goal_yaw;
  float fp_f, fp_b, fp_l, fp_r, fp_sp, rear_cost;
  std::vector<float> path_x, path_y;
  int num_path;
  float fgx, fgy;
};

BenchData generate_random_data(int N, int H)
{
  std::mt19937 rng(42);  // 固定种子, 可复现
  std::uniform_real_distribution<float> u01(0.0f, 1.0f);
  std::normal_distribution<float> n01(0.0f, 1.0f);
  auto rand_vel = [&]() { return n01(rng) * 0.5f; };

  BenchData d;
  d.N = N;
  d.H = H;

  // 速度序列: N×H, 模拟 MPPI 采样输出
  d.vx.resize(N * H);
  d.vy.resize(N * H);
  d.omega.resize(N * H);
  for (int i = 0; i < N * H; ++i) {
    d.vx[i] = rand_vel();
    d.vy[i] = rand_vel() * 0.5f;
    d.omega[i] = rand_vel() * 0.3f;
  }

  // 起始位姿
  d.start_x = 0.0f; d.start_y = 0.0f; d.start_theta = 0.0f;
  // 前瞻点 (前方 0.4m, 偏右 0.1m)
  d.target_x = 0.38f; d.target_y = 0.12f;
  // 终点
  d.fgx = 2.5f; d.fgy = 0.5f;

  // 代价地图: 60×60 cell, 0.05m 分辨率 (3m×3m)
  d.cm_w = 60; d.cm_h = 60;
  d.cm_res = 0.05f;
  d.cm_ox = -1.5f; d.cm_oy = -1.5f;
  d.costmap.resize(d.cm_w * d.cm_h);
  for (auto& c : d.costmap) {
    float r = u01(rng);
    if (r < 0.05f)      c = 254;  // 5% 致命障碍
    else if (r < 0.15f) c = static_cast<unsigned char>(u01(rng) * 200);  // 10% 膨胀区
    else                c = 0;     // 85% 自由空间
  }

  // 参数
  d.dt = 0.1f;
  d.obs_w = 10.0f; d.track_w = 30.0f; d.prog_w = 1.5f;

  // 前瞻点方向 (机器人系)
  float lh_angle = std::atan2(d.target_y - d.start_y, d.target_x - d.start_x);
  d.path_vx_r = std::cos(lh_angle);
  d.path_vy_r = std::sin(lh_angle);
  d.path_tangent = lh_angle;
  d.goal_yaw = 0.3f;

  // 足迹
  d.fp_f = 0.17f; d.fp_b = 0.17f; d.fp_l = 0.28f; d.fp_r = 0.28f;
  d.fp_sp = 0.04f; d.rear_cost = 200.0f;

  // 参考路径: 从起点到终点的折线 (30点)
  d.num_path = 30;
  d.path_x.resize(d.num_path);
  d.path_y.resize(d.num_path);
  for (int i = 0; i < d.num_path; ++i) {
    float t = static_cast<float>(i) / (d.num_path - 1);
    d.path_x[i] = d.start_x + t * (d.fgx - d.start_x);
    d.path_y[i] = d.start_y + t * (d.fgy - d.start_y)
                + 0.1f * std::sin(t * 3.14159f);  // 加一点弯曲
  }

  return d;
}

// ════════════════════════════════════════════
// Main
// ════════════════════════════════════════════

int main(int argc, char** argv)
{
  int N = (argc > 1) ? std::atoi(argv[1]) : 3000;
  int H = (argc > 2) ? std::atoi(argv[2]) : 12;
  int WARMUP = (argc > 3) ? std::atoi(argv[3]) : 5;
  int RUNS   = (argc > 4) ? std::atoi(argv[4]) : 50;

  std::cout << "\n============================================\n";
  std::cout << "  MPPI Cost Benchmark: GPU vs CPU\n";
  std::cout << "  Samples=" << N << "  Horizon=" << H
            << "  Warmup=" << WARMUP << "  Runs=" << RUNS << "\n";
  std::cout << "============================================\n\n";

  // 生成数据
  std::cout << "[1/4] Generating random data... " << std::flush;
  auto data = generate_random_data(N, H);
  size_t data_mb = (data.vx.size() + data.vy.size() + data.omega.size()) * sizeof(float)
                 + data.costmap.size() + (data.path_x.size() + data.path_y.size()) * sizeof(float);
  std::cout << std::fixed << std::setprecision(1)
            << (data_mb / 1024.0 / 1024.0) << " MB\n";

  // GPU warmup
  std::cout << "[2/4] GPU warmup (" << WARMUP << " runs)... " << std::flush;
  std::vector<float> gpu_costs(N);
  float dummy_up, dummy_kernel, dummy_down;
  for (int i = 0; i < WARMUP; ++i) {
    run_gpu_benchmark(
        data.vx.data(), data.vy.data(), data.omega.data(),
        N, H,
        data.start_x, data.start_y, data.start_theta,
        data.target_x, data.target_y,
        data.costmap.data(), data.cm_w, data.cm_h,
        data.cm_res, data.cm_ox, data.cm_oy,
        data.dt, data.obs_w, data.track_w, data.prog_w,
        data.path_vx_r, data.path_vy_r, data.path_tangent, data.goal_yaw,
        data.fp_f, data.fp_b, data.fp_l, data.fp_r,
        data.fp_sp, data.rear_cost,
        data.path_x.data(), data.path_y.data(), data.num_path,
        data.fgx, data.fgy,
        gpu_costs.data(), &dummy_up, &dummy_kernel, &dummy_down);
  }
  std::cout << "done\n";

  // GPU benchmark
  std::cout << "[3/4] GPU benchmark (" << RUNS << " runs)... " << std::flush;
  std::vector<double> gpu_times, gpu_upload, gpu_kernel, gpu_download;
  for (int i = 0; i < RUNS; ++i) {
    float up, kern, down;
    float ms = run_gpu_benchmark(
        data.vx.data(), data.vy.data(), data.omega.data(),
        N, H,
        data.start_x, data.start_y, data.start_theta,
        data.target_x, data.target_y,
        data.costmap.data(), data.cm_w, data.cm_h,
        data.cm_res, data.cm_ox, data.cm_oy,
        data.dt, data.obs_w, data.track_w, data.prog_w,
        data.path_vx_r, data.path_vy_r, data.path_tangent, data.goal_yaw,
        data.fp_f, data.fp_b, data.fp_l, data.fp_r,
        data.fp_sp, data.rear_cost,
        data.path_x.data(), data.path_y.data(), data.num_path,
        data.fgx, data.fgy,
        gpu_costs.data(), &up, &kern, &down);
    gpu_times.push_back(static_cast<double>(ms));
    gpu_upload.push_back(static_cast<double>(up));
    gpu_kernel.push_back(static_cast<double>(kern));
    gpu_download.push_back(static_cast<double>(down));
  }
  std::sort(gpu_times.begin(), gpu_times.end());
  double gpu_med = gpu_times[gpu_times.size() / 2];
  double gpu_min = gpu_times.front(), gpu_max = gpu_times.back();
  double gpu_mean = std::accumulate(gpu_times.begin(), gpu_times.end(), 0.0) / RUNS;
  std::cout << "done\n";

  // CPU benchmark
  std::cout << "[4/4] CPU benchmark (" << RUNS << " runs)... " << std::flush;
  std::vector<double> cpu_times;
  std::vector<float> cpu_costs;
  for (int i = 0; i < RUNS; ++i) {
    auto result = mppi_benchmark::run_cpu_benchmark(
        data.vx.data(), data.vy.data(), data.omega.data(),
        N, H,
        data.start_x, data.start_y, data.start_theta,
        data.target_x, data.target_y,
        data.costmap.data(), data.cm_w, data.cm_h,
        data.cm_res, data.cm_ox, data.cm_oy,
        data.dt, data.obs_w, data.track_w, data.prog_w,
        data.path_vx_r, data.path_vy_r, data.path_tangent, data.goal_yaw,
        data.fp_f, data.fp_b, data.fp_l, data.fp_r,
        data.fp_sp, data.rear_cost,
        data.path_x.data(), data.path_y.data(), data.num_path,
        data.fgx, data.fgy);
    cpu_times.push_back(result.elapsed_ms);
    if (i == 0) cpu_costs = std::move(result.costs);
  }
  std::sort(cpu_times.begin(), cpu_times.end());
  double cpu_med = cpu_times[cpu_times.size() / 2];
  double cpu_min = cpu_times.front(), cpu_max = cpu_times.back();
  double cpu_mean = std::accumulate(cpu_times.begin(), cpu_times.end(), 0.0) / RUNS;
  std::cout << "done\n\n";

  // 结果一致性检查
  double max_diff = 0.0;
  for (int i = 0; i < N; ++i) {
    double diff = std::abs(static_cast<double>(gpu_costs[i]) - static_cast<double>(cpu_costs[i]));
    if (diff > max_diff) max_diff = diff;
  }

  auto med = [](std::vector<double>& v) { std::sort(v.begin(), v.end()); return v[v.size()/2]; };

  std::cout << "=============== Results ===============\n";
  std::cout << std::fixed << std::setprecision(3);
  std::cout << "GPU total:   median=" << med(gpu_times)
            << " ms  (upload=" << med(gpu_upload)
            << " + kernel=" << med(gpu_kernel)
            << " + download=" << med(gpu_download) << ")\n";
  std::cout << "CPU:         median=" << med(cpu_times) << " ms\n";
  std::cout << "Speedup:     " << std::setprecision(1)
            << (med(cpu_times) / med(gpu_times)) << "x (median)\n";
  std::cout << "Cost match:  max diff = " << std::scientific << max_diff
            << " (should be < 1e-3)\n";
  if (max_diff > 0.01f) {
    std::cout << "WARNING: GPU/CPU cost mismatch detected!\n";
  }
  std::cout << "======================================\n\n";

  return 0;
}
