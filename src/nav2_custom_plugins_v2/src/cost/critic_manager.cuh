/**
 * @file critic_manager.cuh
 * @brief CriticManager — 3 大分类容器直调, 大类间加权求和
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 调用关系 (kernel 视角)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   cost_eval_kernel()
 *     ├── for t in 0..H-1:
 *     │     dist_to_final = hypot(goal_x - x, goal_y - y)
 *     │     total += mgr.evaluate(x, y, θ, vx, vy, cmap, fp, path, dist_to_final, goal)
 *     │       │
 *     │       ├── cat_w[OBSTACLE] × obstacle_.evaluate(x, y, cos, sin, vx, vy, cmap, fp)
 *     │       │     → FootprintCritic: 足迹碰撞检测 (n⁴ 惩罚)
 *     │       │
 *     │       ├── cat_w[HEADING]  × heading_.evaluate(x, y, θ, path, dist_to_final)
 *     │       │     → PathAlignCritic:     点到路径最近距离
 *     │       │     → PathAngleCritic:     朝向对齐 (180° 对称 + 余弦退火)
 *     │       │     → PathDeviationCritic: 走廊偏离软墙
 *     │       │
 *     │       └── cat_w[SPEED]    × speed_.evaluate(vx, vy, goal)
 *     │             → SpeedRewardCritic: 速度方向对齐前瞻点
 *     │
 *     └── (loop 后) terminal_dist_cost = cat_w_speed × sqrt((gx-x)²+(gy-y)²)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 两层归一化
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   层 1 — 大类内加权和: 各 Category 容器内部 Σ (w × fn), 不平均
 *   层 2 — 大类间加权: cat_weights[] 乘大类结果
 *
 *   total = Σ cat_weights[c] × category.evaluate(...)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 新增代价子类的影响范围
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   例如给 HEADING 加新子类:
 *     1. 写子类 class NewCritic : public HeadingCritic { compute() }
 *     2. 写 wrapper 函数 (HeadingCategory 内静态方法)
 *     3. init() 注册: subs_[N] = { newFn, true, weight }
 *   → CriticManager 不需要修改
 */

#ifndef MPPI_CRITIC_MANAGER_CUH_
#define MPPI_CRITIC_MANAGER_CUH_

#include "critic_common.cuh"     // CriticCategory
#include "obstacle_critic.cuh"   // ObstacleCritic, FootprintCritic, ObstacleCategory
#include "heading_critic.cuh"    // HeadingCritic, PathAlignCritic, PathAngleCritic, ...
#include "speed_critic.cuh"      // SpeedCritic, SpeedRewardCritic, SpeedCategory

// ═══════════════════════════════════════════════════════════════════════════════
// CriticManager
// ═══════════════════════════════════════════════════════════════════════════════
//
// 持有 3 个分类容器 (值成员), evaluate() 直调它们。
// 每 CUDA 线程栈上独立构造, 无共享状态, evaluate() 是 const → 线程安全。
//
// evaluate() 参数很多 — 因为三个大类各自需要不同的数据:
//   OBSTACLE → cos_t, sin_t, cmap, fp
//   HEADING  → theta, path, dist_to_final
//   SPEED    → vx, vy, goal
//
// 但这正是 "直调" 的代价 — 没有虚函数或 switch 中转, 调用关系一目了然。

class CriticManager
{
public:
  /// @param cp 代价参数 (大类+子类权重, 从 CPU 端 MPPIParams 映射后传入)
  __host__ __device__ void init(const CriticParams &cp)
  {
    obstacle_.init();
    heading_ .init();
    speed_   .init();
    cat_weights_[0] = cp.obstacle_ratio;
    cat_weights_[1] = cp.tracking_ratio;
    cat_weights_[2] = cp.speed_ratio;

    // 子类权重 — 大类内的各子代价相对重要性
    obstacle_.setWeight(0, cp.footprint_weight);        // FootprintCritic
    heading_ .setWeight(0, cp.path_align_weight);       // PathAlignCritic
    heading_ .setWeight(1, cp.path_angle_weight);       // PathAngleCritic
    heading_ .setWeight(2, cp.path_deviation_weight);   // PathDeviationCritic
    speed_   .setWeight(0, cp.speed_reward_weight);     // SpeedRewardCritic
    speed_   .setWeight(1, cp.base_similarity_weight);  // BaseSimilarityCritic
  }

  __host__ __device__ void setCategoryWeight(CriticCategory cat, float w)
  {
    int c = static_cast<int>(cat);
    if (c >= 0 && c < static_cast<int>(CriticCategory::NUM_CATEGORIES))
      cat_weights_[c] = w;
  }

  // ── Per-step evaluate: kernel 内每一步调用 ──

  /// @brief 评估单步代价 — 三大类直调, 加权求和
  ///
  /// @param x, y, cos_t, sin_t, theta  轨迹点位姿 (cos_t/sin_t 预计算, 省三角函数)
  /// @param vx, vy, omega              当前步控制量
  /// @param cmap                       代价地图
  /// @param fp                         足迹参数
  /// @param path                       全局路径数据 (HEADING 大类使用)
  /// @param goal                       目标/速度参考 (SPEED 大类使用)
  /// @param t                          当前时间步 (0..H-1)
  /// @param base_vx, base_vy, base_omega  warm-start 基序列指针 [H] (SPEED 大类 BaseSimilarityCritic 使用)
  /// @return                           加权总代价
  __device__ float evaluate(
      float x, float y, float cos_t, float sin_t, float theta,
      float vx, float vy, float omega,
      const CostmapInfo &cmap, const Footprint &fp,
      const PathInfo &path, const GoalInfo &goal,
      int t, const float *base_vx,
      const float *base_vy, const float *base_omega) const
  {
    // OBSTACLE: 检测碰撞
    float obst = obstacle_.evaluate(x, y, cos_t, sin_t, vx, vy, cmap, fp);

    // HEADING:  路径对准 + 朝向对齐 (delta) + 走廊偏离
    float head = heading_.evaluate(x, y, theta, omega, path);

    // SPEED:    速度方向对齐 + warm-start 一致性约束
    float spd  = speed_.evaluate(vx, vy, omega, goal, t, base_vx, base_vy, base_omega);

    // 大类间加权求和 — 直接明了, 无任何中转
    return cat_weights_[0] * obst + cat_weights_[1] * head + cat_weights_[2] * spd;
  }

private:
  ObstacleCategory obstacle_;  ///< OBSTACLE 分类容器
  HeadingCategory  heading_;   ///< HEADING  分类容器
  SpeedCategory    speed_;     ///< SPEED    分类容器

  /// 大类间权重 — 控制三大代价的相对重要性
  /// 默认值仅作 fallback, 实际由 init() 覆盖为 {0.60, 0.30, 0.10}
  float cat_weights_[static_cast<int>(CriticCategory::NUM_CATEGORIES)] = {0.4f, 0.3f, 0.3f};
};

#endif  // MPPI_CRITIC_MANAGER_CUH_
