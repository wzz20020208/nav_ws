// Copyright (c) 2024 THEMIS
//
// EscapeObstacle: 简单的避障恢复行为
//   当全局规划器无法规划路径时触发。
//   扫描周围方向找到最近障碍物，向反方向移动 10cm 后返回成功，
//   让规划器在新位置重新规划。

#ifndef NAV2_CUSTOM_PLUGINS__ESCAPE_OBSTACLE_HPP_
#define NAV2_CUSTOM_PLUGINS__ESCAPE_OBSTACLE_HPP_

#include <memory>
#include <string>
#include <cmath>
#include <utility>

#include "nav2_behaviors/timed_behavior.hpp"
#include "nav2_msgs/action/drive_on_heading.hpp"
#include "nav2_costmap_2d/costmap_topic_collision_checker.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/pose2_d.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

namespace nav2_custom_plugins
{

using DriveOnHeadingAction = nav2_msgs::action::DriveOnHeading;

class EscapeObstacle : public nav2_behaviors::TimedBehavior<DriveOnHeadingAction>
{
public:
  EscapeObstacle() = default;
  ~EscapeObstacle() override = default;

  nav2_behaviors::Status onRun(
    const std::shared_ptr<const DriveOnHeadingAction::Goal> command) override;

  nav2_behaviors::Status onCycleUpdate() override;

  void onConfigure() override;

private:
  double findEscapeDirection();
  bool getCurrentPose(geometry_msgs::msg::PoseStamped & pose);

  double escape_distance_{0.10};
  double scan_radius_{0.15};
  int    scan_directions_{12};
  double escape_speed_{0.10};

  double start_x_{0}, start_y_{0};
  double escape_yaw_{0};
  bool   escape_initialized_{false};
};

}  // namespace nav2_custom_plugins

#endif  // NAV2_CUSTOM_PLUGINS__ESCAPE_OBSTACLE_HPP_
