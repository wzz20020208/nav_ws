/**
 * @file critic_common.cuh
 * @brief MPPI 代价函数基础设施 — 大类枚举 + 顶级基类
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 架构
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   CriticManager 持有 3 个分类实例, 直接调各自的 evaluate():
 *
 *     total  = cat_w[OBSTACLE] × obstacle_.evaluate(x,y,θ,vx,vy,cmap,fp)
 *            + cat_w[HEADING]  × heading_.evaluate(x,y,θ,vx,vy,cmap,fp)
 *            + cat_w[SPEED]    × speed_.evaluate(x,y,θ,vx,vy,cmap,fp)
 *
 *   每个分类内部管理自己的子类 (FootprintCritic 等),
 *   子类继承自分类基类, 只需实现 compute()。
 *
 *   类层级:
 *
 *     CriticBase                 ← 本文件: 顶级标签基类
 *     ├── ObstacleCritic         ← obstacle_critic.cuh: 分类基类 + evaluate()
 *     │   └── FootprintCritic    ← 具体子类: compute()
 *     ├── HeadingCritic          ← heading_critic.cuh
 *     └── SpeedCritic            ← speed_critic.cuh
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 新增代价子类的步骤 (以给 OBSTACLE 加 DistanceFieldCritic 为例)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   1. 写子类: class DistanceFieldCritic : public ObstacleCritic { compute() }
 *   2. 在 ObstacleCritic 加成员 + evaluate() 内加一行调用
 *   → CriticManager 不需要任何修改
 */

#ifndef MPPI_CRITIC_COMMON_CUH_
#define MPPI_CRITIC_COMMON_CUH_

// ═══════════════════════════════════════════════════════════════════════════════
// CriticCategory — 代价大类
// ═══════════════════════════════════════════════════════════════════════════════
//
// 三个大类按领域语义划分, 各类内部管理自己的子代价函数。
// CriticManager 只关心大类间的权重分配。

enum class CriticCategory : int
{
  OBSTACLE = 0,   ///< 障碍物: 碰撞检测, 软墙偏离
  HEADING  = 1,   ///< 朝向:   路径对准, 角度偏差
  SPEED    = 2,   ///< 速度:   期望速度, 终端距离
  NUM_CATEGORIES = 3
};

/// 大类 → 可读名称 (调试/日志用)
__device__ inline const char *categoryName(CriticCategory c)
{
  switch (c) {
    case CriticCategory::OBSTACLE: return "obstacle";
    case CriticCategory::HEADING:  return "heading";
    case CriticCategory::SPEED:    return "speed";
    default:                       return "?";
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// CriticBase — 所有代价类的顶级基类
// ═══════════════════════════════════════════════════════════════════════════════
//
// 标签基类: 无虚函数, 无虚表, GPU 友好。
// 分类基类 (ObstacleCritic 等) 继承此类并声明 kCategory。
// 具体子类 (FootprintCritic 等) 继承分类基类并实现 compute()。

class CriticBase
{
public:
  /// 分类基类必须定义: static constexpr CriticCategory kCategory = ...;
};

#endif  // MPPI_CRITIC_COMMON_CUH_
