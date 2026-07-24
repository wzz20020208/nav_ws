/**
 * @file hybrid_a_star_planner.cpp
 * @brief HybridAStarPlanner — Nav2 全局规划器插件实现
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * lifecycle
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   configure → activate → [createPlan × N] → deactivate → cleanup
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * createPlan 数据流
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ① 参数校验: start/goal 坐标在 costmap 内
 *   ② 世界 → 地图坐标转换
 *   ③ 锁 costmap (只在读取时持有锁)
 *   ④ hybrid_a_star_.createPlan(start, goal, costmap) → 规划
 *   ⑤ 填充 nav_msgs::msg::Path (frame_id = global_frame_, stamp = now)
 */

#include "nav2_hybrid_a_star_planner/hybrid_a_star_planner.hpp"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "pluginlib/class_list_macros.hpp"
#include "nav2_util/node_utils.hpp"
#include "nav2_costmap_2d/cost_values.hpp"

namespace nav2_hybrid_a_star_planner
{

// ═══════════════════════════════════════════════════════════════════════════════
// configure
// ═══════════════════════════════════════════════════════════════════════════════

void HybridAStarPlanner::configure(
    const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
    std::string name,
    std::shared_ptr<tf2_ros::Buffer> tf,
    std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros)
{
  node_ = parent;
  plugin_name_ = name;
  tf_ = tf;
  costmap_ros_ = costmap_ros;

  auto node = parent.lock();
  if (!node) {
    throw std::runtime_error("HybridAStarPlanner: node expired in configure()");
  }

  clock_ = node->get_clock();
  logger_ = node->get_logger();

  // costmap 访问
  costmap_ = costmap_ros_->getCostmap();
  global_frame_ = costmap_ros_->getGlobalFrameID();

  // 加载参数
  loadParams(node->get_node_parameters_interface());

  // 初始化算法
  hybrid_a_star_.initialize(
      params_,
      costmap_->getSizeInCellsX(),
      costmap_->getSizeInCellsY(),
      costmap_->getResolution(),
      costmap_->getOriginX(),
      costmap_->getOriginY());

  RCLCPP_INFO(logger_,
    "HybridAStarPlanner configured: map %dx%d @ %.3fm resolution, "
    "turning_radius=%.2fm, heading_bins=%d, max_iterations=%d",
    costmap_->getSizeInCellsX(), costmap_->getSizeInCellsY(),
    costmap_->getResolution(),
    params_.turning_radius, params_.heading_bins, params_.max_iterations);
}

// ═══════════════════════════════════════════════════════════════════════════════
// activate / deactivate / cleanup
// ═══════════════════════════════════════════════════════════════════════════════

void HybridAStarPlanner::activate()
{
  RCLCPP_INFO(logger_, "HybridAStarPlanner activated");
}

void HybridAStarPlanner::deactivate()
{
  RCLCPP_INFO(logger_, "HybridAStarPlanner deactivated");
}

void HybridAStarPlanner::cleanup()
{
  RCLCPP_INFO(logger_, "HybridAStarPlanner cleanup");
  costmap_ = nullptr;
}

// ═══════════════════════════════════════════════════════════════════════════════
// createPlan
// ═══════════════════════════════════════════════════════════════════════════════

nav_msgs::msg::Path HybridAStarPlanner::createPlan(
    const geometry_msgs::msg::PoseStamped & start,
    const geometry_msgs::msg::PoseStamped & goal)
{
  nav_msgs::msg::Path plan;
  plan.header.stamp = clock_->now();
  plan.header.frame_id = global_frame_;

  // ── ① frame 一致性检查 ──
  if (start.header.frame_id != global_frame_) {
    RCLCPP_ERROR(logger_,
      "start frame_id '%s' != global_frame '%s'",
      start.header.frame_id.c_str(), global_frame_.c_str());
    return plan;
  }
  if (goal.header.frame_id != global_frame_) {
    RCLCPP_ERROR(logger_,
      "goal frame_id '%s' != global_frame '%s'",
      goal.header.frame_id.c_str(), global_frame_.c_str());
    return plan;
  }

  // ── ② 获取 tf 中的 yaw ──
  double start_yaw = tf2::getYaw(start.pose.orientation);
  double goal_yaw = tf2::getYaw(goal.pose.orientation);

  // ── ③ costmap 尺寸变化检查, 自动重初始化 ──
  if (hybrid_a_star_.params().max_iterations > 0) {  // 已初始化
    if (static_cast<unsigned int>(costmap_->getSizeInCellsX()) != 0) {
      // 可在此做 size change 检测 (后续版本)
    }
  }

  // ── ④ 坐标合法性检查 ──
  unsigned int sx, sy, gx, gy;
  if (!costmap_->worldToMap(start.pose.position.x, start.pose.position.y,
                            sx, sy)) {
    RCLCPP_WARN(logger_, "start pose off costmap");
    return plan;
  }
  if (!costmap_->worldToMap(goal.pose.position.x, goal.pose.position.y,
                            gx, gy)) {
    RCLCPP_WARN(logger_, "goal pose off costmap");
    return plan;
  }

  // ── ⑤ 起点碰撞检查 ──
  unsigned char start_cost = costmap_->getCost(sx, sy);
  if (start_cost >= nav2_costmap_2d::LETHAL_OBSTACLE) {
    RCLCPP_WARN(logger_, "start pose in obstacle");
    return plan;
  }

  // ── ⑥ 构建 SE2State ──
  SE2State start_state;
  start_state.x = start.pose.position.x;
  start_state.y = start.pose.position.y;
  start_state.theta = start_yaw;

  SE2State goal_state;
  goal_state.x = goal.pose.position.x;
  goal_state.y = goal.pose.position.y;
  goal_state.theta = goal_yaw;

  // ── ⑦ 规划 ──
  std::vector<SE2State> se2_path;

  {
    // 锁定 costmap 只在内部数据拷贝期间
    std::unique_lock<nav2_costmap_2d::Costmap2D::mutex_t> lock(
        *(costmap_->getMutex()));

    if (!hybrid_a_star_.createPlan(start_state, goal_state, costmap_, se2_path)) {
      RCLCPP_WARN(logger_,
        "HybridAStarPlanner: failed to find path from (%.2f, %.2f) to (%.2f, %.2f)",
        start_state.x, start_state.y, goal_state.x, goal_state.y);
      lock.unlock();
      return plan;
    }
    lock.unlock();
  }

  // ── ⑧ 填充 nav_msgs::msg::Path ──
  plan.poses.reserve(se2_path.size());
  for (const auto & s : se2_path) {
    geometry_msgs::msg::PoseStamped pose;
    pose.header = plan.header;
    pose.pose.position.x = s.x;
    pose.pose.position.y = s.y;
    pose.pose.position.z = 0.0;

    // yaw → quaternion
    tf2::Quaternion q;
    q.setRPY(0.0, 0.0, s.theta);
    pose.pose.orientation = tf2::toMsg(q);

    plan.poses.push_back(pose);
  }

  RCLCPP_INFO(logger_,
    "HybridAStarPlanner: found path with %zu poses",
    plan.poses.size());

  return plan;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 参数加载
// ═══════════════════════════════════════════════════════════════════════════════

void HybridAStarPlanner::loadParams(
    const rclcpp::node_interfaces::NodeParametersInterface::SharedPtr & node_params)
{
  // 使用 nav2_util 提供的 declare_parameter_if_not_declared 模板函数
  // 每个参数前加 plugin_name_ 前缀

  auto declare_and_get_double = [&](const std::string & name, double & out, double default_val) {
    std::string full_name = plugin_name_ + "." + name;
    if (!node_params->has_parameter(full_name)) {
      node_params->declare_parameter(
          full_name, rclcpp::ParameterValue(default_val));
    }
    out = node_params->get_parameter(full_name).as_double();
  };

  auto declare_and_get_int = [&](const std::string & name, int & out, int default_val) {
    std::string full_name = plugin_name_ + "." + name;
    if (!node_params->has_parameter(full_name)) {
      node_params->declare_parameter(
          full_name, rclcpp::ParameterValue(default_val));
    }
    out = node_params->get_parameter(full_name).as_int();
  };

  auto declare_and_get_bool = [&](const std::string & name, bool & out, bool default_val) {
    std::string full_name = plugin_name_ + "." + name;
    if (!node_params->has_parameter(full_name)) {
      node_params->declare_parameter(
          full_name, rclcpp::ParameterValue(default_val));
    }
    out = node_params->get_parameter(full_name).as_bool();
  };

  // 搜索参数
  declare_and_get_int("max_iterations", params_.max_iterations, 50000);
  declare_and_get_double("goal_tolerance", params_.goal_tolerance, 0.3);
  declare_and_get_double("goal_angle_tolerance", params_.goal_angle_tolerance, 0.17);

  // 运动学参数
  declare_and_get_double("turning_radius", params_.turning_radius, 0.5);
  declare_and_get_double("arc_length", params_.arc_length, 0.4);
  declare_and_get_int("num_curvature_levels", params_.num_curvature_levels, 1);
  declare_and_get_double("reverse_cost_multiplier", params_.reverse_cost_multiplier, 2.0);
  declare_and_get_double("steering_cost_multiplier", params_.steering_cost_multiplier, 1.2);

  // 计算 max_curvature
  params_.max_curvature = 1.0 / params_.turning_radius;

  // 离散化参数
  declare_and_get_int("heading_bins", params_.heading_bins, 72);

  // 分析扩张参数
  declare_and_get_int("analytic_expansion_interval",
                      params_.analytic_expansion_interval, 10);

  // 碰撞检测参数
  declare_and_get_double("robot_length", params_.robot_length, 0.34);
  declare_and_get_double("robot_width", params_.robot_width, 0.56);
  declare_and_get_double("collision_sample_spacing",
                         params_.collision_sample_spacing, 0.08);

  // costmap 参数
  declare_and_get_bool("allow_unknown", params_.allow_unknown, false);

  // 后处理
  declare_and_get_bool("enable_path_simplification",
                       params_.enable_path_simplification, true);

  RCLCPP_INFO(logger_,
    "HybridAStarPlanner params: turning_radius=%.2f arc_length=%.2f "
    "heading_bins=%d max_curvature=%.3f",
    params_.turning_radius, params_.arc_length,
    params_.heading_bins, params_.max_curvature);
}

}  // namespace nav2_hybrid_a_star_planner

// ═══════════════════════════════════════════════════════════════════════════════
// Pluginlib 注册
// ═══════════════════════════════════════════════════════════════════════════════

PLUGINLIB_EXPORT_CLASS(
    nav2_hybrid_a_star_planner::HybridAStarPlanner,
    nav2_core::GlobalPlanner)
