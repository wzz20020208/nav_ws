/**
 * @file mppi_steering_controller.cpp
 * @brief MPPI Steering Controller — Nav2 插件, 每帧运行 MPPI 数据流
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 一帧 computeVelocityCommands 的数据流 (@10Hz)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ① 读取当前位姿 (pose) + 速度 (velocity)
 *   ② path_mgr_.findClosest / computeLookahead / getYaw  ← 路径查询
 *   ②b state_machine_.evaluateHeading → rotate_in_place?   ← 朝向判定 (优先)
 *       → true: 直接输出旋转指令, 跳过 MPPI
 *   ③ noise_gen_.generate(N, H)                            ← CPU 纯零均值 NLN 噪声
 *   ④ auto batch = batch_rollout(start, ..., path_vx_r, ...) ← guidance 混合采样+积分
 *   ⑤ pipeline_->uploadBase(base_seq_, H, stream)        ← warm-start → GPU
 *   ⑥ pipeline_->uploadRollout(batch, N, H, stream)      ← 轨迹 → GPU
 *   ⑦ 上传路径 + 组装 CostmapInfo / Footprint / PathInfo / GoalInfo
 *   ⑧ auto costs = pipeline_->launchCost(cmap, fp, ...)  ← GPU 代价
 *   ⑨ float min_c = *std::min_element(costs)             ← CPU 扫描
 *   ⑩ auto result = pipeline_->launchWeightedSum(...)    ← GPU 加权
 *   ⑪ auto proc = vel_postprocessor_->process(result, yaw) ← 提取+clamp+δ→ω
 *   ⑫ base_seq_.shiftAndDecay() + 填入 proc.control     ← 更新 warm-start
 *   ⑬ 填充 TwistStamped 返回 Nav2
 *
 *   TwistStamped 兼容: linear.x=vx, linear.y=vy, angular.z=δ→ω(max_w 限速)
 */

#include "nav2_custom_plugins_v2/mppi_steering_controller.hpp"
#include "nav2_custom_plugins_v2/pipeline/mppi_pipeline.hpp"

#include <stdexcept>
#include <algorithm>
#include <cmath>
#include "nav2_util/node_utils.hpp"

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════════
// configure
// ═══════════════════════════════════════════════════════════════════════════════

void MPPISteeringController::configure(
    const rclcpp_lifecycle::LifecycleNode::WeakPtr &parent,
    std::string name,
    std::shared_ptr<tf2_ros::Buffer> tf,
    std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros)
{
  node_        = parent;
  plugin_name_ = name;
  tf_          = tf;
  costmap_ros_ = costmap_ros;

  auto node = node_.lock();
  if (!node) throw std::runtime_error("Node expired");

  // 读 YAML 参数 → MPPIParams
  // 读 YAML 参数 — 循环公共表, 新增参数只需在 mppi_core.hpp 加一行
  for (auto &p : kIntParams) {
    nav2_util::declare_parameter_if_not_declared(
        node.get(), plugin_name_ + "." + p.name, rclcpp::ParameterValue(params_.*p.ptr));
    node->get_parameter(plugin_name_ + "." + p.name, params_.*p.ptr);
  }
  for (auto &p : kDblParams) {
    nav2_util::declare_parameter_if_not_declared(
        node.get(), plugin_name_ + "." + p.name, rclcpp::ParameterValue(params_.*p.ptr));
    node->get_parameter(plugin_name_ + "." + p.name, params_.*p.ptr);
  }
  for (auto &p : kBoolParams) {
    nav2_util::declare_parameter_if_not_declared(
        node.get(), plugin_name_ + "." + p.name, rclcpp::ParameterValue(params_.*p.ptr));
    node->get_parameter(plugin_name_ + "." + p.name, params_.*p.ptr);
  }

  const int N = params_.num_samples;
  const int H = params_.prediction_horizon;

  // GPU 资源
  gpu_engine_   = std::make_unique<GPUEngine>();
  gpu_uploader_ = std::make_unique<GPUUploader>(*gpu_engine_);
  gpu_uploader_->registerAll(N, H);

  // MPPI 算法
  noise_gen_          = std::make_unique<NoiseGenerator>(params_);
  pipeline_           = std::make_unique<MPPIPipeline>(*gpu_engine_, *gpu_uploader_, params_);
  vel_postprocessor_  = std::make_unique<VelocityPostProcessor>(params_);
  base_seq_.resize(H);
  vis_pub_.init(node.get());  // LifecycleNode* → LifecycleNode*
  steering_pub_ = node->create_publisher<msg::VelocitySteering>(
      "/cmd_vel_steering", 10);
  cmd_vel_pub_ = node->create_publisher<geometry_msgs::msg::TwistStamped>(
      "/cmd_vel_mppi", 10);

  RCLCPP_INFO(node->get_logger(),
      "[%s] configured N=%d H=%d dt=%.2f v=[%.2f,%.2f] vy=±%.2f",
      plugin_name_.c_str(), N, H, params_.dt,
      params_.min_v, params_.max_v, params_.max_vy);
}

// ═══════════════════════════════════════════════════════════════════════════════
// cleanup
// ═══════════════════════════════════════════════════════════════════════════════

void MPPISteeringController::cleanup()
{
  pipeline_.reset();
  noise_gen_.reset();
  gpu_uploader_.reset();
  gpu_engine_.reset();
  cmd_vel_pub_.reset();
  steering_pub_.reset();
}

void MPPISteeringController::activate()   {}
void MPPISteeringController::deactivate() {}

void MPPISteeringController::setPlan(const nav_msgs::msg::Path &path)
{
  global_plan_ = path;
  path_mgr_.setPath(path);
  state_machine_.reset();
}

void MPPISteeringController::setSpeedLimit(const double &, const bool &) {}

// ═══════════════════════════════════════════════════════════════════════════════
// computeVelocityCommands — 10Hz 主循环
// ═══════════════════════════════════════════════════════════════════════════════

geometry_msgs::msg::TwistStamped MPPISteeringController::computeVelocityCommands(
    const geometry_msgs::msg::PoseStamped &pose,
    const geometry_msgs::msg::Twist &velocity,
    nav2_core::GoalChecker *goal_check)
{
  // 无路径时返回零速度, 不跑 MPPI
  if (global_plan_.poses.empty()) {
    geometry_msgs::msg::TwistStamped cmd;
    cmd.header.frame_id = params_.use_global_mode ? "odom" : "BASE_LINK";
    cmd.header.stamp = node_.lock()->now();
    if (params_.use_global_mode) {
      double yaw = 2.0 * atan2(pose.pose.orientation.z, pose.pose.orientation.w);
      cmd.twist.angular.z = yaw;  // global: angular.z = 目标朝向, 保持当前不转
    }
    cmd_vel_pub_->publish(cmd);
    return cmd;
  }

  const int N = params_.num_samples;
  const int H = params_.prediction_horizon;

  // ── ① 当前位姿 ──
  RobotState start;
  start.x     = pose.pose.position.x;
  start.y     = pose.pose.position.y;
  double yaw  = 2.0 * atan2(pose.pose.orientation.z, pose.pose.orientation.w);
  start.theta = yaw;

  // ── ② PathManager: 最近点 → 前瞻点 → 推荐朝向 ──
  double lookahead_yaw = yaw;
  PathInfo path_info;
  GoalInfo goal_info;
  LookaheadResult lp;  // 供后续可视化
  double dist_lh_to_goal = 0.0;

  if (path_mgr_.valid()) {
    int closest = path_mgr_.findClosestIndex(start.x, start.y);
    lp = path_mgr_.computeLookahead(closest, params_.min_lookahead_dist);

    // yaw 来源切换
    lookahead_yaw = params_.use_planner_yaw
      ? path_mgr_.getPlannerYaw(lp.idx)
      : path_mgr_.getTangentYaw(lp.idx);

    // GPU 端路径数据
    path_mgr_.buildPathInfo(path_info, lookahead_yaw);

    // 路径末端: 朝向 target 切换到终点朝向
    int path_size = static_cast<int>(global_plan_.poses.size());
    if (lp.idx >= path_size - 1) {
      path_info.path_tangent = path_info.goal_yaw;
    }

    // target 方向 = 机器人→前瞻点, 转到机器人系 (与 vx/vy 同系)
    double dir_to_lh = std::atan2(lp.wy - start.y, lp.wx - start.x);
    path_mgr_.buildGoalInfo(goal_info, dir_to_lh - yaw);

    // 前瞻点到终点距离
    double dx = goal_info.goal_x - lp.wx;
    double dy = goal_info.goal_y - lp.wy;
    dist_lh_to_goal = std::sqrt(dx * dx + dy * dy);
  }

  // ── ②b StateMachine: heading 判定 (优先于 MPPI) ──
  if (params_.enable_heading_speed_limit) {
    auto dec = state_machine_.evaluateHeading(
        yaw, lookahead_yaw, dist_lh_to_goal, params_);
    if (dec.rotate_in_place) {
      // 跳过 MPPI: 直接输出原地旋转指令, 返回
      geometry_msgs::msg::TwistStamped cmd;
      cmd.header.frame_id = params_.use_global_mode ? "odom" : "BASE_LINK";
      cmd.header.stamp = node_.lock()->now();
      cmd.twist.linear.x = 0.0;
      cmd.twist.linear.y = 0.0;
      cmd.twist.angular.z = dec.omega_sign * params_.max_w;  // 原地旋转, omega=±max_w
      return cmd;
    }
  }

  // ── ③ CPU 纯零均值噪声 + guidance 混合 rollout ──
  noise_gen_->generate(N, H, lookahead_yaw, yaw);
  auto batch = batch_rollout(start, base_seq_, *noise_gen_, params_, N, H);

  // ── ④ ⑤ 上传 ──
  cudaStream_t stream;
  cudaStreamCreate(&stream);

  pipeline_->uploadBase(base_seq_, H, stream);
  pipeline_->uploadRollout(batch, N, H, stream);

  // ── ⑥ 上传路径到 GPU ──
  {
    std::vector<float> px, py;
    path_mgr_.extractPathArrays(px, py);
    gpu_uploader_->uploadPath(px.data(), py.data(),
                              static_cast<int>(px.size()), stream);
    path_info.x = static_cast<const float *>(gpu_engine_->getDevicePtr(buf::path_x));
    path_info.y = static_cast<const float *>(gpu_engine_->getDevicePtr(buf::path_y));
  }

  // ── ⑦ CostmapInfo + Footprint ──
  auto *cm = costmap_ros_->getCostmap();
  CostmapInfo cmap;
  cmap.data     = cm->getCharMap();
  cmap.w        = cm->getSizeInCellsX();
  cmap.h        = cm->getSizeInCellsY();
  cmap.res      = cm->getResolution();
  cmap.origin_x = cm->getOriginX();
  cmap.origin_y = cm->getOriginY();

  Footprint fp;

  // ── ⑧ GPU 代价 ──
  auto costs = pipeline_->launchCost(cmap, fp, path_info, goal_info, N, H, stream);

  // ── ⑨ 找 min + best_idx ──
  auto min_it = std::min_element(costs.begin(), costs.end());
  float min_cost = *min_it;
  int best_idx = static_cast<int>(std::distance(costs.begin(), min_it));

  // ── ⑩ GPU 加权 ──
  auto result = pipeline_->launchWeightedSum(min_cost,
      static_cast<float>(params_.lambda), N, H, stream);

  cudaStreamDestroy(stream);

  // ── ⑪ 控制量后处理: 提取 + clamp + (δ→ω 或 TF 旋转) ──
  auto proc = vel_postprocessor_->process(result, yaw, params_.use_global_mode);

  // ── vis: 发布 RViz markers ──
  Control vis_cmd = proc.control;
  vis_cmd.vx = proc.vx_out;  // odom 系 (已旋转)
  vis_cmd.vy = proc.vy_out;
  vis_pub_.publish(start, lp, batch, costs, best_idx, N, H,
                   vis_cmd, params_.use_global_mode);

  // ── ⑫ 更新 warm-start 基序列 (始终 body 系) ──
  base_seq_.shiftAndDecay(0.5);
  base_seq_.vx.back()    = proc.control.vx;
  base_seq_.vy.back()    = proc.control.vy;
  base_seq_.omega.back() = proc.control.omega;


  // ── ⑬ 填充 TwistStamped ──
  geometry_msgs::msg::TwistStamped cmd;
  cmd.header.frame_id = params_.use_global_mode ? "odom" : "BASE_LINK";
  cmd.header.stamp = node_.lock()->now();
  cmd.twist.linear.x  = proc.vx_out;
  cmd.twist.linear.y  = proc.vy_out;
  // global 模式: omega_out (已转 odom 系); base_link 模式: omega
  cmd.twist.angular.z = proc.omega_out;

  // 发布到独立话题, 与 Nav2 /cmd_vel 隔离
  cmd_vel_pub_->publish(cmd);

  // 同时发布自定义 VelocitySteering 消息
  if (steering_pub_) {
    msg::VelocitySteering vs;
    vs.vx = proc.control.vx;
    vs.vy = proc.control.vy;
    vs.steering_angle = proc.control.omega;
    steering_pub_->publish(vs);
  }

  return cmd;
}

}  // namespace nav2_custom_plugins_v2

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nav2_custom_plugins_v2::MPPISteeringController, nav2_core::Controller)
