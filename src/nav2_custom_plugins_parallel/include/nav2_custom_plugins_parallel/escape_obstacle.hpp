// Copyright (c) 2024 THEMIS
//
// SafeEscape: 综合多方向障碍物检测的安全逃逸/后退行为
//   扫描 360° 检测障碍物密度，选择最安全方向移动。
//   移动过程中实时检测前方障碍物，遇阻则立即停止。

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

namespace nav2_custom_plugins_parallel
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
  /// 扫描 360° 找出障碍物代价最低的方向
  double findSafestDirection(double cx, double cy, double yaw);

  /// 获取当前位姿
  bool getCurrentPose(geometry_msgs::msg::PoseStamped & pose);

  /// 检查给定方向是否有足量安全空间
  bool isDirectionSafe(double cx, double cy, double dir_global, double check_dist);

  // ── 参数 ──
  double escape_distance_{0.30};   // 逃逸移动距离 (m)
  double scan_radius_{0.35};       // 障碍物扫描半径 (m)
  int    scan_directions_{24};     // 360° 扫描方向数 (越多越精细)
  int    scan_depth_{4};           // 每个方向沿径向的采样层数
  double scan_lateral_span_{0.12}; // 每层横向采样宽度 (m)
  double escape_speed_{0.15};      // 逃逸移动速度 (m/s)
  double obstacle_threshold_{128}; // 障碍物代价阈值 (>此值视为有障碍)
  double prefer_back_ratio_{0.5};  // 后退偏好 [0,1]: 0=纯安全方向, 1=偏向后退
  double check_ahead_dist_{0.15};  // 移动中前方安全检测距离 (m)

  // ── 运行时状态 ──
  double start_x_{0}, start_y_{0};
  double escape_yaw_{0};
  bool   escape_initialized_{false};
};

}  // namespace nav2_custom_plugins_parallel

#endif  // NAV2_CUSTOM_PLUGINS__ESCAPE_OBSTACLE_HPP_
