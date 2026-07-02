/**
 * @file mppi_cost_cpu.hpp
 * @brief CPU 版 MPPI 代价计算 — 与 GPU 版逻辑逐位等价
 *
 * 用于与 GPU 版对比延迟, 验证加速比。
 */

#ifndef MPPI_COST_CPU_HPP_
#define MPPI_COST_CPU_HPP_

#include "mppi_benchmark/mppi_cost_common.h"
#include <vector>
#include <chrono>

namespace mppi_benchmark
{

/// 同 GPU 版 compute_obstacle_cost_gpu
inline float compute_obstacle_cost_cpu(
    float x, float y, float cos_theta, float sin_theta,
    const unsigned char* costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float fp_sample_spacing, float rear_obstacle_cost, float vx)
{
  if (costmap == nullptr || costmap_w <= 0 || costmap_h <= 0) return 0.0f;
  float flx = fp_front + fp_back, fly = fp_left + fp_right;
  int nx = static_cast<int>(std::ceil(flx / fp_sample_spacing)) + 1;
  int ny = static_cast<int>(std::ceil(fly / fp_sample_spacing)) + 1;
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
      float val = costmap_bilinear_cpu(wx, wy, costmap, costmap_w, costmap_h,
                                       costmap_res, costmap_origin_x, costmap_origin_y);
      if (lx < 0.0f && vx < 0.0f && rear_obstacle_cost > 0.0f)
        val = std::max(val, rear_obstacle_cost);
      if (val >= 1.0f) { float n = val / 255.0f; acc += n * n * n * n; }
    }
  }
  return acc / static_cast<float>(nx * ny);
}

/// 同 GPU 版 compute_path_align_cost_gpu
inline float compute_path_align_cost_cpu(
    float x, float y,
    const float* path_x, const float* path_y,
    int num_path_pts, int horizon)
{
  float min_sq = FLT_MAX;
  if (num_path_pts < 2 || path_x == nullptr) return 0.0f;
  for (int p = 0; p < num_path_pts - 1; ++p) {
    float d = point_to_segment_dist_sq(x, y, path_x[p], path_y[p],
                                       path_x[p+1], path_y[p+1]);
    if (d < min_sq) min_sq = d;
  }
  return min_sq / static_cast<float>(horizon);
}

/// 同 GPU 版 compute_path_angle_cost_gpu
inline float compute_path_angle_cost_cpu(
    float theta, float path_tangent, float goal_yaw,
    float dist_to_final, int horizon)
{
  if (dist_to_final < GOAL_ANGLE_THRESHOLD) {
    float t = std::min(1.0f, dist_to_final / HEADING_ANNEAL_DIST);
    float alpha = (1.0f + std::cos(M_PI_F * t)) * 0.5f;
    float diff = normalize_angle(goal_yaw - path_tangent);
    float target = path_tangent + alpha * diff;
    float err = normalize_angle(theta - target);
    return err * err / static_cast<float>(horizon);
  }
  float err = normalize_angle(theta - path_tangent);
  if (std::abs(err) > PATH_ANGLE_THRESHOLD)
    return err * err / static_cast<float>(horizon);
  return 0.0f;
}

/// 同 GPU 版 compute_speed_reward_gpu
inline float compute_speed_reward_cpu(
    float vx, float vy, float target_vx_r, float target_vy_r)
{
  float speed = std::hypot(vx, vy);
  if (speed < 1e-6f) return 0.0f;
  float vel_angle = std::atan2(vy, vx);
  float target_angle = std::atan2(target_vy_r, target_vx_r);
  float angle_err = vel_angle - target_angle;
  while (angle_err > M_PI_F)  angle_err -= 2.0f * M_PI_F;
  while (angle_err < -M_PI_F) angle_err += 2.0f * M_PI_F;
  float alignment = std::cos(angle_err);
  if (alignment < 0.0f) return speed * 5.0f;
  return -speed * alignment + speed * std::abs(std::sin(angle_err));
}

/// 同 GPU 版 compute_terminal_dist_cost_gpu
inline float compute_terminal_dist_cost_cpu(float x, float y, float gx, float gy)
{
  float dx = gx - x, dy = gy - y;
  return std::sqrt(dx * dx + dy * dy);
}

// ════════════════════════════════════════════
// CPU 版完整代价计算 (串行 N 条轨迹)
// ════════════════════════════════════════════

struct CPUBenchResult {
  double elapsed_ms;
  std::vector<float> costs;
};

inline CPUBenchResult run_cpu_benchmark(
    const float* vx_seq, const float* vy_seq, const float* omega_seq,
    int num_samples, int horizon,
    float start_x, float start_y, float start_theta,
    float target_x, float target_y,
    const unsigned char* costmap, int cm_w, int cm_h,
    float cm_res, float cm_ox, float cm_oy,
    float dt,
    float obs_w, float track_w, float prog_w,
    float path_vx_r, float path_vy_r,
    float path_tangent, float goal_yaw,
    float fp_f, float fp_b, float fp_l, float fp_r,
    float fp_sp, float rear_cost,
    const float* path_x, const float* path_y, int num_path,
    float fgx, float fgy)
{
  std::vector<float> costs(num_samples);

  auto t0 = std::chrono::high_resolution_clock::now();

#pragma omp parallel for
  for (int s = 0; s < num_samples; ++s) {
    float x = start_x, y = start_y, theta = start_theta;
    float obst_acc = 0.0f, track_acc = 0.0f, prog_acc = 0.0f;

    for (int t = 0; t < horizon; ++t) {
      int idx = s * horizon + t;
      float vx = vx_seq[idx], vy = vy_seq[idx], omega = omega_seq[idx];

      float half_dt = 0.5f * dt;
      float theta_mid = theta + omega * half_dt;
      x += (vx * std::cos(theta_mid) - vy * std::sin(theta_mid)) * dt;
      y += (vx * std::sin(theta_mid) + vy * std::cos(theta_mid)) * dt;
      theta += omega * dt;

      obst_acc += compute_obstacle_cost_cpu(x, y, std::cos(theta), std::sin(theta),
          costmap, cm_w, cm_h, cm_res, cm_ox, cm_oy,
          fp_f, fp_b, fp_l, fp_r, fp_sp, rear_cost, vx);

      float dist_to_final = std::sqrt((fgx - x) * (fgx - x) + (fgy - y) * (fgy - y));
      track_acc += compute_path_align_cost_cpu(x, y, path_x, path_y, num_path, horizon)
                 + compute_path_angle_cost_cpu(theta, path_tangent, goal_yaw,
                                               dist_to_final, horizon);

      prog_acc += compute_speed_reward_cpu(vx, vy, path_vx_r, path_vy_r);
    }

    prog_acc += compute_terminal_dist_cost_cpu(x, y, fgx, fgy);

    costs[s] = obs_w * obst_acc + track_w * track_acc + prog_w * prog_acc;
  }

  auto t1 = std::chrono::high_resolution_clock::now();
  double elapsed = std::chrono::duration<double, std::milli>(t1 - t0).count();

  return {elapsed, std::move(costs)};
}

}  // namespace mppi_benchmark

#endif  // MPPI_COST_CPU_HPP_
