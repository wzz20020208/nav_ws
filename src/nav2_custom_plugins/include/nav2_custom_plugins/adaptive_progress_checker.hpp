// Copyright (c) 2025
// Adaptive Progress Checker for Nav2 — oscillation-aware, distance-adaptive

#ifndef NAV2_CUSTOM_PLUGINS__ADAPTIVE_PROGRESS_CHECKER_HPP_
#define NAV2_CUSTOM_PLUGINS__ADAPTIVE_PROGRESS_CHECKER_HPP_

#include <string>
#include <deque>
#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "nav2_core/progress_checker.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/pose2_d.hpp"
#include "std_msgs/msg/string.hpp"

namespace nav2_custom_plugins
{

/**
 * @class AdaptiveProgressChecker
 * @brief 振荡感知 + 诊断发布 + 距离自适应的进度检测器
 *
 * 直接继承 nav2_core::ProgressChecker，自包含基线追踪逻辑，
 * 不依赖 nav2_controller::SimpleProgressChecker 的 .so。
 *
 * 检测能力:
 * 1. 旋转感知 — 原地旋转也算有效进度
 * 2. 振荡检测 — 连续 N 帧反复改变运动方向 → 判定卡死
 * 3. 退行检测 — 净位移远小于总路径 → 原地绕圈
 * 4. 距离自适应 — baseline 未更新时间越长，有效半径越收紧
 * 5. 诊断发布 — 卡死时发布原因到 /progress_checker_diagnosis
 */
class AdaptiveProgressChecker : public nav2_core::ProgressChecker
{
public:
  void initialize(
    const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
    const std::string & plugin_name) override;

  bool check(geometry_msgs::msg::PoseStamped & current_pose) override;
  void reset() override;

protected:
  // ── 基线追踪 (从 SimpleProgressChecker 自包含) ──
  static double pose_distance(
    const geometry_msgs::msg::Pose2D & a,
    const geometry_msgs::msg::Pose2D & b);
  void resetBaselinePose(const geometry_msgs::msg::Pose2D & pose);
  bool isRobotMovedEnough(const geometry_msgs::msg::Pose2D & pose);

  // ── 振荡检测 ──
  static double poseAngleDistance(
    const geometry_msgs::msg::Pose2D & a,
    const geometry_msgs::msg::Pose2D & b);
  void updateOscillationHistory(const geometry_msgs::msg::Pose2D & current);
  bool isOscillating() const;
  bool isMovingAwayFromGoal() const;

  // ── 诊断发布 ──
  enum class Diag { OK, OSCILLATION, RETREATING, TIMEOUT };
  Diag last_diag_{Diag::OK};
  void publishDiagnosis(Diag diag);

  // ── 基线参数 ──
  rclcpp::Clock::SharedPtr clock_;
  double radius_{0.5};
  rclcpp::Duration time_allowance_{0, 0};
  geometry_msgs::msg::Pose2D baseline_pose_;
  rclcpp::Time baseline_time_;
  bool baseline_pose_set_{false};

  // ── 自有参数 ──
  double required_movement_angle_{0.5};
  int oscillation_window_{8};
  int oscillation_threshold_{4};
  double dist_adaptive_scale_{2.0};

  // ── 振荡检测状态 ──
  struct MovementSample { double vx, vy; };
  std::deque<MovementSample> movement_history_;
  geometry_msgs::msg::Pose2D prev_pose_;
  bool prev_pose_set_{false};

  // ── 诊断发布者 ──
  rclcpp_lifecycle::LifecyclePublisher<std_msgs::msg::String>::SharedPtr diag_pub_;
  rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr dyn_params_handler_;
  std::string plugin_name_;

  rcl_interfaces::msg::SetParametersResult
  dynamicParametersCallback(std::vector<rclcpp::Parameter> parameters);
};

}  // namespace nav2_custom_plugins

#endif  // NAV2_CUSTOM_PLUGINS__ADAPTIVE_PROGRESS_CHECKER_HPP_
