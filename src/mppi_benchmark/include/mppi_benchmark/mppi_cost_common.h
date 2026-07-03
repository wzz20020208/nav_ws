/**
 * @file mppi_cost_common.h
 * @brief GPU/CPU 共用的代价计算工具函数与常量
 */

#ifndef MPPI_COST_COMMON_H_
#define MPPI_COST_COMMON_H_

#include <cmath>
#include <cfloat>
#include <algorithm>

#ifndef M_PI_F
#define M_PI_F 3.14159265358979323846f
#endif

#define GOAL_ANGLE_THRESHOLD  0.5f
#define PATH_ANGLE_THRESHOLD  0.262f
#define HEADING_ANNEAL_DIST   1.0f

/// 角度归一化到 [-π, π]
inline float normalize_angle(float angle)
{
  while (angle > M_PI_F)  angle -= 2.0f * M_PI_F;
  while (angle < -M_PI_F) angle += 2.0f * M_PI_F;
  return angle;
}

/// 点到线段的最短距离平方
inline float point_to_segment_dist_sq(
    float px, float py,
    float ax, float ay,
    float bx, float by)
{
  float abx = bx - ax, aby = by - ay;
  float len_sq = abx * abx + aby * aby;
  if (len_sq < 1e-9f) {
    float dx = px - ax, dy = py - ay;
    return dx * dx + dy * dy;
  }
  float apx = px - ax, apy = py - ay;
  float t = (apx * abx + apy * aby) / len_sq;
  if (t < 0.0f) t = 0.0f;
  if (t > 1.0f) t = 1.0f;
  float cx = ax + t * abx, cy = ay + t * aby;
  float dx = px - cx, dy = py - cy;
  return dx * dx + dy * dy;
}

/// 代价地图双线性插值 (CPU 版)
inline float costmap_bilinear_cpu(
    float wx, float wy,
    const unsigned char* costmap,
    int w, int h, float res, float ox, float oy)
{
  float mx = (wx - ox) / res - 0.5f;
  float my = (wy - oy) / res - 0.5f;
  int x0 = static_cast<int>(floorf(mx));
  int y0 = static_cast<int>(floorf(my));
  int x1 = x0 + 1, y1 = y0 + 1;
  if (x0 < 0 || x1 >= w || y0 < 0 || y1 >= h) return 255.0f;
  float dx = mx - static_cast<float>(x0);
  float dy = my - static_cast<float>(y0);
  float v00 = static_cast<float>(costmap[y0 * w + x0]);
  float v10 = static_cast<float>(costmap[y0 * w + x1]);
  float v01 = static_cast<float>(costmap[y1 * w + x0]);
  float v11 = static_cast<float>(costmap[y1 * w + x1]);
  return (1.0f - dx) * (1.0f - dy) * v00
       +         dx  * (1.0f - dy) * v10
       + (1.0f - dx) *         dy  * v01
       +         dx  *         dy  * v11;
}

#endif  // MPPI_COST_COMMON_H_
