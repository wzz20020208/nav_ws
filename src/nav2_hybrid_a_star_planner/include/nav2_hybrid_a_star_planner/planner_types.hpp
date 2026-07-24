/**
 * @file planner_types.hpp
 * @brief PlannerTypes — Hybrid-A* 共享类型、参数与常量
 */

#ifndef NAV2_HYBRID_A_STAR_PLANNER__PLANNER_TYPES_HPP_
#define NAV2_HYBRID_A_STAR_PLANNER__PLANNER_TYPES_HPP_

#include <vector>
#include <cmath>
#include <cstdint>

namespace nav2_hybrid_a_star_planner
{

// ═══════════════════════════════════════════════════════════════════════════════
// 基础几何类型
// ═══════════════════════════════════════════════════════════════════════════════

/// SE(2) 状态: 连续世界坐标 + 连续朝向
struct SE2State
{
  double x = 0.0;
  double y = 0.0;
  double theta = 0.0;  ///< heading angle (rad), [-π, π)
};

/// 2D 网格坐标 (costmap cell)
struct GridCoord
{
  int x = 0;
  int y = 0;
};

// ═══════════════════════════════════════════════════════════════════════════════
// 运动原语
// ═══════════════════════════════════════════════════════════════════════════════

/// 恒定曲率弧线运动原语
struct MotionPrimitive
{
  double curvature = 0.0;       ///< 曲率 κ = 1/R (0=直线, >0=左转, <0=右转)
  double arc_length = 0.4;      ///< 弧长 (m)
  double delta_theta = 0.0;     ///< 朝向变化量 = curvature * arc_length
  double cost_multiplier = 1.0; ///< 代价倍率 (后退 >1.0, 转向 >1.0)
  bool is_reverse = false;      ///< 是否后退
};

// ═══════════════════════════════════════════════════════════════════════════════
// Reeds-Shepp 路径段类型
// ═══════════════════════════════════════════════════════════════════════════════

/// 路径段方向
enum class SteerDir : uint8_t
{
  LEFT = 0,
  RIGHT = 1,
  STRAIGHT = 2
};

/// 运动方向
enum class GearDir : uint8_t
{
  FORWARD = 0,
  BACKWARD = 1
};

/// 一个 RS 路径段
struct RSSegment
{
  double length = 0.0;
  SteerDir steer = SteerDir::STRAIGHT;
  GearDir gear = GearDir::FORWARD;
};

/// RS 路径 = 若干段
struct RSPath
{
  std::vector<RSSegment> segments;
  double total_length = 0.0;
  bool valid = false;
};

// ═══════════════════════════════════════════════════════════════════════════════
// Hybrid-A* 搜索节点
// ═══════════════════════════════════════════════════════════════════════════════

/// A* 搜索节点 (用于 priority queue)
struct HybridNode
{
  SE2State state;
  double g_cost = 0.0;     ///< 从起点到当前节点的实际代价
  double h_cost = 0.0;     ///< 启发式: 当前节点到终点的估计代价

  double fCost() const { return g_cost + h_cost; }

  int parent_idx = -1;     ///< 父节点在 closed_list 中的索引, -1=root
  int prim_idx = -1;       ///< 展开时使用的运动原语索引
};

/// priority_queue 比较器: 最小 f_cost 优先
struct NodeComparator
{
  bool operator()(const HybridNode & a, const HybridNode & b) const
  {
    return a.fCost() > b.fCost();  // std::priority_queue 是最大堆, 反转比较
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// 参数 (YAML → configure 时读入)
// ═══════════════════════════════════════════════════════════════════════════════

struct PlannerParams
{
  // 搜索
  int max_iterations = 50000;          ///< A* 最大迭代次数
  double goal_tolerance = 0.3;         ///< 到达目标位置容差 (m)
  double goal_angle_tolerance = 0.17;  ///< 到达目标角度容差 (rad)

  // 运动学
  double turning_radius = 0.5;         ///< 最小转弯半径 (m)
  double max_curvature = 2.0;          ///< 最大曲率 = 1/turning_radius
  double arc_length = 0.4;             ///< 运动原语弧长 (m)

  // 原语
  int num_curvature_levels = 1;        ///< 曲率层级 (1=仅max, 2=max+half_max)
  double reverse_cost_multiplier = 2.0;///< 后退代价倍率
  double steering_cost_multiplier = 1.2; ///< 转向代价倍率 (vs 直行)

  // 状态离散化
  int heading_bins = 72;               ///< 角度离散化仓数 (72=5°)

  // 分析扩张
  int analytic_expansion_interval = 10; ///< 每 N 次扩展尝试 RS 直连

  // 碰撞检测
  double robot_length = 0.34;          ///< 机器人长度 (m), front+back
  double robot_width = 0.56;           ///< 机器人宽度 (m)
  double collision_sample_spacing = 0.08; ///< 足迹采样间距 (m)

  // costmap
  bool allow_unknown = false;          ///< 是否允许经过未知区域

  // 路径后处理
  bool enable_path_simplification = true; ///< 是否简化路径
};

// ═══════════════════════════════════════════════════════════════════════════════
// costmap 常量 (与 nav2_costmap_2d 对齐)
// ═══════════════════════════════════════════════════════════════════════════════

constexpr unsigned char FREE_SPACE = 0;
constexpr unsigned char LETHAL_OBSTACLE = 254;
constexpr unsigned char NO_INFORMATION = 255;

// ═══════════════════════════════════════════════════════════════════════════════
// 工具函数
// ═══════════════════════════════════════════════════════════════════════════════

/// 角度标准化到 [-π, π)
inline double normalizeAngle(double theta)
{
  while (theta >= M_PI) theta -= 2.0 * M_PI;
  while (theta < -M_PI) theta += 2.0 * M_PI;
  return theta;
}

/// 角度差 (最短方向)
inline double angleDiff(double a, double b)
{
  return normalizeAngle(a - b);
}

/// 两点距离
inline double distance(double x1, double y1, double x2, double y2)
{
  double dx = x1 - x2;
  double dy = y1 - y2;
  return std::sqrt(dx * dx + dy * dy);
}

}  // namespace nav2_hybrid_a_star_planner

#endif  // NAV2_HYBRID_A_STAR_PLANNER__PLANNER_TYPES_HPP_
