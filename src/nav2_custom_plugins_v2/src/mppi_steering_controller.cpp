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
#include "nav2_custom_plugins_v2/modules/param_loader.hpp"

#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include "nav2_util/node_utils.hpp"
#include "tf2_ros/buffer.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

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
  ParamLoader loader;
  loader.configure(node, plugin_name_, params_);

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
  twist_pub_ = node->create_publisher<geometry_msgs::msg::Twist>(
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
  twist_pub_.reset();
}

void MPPISteeringController::activate()   {}
void MPPISteeringController::deactivate() {}

void MPPISteeringController::setPlan(const nav_msgs::msg::Path &path)
{
  global_plan_ = path;
  path_mgr_.setPath(path);
  state_machine_.reset();
  in_terminal_align_ = false;
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
    cmd.header.stamp = pose.header.stamp;
    if (params_.use_global_mode) {
      double yaw = 2.0 * atan2(pose.pose.orientation.z, pose.pose.orientation.w);
      cmd.twist.angular.z = yaw;  // global: angular.z = 目标朝向, 保持当前不转
    }
    twist_pub_->publish(cmd.twist);
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

  // ── ①b 每帧 tf 变换: 重定位后 map→odom 会变, 必须每帧用最新 tf 刷新路径坐标 ──
  if (costmap_ros_ && tf_) {
    std::string costmap_frame = costmap_ros_->getGlobalFrameID();
    if (!global_plan_.poses.empty() && !global_plan_.header.frame_id.empty()
        && global_plan_.header.frame_id != costmap_frame) {
      try {
        auto tf_stamped = tf_->lookupTransform(
            costmap_frame, global_plan_.header.frame_id,
            tf2::TimePointZero, tf2::durationFromSec(0.1));
        nav_msgs::msg::Path transformed;
        transformed.header.frame_id = costmap_frame;
        transformed.header.stamp    = global_plan_.header.stamp;
        transformed.poses.reserve(global_plan_.poses.size());
        for (const auto &p : global_plan_.poses) {
          geometry_msgs::msg::PoseStamped tp = p;
          tf2::doTransform(p.pose, tp.pose, tf_stamped);
          tp.header.frame_id = costmap_frame;
          transformed.poses.push_back(tp);
        }
        path_mgr_.refreshTransform(transformed);
      } catch (const std::exception &) {
        // tf 查询失败时保持 path_mgr_ 当前状态, 不更新
      }
    }
  }

  // ── ② PathManager: 最近点 → 前瞻点 → 推荐朝向 ──
  double lookahead_yaw = yaw;
  PathInfo path_info;
  GoalInfo goal_info;
  LookaheadResult lp;  // 供后续可视化
  double dist_lh_to_goal = 0.0;

  if (path_mgr_.valid()) {
    int closest = path_mgr_.findClosestIndex(start.x, start.y);
    lp = path_mgr_.computeLookahead(closest, params_.min_lookahead_dist, start.x, start.y);

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
    path_mgr_.buildGoalInfo(goal_info, dir_to_lh - yaw,
                             params_.max_v, params_.max_vy);

    // ── 前瞻点数据: kernel 越界惩罚 + rollout 到达检测 ──
    goal_info.lookahead_x   = static_cast<float>(lp.wx);
    goal_info.lookahead_y   = static_cast<float>(lp.wy);
    goal_info.lookahead_overshoot_weight = static_cast<float>(params_.lookahead_overshoot_weight);

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
      if (params_.enable_file_log) {
        std::ofstream flog(params_.log_file_path, std::ios::app);
        flog << std::fixed << std::setprecision(4)
             << "rotate: yaw=" << yaw << " lh_yaw=" << lookahead_yaw
             << " err=" << (lookahead_yaw - yaw)
             << " dist2goal=" << dist_lh_to_goal
             << " target=" << dec.omega << " sign=" << dec.omega_sign
             << std::endl;
      }
      geometry_msgs::msg::TwistStamped cmd;
      cmd.header.frame_id = params_.use_global_mode ? "odom" : "BASE_LINK";
      cmd.header.stamp = pose.header.stamp;
      cmd.twist.linear.x = 0.0;
      cmd.twist.linear.y = 0.0;
      // global: 目标角度 wrap 到 [0, 2π); base_link: 角速度 ±max_w
      if (params_.use_global_mode) {
        cmd.twist.angular.z = std::fmod(dec.omega, 2.0 * M_PI);
        if (cmd.twist.angular.z < 0.0) cmd.twist.angular.z += 2.0 * M_PI;
      } else {
        cmd.twist.angular.z = dec.omega_sign * params_.max_w;
      }
      twist_pub_->publish(cmd.twist);

      // ── vis: heading 模式下也发布前瞻点 + 机器人位姿, 跳过 MPPI 轨迹 ──
      {
        Control vis_cmd;
        vis_cmd.vx = 0.0; vis_cmd.vy = 0.0;
        vis_cmd.omega = (params_.use_global_mode)
            ? static_cast<double>(cmd.twist.angular.z)
            : dec.omega_sign * params_.max_w;
        BatchTrajectories empty_batch;
        std::vector<float> empty_costs;
        vis_pub_.publish(start, lp, empty_batch, empty_costs, -1, N, H,
                         vis_cmd, params_.use_global_mode, "odom", true);
      }

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

  Footprint fp = ParamLoader::buildFootprint(params_);
  CriticParams critic_params = ParamLoader::buildCriticParams(params_);

  // ── ⑧ GPU 代价 ──
  auto costs = pipeline_->launchCost(cmap, fp, path_info, goal_info,
                                     critic_params, N, H, stream);

  // ── ⑨ 找 min + best_idx ──
  auto min_it = std::min_element(costs.begin(), costs.end());
  float min_cost = *min_it;
  int best_idx = static_cast<int>(std::distance(costs.begin(), min_it));

  // ── debug: CPU 侧计算 best 轨迹 step0 的各代价分量, 写入 log 文件 ──
  if (params_.enable_file_log) {
    int i0 = best_idx * H;
    float bx = batch.x[i0], by = batch.y[i0], bth = batch.theta[i0];
    float bvx = batch.vx[i0], bvy = batch.vy[i0], bomg = batch.omega[i0];

    // path_angle: normalize_angle(theta - path_tangent)
    float angle_err = bth - static_cast<float>(lookahead_yaw);
    while (angle_err > M_PI)  angle_err -= 2.0f * M_PI;
    while (angle_err < -M_PI) angle_err += 2.0f * M_PI;
    float angle_raw = 4.0f * angle_err * angle_err;
    float angle_w  = critic_params.tracking_ratio * critic_params.path_angle_weight * angle_raw;

    // path_align: min distance to path (simplified: dist to lookahead point)
    float dx_lh = bx - static_cast<float>(lp.wx);
    float dy_lh = by - static_cast<float>(lp.wy);
    float align_raw = dx_lh * dx_lh + dy_lh * dy_lh;
    float align_w  = critic_params.tracking_ratio * critic_params.path_align_weight * align_raw;

    // speed: THEMIS — 对齐奖励 (reward_speed cap) + 侧向重罚 (×6)
    float spd = hypotf(bvx, bvy);
    float mfv = goal_info.max_feasible_v;
    float speed_raw, speed_w, reward_spd, al, lt;
    if (spd < 0.02f) {
      speed_raw = 0.0f; speed_w = 0.0f; reward_spd = 0.0f;
      al = 1.0f; lt = 0.0f;
    } else {
      al  = (bvx * goal_info.target_vx_r + bvy * goal_info.target_vy_r) / spd;
      lt  = fabsf(bvx * goal_info.target_vy_r - bvy * goal_info.target_vx_r) / spd;
      reward_spd = fminf(spd, mfv);
      if (al < 0.0f) {
        speed_raw = spd * 5.0f;
      } else {
        speed_raw = -reward_spd * al + 6.0f * spd * lt;
      }
      speed_w = critic_params.speed_ratio * critic_params.speed_reward_weight * speed_raw;
    }

    // obstacle: simplified — sample costmap at best trajectory step0 position
    auto *cm = costmap_ros_->getCostmap();
    unsigned char cost_val = cm->getCost(
        static_cast<unsigned int>((bx - cm->getOriginX()) / cm->getResolution()),
        static_cast<unsigned int>((by - cm->getOriginY()) / cm->getResolution()));
    float obst_raw = cost_val / 255.0f;
    float obst_w = critic_params.obstacle_ratio * critic_params.footprint_weight * obst_raw;

    std::ofstream flog(params_.log_file_path, std::ios::app);
    flog << std::fixed << std::setprecision(4)
         << "cost: total=" << min_cost
         << " | angle=" << angle_raw << "(w=" << angle_w << ")"
         << " align=" << align_raw << "(w=" << align_w << ")"
         << " speed=" << speed_raw << "(w=" << speed_w << ")"
         << " obst=" << obst_raw << "(w=" << obst_w << ")"
         << " | spd=" << spd << " mfv=" << mfv << " rspd=" << reward_spd
         << " al=" << al << " lt=" << lt
         << " | yaw=" << yaw << " lh_yaw=" << lookahead_yaw
         << " err=" << angle_err
         << " dist2goal=" << dist_lh_to_goal
         << " vx=" << bvx << " vy=" << bvy << " omg=" << bomg
         << std::endl;
  }

  // ── ⑩ GPU 加权 ──
  auto result = pipeline_->launchWeightedSum(min_cost,
      static_cast<float>(params_.lambda), N, H, stream);

  cudaStreamDestroy(stream);

  // ── ⑪ 控制量后处理: 提取 + clamp + (δ→ω 或 TF 旋转) ──
  auto proc = vel_postprocessor_->process(result, yaw, params_.use_global_mode);

  // ── global 模式: 目标角度直接发前瞻点推荐朝向, 不走 MPPI omega*dt 增量 ──
  if (params_.use_global_mode) {
    proc.omega_out = std::fmod(lookahead_yaw, 2.0 * M_PI);
    if (proc.omega_out < 0.0) proc.omega_out += 2.0 * M_PI;
  }

  // ── 前瞻点距离减速 (后处理, vx/vy 线性缩放, omega 不受影响) ──
  // 1:1 THEMIS: scale = kp + (1-kp) × min(1.0, dist/decel_dist)
  {
    double dist_to_lh = std::hypot(lp.wx - start.x, lp.wy - start.y);
    if (dist_to_lh < params_.lookahead_decel_dist) {
      double ratio = dist_to_lh / params_.lookahead_decel_dist;
      double scale = params_.lookahead_kp + (1.0 - params_.lookahead_kp) * ratio;
      proc.control.vx *= scale;
      proc.control.vy *= scale;
      proc.vx_out    *= scale;
      proc.vy_out    *= scale;
    }
  }

  // ── 终端朝向: 迟滞进入/退出, 直接发 goal_yaw 不通过 MPPI ──
  {
    double dist_to_final = std::hypot(goal_info.goal_x - start.x, goal_info.goal_y - start.y);
    double angle_err = path_info.goal_yaw - static_cast<float>(yaw);
    while (angle_err > M_PI)  angle_err -= 2.0 * M_PI;
    while (angle_err < -M_PI) angle_err += 2.0 * M_PI;

    // 进入条件
    if (!in_terminal_align_ && dist_to_final < params_.terminal_angle_dist
        && std::abs(angle_err) > params_.terminal_angle_tolerance) {
      in_terminal_align_ = true;
    }
    // 退出条件: 对齐完成 或 距离超迟滞边界
    if (in_terminal_align_) {
      if (std::abs(angle_err) <= params_.terminal_angle_tolerance ||
          dist_to_final > params_.terminal_angle_dist * 2.0) {
        in_terminal_align_ = false;
      }
    }
    // 终端对齐模式下直接发 goal_yaw
    if (in_terminal_align_ && params_.use_global_mode) {
      proc.omega_out = std::fmod(path_info.goal_yaw, 2.0 * M_PI);
      if (proc.omega_out < 0.0) proc.omega_out += 2.0 * M_PI;
    }
  }

  // ── debug: 输出实际 omega vs MPPI 采样 omega ──
  if (params_.enable_file_log) {
    std::ofstream flog(params_.log_file_path, std::ios::app);
    flog << std::fixed << std::setprecision(4)
         << "output: ctrl_omg=" << proc.control.omega
         << " out_omg=" << proc.omega_out
         << " ctrl_vx=" << proc.control.vx
         << " ctrl_vy=" << proc.control.vy
         << " global=" << params_.use_global_mode
         << std::endl;
  }

  // ── vis: 发布 RViz markers ──
  Control vis_cmd = proc.control;
  vis_cmd.vx    = proc.vx_out;  // global=odom 系, base_link=body 系
  vis_cmd.vy    = proc.vy_out;
  vis_cmd.omega = proc.omega_out;  // global=目标角度, base_link=角速度
  vis_pub_.publish(start, lp, batch, costs, best_idx, N, H,
                   vis_cmd, params_.use_global_mode,
                   "odom");

  // ── ⑫ 更新 warm-start 基序列 (始终 body 系) ──
  base_seq_.shiftAndDecay(0.5);
  base_seq_.vx.back()    = proc.control.vx;
  base_seq_.vy.back()    = proc.control.vy;
  base_seq_.omega.back() = proc.control.omega;


  // ── ⑬ 填充 TwistStamped ──
  geometry_msgs::msg::TwistStamped cmd;
  cmd.header.frame_id = params_.use_global_mode ? "odom" : "BASE_LINK";
  cmd.header.stamp = pose.header.stamp;
  cmd.twist.linear.x  = proc.vx_out;
  cmd.twist.linear.y  = proc.vy_out;
  // global: omega_out=目标角度; base_link: omega_out=角速度
  cmd.twist.angular.z = proc.omega_out;

  twist_pub_->publish(cmd.twist);

  return cmd;
}

}  // namespace nav2_custom_plugins_v2

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nav2_custom_plugins_v2::MPPISteeringController, nav2_core::Controller)
