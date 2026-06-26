#ifndef NAV2_CUSTOM_PLUGINS__MPPI_CONTROLLER_HPP_
#define NAV2_CUSTOM_PLUGINS__MPPI_CONTROLLER_HPP_

#include <string>
#include <memory>
#include <vector>
#include <random>

#include "nav2_core/controller.hpp"
#include "rclcpp/rclcpp.hpp"
#include "pluginlib/class_list_macros.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/path.hpp"
#include "nav2_costmap_2d/costmap_2d_ros.hpp"
#include "visualization_msgs/msg/marker_array.hpp"

namespace nav2_custom_plugins
{

/**
 * @class MPPIController
 * @brief MPPI (Model Predictive Path Integral Control) 控制器
 *
 * MPPI 是一种基于采样的最优控制算法，通过蒙特卡洛采样生成候选控制序列，
 * 根据代价函数评估并加权平均得到最优控制指令。
 *
 * 参考论文：
 * G. W. Schwartz, "MPPI: Model Predictive Path Integral Control for Autonomous Driving", 2017.
 */
class MPPIController : public nav2_core::Controller
{
public:
  MPPIController() = default;
  ~MPPIController() override = default;

  void configure(
    const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
    std::string name, std::shared_ptr<tf2_ros::Buffer> tf,
    std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros) override;

  void cleanup() override;
  void activate() override;
  void deactivate() override;

  void setPlan(const nav_msgs::msg::Path & path) override;

  geometry_msgs::msg::TwistStamped computeVelocityCommands(
    const geometry_msgs::msg::PoseStamped & pose,
    const geometry_msgs::msg::Twist & velocity,
    nav2_core::GoalChecker * goal_checker) override;

  void setSpeedLimit(const double & speed_limit, const bool & percentage) override;

private:
  rclcpp_lifecycle::LifecycleNode::WeakPtr node_;
  std::shared_ptr<tf2_ros::Buffer> tf_;
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros_;
  nav_msgs::msg::Path global_plan_;
  std::string plugin_name_;

  // MPPI 参数
  int num_samples_ = 1000;
  int prediction_horizon_ = 15;
  double dt_ = 0.3;
  double max_v_ = 2.0;
  double min_v_ = -2.0;
  double max_w_ = 0.5;
  double action_std_v_ = 0.2;
  double action_std_w_ = 0.2;
  double lambda_ = 50.0;
  double collision_cost_ = 10.0;
  double heading_weight_ = 0.5;      // 朝向目标奖励权重
  double lookahead_distance_ = 1.0;

  // 最优控制序列记忆（滚动窗口）
  std::vector<double> optimal_vx_seq_;
  std::vector<double> optimal_vy_seq_;
  std::vector<double> optimal_omega_seq_;
  bool initialized_ = false;

  // 随机数生成器
  std::mt19937 generator_;
  std::normal_distribution<> dist_vx_;
  std::normal_distribution<> dist_vy_;
  std::normal_distribution<> dist_w_;

  // 可视化发布者
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr vis_pub_;

  // 内部辅助方法
  void publishVisualization(
    double robot_x, double robot_y, double robot_theta,
    double target_x, double target_y,
    const std::vector<std::vector<double>>& trajectories,
    const std::vector<double>& trajectory_costs,
    int best_idx);
};

}  // namespace nav2_custom_plugins

#endif  // NAV2_CUSTOM_PLUGINS__MPPI_CONTROLLER_HPP_