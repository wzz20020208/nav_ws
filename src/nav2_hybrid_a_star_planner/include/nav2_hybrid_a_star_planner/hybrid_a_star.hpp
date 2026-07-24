/**
 * @file hybrid_a_star.hpp
 * @brief HybridAStar — 连续状态空间 Hybrid-A* 搜索算法
 *
 * 核心流程:
 *   ① 2D Dijkstra 障碍物距离启发式 (从 goal 反向传播)
 *   ② A* 主循环: 运动原语展开 + 分析扩张
 *   ③ 路径提取 + 可见性简化 + 朝向赋值
 */

#ifndef NAV2_HYBRID_A_STAR_PLANNER__HYBRID_A_STAR_HPP_
#define NAV2_HYBRID_A_STAR_PLANNER__HYBRID_A_STAR_HPP_

#include <vector>
#include <queue>
#include <memory>
#include <cmath>

#include "nav2_hybrid_a_star_planner/planner_types.hpp"

// forward declare costmap types (avoid dependency on nav2_costmap_2d in header)
namespace nav2_costmap_2d
{
class Costmap2D;
}

namespace nav2_hybrid_a_star_planner
{

class HybridAStar
{
public:
  HybridAStar();
  ~HybridAStar();

  // ═══════════════════════════════════════════════════════════════════════════
  // 初始化
  // ═══════════════════════════════════════════════════════════════════════════

  /// 根据 costmap 尺寸分配内部缓冲区
  void initialize(const PlannerParams & params, unsigned int size_x,
                  unsigned int size_y, double resolution,
                  double origin_x, double origin_y);

  // ═══════════════════════════════════════════════════════════════════════════
  // 主入口
  // ═══════════════════════════════════════════════════════════════════════════

  /**
   * @brief 创建规划路径
   * @param start 起点 (世界坐标)
   * @param goal  终点 (世界坐标)
   * @param costmap 代价地图 (调用前已锁定)
   * @param plan 输出路径 (世界坐标)
   * @return true 如果找到路径
   */
  bool createPlan(const SE2State & start, const SE2State & goal,
                  const nav2_costmap_2d::Costmap2D * costmap,
                  std::vector<SE2State> & plan);

  // ═══════════════════════════════════════════════════════════════════════════
  // 查询
  // ═══════════════════════════════════════════════════════════════════════════

  const PlannerParams & params() const { return params_; }

private:
  // ═══════════════════════════════════════════════════════════════════════════
  // 启发式
  // ═══════════════════════════════════════════════════════════════════════════

  /// 2D Dijkstra: 从 goal grid 出发向全图传播, 计算每个 cell 的无障碍最短距离
  void computeObstacleHeuristic(const GridCoord & goal_cell,
                                const nav2_costmap_2d::Costmap2D * costmap);

  /// 查询某世界坐标的障碍物距离启发式值
  double getObstacleHeuristic(double wx, double wy) const;

  /// 非完整启发式: Reeds-Shepp 无碰撞最短路径长度
  double getNonHolonomicHeuristic(const SE2State & from, const SE2State & to) const;

  // ═══════════════════════════════════════════════════════════════════════════
  // 运动原语
  // ═══════════════════════════════════════════════════════════════════════════

  /// 根据 turning_radius + arc_length 生成运动原语集合
  void generateMotionPrimitives();

  /// 运动学积分: 从 state 沿 primitive 推进一步
  SE2State expand(const SE2State & state, const MotionPrimitive & prim) const;

  // ═══════════════════════════════════════════════════════════════════════════
  // 碰撞检测
  // ═══════════════════════════════════════════════════════════════════════════

  /// 检测状态是否有效 (边界 + 碰撞)
  bool isStateValid(const SE2State & state,
                    const nav2_costmap_2d::Costmap2D * costmap) const;

  /// 检测整条圆弧是否无碰撞 (采样间距 = costmap resolution)
  bool isArcCollisionFree(const SE2State & from, const MotionPrimitive & prim,
                          const nav2_costmap_2d::Costmap2D * costmap) const;

  /// 足迹碰撞检测: 在世界坐标采样外包矩形点, 逐一查 costmap
  bool isFootprintCollisionFree(const SE2State & state,
                                const nav2_costmap_2d::Costmap2D * costmap) const;

  /// 检测 RS 路径段是否无碰撞
  bool isRSPathCollisionFree(const std::vector<SE2State> & rs_states,
                             const nav2_costmap_2d::Costmap2D * costmap) const;

  // ═══════════════════════════════════════════════════════════════════════════
  // Reeds-Shepp
  // ═══════════════════════════════════════════════════════════════════════════

  /// 计算从 from 到 to 的 Reeds-Shepp 最短路径
  RSPath reedsShepp(const SE2State & from, const SE2State & to) const;

  /// 将 RS 路径采样为离散 SE2State 序列
  std::vector<SE2State> sampleRSPath(const SE2State & from,
                                     const RSPath & rs_path,
                                     double sample_spacing) const;

  // ═══════════════════════════════════════════════════════════════════════════
  // 路径后处理
  // ═══════════════════════════════════════════════════════════════════════════

  /// 从 closed_list_ 回溯重建原始路径
  std::vector<SE2State> reconstructPath(int goal_idx) const;

  /// 贪心可见性路径简化: 移除中间冗余点
  std::vector<SE2State> simplifyPath(
      const std::vector<SE2State> & raw_path,
      const nav2_costmap_2d::Costmap2D * costmap) const;

  /// 从路径点切线方向计算朝向
  void assignOrientations(std::vector<SE2State> & path) const;

  // ═══════════════════════════════════════════════════════════════════════════
  // 坐标变换
  // ═══════════════════════════════════════════════════════════════════════════

  GridCoord worldToGrid(double wx, double wy) const;
  void gridToWorld(int mx, int my, double & wx, double & wy) const;

  /// 获取 (mx, my) 在 visited_ 数组中的索引
  int visitedIndex(GridCoord cell, int heading_bin) const;
  int headingBin(double theta) const;

  // ═══════════════════════════════════════════════════════════════════════════
  // 数据成员
  // ═══════════════════════════════════════════════════════════════════════════

  PlannerParams params_;

  // 代价地图元数据
  unsigned int map_size_x_ = 0;
  unsigned int map_size_y_ = 0;
  double map_resolution_ = 0.05;
  double map_origin_x_ = 0.0;
  double map_origin_y_ = 0.0;

  // 障碍物距离启发式 (2D float 数组, 大小 = map_size_x_ × map_size_y_)
  std::vector<float> obstacle_heuristic_;

  // 运动原语
  std::vector<MotionPrimitive> primitives_;

  // 搜索状态
  std::vector<HybridNode> closed_list_;     ///< 所有展开过的节点
  std::vector<uint8_t> visited_;            ///< 3D 去重标记 (flat: cell + heading_bin)
};

}  // namespace nav2_hybrid_a_star_planner

#endif  // NAV2_HYBRID_A_STAR_PLANNER__HYBRID_A_STAR_HPP_
