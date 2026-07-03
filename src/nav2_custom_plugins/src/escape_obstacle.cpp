// Copyright (c) 2024 THEMIS

#include "nav2_custom_plugins/escape_obstacle.hpp"
#include "tf2/utils.h"

namespace nav2_custom_plugins
{

void EscapeObstacle::onConfigure()
{
  auto node = node_.lock();
  if (!node) return;

  node->get_parameter_or(behavior_name_ + ".escape_distance", escape_distance_, 0.10);
  node->get_parameter_or(behavior_name_ + ".scan_radius",     scan_radius_,     0.15);
  node->get_parameter_or(behavior_name_ + ".scan_directions", scan_directions_, 12);
  node->get_parameter_or(behavior_name_ + ".escape_speed",    escape_speed_,    0.10);

  RCLCPP_INFO(logger_, "EscapeObstacle configured: dist=%.2f scan_r=%.2f n=%d sp=%.2f",
    escape_distance_, scan_radius_, scan_directions_, escape_speed_);
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

double EscapeObstacle::findEscapeDirection()
{
  geometry_msgs::msg::PoseStamped pose;
  if (!getCurrentPose(pose)) return 0.0;

  double cx = pose.pose.position.x;
  double cy = pose.pose.position.y;
  double yaw = tf2::getYaw(pose.pose.orientation);

  int n = std::max(4, scan_directions_);
  const int N_DIST = 3;
  const int N_LAT  = 3;
  const double LAT_SPAN = 0.08;

  double best_dir = 0.0;
  double best_cost = 1e9;

  for (int i = 0; i < n; ++i) {
    double dir = i * 2.0 * M_PI / n;
    double cos_dir = std::cos(dir), sin_dir = std::sin(dir);
    double perp_x = -sin_dir, perp_y = cos_dir;

    double cost = 0.0;
    bool first = true;
    for (int d = 0; d < N_DIST; ++d) {
      double r = scan_radius_ * (0.3 + 0.7 * d / (N_DIST - 1));
      for (int lat = 0; lat < N_LAT; ++lat) {
        double offset = LAT_SPAN * (-1.0 + 2.0 * lat / (N_LAT - 1));
        geometry_msgs::msg::Pose2D test_pose;
        test_pose.x = cx + cos_dir * r + perp_x * offset;
        test_pose.y = cy + sin_dir * r + perp_y * offset;
        test_pose.theta = yaw;
        double sc = collision_checker_->scorePose(test_pose, first);
        first = false;
        double w = (1.0 - 0.5 * d / (N_DIST - 1));
        cost += sc * w;
      }
    }
    if (cost < best_cost) {
      best_cost = cost;
      best_dir = dir;
    }
  }

  RCLCPP_INFO(logger_, "EscapeObstacle: best_dir=%.1f deg cost=%.1f",
    best_dir * 180.0 / M_PI, best_cost);
  return best_dir;
}

nav2_behaviors::Status EscapeObstacle::onRun(
  const std::shared_ptr<const DriveOnHeadingAction::Goal> command)
{
  (void)command;

  geometry_msgs::msg::PoseStamped pose;
  if (!getCurrentPose(pose)) {
    RCLCPP_WARN(logger_, "EscapeObstacle: cannot get current pose on start");
    return nav2_behaviors::Status::FAILED;
  }

  start_x_ = pose.pose.position.x;
  start_y_ = pose.pose.position.y;
  escape_yaw_ = findEscapeDirection();
  escape_initialized_ = true;

  RCLCPP_INFO(logger_, "EscapeObstacle: START pos=(%.3f,%.3f) yaw=%.1f deg dist=%.2f speed=%.2f",
    start_x_, start_y_, escape_yaw_ * 180.0 / M_PI, escape_distance_, escape_speed_);

  return nav2_behaviors::Status::SUCCEEDED;
}

nav2_behaviors::Status EscapeObstacle::onCycleUpdate()
{
  if (!escape_initialized_) return nav2_behaviors::Status::FAILED;

  geometry_msgs::msg::PoseStamped pose;
  if (!getCurrentPose(pose)) {
    RCLCPP_WARN(logger_, "EscapeObstacle: TF lost during escape");
    return nav2_behaviors::Status::FAILED;
  }

  double dx = pose.pose.position.x - start_x_;
  double dy = pose.pose.position.y - start_y_;
  double moved = std::hypot(dx, dy);

  if (moved >= escape_distance_) {
    RCLCPP_INFO(logger_, "EscapeObstacle: DONE moved=%.3f m", moved);
    stopRobot();
    escape_initialized_ = false;
    return nav2_behaviors::Status::SUCCEEDED;
  }

  auto cmd_vel = std::make_unique<geometry_msgs::msg::Twist>();
  double c = std::cos(escape_yaw_), s = std::sin(escape_yaw_);
  double yaw = tf2::getYaw(pose.pose.orientation);
  double cos_t = std::cos(-yaw), sin_t = std::sin(-yaw);
  cmd_vel->linear.x = (c * cos_t - s * sin_t) * escape_speed_;
  cmd_vel->linear.y = (c * sin_t + s * cos_t) * escape_speed_;
  cmd_vel->angular.z = 0.0;

  vel_pub_->publish(std::move(cmd_vel));

  RCLCPP_INFO_THROTTLE(logger_, *clock_, 500,
    "EscapeObstacle: moving vx=%.2f vy=%.2f moved=%.3f/%.3f",
    (c * cos_t - s * sin_t) * escape_speed_,
    (c * sin_t + s * cos_t) * escape_speed_,
    moved, escape_distance_);

  return nav2_behaviors::Status::RUNNING;
}

}  // namespace nav2_custom_plugins

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nav2_custom_plugins::EscapeObstacle, nav2_core::Behavior)
