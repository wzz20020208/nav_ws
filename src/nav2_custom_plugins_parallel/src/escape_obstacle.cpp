// Copyright (c) 2024 THEMIS
//
// SafeEscape: 综合多方向障碍物检测的安全逃逸/后退
//   - 扫描 360° 全方向，每个方向多深度采样
//   - 选择障碍物密度最低的方向
//   - 移动中实时检测，遇新障碍立即停止
//   - 支持后退偏好（用作安全 BackUp 替代）

#include "nav2_custom_plugins_parallel/escape_obstacle.hpp"
#include "tf2/utils.h"
#include <algorithm>

namespace nav2_custom_plugins_parallel
{

void EscapeObstacle::onConfigure()
{
  auto node = node_.lock();
  if (!node) return;

  node->get_parameter_or(behavior_name_ + ".escape_distance",     escape_distance_,     0.30);
  node->get_parameter_or(behavior_name_ + ".scan_radius",         scan_radius_,         0.35);
  node->get_parameter_or(behavior_name_ + ".scan_directions",     scan_directions_,     24);
  node->get_parameter_or(behavior_name_ + ".scan_depth",          scan_depth_,          4);
  node->get_parameter_or(behavior_name_ + ".scan_lateral_span",   scan_lateral_span_,   0.12);
  node->get_parameter_or(behavior_name_ + ".escape_speed",        escape_speed_,        0.15);
  node->get_parameter_or(behavior_name_ + ".obstacle_threshold",  obstacle_threshold_,  128.0);
  node->get_parameter_or(behavior_name_ + ".prefer_back_ratio",   prefer_back_ratio_,   0.5);
  node->get_parameter_or(behavior_name_ + ".check_ahead_dist",    check_ahead_dist_,    0.15);

  RCLCPP_INFO(logger_,
    "SafeEscape configured: dist=%.2f scan_r=%.2f dirs=%d depth=%d lat=%.2f "
    "speed=%.2f obs_th=%.0f back_bias=%.2f check_ahead=%.2f",
    escape_distance_, scan_radius_, scan_directions_, scan_depth_,
    scan_lateral_span_, escape_speed_, obstacle_threshold_,
    prefer_back_ratio_, check_ahead_dist_);
}

bool EscapeObstacle::getCurrentPose(geometry_msgs::msg::PoseStamped & pose)
{
  auto node = node_.lock();
  if (!node) return false;

  try {
    auto tf_future = tf_->lookupTransform(
      global_frame_, robot_base_frame_, tf2::TimePointZero, tf2::durationFromSec(0.5));
    pose.header.frame_id = global_frame_;
    pose.header.stamp = node->now();
    pose.pose.position.x = tf_future.transform.translation.x;
    pose.pose.position.y = tf_future.transform.translation.y;
    pose.pose.orientation = tf_future.transform.rotation;
    return true;
  } catch (const tf2::TransformException & e) {
    RCLCPP_WARN(logger_, "TF lookup failed: %s", e.what());
    return false;
  }
}

bool EscapeObstacle::isDirectionSafe(
  double cx, double cy, double dir_global, double check_dist)
{
  double cos_d = std::cos(dir_global), sin_d = std::sin(dir_global);
  geometry_msgs::msg::Pose2D test_pose;
  test_pose.x = cx + cos_d * check_dist;
  test_pose.y = cy + sin_d * check_dist;
  test_pose.theta = dir_global;
  double score = collision_checker_->scorePose(test_pose, true);
  return score < obstacle_threshold_;
}

double EscapeObstacle::findSafestDirection(double cx, double cy, double yaw)
{
  const int n = std::max(8, scan_directions_);
  const int n_depth = std::max(2, scan_depth_);
  const double lat_span = std::max(0.02, scan_lateral_span_);

  // robot_yaw 用于后退偏好: dir 偏离 robot_yaw 越远，偏置权重越大
  double back_dir = yaw + M_PI;
  while (back_dir > M_PI)  back_dir -= 2.0 * M_PI;
  while (back_dir < -M_PI) back_dir += 2.0 * M_PI;

  double best_dir = 0.0;
  double best_cost = 1e9;

  for (int i = 0; i < n; ++i) {
    double dir = i * 2.0 * M_PI / n;
    double cos_dir = std::cos(dir), sin_dir = std::sin(dir);
    double perp_x = -sin_dir, perp_y = cos_dir;

    double cost = 0.0;
    bool first = true;
    int obstacle_hits = 0;

    // 多深度采样: 沿 dir 方向从近到远逐层检测
    for (int d = 0; d < n_depth; ++d) {
      double r = scan_radius_ * (0.25 + 0.75 * d / (n_depth - 1));

      for (int lat = 0; lat < 3; ++lat) {
        double offset = lat_span * (-1.0 + 1.0 * lat);  // -lat_span, 0, +lat_span
        geometry_msgs::msg::Pose2D test_pose;
        test_pose.x = cx + cos_dir * r + perp_x * offset;
        test_pose.y = cy + sin_dir * r + perp_y * offset;
        test_pose.theta = yaw;
        double sc = collision_checker_->scorePose(test_pose, first);
        first = false;

        // 命中障碍物: 计入惩罚
        if (sc >= obstacle_threshold_) {
          obstacle_hits++;
          cost += sc * (1.5 - 0.3 * d / (n_depth - 1));  // 近处障碍惩罚更重
        } else {
          cost += sc;
        }
      }
    }

    // 后退偏好: 如果该方向接近 robot_back_dir，略微降低代价
    if (prefer_back_ratio_ > 0.0) {
      double angle_to_back = std::abs(dir - back_dir);
      if (angle_to_back > M_PI) angle_to_back = 2.0 * M_PI - angle_to_back;
      // 越接近后退方向，减分越多 (prefer_back_ratio=1 时最多减 30%)
      double bias = prefer_back_ratio_ * 0.3 * (1.0 - angle_to_back / M_PI);
      cost *= (1.0 - bias);
    }

    // 所有采样点都被障碍物阻挡 → 大幅惩罚
    if (obstacle_hits >= n_depth * 2) {
      cost *= 3.0;
    }

    if (cost < best_cost) {
      best_cost = cost;
      best_dir = dir;
    }
  }

  RCLCPP_INFO(logger_,
    "SafeEscape: safest_dir=%.1f deg cost=%.1f (back_dir=%.1f deg)",
    best_dir * 180.0 / M_PI, best_cost, back_dir * 180.0 / M_PI);

  return best_dir;
}

nav2_behaviors::Status EscapeObstacle::onRun(
  const std::shared_ptr<const DriveOnHeadingAction::Goal> command)
{
  (void)command;

  geometry_msgs::msg::PoseStamped pose;
  if (!getCurrentPose(pose)) {
    RCLCPP_WARN(logger_, "SafeEscape: cannot get current pose on start");
    return nav2_behaviors::Status::FAILED;
  }

  double cx = pose.pose.position.x;
  double cy = pose.pose.position.y;
  double yaw = tf2::getYaw(pose.pose.orientation);

  // 1. 检查前方是否有足够空间 (若已经安全则无需逃逸)
  // 2. 全方向扫描找最安全方向
  escape_yaw_ = findSafestDirection(cx, cy, yaw);
  start_x_ = cx;
  start_y_ = cy;
  escape_initialized_ = true;

  RCLCPP_INFO(logger_,
    "SafeEscape: START pos=(%.3f,%.3f) yaw=%.1f deg escape_dir=%.1f deg "
    "dist=%.2f speed=%.2f",
    start_x_, start_y_, yaw * 180.0 / M_PI,
    escape_yaw_ * 180.0 / M_PI, escape_distance_, escape_speed_);

  return nav2_behaviors::Status::SUCCEEDED;
}

nav2_behaviors::Status EscapeObstacle::onCycleUpdate()
{
  if (!escape_initialized_) return nav2_behaviors::Status::FAILED;

  geometry_msgs::msg::PoseStamped pose;
  if (!getCurrentPose(pose)) {
    RCLCPP_WARN(logger_, "SafeEscape: TF lost during escape");
    return nav2_behaviors::Status::FAILED;
  }

  double cx = pose.pose.position.x;
  double cy = pose.pose.position.y;

  double dx = cx - start_x_;
  double dy = cy - start_y_;
  double moved = std::hypot(dx, dy);

  // ── 到达目标距离 → 成功 ──
  if (moved >= escape_distance_) {
    RCLCPP_INFO(logger_, "SafeEscape: DONE moved=%.3f m", moved);
    stopRobot();
    escape_initialized_ = false;
    return nav2_behaviors::Status::SUCCEEDED;
  }

  // ── 实时安全检测: 前方有障碍物 → 提前终止 ──
  if (!isDirectionSafe(cx, cy, escape_yaw_, check_ahead_dist_)) {
    RCLCPP_WARN(logger_,
      "SafeEscape: BLOCKED ahead at dist=%.2f, stopping early (moved=%.3f/%.3f)",
      check_ahead_dist_, moved, escape_distance_);
    stopRobot();
    escape_initialized_ = false;
    return nav2_behaviors::Status::SUCCEEDED;  // 返回成功让 BT 继续尝试其他恢复
  }

  // ── 侧向 + 后方障碍物检测: 如果其他方向突然出现障碍物，重新规划方向 ──
  {
    double yaw = tf2::getYaw(pose.pose.orientation);
    double current_dir = escape_yaw_;
    // 如果当前方向不再安全 (侧面新障碍物)，尝试找新方向
    if (!isDirectionSafe(cx, cy, current_dir, scan_radius_ * 0.6)) {
      double new_dir = findSafestDirection(cx, cy, yaw);
      double angle_diff = std::abs(new_dir - current_dir);
      if (angle_diff > M_PI) angle_diff = 2.0 * M_PI - angle_diff;
      // 新方向显著不同 → 切换
      if (angle_diff > M_PI / 6.0) {  // >30°
        RCLCPP_INFO(logger_,
          "SafeEscape: rerouting from %.1f to %.1f deg",
          current_dir * 180.0 / M_PI, new_dir * 180.0 / M_PI);
        escape_yaw_ = new_dir;
      }
    }
  }

  // ── 发布速度指令 ──
  auto cmd_vel = std::make_unique<geometry_msgs::msg::Twist>();
  double c = std::cos(escape_yaw_), s = std::sin(escape_yaw_);
  double yaw_robot = tf2::getYaw(pose.pose.orientation);
  double cos_t = std::cos(-yaw_robot), sin_t = std::sin(-yaw_robot);
  cmd_vel->linear.x = (c * cos_t - s * sin_t) * escape_speed_;
  cmd_vel->linear.y = (c * sin_t + s * cos_t) * escape_speed_;
  cmd_vel->angular.z = 0.0;

  vel_pub_->publish(std::move(cmd_vel));

  RCLCPP_INFO_THROTTLE(logger_, *clock_, 500,
    "SafeEscape: vx=%.2f vy=%.2f moved=%.3f/%.3f dir=%.0f deg",
    (c * cos_t - s * sin_t) * escape_speed_,
    (c * sin_t + s * cos_t) * escape_speed_,
    moved, escape_distance_, escape_yaw_ * 180.0 / M_PI);

  return nav2_behaviors::Status::RUNNING;
}

}  // namespace nav2_custom_plugins_parallel

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nav2_custom_plugins_parallel::EscapeObstacle, nav2_core::Behavior)
