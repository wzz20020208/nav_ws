/**
 * @file mppi_cost_gpu.cuh
 * @brief GPU 版 MPPI 代价计算 — 提取自 nav2_custom_plugins
 *
 * 三组件归一化架构:
 *   total = cost_scale × (obs_ratio × obst_acc/horizon
 *                        + trk_ratio × track_acc/horizon
 *                        + spd_ratio × prog_acc/horizon)
 *
 * 本文件包含 __device__ 函数和 __global__ kernel 定义,
 * 供 mppi_cost_gpu.cu 编译。
 */

#ifndef MPPI_COST_GPU_CUH_
#define MPPI_COST_GPU_CUH_

#include <cuda_runtime.h>
#include <cfloat>
#include <cmath>

#ifndef M_PI_F
#define M_PI_F 3.14159265358979323846f
#endif

#define GOAL_ANGLE_THRESHOLD  0.5f
#define PATH_ANGLE_THRESHOLD  0.262f
#define HEADING_ANNEAL_DIST   1.0f

// ════════════════════════════════════════════
// GPU 工具函数
// ════════════════════════════════════════════

__device__ inline float gpu_normalize_angle(float angle)
{
  while (angle > M_PI_F)  angle -= 2.0f * M_PI_F;
  while (angle < -M_PI_F) angle += 2.0f * M_PI_F;
  return angle;
}

/// 180° 对称角度差: 框体 θ 与 θ+π 等价, 选 ≤90° 的最短旋转
__device__ inline float gpu_sym_angle_diff(float a, float b)
{
  float d = gpu_normalize_angle(a - b);
  if (d > M_PI_F / 2.0f)       d -= M_PI_F;
  else if (d < -M_PI_F / 2.0f) d += M_PI_F;
  return d;
}

__device__ float gpu_point_to_segment_dist_sq(
    float px, float py, float ax, float ay, float bx, float by)
{
  float abx = bx - ax, aby = by - ay;
  float len_sq = abx * abx + aby * aby;
  if (len_sq < 1e-9f) {
    float dx = px - ax, dy = py - ay;
    return dx * dx + dy * dy;
  }
  float apx = px - ax, apy = py - ay;
  float t = (apx * abx + apy * aby) / len_sq;
  if (t < 0.0f) t = 0.0f; if (t > 1.0f) t = 1.0f;
  float cx = ax + t * abx, cy = ay + t * aby;
  float dx = px - cx, dy = py - cy;
  return dx * dx + dy * dy;
}

__device__ float gpu_costmap_bilinear(
    float wx, float wy, const unsigned char* __restrict__ cm,
    int w, int h, float res, float ox, float oy)
{
  float mx = (wx - ox) / res - 0.5f, my = (wy - oy) / res - 0.5f;
  int x0 = static_cast<int>(floorf(mx)), y0 = static_cast<int>(floorf(my));
  int x1 = x0 + 1, y1 = y0 + 1;
  if (x0 < 0 || x1 >= w || y0 < 0 || y1 >= h) return 255.0f;
  float dx = mx - static_cast<float>(x0), dy = my - static_cast<float>(y0);
  float v00 = static_cast<float>(cm[y0 * w + x0]);
  float v10 = static_cast<float>(cm[y0 * w + x1]);
  float v01 = static_cast<float>(cm[y1 * w + x0]);
  float v11 = static_cast<float>(cm[y1 * w + x1]);
  return (1.0f-dx)*(1.0f-dy)*v00 + dx*(1.0f-dy)*v10
       + (1.0f-dx)*dy*v01 + dx*dy*v11;
}

// ════════════════════════════════════════════
// 组件 1: 障碍物代价
// ════════════════════════════════════════════

__device__ float compute_obstacle_cost_gpu(
    float x, float y, float cos_theta, float sin_theta,
    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float fp_sample_spacing, float rear_obstacle_cost, float vx)
{
  if (costmap == nullptr || costmap_w <= 0 || costmap_h <= 0) return 0.0f;
  float flx = fp_front + fp_back, fly = fp_left + fp_right;
  int nx = static_cast<int>(ceilf(flx / fp_sample_spacing)) + 1;
  int ny = static_cast<int>(ceilf(fly / fp_sample_spacing)) + 1;
  nx = (nx < 2) ? 2 : ((nx > 10) ? 10 : nx);
  ny = (ny < 2) ? 2 : ((ny > 10) ? 10 : ny);
  float sx = (nx > 1) ? flx / (nx - 1) : 0.0f;
  float sy = (ny > 1) ? fly / (ny - 1) : 0.0f;
  float acc = 0.0f;
  for (int iy = 0; iy < ny; ++iy) {
    float ly = -fp_right + iy * sy;
    for (int ix = 0; ix < nx; ++ix) {
      float lx = -fp_back + ix * sx;
      float wx = x + lx * cos_theta - ly * sin_theta;
      float wy = y + lx * sin_theta + ly * cos_theta;
      float val = gpu_costmap_bilinear(wx, wy, costmap, costmap_w, costmap_h,
                                       costmap_res, costmap_origin_x, costmap_origin_y);
      if (lx < 0.0f && vx < 0.0f && rear_obstacle_cost > 0.0f)
        val = fmaxf(val, rear_obstacle_cost);
      if (val >= 1.0f) { float n = val / 255.0f; acc += n * n * n * n; }
    }
  }
  return acc / static_cast<float>(nx * ny);
}

// ════════════════════════════════════════════
// 组件 2: 跟踪代价
// ════════════════════════════════════════════

/// 注意: 不再内部除以 horizon, 统一在代价组合处做 per-step 归一化
__device__ float compute_path_align_cost_gpu(
    float x, float y,
    const float* __restrict__ path_x, const float* __restrict__ path_y,
    int num_path_pts, int horizon)
{
  float min_sq = FLT_MAX;
  if (num_path_pts < 2 || path_x == nullptr) return 0.0f;
  for (int p = 0; p < num_path_pts - 1; ++p) {
    float d = gpu_point_to_segment_dist_sq(x, y, path_x[p], path_y[p],
                                           path_x[p+1], path_y[p+1]);
    if (d < min_sq) min_sq = d;
  }
  (void)horizon;  // 保留参数兼容性, 归一化移至代价组合
  return min_sq;
}

/// 注意: 不再内部除以 horizon, 统一在代价组合处做 per-step 归一化
__device__ float compute_path_angle_cost_gpu(
    float theta, float path_tangent, float goal_yaw,
    float dist_to_final, int horizon)
{
  (void)horizon;  // 保留参数兼容性, 归一化移至代价组合
  // GoalAngleCritic 区域: 余弦退火 + 180° 对称
  if (dist_to_final < GOAL_ANGLE_THRESHOLD) {
    float t = fminf(1.0f, dist_to_final / HEADING_ANNEAL_DIST);
    float alpha = (1.0f + cosf(M_PI_F * t)) * 0.5f;
    float diff = gpu_sym_angle_diff(goal_yaw, path_tangent);
    float target = path_tangent + alpha * diff;
    float err = gpu_sym_angle_diff(theta, target);
    return 4.0f * err * err;
  }
  // PathAngleCritic 区域: 180° 对称 — 连续惩罚, 角度越大代价越高
  float err = gpu_sym_angle_diff(theta, path_tangent);
  return 4.0f * err * err;
}

// ════════════════════════════════════════════
// 组件 3: 进度代价
// ════════════════════════════════════════════

__device__ float compute_speed_reward_gpu(
    float vx, float vy, float target_vx_r, float target_vy_r)
{
  float speed = hypotf(vx, vy);
  if (speed < 1e-6f) return 0.0f;
  float vel_angle = atan2f(vy, vx);
  float target_angle = atan2f(target_vy_r, target_vx_r);
  float angle_err = vel_angle - target_angle;
  while (angle_err > M_PI_F)  angle_err -= 2.0f * M_PI_F;
  while (angle_err < -M_PI_F) angle_err += 2.0f * M_PI_F;
  float alignment = cosf(angle_err);
  // 背离前瞻点 (>90°): 5倍重罚, 防止反向
  if (alignment < 0.0f) return speed * 5.0f;
  return -speed * alignment + 2.0f * speed * fabsf(sinf(angle_err));
}

__device__ float compute_terminal_dist_cost_gpu(float x, float y, float gx, float gy)
{
  float dx = gx - x, dy = gy - y;
  return sqrtf(dx * dx + dy * dy);
}

// ════════════════════════════════════════════
// GPU 内核
// ════════════════════════════════════════════

__global__ void mppi_cost_benchmark_kernel(
    const float* __restrict__ vx_seq,
    const float* __restrict__ vy_seq,
    const float* __restrict__ omega_seq,
    float start_x, float start_y, float start_theta,
    float target_x, float target_y,
    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float dt, int horizon,
    float cost_scale, float obstacle_ratio, float tracking_ratio, float speed_ratio,
    float path_vx_r, float path_vy_r,
    float path_tangent, float goal_yaw,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float fp_sample_spacing, float rear_obstacle_cost,
    const float* __restrict__ path_x, const float* __restrict__ path_y,
    int num_path_pts,
    float final_goal_x, float final_goal_y,
    int num_samples,
    float* __restrict__ costs)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  if (s >= num_samples) return;

  float x = start_x, y = start_y, theta = start_theta;
  float obst_acc = 0.0f, track_acc = 0.0f, prog_acc = 0.0f;

  for (int t = 0; t < horizon; ++t) {
    int idx = s * horizon + t;
    float vx = vx_seq[idx], vy = vy_seq[idx], omega = omega_seq[idx];

    float half_dt = 0.5f * dt;
    float theta_mid = theta + omega * half_dt;
    x += (vx * cosf(theta_mid) - vy * sinf(theta_mid)) * dt;
    y += (vx * sinf(theta_mid) + vy * cosf(theta_mid)) * dt;
    theta += omega * dt;

    obst_acc += compute_obstacle_cost_gpu(x, y, cosf(theta), sinf(theta),
        costmap, costmap_w, costmap_h, costmap_res, costmap_origin_x, costmap_origin_y,
        fp_front, fp_back, fp_left, fp_right, fp_sample_spacing, rear_obstacle_cost, vx);

    float dist_to_final = sqrtf((final_goal_x - x) * (final_goal_x - x)
                              + (final_goal_y - y) * (final_goal_y - y));
    track_acc += compute_path_align_cost_gpu(x, y, path_x, path_y, num_path_pts, horizon)
               + compute_path_angle_cost_gpu(theta, path_tangent, goal_yaw,
                                             dist_to_final, horizon);

    prog_acc += compute_speed_reward_gpu(vx, vy, path_vx_r, path_vy_r);
  }

  prog_acc += compute_terminal_dist_cost_gpu(x, y, final_goal_x, final_goal_y);

  // ── 归一化代价: per-step 均值 × 占比权重 × 全局缩放 ──
  float inv_h = 1.0f / static_cast<float>(horizon);
  costs[s] = cost_scale * (obstacle_ratio * obst_acc * inv_h
                         + tracking_ratio * track_acc * inv_h
                         + speed_ratio     * prog_acc * inv_h);
}

#endif  // MPPI_COST_GPU_CUH_
