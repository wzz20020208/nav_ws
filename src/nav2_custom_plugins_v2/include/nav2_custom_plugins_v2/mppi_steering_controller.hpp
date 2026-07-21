#ifndef NAV2_CUSTOM_PLUGINS_V2__MPPI_STEERING_CONTROLLER_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MPPI_STEERING_CONTROLLER_HPP_

#include <memory>
#include <string>

#include "nav2_core/controller.hpp"
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/path.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav2_costmap_2d/costmap_2d_ros.hpp"

#include "nav2_custom_plugins_v2/core/mppi_core.hpp"
#include "nav2_custom_plugins_v2/gpu/gpu_engine.hpp"
#include "nav2_custom_plugins_v2/gpu/gpu_uploader.hpp"
#include "nav2_custom_plugins_v2/modules/path_manager.hpp"
#include "nav2_custom_plugins_v2/modules/velocity_postprocessor.hpp"
#include "nav2_custom_plugins_v2/modules/state_machine.hpp"
#include "nav2_custom_plugins_v2/modules/visualization.hpp"

namespace nav2_custom_plugins_v2
{

class MPPIPipeline;

class MPPISteeringController : public nav2_core::Controller
{
public:
  MPPISteeringController() = default;
  ~MPPISteeringController() override = default;

  void configure(
      const rclcpp_lifecycle::LifecycleNode::WeakPtr &parent,
      std::string name,
      std::shared_ptr<tf2_ros::Buffer> tf,
      std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros) override;

  void cleanup() override;
  void activate() override;
  void deactivate() override;
  void setPlan(const nav_msgs::msg::Path &path) override;
  void setSpeedLimit(const double &speed_limit, const bool &percentage) override;

  geometry_msgs::msg::TwistStamped computeVelocityCommands(
      const geometry_msgs::msg::PoseStamped &pose,
      const geometry_msgs::msg::Twist &velocity,
      nav2_core::GoalChecker *goal_checker) override;

private:
  rclcpp_lifecycle::LifecycleNode::WeakPtr node_;
  std::shared_ptr<tf2_ros::Buffer> tf_;
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros_;
  std::string plugin_name_;
  nav_msgs::msg::Path global_plan_;

  // ── 参数 ──
  MPPIParams params_;

  // ── GPU 资源 ──
  std::unique_ptr<GPUEngine>    gpu_engine_;
  std::unique_ptr<GPUUploader>  gpu_uploader_;

  // ── MPPI 算法 ──
  std::unique_ptr<NoiseGenerator> noise_gen_;
  std::unique_ptr<MPPIPipeline>   pipeline_;
  PathManager                        path_mgr_;            // 路径管理 (最近点/前瞻/yaw)
  std::unique_ptr<VelocityPostProcessor> vel_postprocessor_; // 控制量后处理 (提取+clamp+δ→ω)
  StateMachine                        state_machine_;        // 状态机 (heading 原地旋转判定)
  bool                                in_terminal_align_ = false;  // 终端对齐迟滞: 进入后不轻易退出
  VisualizationPublisher              vis_pub_;              // RViz 可视化
  ControlSequence                     base_seq_;             // warm-start 基序列

  // ── 输出 ──
  rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr
    twist_pub_;                          // /cmd_vel_mppi 独立话题
};

}  // namespace nav2_custom_plugins_v2

#endif
