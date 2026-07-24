/**
 * @file heading_critic.cuh
 * @brief HEADING 分类 — 路径对准 + 朝向偏差 + 走廊偏离
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 三层角色 (同 obstacle 模式)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   HeadingCritic : CriticBase           ← 分类基类: kCategory=HEADING
 *     ↑ 继承
 *   PathAlignCritic    : HeadingCritic   ← 轨迹点到路径最近距离
 *   PathAngleCritic    : HeadingCritic   ← 朝向与路径切线偏差 (180° 对称 + 终点退火)
 *   PathDeviationCritic: HeadingCritic   ← 路径走廊外软墙惩罚
 *
 *   HeadingCategory                      ← 容器: 注册表 + evaluate()
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 算法参考
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   PathAlignCritic:
 *     遍历所有路径段 → 点到线段最短距离平方 → 返回 min_sq
 *
 *   PathAngleCritic:
 *     if dist_to_final < GOAL_ANGLE_THRESHOLD (0.5m):
 *       余弦退火: target = path_tangent + α*(goal_yaw - path_tangent)
 *       α = (1+cos(π·t))/2, t = clamp(dist/anneal_dist, 0, 1)
 *     else:
 *       target = path_tangent
 *     err = sym_angle_diff(theta, target)   ← 180° 对称, 选 ≤90° 旋转
 *     return 4.0 * err²
 *
 *   PathDeviationCritic:
 *     遍历路径 → min_sq → dist = sqrt(min_sq)
 *     if dist > corridor: return weight * (dist - corridor)²
 *     else: return 0
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 调用链
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   CriticManager::evaluate(..., path, dist_to_final)
 *     → cat_w[HEADING] × heading_.evaluate(x, y, theta, path, dist_to_final)
 *         → for i in subs_:
 *             subs_[i].fn(x, y, theta, path, dist_to_final)  ← 函数指针直调
 *           return Σ(w × fn) / active_count
 */

#ifndef MPPI_HEADING_CRITIC_CUH_
#define MPPI_HEADING_CRITIC_CUH_

#include "mppi_gpu_common.cuh"                        // point_to_segment_dist_sq, normalize_angle
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"  // PathInfo
#include "critic_common.cuh"                          // CriticBase, CriticCategory

// ═══════════════════════════════════════════════════════════════════════════════
// HeadingCritic — HEADING 分类基类
// ═══════════════════════════════════════════════════════════════════════════════

class HeadingCritic : public CriticBase
{
public:
  /// 归属大类 — CriticManager 用它做第二大层加权
  static constexpr CriticCategory kCategory = CriticCategory::HEADING;
};

// ═══════════════════════════════════════════════════════════════════════════════
// PathAlignCritic — 轨迹点到全局路径的最短距离
// ═══════════════════════════════════════════════════════════════════════════════
//
// 遍历路径所有线段 → 点到线段最短距离 → 指数映射到 [0,1)
//   dist = sqrt(min_sq),  cost = 1 - exp(-dist²)
//   0m→0, 0.5m→0.22, 1m→0.63, 2m→0.98
//
// 完全无状态 — compute() 所有数据从参数传入, 临时构造零开销。

class PathAlignCritic : public HeadingCritic
{
public:
  __device__ float compute(
      float x, float y, float theta, float omega, const PathInfo &path) const
  {
    (void)theta; (void)omega;

    if (path.num_pts < 2 || path.x == nullptr || path.y == nullptr) return 0.0f;

    float min_sq = 1e10f;
    for (int p = 0; p < path.num_pts - 1; ++p) {
      float d = point_to_segment_dist_sq(
          x, y, path.x[p], path.y[p], path.x[p + 1], path.y[p + 1]);
      if (d < min_sq) min_sq = d;
    }

    return min_sq;  // 未归一化: 距离平方 (m²), 0=在路径上
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// PathAngleCritic — 当前朝向 + 目标朝向 vs 推荐朝向
// ═══════════════════════════════════════════════════════════════════════════════
//
//   err_now = normalize_angle(θ - path_tangent)
//   err_cmd = normalize_angle(δ - path_tangent)
//   返回 err_now² + err_cmd² (rad²), 未归一化

class PathAngleCritic : public HeadingCritic
{
public:
  __device__ float compute(
      float x, float y, float theta, float omega, const PathInfo &path) const
  {
    (void)x; (void)y;

    float err = normalize_angle(theta - path.path_tangent);  // 非对称, 区分前后
    return 4.0f * err * err;
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// PathDeviationCritic — 路径走廊偏离软墙
// ═══════════════════════════════════════════════════════════════════════════════
//
//   在路径两侧 corridor 半宽内不惩罚
//   excess = max(0, dist - corridor), 返回 excess² (m²), 未归一化

class PathDeviationCritic : public HeadingCritic
{
public:
  __device__ float compute(
      float x, float y, float theta, float omega, const PathInfo &path) const
  {
    (void)theta; (void)omega;

    if (path.num_pts < 2 || path.x == nullptr || path.y == nullptr) return 0.0f;

    float min_sq = 1e10f;
    for (int p = 0; p < path.num_pts - 1; ++p) {
      float d = point_to_segment_dist_sq(
          x, y, path.x[p], path.y[p], path.x[p + 1], path.y[p + 1]);
      if (d < min_sq) min_sq = d;
    }

    constexpr float corridor = 0.5f;
    float excess = sqrtf(min_sq) - corridor;
    if (excess <= 0.0f) return 0.0f;

    return excess * excess;
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// HeadingCategory — HEADING 分类容器
// ═══════════════════════════════════════════════════════════════════════════════
//
// 管理 HEADING 大类下的 3 个子代价函数, 通过函数指针注册表统一调度。
// 模式与 ObstacleCategory 完全一致: 注册表 → 循环 → 加权平均。

class HeadingCategory
{
public:
  // ═════════════════════════════════════════════════════════════════════════
  // SubFn — 子代价函数签名
  // ═════════════════════════════════════════════════════════════════════════
  //
  // 参数集覆盖所有 HEADING 子类的需求:
  //   x, y   — 世界坐标 (PathAlign, PathDeviation 使用)
  //   theta  — 当前朝向 (PathAngle 使用)
  //   path   — 路径数据 (所有子类使用)

  typedef float (*SubFn)(float x, float y, float theta, float omega,
                         const PathInfo &path);

  /// 注册表条目: { 函数指针, 开关, 大类内权重 }
  struct SubEntry
  {
    SubFn fn;         ///< 函数指针 → 直调 compute()
    bool  enabled;    ///< false 时 evaluate() 跳过
    float weight;     ///< 大类内权重系数
  };

  // ═════════════════════════════════════════════════════════════════════════
  // 静态包装函数 — 每个子类一个
  // ═════════════════════════════════════════════════════════════════════════

  __device__ static float pathAlignFn(
      float x, float y, float theta, float omega, const PathInfo &path)
  {
    PathAlignCritic c;
    return c.compute(x, y, theta, omega, path);
  }

  __device__ static float pathAngleFn(
      float x, float y, float theta, float omega, const PathInfo &path)
  {
    PathAngleCritic c;
    return c.compute(x, y, theta, omega, path);
  }

  __device__ static float pathDeviationFn(
      float x, float y, float theta, float omega, const PathInfo &path)
  {
    PathDeviationCritic c;
    return c.compute(x, y, theta, omega, path);
  }

  // ═════════════════════════════════════════════════════════════════════════
  // 注册表操作
  // ═════════════════════════════════════════════════════════════════════════

  __host__ __device__ void init()
  {
    // 默认注册 3 个子类, 均启用
    subs_[0] = { pathAlignFn,      true, 1.0f };   // 路径对准
    subs_[1] = { pathAngleFn,      true, 2.0f };   // 朝向对齐 (区分前后)
    subs_[2] = { pathDeviationFn,  true, 0.3f };   // 走廊偏离 (软墙, 辅助)
    count_ = 3;
  }

  __host__ __device__ void setWeight(int idx, float w)
  {
    if (idx >= 0 && idx < count_) subs_[idx].weight = w;
  }

  __host__ __device__ void setEnabled(int idx, bool on)
  {
    if (idx >= 0 && idx < count_) subs_[idx].enabled = on;
  }

  // ═════════════════════════════════════════════════════════════════════════
  // evaluate — HEADING 大类求值
  // ═════════════════════════════════════════════════════════════════════════
  //
  /// 遍历注册表 → 函数指针直调 → 加权和
  /// @return HEADING 大类归一化代价 (类内平均后)
  __device__ float evaluate(
      float x, float y, float theta, float omega, const PathInfo &path) const
  {
    float total = 0.0f;
    int active = 0;

    for (int i = 0; i < count_; ++i) {
      const SubEntry &e = subs_[i];
      if (!e.enabled) continue;
      total += e.weight * e.fn(x, y, theta, omega, path);
      active++;
    }

    return total;  // THEMIS: 加权和, 不平均
  }

private:
  static constexpr int kMaxSubs = 4;
  SubEntry subs_[kMaxSubs];
  int count_ = 0;
};

#endif  // MPPI_HEADING_CRITIC_CUH_
