#ifndef MPPI_GPU_COMMON_CUH_
#define MPPI_GPU_COMMON_CUH_

/**
 * @file mppi_gpu_common.cuh
 * @brief MPPI GPU 公共工具函数与常量
 *
 * 本文件提供 MPPI GPU 内核中复用的基础数学工具函数，包括：
 *   - 点到线段的最近距离计算
 *   - 代价地图双线性插值
 *   - 角度归一化
 *
 * 所有函数均为 __device__ 修饰，可在 GPU 内核中直接调用。
 */

#include <cfloat>
#include <cmath>

// ═══════════════════════════════════════════════════════════════════════════
// 数学常量
// ═══════════════════════════════════════════════════════════════════════════

#ifndef M_PI_F
/** @brief 单精度 π，用于角度计算 */
#define M_PI_F 3.14159265358979323846f
#endif

/** @brief 2π，角度归一化周期 */
#define TWO_PI_F (2.0f * M_PI_F)

/** @brief π/2，180° 对称分界 */
#define M_PI_2_F (M_PI_F / 2.0f)

// ═══════════════════════════════════════════════════════════════════════════
// 工具函数
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 将角度归一化到 [-π, π] 区间
 *
 * 公式: θ_norm = θ - 2π · round(θ / 2π)
 *
 * @param  angle  待归一化的角度 (rad)，可为任意值
 * @return        归一化后的角度，∈ [-π, π]
 */
__device__ inline float normalize_angle(float angle)
{
  while (angle > M_PI_F)  angle -= TWO_PI_F;
  while (angle < -M_PI_F) angle += TWO_PI_F;
  return angle;
}

/// 180° 对称角度差: 框体 θ 与 θ+π 等价, 选 ≤90° 的最短旋转
__device__ inline float sym_angle_diff(float a, float b)
{
  float d = normalize_angle(a - b);
  if (d > M_PI_2_F)       d -= M_PI_F;
  else if (d < -M_PI_2_F) d += M_PI_F;
  return d;
}

/**
 * @brief 计算点到线段的最短距离平方
 *
 * 用于计算轨迹点偏离全局路径的 cross-track error 平方。
 *
 * 数学推导:
 *   设线段 AB，点 P。
 *   投影参数 t = ((P-A)·(B-A)) / |B-A|²
 *   将 t 钳制到 [0, 1]，则最近点 Q = A + clamp(t,0,1) · (B-A)
 *   最短距离平方 = |P - Q|²
 *
 * 退化情况:
 *   当 |B-A|² < ε 时，线段退化为点，直接返回 |P-A|²
 *
 * @param  px, py  待求点 P 的坐标 (世界坐标系, m)
 * @param  ax, ay  线段起点 A 的坐标 (世界坐标系, m)
 * @param  bx, by  线段终点 B 的坐标 (世界坐标系, m)
 * @return         P 到线段 AB 的最短距离平方 (m²)
 */
__device__ float point_to_segment_dist_sq(
    float px, float py,
    float ax, float ay,
    float bx, float by)
{
  // ── 线段向量 AB 和 向量 AP ──
  float abx = bx - ax;
  float aby = by - ay;
  float apx = px - ax;
  float apy = py - ay;

  // ── 线段长度的平方 ──
  float ab_len_sq = abx * abx + aby * aby;

  // ── 退化检查: 线段退化为点 ──
  const float EPS = 1e-8f;
  if (ab_len_sq < EPS) {
    return apx * apx + apy * apy;
  }

  // ── 投影参数 t = ((P-A)·(B-A)) / |B-A|² ──
  // clamp 到 [0, 1] 确保最近点在线段上而非延长线上
  float t = (apx * abx + apy * aby) / ab_len_sq;
  t = fmaxf(0.0f, fminf(1.0f, t));

  // ── 最近点 Q = A + t · (B - A) ──
  float closest_x = ax + t * abx;
  float closest_y = ay + t * aby;

  // ── 距离平方 ──
  float dx = px - closest_x;
  float dy = py - closest_y;
  return dx * dx + dy * dy;
}

/**
 * @brief 代价地图双线性插值 — 消除栅格边界代价跳变
 *
 * 将离散的栅格代价图转换为连续函数，使 MPPI 代价梯度平滑。
 *
 * 坐标映射:
 *   cell(i,j) 的中心位于世界坐标:
 *     x = (i + 0.5) · resolution + origin_x
 *     y = (j + 0.5) · resolution + origin_y
 *
 * 因此世界坐标 → 连续栅格坐标:
 *     fx = (wx - origin_x) / resolution - 0.5
 *     fy = (wy - origin_y) / resolution - 0.5
 *
 * 双线性插值公式:
 *     c(p) = lerp( lerp(c00, c10, αx), lerp(c01, c11, αx), αy )
 *
 *   其中:
 *     c00 = costmap[floor(fy)][floor(fx)]  (左下)
 *     c10 = costmap[floor(fy)][ceil(fx)]   (右下)
 *     c01 = costmap[ceil(fy)][floor(fx)]   (左上)
 *     c11 = costmap[ceil(fy)][ceil(fx)]    (右上)
 *     αx  = fx - floor(fx)                  (x方向小数部分)
 *     αy  = fy - floor(fy)                  (y方向小数部分)
 *
 * 边界处理:
 *   越界邻居值取 255（未知/致命），与原始 nav2 行为一致
 *
 * @param  wx, wy              查询点的世界坐标 (m)
 * @param  costmap             代价地图数据指针 (row-major, unsigned char [0,255])
 * @param  costmap_w           代价地图宽度 (栅格数)
 * @param  costmap_h           代价地图高度 (栅格数)
 * @param  costmap_res         代价地图分辨率 (m/cell)
 * @param  costmap_origin_x    代价地图原点 x 坐标 (左下角, m)
 * @param  costmap_origin_y    代价地图原点 y 坐标 (左下角, m)
 * @return                     插值后的连续代价 ∈ [0, 255]
 */
__device__ float costmap_bilinear(
    float wx, float wy,
    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y)
{
  // ── 世界坐标 → 连续栅格坐标 (cell 中心在 (i+0.5)*res + origin) ──
  float fx = (wx - costmap_origin_x) / costmap_res - 0.5f;
  float fy = (wy - costmap_origin_y) / costmap_res - 0.5f;

  // ── 四个邻居栅格索引 (未钳制) ──
  int mx0_raw = static_cast<int>(floorf(fx));
  int my0_raw = static_cast<int>(floorf(fy));
  int mx1_raw = mx0_raw + 1;
  int my1_raw = my0_raw + 1;

  // ── 插值权重 (小数部分, 基于未钳制索引) ──
  float wx_frac = fx - static_cast<float>(mx0_raw);
  float wy_frac = fy - static_cast<float>(my0_raw);

  // ── 钳制到代价地图有效范围，越界点取边界栅格值 ──
  int mx0 = max(0, min(mx0_raw, costmap_w - 1));
  int my0 = max(0, min(my0_raw, costmap_h - 1));
  int mx1 = max(0, min(mx1_raw, costmap_w - 1));
  int my1 = max(0, min(my1_raw, costmap_h - 1));

  // ── 读取四个邻居值 ──
  float c00 = static_cast<float>(costmap[my0 * costmap_w + mx0]);
  float c10 = static_cast<float>(costmap[my0 * costmap_w + mx1]);
  float c01 = static_cast<float>(costmap[my1 * costmap_w + mx0]);
  float c11 = static_cast<float>(costmap[my1 * costmap_w + mx1]);

  // ── 双线性插值: 先沿 x 方向插值，再沿 y 方向 ──
  // c0 = lerp(c00, c10, wx_frac)   ← 下边插值
  // c1 = lerp(c01, c11, wx_frac)   ← 上边插值
  // c  = lerp(c0,  c1,  wy_frac)  ← 纵向插值
  float c0 = c00 + (c10 - c00) * wx_frac;
  float c1 = c01 + (c11 - c01) * wx_frac;
  return c0 + (c1 - c0) * wy_frac;
}

#endif  // MPPI_GPU_COMMON_CUH_
