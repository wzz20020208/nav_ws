#ifndef NAV2_CUSTOM_PLUGINS_V2__MPPI_STEERING_CONTROLLER_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MPPI_STEERING_CONTROLLER_HPP_

#include <memory>
#include <string>

#include "nav2_core/controller.hpp"
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/path.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_costmap_2d/costmap_2d_ros.hpp"

#include "nav2_custom_plugins_v2/msg/velocity_steering.hpp"
#include "nav2_custom_plugins_v2/mppi_core.hpp"
#include "nav2_custom_plugins_v2/path_manager.hpp"
#include "nav2_custom_plugins_v2/cost_evaluator.hpp"
#include "nav2_custom_plugins_v2/velocity_postprocessor.hpp"
#include "nav2_custom_plugins_v2/gpu_engine.hpp"
#include "nav2_custom_plugins_v2/state_machine.hpp"
#include "nav2_custom_plugins_v2/visualization.hpp"
#include "nav2_custom_plugins_v2/mppi_logger.hpp"

namespace nav2_custom_plugins_v2 {

/// MPPI 转向控制器 — Nav2 接口适配 + 流程编排
///
/// computeVelocityCommands() 流程:
///   1. PathManager  → 解析路径, 找前瞻点
///   2. StateMachine → 判定是否需要特殊处理
///   3. GPUEngine     → 噪声生成 + GPU 采样 + 代价计算
///   4. CostEvaluator → 找最优, 加权求和
///   5. PostProcessor → EMA, 减速, 钳位, δ→ω
///   6. 发布 VelocitySteering + TwistStamped + 可视化
class MPPISteeringController : public nav2_core::Controller
{
public:
  MPPISteeringController() = default;
  ~MPPISteeringController() override = default;

  // ═════════════════════════════════════════════════════════════════════
  // Nav2 Controller 标准接口
  // ═════════════════════════════════════════════════════════════════════

  void configure(
      const rclcpp_lifecycle::LifecycleNode::WeakPtr& parent,
      std::string name,
      std::shared_ptr<tf2_ros::Buffer> tf,
      std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros) override;

  void cleanup() override;
  void activate() override;
  void deactivate() override;

  void setPlan(const nav_msgs::msg::Path& path) override;

  void setSpeedLimit(const double& speed_limit, const bool& percentage) override;

  /// 核心: 计算速度指令 — 必须返回 TwistStamped (Nav2 接口不可改)
  geometry_msgs::msg::TwistStamped computeVelocityCommands(
      const geometry_msgs::msg::PoseStamped& pose,
      const geometry_msgs::msg::Twist& velocity,
      nav2_core::GoalChecker* goal_checker) override;

private:
  // ═════════════════════════════════════════════════════════════════════
  // ROS 接口
  // ═════════════════════════════════════════════════════════════════════

  rclcpp_lifecycle::LifecycleNode::WeakPtr node_;
  std::shared_ptr<tf2_ros::Buffer> tf_;
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros_;
  std::string plugin_name_;

  // 发布者
  rclcpp::Publisher<msg::VelocitySteering>::SharedPtr steering_pub_;

  // 全局代价地图订阅 (供 GPU 上传)
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr global_costmap_sub_;
  nav_msgs::msg::OccupancyGrid::SharedPtr latest_global_costmap_;
  bool use_global_costmap_ = true;
  std::string global_costmap_topic_ = "/global_costmap/costmap";

  void globalCostmapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

  // ═════════════════════════════════════════════════════════════════════
  // 子模块 (组合, 非继承)
  // ═════════════════════════════════════════════════════════════════════

  MPPIParams params_;
  KinematicModel kinematics_{params_};
  PathManager path_manager_;
  CostEvaluator cost_evaluator_{params_};
  VelocityPostProcessor postprocessor_{params_};
  StateMachine state_machine_;
  VisualizationPublisher vis_;
  MPPILogger logger_;

  // GPUEngine 在 configure() 中根据 N,H 动态构造
  std::unique_ptr<GPUEngine> gpu_engine_;

  // ═════════════════════════════════════════════════════════════════════
  // 持久状态
  // ═════════════════════════════════════════════════════════════════════

  ControlSequence warm_start_;
  bool initialized_ = false;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__MPPI_STEERING_CONTROLLER_HPP_
