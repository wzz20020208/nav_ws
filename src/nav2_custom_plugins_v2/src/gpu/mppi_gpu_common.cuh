/**
 * @file mppi_gpu_common.cuh
 * @brief GPU 端公共工具 — 角度归一化、costmap 双线性插值、几何计算
 *
 * 所有函数均为 __device__, 在 GPU kernel 中直接调用。
 * 本文件不包含 ROS 依赖, 纯 CUDA + 数学库。
 */

#ifndef MPPI_GPU_COMMON_CUH_
#define MPPI_GPU_COMMON_CUH_

// ═══════════════════════════════════════════════════════════════════════════
// 常量
// ═══════════════════════════════════════════════════════════════════════════

#define M_PI_F   3.14159265358979323846f
#define TWO_PI_F (2.0f * M_PI_F)
#define M_PI_2_F (M_PI_F / 2.0f)

// ═══════════════════════════════════════════════════════════════════════════
// 角度工具
// ═══════════════════════════════════════════════════════════════════════════

__device__ inline float normalize_angle(float angle)
{
  while (angle > M_PI_F)  angle -= TWO_PI_F;
  while (angle < -M_PI_F) angle += TWO_PI_F;
  return angle;
}

__device__ inline float sym_angle_diff(float a, float b)
{
  float d = normalize_angle(a - b);
  if (d > M_PI_2_F)       d -= M_PI_F;
  else if (d < -M_PI_2_F) d += M_PI_F;
  return d;
}

// ═══════════════════════════════════════════════════════════════════════════
// costmap 双线性插值
// ═══════════════════════════════════════════════════════════════════════════

/// 世界坐标 (wx, wy) → costmap 栅格, 四点双线性插值 → [0, 255]
/// 越界点取边界栅格值 (边缘钳位)
__device__ inline float costmap_bilinear(
    float wx, float wy,
    const unsigned char *__restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y)
{
  float fx = (wx - costmap_origin_x) / costmap_res - 0.5f;
  float fy = (wy - costmap_origin_y) / costmap_res - 0.5f;

  int mx0_raw = static_cast<int>(floorf(fx));
  int my0_raw = static_cast<int>(floorf(fy));
  int mx1_raw = mx0_raw + 1;
  int my1_raw = my0_raw + 1;

  float wx_frac = fx - static_cast<float>(mx0_raw);
  float wy_frac = fy - static_cast<float>(my0_raw);

  int mx0 = max(0, min(mx0_raw, costmap_w - 1));
  int my0 = max(0, min(my0_raw, costmap_h - 1));
  int mx1 = max(0, min(mx1_raw, costmap_w - 1));
  int my1 = max(0, min(my1_raw, costmap_h - 1));

  float c00 = static_cast<float>(costmap[my0 * costmap_w + mx0]);
  float c10 = static_cast<float>(costmap[my0 * costmap_w + mx1]);
  float c01 = static_cast<float>(costmap[my1 * costmap_w + mx0]);
  float c11 = static_cast<float>(costmap[my1 * costmap_w + mx1]);

  float c0 = c00 + (c10 - c00) * wx_frac;
  float c1 = c01 + (c11 - c01) * wx_frac;
  return c0 + (c1 - c0) * wy_frac;
}

// ═══════════════════════════════════════════════════════════════════════════
// 几何工具
// ═══════════════════════════════════════════════════════════════════════════

__device__ inline float point_to_segment_dist_sq(
    float px, float py,
    float ax, float ay,
    float bx, float by)
{
  float abx = bx - ax, aby = by - ay;
  float apx = px - ax, apy = py - ay;
  float ab_len_sq = abx * abx + aby * aby;

  if (ab_len_sq < 1e-8f) return apx * apx + apy * apy;

  float t = (apx * abx + apy * aby) / ab_len_sq;
  t = fmaxf(0.0f, fminf(1.0f, t));

  float cx = ax + t * abx, cy = ay + t * aby;
  float dx = px - cx, dy = py - cy;
  return dx * dx + dy * dy;
}

#endif  // MPPI_GPU_COMMON_CUH_
