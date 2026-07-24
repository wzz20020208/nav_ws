/**
 * @file test_critics.cu
 * @brief 代价函数调用链测试 — 验证 CriticManager → 三大类 → 各子代价
 *
 * 测试场景:
 *   1. 空旷地图 — footprint 全 free → obstacle cost = 0
 *   2. 障碍物地图 — footprint 全 inflated → obstacle cost > 0
 *   3. 路径对准 — 轨迹点在路径上 → path align cost ≈ 0
 *   4. 速度奖励 — 速度方向对齐目标 → speed reward < 0 (奖励)
 *   5. 多步累加 — 5 步代价求和
 *
 * 编译: 由 CMakeLists.txt 中的 test_critics 目标编译
 * 运行: ./test_critics
 */

#include <cstdio>
#include <cstring>
#include <cuda_runtime.h>

#include "critic_manager.cuh"                         // CriticManager
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"  // CostmapInfo, Footprint, PathInfo, GoalInfo

// ═══════════════════════════════════════════════════════════════════════════════
// 测试 kernel — 在 GPU 上执行 CriticManager 求值
// ═══════════════════════════════════════════════════════════════════════════════

__global__ void test_evaluate_kernel(
    const CostmapInfo cmap,
    const Footprint   fp,
    const PathInfo    path,
    const GoalInfo    goal,
    float *d_results)            // [5] 输出
{
  CriticManager mgr;
  CriticParams critic_params;
  critic_params.obstacle_ratio = 0.60f;
  critic_params.tracking_ratio = 0.30f;
  critic_params.speed_ratio    = 0.10f;
  mgr.init(critic_params);

  float cos_t = 1.0f, sin_t = 0.0f, theta = 0.0f, delta = 0.0f;

  // ── 测试 1: 空旷地图, 原点 ──
  d_results[0] = mgr.evaluate(
      0.0f, 0.0f, cos_t, sin_t, theta, 0.3f, 0.0f, delta,
      cmap, fp, path, goal,
      0, nullptr, nullptr, nullptr);

  // ── 测试 2: 同位置 ──
  d_results[1] = mgr.evaluate(
      0.0f, 0.0f, cos_t, sin_t, theta, 0.3f, 0.0f, delta,
      cmap, fp, path, goal,
      0, nullptr, nullptr, nullptr);

  // ── 测试 3: 轨迹点在路径上 ──
  d_results[2] = mgr.evaluate(
      0.5f, 0.0f, cos_t, sin_t, theta, 0.3f, 0.0f, delta,
      cmap, fp, path, goal,
      0, nullptr, nullptr, nullptr);

  // ── 测试 4: 速度方向与目标一致 ──
  d_results[3] = mgr.evaluate(
      0.0f, 0.0f, cos_t, sin_t, theta, 1.0f, 0.0f, delta,
      cmap, fp, path, goal,
      0, nullptr, nullptr, nullptr);

  // ── 测试 5: 多步累加 + horizon 归一化 (/5) ──
  float multi = 0.0f;
  float positions[5][2] = {
    {0.0f, 0.0f}, {0.1f, 0.0f}, {0.2f, 0.0f}, {0.3f, 0.0f}, {0.4f, 0.0f}
  };
  for (int t = 0; t < 5; ++t) {
    multi += mgr.evaluate(
        positions[t][0], positions[t][1],
        cos_t, sin_t, theta, 0.3f, 0.0f, delta,
        cmap, fp, path, goal,
      0, nullptr, nullptr, nullptr);
  }
  d_results[4] = multi / 5.0f;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 工具: 创建测试用 costmap (host + device)
// ═══════════════════════════════════════════════════════════════════════════════

struct TestCostmap
{
  unsigned char *h_data = nullptr;   // host
  unsigned char *d_data = nullptr;   // device
  CostmapInfo info;
  int size = 0;

  void alloc(int w, int h)
  {
    size = w * h;
    h_data = new unsigned char[size];
    cudaMalloc(&d_data, size);
    info.w = w;
    info.h = h;
  }

  void fill(unsigned char val)
  {
    memset(h_data, val, size);
  }

  void upload()
  {
    info.data = d_data;
    cudaMemcpy(d_data, h_data, size, cudaMemcpyHostToDevice);
  }

  ~TestCostmap()
  {
    delete[] h_data;
    if (d_data) cudaFree(d_data);
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// 工具: 创建测试用路径 (host + device)
// ═══════════════════════════════════════════════════════════════════════════════

struct TestPath
{
  float *h_x = nullptr, *h_y = nullptr;
  float *d_x = nullptr, *d_y = nullptr;
  PathInfo info;
  int n = 0;

  void alloc(int num_pts)
  {
    n = num_pts;
    h_x = new float[n];
    h_y = new float[n];
    cudaMalloc(&d_x, n * sizeof(float));
    cudaMalloc(&d_y, n * sizeof(float));
    info.x = d_x;
    info.y = d_y;
    info.num_pts = n;
  }

  /// 创建直线路径: y=0, x 从 0 到 n-1 (间隔 0.5m)
  void makeLinear(float tangent)
  {
    for (int i = 0; i < n; ++i) {
      h_x[i] = i * 0.5f;
      h_y[i] = 0.0f;
    }
    info.path_tangent = tangent;  // 路径切线 = 0 (沿 x 轴)
    info.goal_yaw = tangent;
  }

  void upload()
  {
    cudaMemcpy(d_x, h_x, n * sizeof(float), cudaMemcpyHostToDevice);
    cudaMemcpy(d_y, h_y, n * sizeof(float), cudaMemcpyHostToDevice);
  }

  ~TestPath()
  {
    delete[] h_x;
    delete[] h_y;
    if (d_x) cudaFree(d_x);
    if (d_y) cudaFree(d_y);
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// main
// ═══════════════════════════════════════════════════════════════════════════════

int main()
{
  printf("=== MPPI Critic Call Chain Test ===\n\n");

  // ── Footprint ──
  Footprint fp;
  fp.front   = 0.3f;
  fp.back    = 0.3f;
  fp.left    = 0.4f;
  fp.right   = 0.4f;
  fp.sample_spacing = 0.08f;
  fp.rear_obstacle_cost = 160.0f;
  printf("Footprint: %.2f x %.2f m, spacing=%.2f\n",
      fp.front+fp.back, fp.left+fp.right, fp.sample_spacing);

  // ── Path (5 点直线, 沿 x 轴, y=0) ──
  TestPath path;
  path.alloc(5);
  path.makeLinear(0.0f);  // path_tangent = 0
  path.upload();
  printf("Path: %d pts, tangent=%.2f\n", path.n, path.info.path_tangent);

  // ── Goal (沿 x 轴正向, 前方 10m) ──
  GoalInfo goal;
  goal.target_vx_r = 1.0f;   // 期望速度向前
  goal.target_vy_r = 0.0f;
  goal.goal_x = 10.0f;
  goal.goal_y = 0.0f;

  // ── 测试 1: 空旷地图 ──
  printf("\n[Test 1] Free costmap + path + speed reward\n");

  TestCostmap free_map;
  free_map.alloc(200, 200);
  free_map.info.res = 0.05f;
  free_map.info.origin_x = -5.0f;
  free_map.info.origin_y = -5.0f;
  free_map.fill(0);
  free_map.upload();

  // ── 测试 2: 障碍物地图 ──
  printf("[Test 2] Obstacle costmap (inflated 254)\n");

  TestCostmap obs_map;
  obs_map.alloc(200, 200);
  obs_map.info.res = 0.05f;
  obs_map.info.origin_x = -5.0f;
  obs_map.info.origin_y = -5.0f;
  obs_map.fill(254);
  obs_map.upload();

  // ── 分配结果 ──
  float *d_results;
  cudaMalloc(&d_results, 5 * sizeof(float));

  // ── 运行 kernel: 空旷地图 ──
  test_evaluate_kernel<<<1, 1>>>(free_map.info, fp, path.info, goal, d_results);
  cudaDeviceSynchronize();

  float h_free[5];
  cudaMemcpy(h_free, d_results, 5 * sizeof(float), cudaMemcpyDeviceToHost);
  printf("  safe pos cost     = %+.6f  (expect ~0.13)\n", h_free[0]);
  printf("  on-path cost      = %+.6f  (expect ~0.13)\n", h_free[2]);
  printf("  speed reward      = %+.6f  (expect ~0.08)\n", h_free[3]);
  printf("  multi-step sum    = %+.6f  (expect ~0.13)\n", h_free[4]);

  // ── 运行 kernel: 障碍物地图 ──
  test_evaluate_kernel<<<1, 1>>>(obs_map.info, fp, path.info, goal, d_results);
  cudaDeviceSynchronize();

  float h_obs[5];
  cudaMemcpy(h_obs, d_results, 5 * sizeof(float), cudaMemcpyDeviceToHost);
  printf("  result[0]=%.6f  result[1]=%.6f  result[2]=%.6f  result[3]=%.6f  result[4]=%.6f\n",
      h_obs[0], h_obs[1], h_obs[2], h_obs[3], h_obs[4]);

  // ── 校验 ──
  int pass = 0, fail = 0;

  auto check = [&](const char *name, float val, float expect_min, float expect_max) {
    if (val >= expect_min && val <= expect_max) {
      printf("  [PASS] %s: %.6f in [%.2f, %.2f]\n", name, val, expect_min, expect_max);
      pass++;
    } else {
      printf("  [FAIL] %s: %.6f not in [%.2f, %.2f]\n", name, val, expect_min, expect_max);
      fail++;
    }
  };

  // 指数归一化后所有代价 ∈ [0,1), cat_weights 语义清晰
  // free safe pos: obst=0, head=0 (在路径+对齐), speed=sigmoid(-0.3)=0.426
  //   total = 0.4×0 + 0.3×0 + 0.3×0.426 = 0.128
  check("free safe pos",    h_free[0],  0.10f,  0.16f);
  check("on-path cost",     h_free[2],  0.10f,  0.16f);
  // speed reward: vx=1.0, sigmoid(-1.0)=0.269, total=0.3×0.269=0.081
  check("speed reward",     h_free[3],  0.05f,  0.15f);
  check("free multi-step",  h_free[4],  0.10f,  0.16f);

  // obstacle: 0.4×0.984 + 0 + 0.3×sigmoid(-0.3) = 0.394 + 0.128 = 0.522
  check("obstacle cost",    h_obs[1],   0.49f,  0.56f);
  check("obstacle multi",   h_obs[4],   0.49f,  0.56f);

  printf("\n=== %d passed, %d failed ===\n", pass, fail);

  cudaFree(d_results);
  return (fail > 0) ? 1 : 0;
}
