/**
 * @file hybrid_a_star_planner.hpp
 * @brief HybridAStarPlanner — Nav2 全局规划器插件入口
 *
 * 继承 nav2_core::GlobalPlanner, 实现 lifecycle + createPlan.
 * 内部委托给 HybridAStar 核心算法.
 */

#ifndef NAV2_HYBRID_A_STAR_PLANNER__HYBRID_A_STAR_PLANNER_HPP_
#define NAV2_HYBRID_A_STAR_PLANNER__HYBRID_A_STAR_PLANNER_HPP_

#include <memory>
#include <string>

#include "nav2_core/global_planner.hpp"
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_costmap_2d/costmap_2d_ros.hpp"

#include "nav2_hybrid_a_star_planner/planner_types.hpp"
#include "nav2_hybrid_a_star_planner/hybrid_a_star.hpp"

namespace nav2_hybrid_a_star_planner
{

class HybridAStarPlanner : public nav2_core::GlobalPlanner
{
public:
  HybridAStarPlanner() = default;
  ~HybridAStarPlanner() override = default;

  // ═══════════════════════════════════════════════════════════════════════════
  // lifecycle
  // ═══════════════════════════════════════════════════════════════════════════

  void configure(
      const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
      std::string name,
      std::shared_ptr<tf2_ros::Buffer> tf,
      std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros) override;

  void cleanup() override;
  void activate() override;
  void deactivate() override;

  // ═══════════════════════════════════════════════════════════════════════════
  // 规划
  // ═══════════════════════════════════════════════════════════════════════════

  /**
   * @brief 创建从 start 到 goal 的全局路径
   * @param start 起点 (costmap 全局坐标系)
   * @param goal  终点
   * @return 规划路径 (空 path 表示失败)
   */
  nav_msgs::msg::Path createPlan(
      const geometry_msgs::msg::PoseStamped & start,
      const geometry_msgs::msg::PoseStamped & goal) override;

private:
  // ═══════════════════════════════════════════════════════════════════════════
  // 参数加载
  // ═══════════════════════════════════════════════════════════════════════════

  /// 从节点参数声明并加载 PlannerParams
  void loadParams(const rclcpp::node_interfaces::NodeParametersInterface::SharedPtr & node_params);

  // ═══════════════════════════════════════════════════════════════════════════
  // 数据成员
  // ═══════════════════════════════════════════════════════════════════════════

  rclcpp_lifecycle::LifecycleNode::WeakPtr node_;
  std::shared_ptr<tf2_ros::Buffer> tf_;
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros_;
  std::string plugin_name_;
  std::string global_frame_;

  // costmap 原始指针 (生命周期由 costmap_ros_ 管理)
  nav2_costmap_2d::Costmap2D * costmap_ = nullptr;

  // 参数
  PlannerParams params_;

  // 核心算法
  HybridAStar hybrid_a_star_;

  // 时钟 + 日志
  rclcpp::Clock::SharedPtr clock_;
  rclcpp::Logger logger_{rclcpp::get_logger("HybridAStarPlanner")};

  // 动态参数回调句柄
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr
      dyn_params_handler_;
};

}  // namespace nav2_hybrid_a_star_planner

#endif  // NAV2_HYBRID_A_STAR_PLANNER__HYBRID_A_STAR_PLANNER_HPP_
