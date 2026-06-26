// Copyright (c) 2025
// Adaptive Progress Checker — 振荡感知 + 诊断发布 + 距离自适应进度检测器

#include "nav2_custom_plugins/adaptive_progress_checker.hpp"
#include "pluginlib/class_list_macros.hpp"
#include <cmath>
#include <algorithm>

PLUGINLIB_EXPORT_CLASS(
  nav2_custom_plugins::AdaptiveProgressChecker,
  nav2_core::ProgressChecker)

namespace nav2_custom_plugins
{

void AdaptiveProgressChecker::initialize(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
  const std::string & plugin_name)
{
  plugin_name_ = plugin_name;
  auto node = parent.lock();
  clock_ = node->get_clock();

  // 父类参数 (SimpleProgressChecker)
  node->declare_parameter(plugin_name_ + ".required_movement_radius", 0.5);
  node->declare_parameter(plugin_name_ + ".movement_time_allowance", 10.0);

  // 自有参数
  node->declare_parameter(plugin_name_ + ".required_movement_angle", 0.5);
  node->declare_parameter(plugin_name_ + ".oscillation_window", 8);
  node->declare_parameter(plugin_name_ + ".oscillation_threshold", 4);
  node->declare_parameter(plugin_name_ + ".dist_adaptive_scale", 2.0);

  node->get_parameter(plugin_name_ + ".required_movement_radius", radius_);
  double time_allow_sec = 10.0;
  node->get_parameter(plugin_name_ + ".movement_time_allowance", time_allow_sec);
  time_allowance_ = rclcpp::Duration::from_seconds(time_allow_sec);
  node->get_parameter(plugin_name_ + ".required_movement_angle", required_movement_angle_);
  node->get_parameter(plugin_name_ + ".oscillation_window", oscillation_window_);
  node->get_parameter(plugin_name_ + ".oscillation_threshold", oscillation_threshold_);
  node->get_parameter(plugin_name_ + ".dist_adaptive_scale", dist_adaptive_scale_);

  // 诊断话题发布者 — 供 BT RecoveryNode 选择 recovery behavior
  diag_pub_ = node->create_publisher<std_msgs::msg::String>(
    "/progress_checker_diagnosis", 10);

  // 动态参数回调
  dyn_params_handler_ = node->add_on_set_parameters_callback(
    std::bind(&AdaptiveProgressChecker::dynamicParametersCallback, this,
              std::placeholders::_1));
}

void AdaptiveProgressChecker::reset()
{
  baseline_pose_set_ = false;
  movement_history_.clear();
  prev_pose_set_ = false;
  min_dist_to_goal_ = 1e9;
  away_counter_ = 0;
  last_diag_ = Diag::OK;
}

// ── 诊断发布 ──
void AdaptiveProgressChecker::publishDiagnosis(Diag diag)
{
  if (diag == last_diag_) return;  // 避免重复发布
  last_diag_ = diag;

  auto msg = std_msgs::msg::String();
  switch (diag) {
    case Diag::OSCILLATION: msg.data = "oscillation"; break;
    case Diag::RETREATING:  msg.data = "retreating";  break;
    case Diag::TIMEOUT:     msg.data = "timeout";     break;
    default:                msg.data = "ok";          break;
  }
  diag_pub_->publish(msg);
}

// ── 角距离计算 ──
double AdaptiveProgressChecker::poseAngleDistance(
  const geometry_msgs::msg::Pose2D & a,
  const geometry_msgs::msg::Pose2D & b)
{
  double diff = std::fabs(a.theta - b.theta);
  diff = std::fmod(diff, 2.0 * M_PI);
  if (diff > M_PI) diff = 2.0 * M_PI - diff;
  return diff;
}

// ── 更新振荡检测历史 ──
void AdaptiveProgressChecker::updateOscillationHistory(
  const geometry_msgs::msg::Pose2D & current)
{
  if (!prev_pose_set_) {
    prev_pose_ = current;
    prev_pose_set_ = true;
    return;
  }

  MovementSample sample;
  sample.vx = current.x - prev_pose_.x;
  sample.vy = current.y - prev_pose_.y;
  movement_history_.push_back(sample);
  prev_pose_ = current;

  while (static_cast<int>(movement_history_.size()) > oscillation_window_) {
    movement_history_.pop_front();
  }
}

// ── 振荡检测 ──
bool AdaptiveProgressChecker::isOscillating() const
{
  if (static_cast<int>(movement_history_.size()) < oscillation_window_) {
    return false;
  }

  int reversals = 0;
  for (size_t i = 1; i < movement_history_.size(); ++i) {
    double dot = movement_history_[i].vx * movement_history_[i - 1].vx +
                 movement_history_[i].vy * movement_history_[i - 1].vy;
    if (dot < 0.0) reversals++;
  }

  double net_dx = 0.0, net_dy = 0.0;
  double total_dist = 0.0;
  for (const auto & m : movement_history_) {
    net_dx += m.vx;
    net_dy += m.vy;
    total_dist += std::hypot(m.vx, m.vy);
  }
  double net = std::hypot(net_dx, net_dy);
  double efficiency = (total_dist > 1e-6) ? (net / total_dist) : 1.0;

  return (reversals >= oscillation_threshold_) || (efficiency < 0.3 && total_dist > 0.05);
}

// ── 目标趋近追踪 ──
void AdaptiveProgressChecker::updateGoalDistance(double /*current_dist*/) {}

bool AdaptiveProgressChecker::isMovingAwayFromGoal(double /*current_dist*/) const
{
  if (movement_history_.size() < 4) return false;

  double net_dx = 0.0, net_dy = 0.0;
  double total_dist = 0.0;
  for (const auto & m : movement_history_) {
    net_dx += m.vx;
    net_dy += m.vy;
    total_dist += std::hypot(m.vx, m.vy);
  }
  double net = std::hypot(net_dx, net_dy);
  return (total_dist > 0.2 && net < total_dist * 0.25);
}

// ── 移动 + 旋转检测 ──
bool AdaptiveProgressChecker::isRobotMovedEnough(const geometry_msgs::msg::Pose2D & pose)
{
  double dist = pose_distance(pose, baseline_pose_);
  double angle_dist = poseAngleDistance(pose, baseline_pose_);

  double elapsed = (clock_->now() - baseline_time_).seconds();
  double adaptive_radius = radius_ / (1.0 + elapsed / dist_adaptive_scale_);
  adaptive_radius = std::max(adaptive_radius, 0.05);

  return (dist > adaptive_radius) || (angle_dist > required_movement_angle_);
}

// ── 主检测逻辑 (诊断发布版) ──
bool AdaptiveProgressChecker::check(geometry_msgs::msg::PoseStamped & current_pose)
{
  // Pose → Pose2D
  geometry_msgs::msg::Pose2D pose2d;
  pose2d.x = current_pose.pose.position.x;
  pose2d.y = current_pose.pose.position.y;
  {
    double qx = current_pose.pose.orientation.x;
    double qy = current_pose.pose.orientation.y;
    double qz = current_pose.pose.orientation.z;
    double qw = current_pose.pose.orientation.w;
    pose2d.theta = std::atan2(2.0 * (qw * qz + qx * qy),
                               1.0 - 2.0 * (qy * qy + qz * qz));
  }

  updateOscillationHistory(pose2d);

  // 振荡 → 卡死，发布原因
  if (isOscillating()) {
    publishDiagnosis(Diag::OSCILLATION);
    resetBaselinePose(pose2d);
    movement_history_.clear();
    return false;
  }

  // 原地绕圈/退行 → 卡死
  if (isMovingAwayFromGoal(0.0)) {
    publishDiagnosis(Diag::RETREATING);
    resetBaselinePose(pose2d);
    movement_history_.clear();
    return false;
  }

  if (!baseline_pose_set_) {
    resetBaselinePose(pose2d);
    publishDiagnosis(Diag::OK);
    return true;
  }

  if (isRobotMovedEnough(pose2d)) {
    resetBaselinePose(pose2d);
    publishDiagnosis(Diag::OK);
    return true;
  }

  if ((clock_->now() - baseline_time_) > time_allowance_) {
    publishDiagnosis(Diag::TIMEOUT);
    resetBaselinePose(pose2d);
    return false;
  }

  return true;
}

// ── 动态参数回调 ──
rcl_interfaces::msg::SetParametersResult
AdaptiveProgressChecker::dynamicParametersCallback(
  std::vector<rclcpp::Parameter> parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  result.successful = true;

  for (const auto & param : parameters) {
    const auto & name = param.get_name();
    if (name == plugin_name_ + ".required_movement_radius") {
      radius_ = param.as_double();
    } else if (name == plugin_name_ + ".movement_time_allowance") {
      time_allowance_ = rclcpp::Duration::from_seconds(param.as_double());
    } else if (name == plugin_name_ + ".required_movement_angle") {
      required_movement_angle_ = param.as_double();
    } else if (name == plugin_name_ + ".oscillation_window") {
      oscillation_window_ = param.as_int();
    } else if (name == plugin_name_ + ".dist_adaptive_scale") {
      dist_adaptive_scale_ = param.as_double();
    } else if (name == plugin_name_ + ".oscillation_threshold") {
      oscillation_threshold_ = param.as_int();
    }
  }
  return result;
}

}  // namespace nav2_custom_plugins
