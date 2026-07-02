#include "nav2_custom_plugins/mppi_gpu_controller.hpp"
#include <limits>
#include <cmath>
#include <algorithm>
#include <random>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <cuda_runtime.h>
#include "tf2/utils.h"

// CUDA host wrapper 函数声明（实现在 mppi_gpu_kernels.cu 中）
extern "C" {
int mppi_gpu_sample_and_cost(
    const float* noise_vx, const float* noise_vy, const float* noise_w,
    const float* base_vx, const float* base_vy, const float* base_w,
    float current_x, float current_y, float current_theta,
    float target_x, float target_y,
    const unsigned char* costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float dt, float min_v, float max_v, float max_vy, float max_w,
    float obstacle_weight, float tracking_weight, float progress_weight,
    float path_dir_x, float path_dir_y, float guidance_weight,
    float cross_track_noise_scale, float noise_decay_rate,
    float exploration_range_scale, float spatial_decay_weight,
    float noise_scale_floor_vx, float noise_scale_floor_vy, float noise_scale_floor_w,
    float pure_rotation_ratio, int pure_rotation_steps, float pure_rotation_w_boost,
    float lateral_guidance_scale, float path_turn_angle,
    float turn_lateral_boost, float turn_lateral_max_boost,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float footprint_sample_spacing, float rear_obstacle_cost,
    const float* path_x, const float* path_y, int num_path_pts,
    float goal_yaw,
    float final_goal_x, float final_goal_y,
    int global_horizon, int num_global_trajs,
    int num_samples, int horizon,
    float* d_noise_vx, float* d_noise_vy, float* d_noise_w,
    float* d_base_vx, float* d_base_vy, float* d_base_w,
    unsigned char* d_costmap, float* d_path_x, float* d_path_y,
    float* d_costs, float* d_sampled_vx, float* d_sampled_vy, float* d_sampled_w,
    float* d_traj_x, float* d_traj_y,
    cudaStream_t stream);

int mppi_gpu_weighted_sum(
    const float* d_costs,
    const float* d_sampled_vx, const float* d_sampled_vy, const float* d_sampled_w,
    float* d_result_seq,
    float min_cost,
    float lambda,
    int num_samples,
    int horizon,
    cudaStream_t stream);
}

// ── CUDA / ROS 参数封装宏，消除重复样板代码 ──

/// cudaMalloc + 自动错误检查（依赖上下文中的 node_）
#define CUDA_MALLOC_CHECK(ptr, size, name)                                          \
  do {                                                                              \
    cudaError_t err_ = cudaMalloc(&(ptr), (size));                                  \
    if (err_ != cudaSuccess) {                                                      \
      RCLCPP_ERROR(node_.lock()->get_logger(),                                      \
        "cudaMalloc(%s) 失败: %s", name, cudaGetErrorString(err_));                 \
    }                                                                               \
  } while (0)

/// cudaFree + 安全置空
#define CUDA_FREE_NULL(ptr)                                                         \
  do {                                                                              \
    cudaFree(ptr);                                                                  \
    (ptr) = nullptr;                                                                \
  } while (0)

/// declare_parameter + get_parameter 合并调用
#define DECLARE_GET_PARAM(node_ptr, prefix, name, var)                              \
  do {                                                                              \
    (node_ptr)->declare_parameter((prefix) + "." + (name), (var));                  \
    (node_ptr)->get_parameter((prefix) + "." + (name), (var));                      \
  } while (0)

namespace nav2_custom_plugins
{

void MPPIGPUController::allocateGPUBuffers()
{
  if (gpu_buffers_allocated_) {
    RCLCPP_WARN(node_.lock()->get_logger(), "GPU 缓冲区已分配，跳过重复分配");
    return;
  }

  int N = num_samples_;
  int H = prediction_horizon_;

  // N×H 噪声 & 轨迹缓冲区
  CUDA_MALLOC_CHECK(d_noise_vx_,   N * H * sizeof(float), "d_noise_vx");
  CUDA_MALLOC_CHECK(d_noise_vy_,   N * H * sizeof(float), "d_noise_vy");
  CUDA_MALLOC_CHECK(d_noise_w_,    N * H * sizeof(float), "d_noise_w");
  CUDA_MALLOC_CHECK(d_sampled_vx_, N * H * sizeof(float), "d_sampled_vx");
  CUDA_MALLOC_CHECK(d_sampled_vy_, N * H * sizeof(float), "d_sampled_vy");
  CUDA_MALLOC_CHECK(d_sampled_w_,  N * H * sizeof(float), "d_sampled_w");
  CUDA_MALLOC_CHECK(d_traj_x_,     N * H * sizeof(float), "d_traj_x");
  CUDA_MALLOC_CHECK(d_traj_y_,     N * H * sizeof(float), "d_traj_y");

  // H 维 base 序列
  CUDA_MALLOC_CHECK(d_base_vx_, H * sizeof(float), "d_base_vx");
  CUDA_MALLOC_CHECK(d_base_vy_, H * sizeof(float), "d_base_vy");
  CUDA_MALLOC_CHECK(d_base_w_,  H * sizeof(float), "d_base_w");

  // N 维代价 & H×4 加权结果
  CUDA_MALLOC_CHECK(d_costs_,      N * sizeof(float),       "d_costs");
  CUDA_MALLOC_CHECK(d_result_seq_, H * 4 * sizeof(float),   "d_result_seq");

  // 路径缓存（固定最大点数）
  CUDA_MALLOC_CHECK(d_path_x_,    MAX_PATH_POINTS * sizeof(float), "d_path_x");
  CUDA_MALLOC_CHECK(d_path_y_,    MAX_PATH_POINTS * sizeof(float), "d_path_y");

  // costmap 显存按需分配
  d_costmap_ = nullptr;
  costmap_w_ = 0;
  costmap_h_ = 0;

  gpu_buffers_allocated_ = true;
}

void MPPIGPUController::freeGPUBuffers()
{
  if (!gpu_buffers_allocated_) return;

  CUDA_FREE_NULL(d_noise_vx_);
  CUDA_FREE_NULL(d_noise_vy_);
  CUDA_FREE_NULL(d_noise_w_);
  CUDA_FREE_NULL(d_base_vx_);
  CUDA_FREE_NULL(d_base_vy_);
  CUDA_FREE_NULL(d_base_w_);
  CUDA_FREE_NULL(d_costs_);
  CUDA_FREE_NULL(d_sampled_vx_);
  CUDA_FREE_NULL(d_sampled_vy_);
  CUDA_FREE_NULL(d_sampled_w_);
  CUDA_FREE_NULL(d_result_seq_);
  CUDA_FREE_NULL(d_traj_x_);
  CUDA_FREE_NULL(d_traj_y_);
  CUDA_FREE_NULL(d_path_x_);
  CUDA_FREE_NULL(d_path_y_);

  CUDA_FREE_NULL(d_costmap_);
  costmap_w_ = 0;
  costmap_h_ = 0;

  gpu_buffers_allocated_ = false;
}

void MPPIGPUController::configure(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
  std::string name,
  std::shared_ptr<tf2_ros::Buffer> tf,
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros)
{
  node_ = parent;
  plugin_name_ = name;
  tf_ = tf;
  costmap_ros_ = costmap_ros;

  auto node_ptr = node_.lock();

  // ── 声明并加载所有可配置参数 ──
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lookahead_time",               lookahead_time_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "min_lookahead_dist",           min_lookahead_dist_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "guidance_weight",              guidance_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "cross_track_noise_scale",      cross_track_noise_scale_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "noise_decay_rate",             noise_decay_rate_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lateral_guidance_scale",       lateral_guidance_scale_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "turn_lateral_boost",           turn_lateral_boost_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "ema_alpha",                    ema_alpha_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "enable_ema",                   enable_ema_);
  // 探索范围距离衰减参数
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "exploration_decay_start",  exploration_decay_start_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "exploration_decay_end",    exploration_decay_end_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "exploration_decay_floor",  exploration_decay_floor_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "spatial_decay_weight",     spatial_decay_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "noise_scale_floor_vx",     noise_scale_floor_vx_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "noise_scale_floor_vy",     noise_scale_floor_vy_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "noise_scale_floor_w",      noise_scale_floor_w_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "pure_rotation_ratio",       pure_rotation_ratio_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "pure_rotation_steps",       pure_rotation_steps_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "pure_rotation_w_boost",     pure_rotation_w_boost_);

  // 碰撞箱尺寸
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "footprint_front",              footprint_front_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "footprint_back",               footprint_back_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "footprint_left",               footprint_left_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "footprint_right",              footprint_right_);

  // 终端角度对准：靠近目标时退化 MPPI 为纯角度追踪
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "terminal_angle_dist",          terminal_angle_dist_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "terminal_angle_kp",            terminal_angle_kp_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "terminal_angle_tolerance",     terminal_angle_tolerance_);

  // 前瞻点 KP 减速
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lookahead_kp",                 lookahead_kp_);

  // 朝向偏差限速
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "enable_heading_speed_limit",   enable_heading_speed_limit_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "heading_misalign_threshold",   heading_misalign_threshold_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "heading_misalign_max_speed",   heading_misalign_max_speed_);

  // 前瞻点朝向变化率限制 (rad/s)，0=禁用限制
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lookahead_theta_rate",        lookahead_theta_rate_);

  // 核心 MPPI 参数
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "num_samples",                  num_samples_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "prediction_horizon",           prediction_horizon_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "dt",                           dt_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "max_v",                        max_v_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "min_v",                        min_v_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "max_vy",                       max_vy_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "max_w",                        max_w_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "action_std_v",                 action_std_v_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "action_std_vy",                action_std_vy_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "action_std_w",                 action_std_w_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lambda",                       lambda_);
  // ── 三组件代价权重 ──
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "obstacle_weight",              obstacle_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "heading_weight",               heading_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "time_weight",                  time_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "enable_lateral_bias",          enable_lateral_bias_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "nln_ratio",                    nln_ratio_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "nln_sigma_mult",               nln_sigma_mult_);

  // ── 障碍物代价内部参数 ──
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "rear_obstacle_cost",           rear_obstacle_cost_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "footprint_sample_spacing",     footprint_sample_spacing_);

  // ── 行为参数 ──
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "cost_discount",                 cost_discount_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "turn_lateral_max_boost",       turn_lateral_max_boost_);

  // ── 分层规划参数 ──
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "global_trajectory_ratio",      global_trajectory_ratio_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "global_horizon",               global_horizon_);

  // 全局代价地图订阅参数
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "use_global_costmap",           use_global_costmap_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "global_costmap_topic",         global_costmap_topic_);

  if (use_global_costmap_) {
    global_costmap_sub_ = node_ptr->create_subscription<nav_msgs::msg::OccupancyGrid>(
      global_costmap_topic_, rclcpp::SystemDefaultsQoS(),
      std::bind(&MPPIGPUController::globalCostmapCallback, this, std::placeholders::_1));
    RCLCPP_INFO(node_.lock()->get_logger(),
      "已订阅全局代价地图: %s", global_costmap_topic_.c_str());
  }

  // 统计数据采集参数
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "enable_stats",                 enable_stats_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "stats_file_path",              stats_file_path_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "mutation_thresh_vx",           mutation_thresh_vx_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "mutation_thresh_vy",           mutation_thresh_vy_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "mutation_thresh_w",            mutation_thresh_w_);

  if (enable_stats_) {
    stats_start_time_ = node_ptr->now().seconds();
    RCLCPP_INFO(node_.lock()->get_logger(),
      "统计数据已启用，输出文件: %s", stats_file_path_.c_str());
  }

  vis_pub_ = node_ptr->create_publisher<visualization_msgs::msg::MarkerArray>(
    "/mppi_gpu_visualization", 10);

  optimal_vx_seq_.resize(prediction_horizon_, 0.0);
  optimal_vy_seq_.resize(prediction_horizon_, 0.0);
  optimal_omega_seq_.resize(prediction_horizon_, 0.0);

  std::random_device rd;
  generator_ = std::mt19937(rd());
  dist_vx_ = std::normal_distribution<>(0.0, action_std_v_);
  dist_vy_ = std::normal_distribution<>(0.0, action_std_vy_);
  dist_w_  = std::normal_distribution<>(0.0, action_std_w_);
  // NLN 混合采样: 对数正态 μ=0, σ=action_std * sigma_mult
  dist_ln_vx_ = std::lognormal_distribution<>(0.0, action_std_v_ * nln_sigma_mult_);
  dist_ln_vy_ = std::lognormal_distribution<>(0.0, action_std_vy_ * nln_sigma_mult_);
  dist_ln_w_  = std::lognormal_distribution<>(0.0, action_std_w_ * nln_sigma_mult_);

  DECLARE_GET_PARAM(node_ptr, plugin_name_, "enable_file_log",   enable_file_log_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "log_file_path",     log_file_path_);

  if (enable_file_log_) {
    std::ostringstream ps;
    ps << std::fixed << std::setprecision(4)
       << "samples=" << num_samples_ << " horizon=" << prediction_horizon_
       << " max_v=" << max_v_ << " max_vy=" << max_vy_
       << " obs_w=" << obstacle_weight_ << " head_w=" << heading_weight_
       << " time_w=" << time_weight_ << " guidance_w=" << guidance_weight_;
    if (logger_.open(log_file_path_, ps.str())) {
      RCLCPP_INFO(node_.lock()->get_logger(), "日志已启用: %s", log_file_path_.c_str());
    }
  }

  allocateGPUBuffers();

  RCLCPP_INFO(node_.lock()->get_logger(), "配置 MPPIGPUController (GPU加速) 成功！");
}

void MPPIGPUController::cleanup()
{
  RCLCPP_INFO(node_.lock()->get_logger(), "清理 MPPIGPUController");
  logger_.close();
  if (narrow_diag_log_.is_open()) narrow_diag_log_.close();
  writeStatsToFile();
  freeGPUBuffers();
}

void MPPIGPUController::activate()
{
  RCLCPP_INFO(node_.lock()->get_logger(), "激活 MPPIGPUController");
  terminal_angle_active_ = false;
}

void MPPIGPUController::deactivate()
{
  RCLCPP_INFO(node_.lock()->get_logger(), "停用 MPPIGPUController");
}

void MPPIGPUController::globalCostmapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
  latest_global_costmap_ = msg;
}

void MPPIGPUController::setPlan(const nav_msgs::msg::Path & path)
{
  // 仅在 goal 显著变化 (>0.1m) 时重置横向锁和终端状态
  if (!global_plan_.poses.empty() && !path.poses.empty()) {
    double old_gx = global_plan_.poses.back().pose.position.x;
    double old_gy = global_plan_.poses.back().pose.position.y;
    double new_gx = path.poses.back().pose.position.x;
    double new_gy = path.poses.back().pose.position.y;
    if (std::hypot(new_gx - old_gx, new_gy - old_gy) > 0.10) {
      terminal_angle_active_ = false;
      lateral_locked_ = false; lateral_target_x_ = 0; lateral_target_y_ = 0;
    }
  }
  global_plan_ = path;
  hesitate_count_ = 0;
}

void MPPIGPUController::setSpeedLimit(const double &, const bool &)
{
}

void MPPIGPUController::narrowDiagHdr() {
  narrow_diag_log_.open("/tmp/narrow_diag.csv", std::ios::out|std::ios::trunc);
  if (narrow_diag_log_.is_open()) narrow_diag_log_ << "t,lh_x,lh_y,moved,at_end,stuck,state,rx,ry,gx,gy,df,near,vspeed,lh_cost\n";
}
void MPPIGPUController::narrowDiagRow(double t, double lh_wx, double lh_wy, double moved, bool at_end, double stuck_dur, ControllerState s, double rx, double ry, double gx, double gy, double df, int near, int lh_cost) {
  if (!narrow_diag_log_.is_open()) return;
  narrow_diag_log_ << std::fixed << std::setprecision(4) << t << "," << lh_wx << "," << lh_wy << "," << moved << "," << (at_end?1:0) << "," << stuck_dur << "," << (int)s << "," << rx << "," << ry << "," << gx << "," << gy << "," << df << "," << near << "," << narrow_verify_speed_ << "," << lh_cost << "\n";
  narrow_diag_log_.flush();
}

ControllerState MPPIGPUController::determineState(
  double lh_wx, double lh_wy, double goal_wx, double goal_wy,
  bool near_goal, double heading_err, bool at_end, bool all_at_end,
  bool mppi_hesitate, double now,
  double cur_x, double cur_y)
{
  // ── 任务锁: 特殊状态未完成任务前不允许切换到其他状态 ──
  // TERMINAL_ALIGN: 三点不全在终点时退出, 回到 MPPI 正常导航
  if (prev_state_ == ControllerState::TERMINAL_ALIGN && all_at_end) {
    return ControllerState::TERMINAL_ALIGN;
  }
  // HEADING_MISALIGN: 必须转到 heading_err <= threshold 才算完成
  if (prev_state_ == ControllerState::HEADING_MISALIGN &&
      std::abs(heading_err) > heading_misalign_threshold_ * 0.5) {
    return ControllerState::HEADING_MISALIGN;
  }
  // NARROW_PASSAGE: 框体锁定后 (ALIGN/ADVANCE 执行中) 不允许其他状态介入
  // SEARCH_BOX 搜索阶段不锁, 搜索失败可自然退回 NORMAL
  if (prev_state_ == ControllerState::NARROW_PASSAGE && narrow_box_locked_) {
    return ControllerState::NARROW_PASSAGE;
  }
  // LATERAL_SHIFT: 横向移动中, 不允许其他状态介入
  if (prev_state_ == ControllerState::LATERAL_SHIFT && lateral_locked_) {
    return ControllerState::LATERAL_SHIFT;
  }

  // ── 候选状态计算 (优先级: 终端 > 横向 > 朝向 > 窄道 > 正常) ──
  ControllerState candidate = ControllerState::NORMAL;
  // 1. 终端对准: 三个前瞻点都到终点 → 进入终端模式
  if (all_at_end) {
    candidate = ControllerState::TERMINAL_ALIGN;
  }
  // 2. 横向移动
  else if (std::abs(heading_err) > heading_misalign_threshold_ &&
           std::abs(heading_err) < 1.92 &&
           std::hypot(goal_wx - lh_wx, goal_wy - lh_wy) < 1.0) { candidate = ControllerState::LATERAL_SHIFT; }
  // 3. 朝向修正
  else if (std::abs(heading_err) > heading_misalign_threshold_ &&
           std::hypot(goal_wx - lh_wx, goal_wy - lh_wy) >= 0.5 &&
           (terminal_exit_time_ < 0 || now - terminal_exit_time_ > 0.5)) { candidate = ControllerState::HEADING_MISALIGN; }
  // 4. 窄道迂回 (at_end=1 时也允许 — 目标在障碍中走不过去)
  else if (mppi_hesitate) { candidate = ControllerState::NARROW_PASSAGE; }
  // 5. 正常 MPPI
  else { candidate = ControllerState::NORMAL; }

  // ── 状态间保护: 特殊情况之间不允许直接跳转, 必须先回 NORMAL ──
  // 防止 HEADING ↔ NARROW 振荡: 从特殊状态A只能去 NORMAL 或回到 A (任务锁)
  if (candidate != ControllerState::NORMAL && prev_state_ != ControllerState::NORMAL && candidate != prev_state_) {
    return ControllerState::NORMAL;
  }
  return candidate;
}

geometry_msgs::msg::TwistStamped MPPIGPUController::computeVelocityCommands(
  const geometry_msgs::msg::PoseStamped & pose,
  const geometry_msgs::msg::Twist & /*current_vel*/,
  nav2_core::GoalChecker *)
{
  // ══════════════════════════════════════════════════════════════════
  // MPPI GPU 控制器 — 控制策略两层架构
  // ══════════════════════════════════════════════════════════════════
  //
  // 本控制器按"距目标远近"将控制分为两层，由远及近：
  //
  // ┌──────────────────────────────────────────────────────────────┐
  // │ 层级 1: MPPI (全程)                                          │
  // │   覆盖整个路径。GPU 并行采样 N 条轨迹，全时域加权求和，      │
  // │   输出平滑连续的控制序列。转弯时自动增强横向运动。           │
  // ├──────────────────────────────────────────────────────────────┤
  // │ 层级 2: 终端角度对准 (dist_to_final < terminal_angle_dist_)  │
  // │   距离目标极近 (<0.10m)，跳过 GPU，纯 P 控制器对准 goal yaw。│
  // │   朝向到位后完全停止。                                       │
  // └──────────────────────────────────────────────────────────────┘
  //
  // 关键设计原则:
  //   - MPPI 是唯一主力，全程负责运动控制
  //   - 不再退化到纯追踪回退 — 保持 MPPI 的避障能力直到终端
  //   - 终端对准是终点，纯旋转对齐 goal pose 朝向
  // ══════════════════════════════════════════════════════════════════
  geometry_msgs::msg::TwistStamped cmd_vel;
  cmd_vel.header.frame_id = "BASE_LINK";
  cmd_vel.header.stamp = node_.lock()->now();

  int N = num_samples_;
  int H = prediction_horizon_;

  double current_x = pose.pose.position.x;
  double current_y = pose.pose.position.y;
  double current_theta = tf2::getYaw(pose.pose.orientation);

  double target_x = current_x;
  double target_y = current_y;
  int closest_idx = 0;

  if (!global_plan_.poses.empty()) {
    double min_dist = std::numeric_limits<double>::max();

    // 增量式最近路径点搜索：从上一帧的 closest_idx 附近开始，
    // 避免全局搜索导致的索引跳变（全路径扫描可能跳到不同分支）。
    // 小窗口前向搜索，确保找到的最近点沿路径推进而非跳变。
    int search_start = std::max(0, prev_closest_idx_ - 5);
    for (size_t i = search_start; i < global_plan_.poses.size(); ++i) {
      double dx = global_plan_.poses[i].pose.position.x - current_x;
      double dy = global_plan_.poses[i].pose.position.y - current_y;
      double dist = std::hypot(dx, dy);
      if (dist < min_dist) {
        min_dist = dist;
        closest_idx = static_cast<int>(i);
      }
    }
    prev_closest_idx_ = closest_idx;

    // ── 终端角度对准模式（带迟滞状态）──
    // 进入后保持激活，防止在 0.10m 边界反复抖动。
    double final_gx = global_plan_.poses.back().pose.position.x;
    double final_gy = global_plan_.poses.back().pose.position.y;
    double dist_to_final = std::hypot(final_gx - current_x, final_gy - current_y);

    // ── 分层规划: 存储最终目标，供全局轨迹使用 ──
    final_goal_x_ = final_gx;
    final_goal_y_ = final_gy;
    if (!global_plan_.poses.empty()) {
      final_goal_yaw_ = tf2::getYaw(global_plan_.poses.back().pose.orientation);
    }

    // 终端对齐已由 determineState + TERMINAL_ALIGN 处理器接管
    // near_goal 时 lh_cost 清零, 使 MPPI 不受 inflation 排斥

    // 前瞻点：从 closest_idx 向后逐个扫描，遇障碍即停，不跳过面前障碍物。
    // 同时满足距离 ≥ min_lookahead_dist_ 且路径中间无障碍。
    int last_idx = static_cast<int>(global_plan_.poses.size()) - 1;
    int lh_idx = last_idx;

    // 先建立 costmap 查询 (后续障碍物感知也复用)
    const unsigned char* chk_cm = nullptr; int chk_w = 0, chk_h = 0;
    float chk_res = 0.05f, chk_ox = 0.0f, chk_oy = 0.0f;
    if (costmap_ros_ != nullptr) {
      auto* cm = costmap_ros_->getCostmap();
      if (cm != nullptr) {
        chk_cm = cm->getCharMap(); chk_w = static_cast<int>(cm->getSizeInCellsX());
        chk_h = static_cast<int>(cm->getSizeInCellsY());
        chk_res = static_cast<float>(cm->getResolution());
        chk_ox = static_cast<float>(cm->getOriginX());
        chk_oy = static_cast<float>(cm->getOriginY());
      }
    }
    auto cell_cost = [&](double wx, double wy) -> unsigned char {
      int mx = (wx - chk_ox) / chk_res, my = (wy - chk_oy) / chk_res;
      if (mx >= 0 && mx < chk_w && my >= 0 && my < chk_h)
        return chk_cm[my * chk_w + mx];
      return nav2_costmap_2d::LETHAL_OBSTACLE;
    };

    if (chk_cm != nullptr && chk_w > 0 && chk_h > 0) {
      // 逐点扫描: 遇到致命障碍物就停在前一个点
      int last_clear = closest_idx;
      bool found_lookahead = false;
      for (int i = closest_idx; i <= last_idx; ++i) {
        double px = global_plan_.poses[i].pose.position.x;
        double py = global_plan_.poses[i].pose.position.y;
        double dist = std::hypot(px - current_x, py - current_y);
        // 沿机器人→路径点连线采样检查
        bool path_blocked = false;
        if (dist > 0.05) {
          int n_chk = static_cast<int>(dist / 0.05);
          for (int k = 1; k <= n_chk && !path_blocked; ++k) {
            double t = static_cast<double>(k) / n_chk;
            double cx = current_x + (px - current_x) * t;
            double cy = current_y + (py - current_y) * t;
            if (cell_cost(cx, cy) >= nav2_costmap_2d::LETHAL_OBSTACLE)
              path_blocked = true;
          }
        }
        if (path_blocked) break;  // 遇到障碍, 停在 last_clear
        last_clear = i;
        if (dist >= min_lookahead_dist_) { lh_idx = i; found_lookahead = true; break; }
      }
      if (!found_lookahead) lh_idx = last_clear;  // 没达到 min_lookahead 就用最后一个安全点
    } else {
      // 无 costmap: 回退到纯距离选择
      for (int i = closest_idx; i <= last_idx; ++i) {
        double dx = global_plan_.poses[i].pose.position.x - current_x;
        double dy = global_plan_.poses[i].pose.position.y - current_y;
        if (std::hypot(dx, dy) >= min_lookahead_dist_) { lh_idx = i; break; }
      }
    }
    target_x = global_plan_.poses[lh_idx].pose.position.x;
    target_y = global_plan_.poses[lh_idx].pose.position.y;
  }

  // ══════════════════════════════════════════
  // 状态机
  // ══════════════════════════════════════════
  {
    double now_sec = node_.lock()->now().seconds();
    bool near_goal = std::hypot(final_goal_x_ - current_x, final_goal_y_ - current_y) < terminal_angle_dist_;
    double h_err = 0.0;
    if (!global_plan_.poses.empty() && closest_idx + 1 < static_cast<int>(global_plan_.poses.size())) {
      double pdx = global_plan_.poses[closest_idx+1].pose.position.x - global_plan_.poses[closest_idx].pose.position.x;
      double pdy = global_plan_.poses[closest_idx+1].pose.position.y - global_plan_.poses[closest_idx].pose.position.y;
      double nl = std::hypot(pdx, pdy); if (nl > 0.01) { pdx /= nl; pdy /= nl; }
      h_err = std::atan2(pdy, pdx) - current_theta;
      while (h_err > M_PI) h_err -= 2.0*M_PI; while (h_err < -M_PI) h_err += 2.0*M_PI;
    }
    bool at_end = false;
    // 三个前瞻点 (主0.4m + 虚拟0.2m/0.4m) 是否都到终点
    bool all_at_end = false;
    if (!global_plan_.poses.empty()) {
      double dx = target_x - final_goal_x_, dy = target_y - final_goal_y_;
      at_end = (dx*dx + dy*dy) < terminal_angle_dist_ * terminal_angle_dist_;
      // 剩余路径 < terminal_angle_dist_ → 三个前瞻点全部clamp到终点
      double path_rem = 0;
      for (int pi = closest_idx; pi + 1 < (int)global_plan_.poses.size(); ++pi) {
        double sx = global_plan_.poses[pi+1].pose.position.x - global_plan_.poses[pi].pose.position.x;
        double sy = global_plan_.poses[pi+1].pose.position.y - global_plan_.poses[pi].pose.position.y;
        path_rem += std::hypot(sx, sy);
      }
      all_at_end = (path_rem < terminal_angle_dist_);
    }
    // 双虚拟 MPPI: 前瞻点 + 前方10cm, 任一踌躇即触发
    bool mppi_hesitate = false;
    {
      auto* cm = costmap_ros_ ? costmap_ros_->getCostmap() : nullptr;
      if (cm && cm->getCharMap() && cm->getSizeInCellsX() > 0) {
        int cw = cm->getSizeInCellsX(), ch = cm->getSizeInCellsY();
        if (cw != costmap_w_ || ch != costmap_h_) {
          if (d_costmap_) { cudaFree(d_costmap_); d_costmap_ = nullptr; }
          cudaMalloc(&d_costmap_, cw * ch * sizeof(unsigned char));
          costmap_w_ = cw; costmap_h_ = ch;
        }
        const unsigned char* vd = cm->getCharMap();
        int vw = cw, vh = ch;
        float vr = cm->getResolution(), vox = cm->getOriginX(), voy = cm->getOriginY();
        double vtheta = h_err + current_theta;
        double pdx = std::cos(vtheta), pdy = std::sin(vtheta);
        const int VN = num_samples_; int VH = prediction_horizon_;
        std::vector<float> vn_vx(VN*VH), vn_vy(VN*VH), vn_w(VN*VH);
        std::vector<float> vb(VH,0), vc(VN), vres(VH*4);
        cudaStream_t vs; cudaStreamCreate(&vs);

        // ── 虚拟机器人位置判定碰撞的 lambda (复用 SEARCH_BOX 的朝向搜索逻辑) ──
        auto vfp_coll = [&](double cx, double cy, double ct) -> bool {
          double cc = std::cos(ct), ss = std::sin(ct);
          double fw = footprint_front_, bw = footprint_back_, lw = footprint_left_, rw = footprint_right_;
          // 足迹四个角
          double cx0 = cx - bw*cc + rw*ss, cy0 = cy - bw*ss - rw*cc;
          double cx1 = cx - bw*cc - lw*ss, cy1 = cy - bw*ss + lw*cc;
          double cx2 = cx + fw*cc - lw*ss, cy2 = cy + fw*ss + lw*cc;
          double cx3 = cx + fw*cc + rw*ss, cy3 = cy + fw*ss - rw*cc;
          double min_wx = std::min({cx0,cx1,cx2,cx3}), max_wx = std::max({cx0,cx1,cx2,cx3});
          double min_wy = std::min({cy0,cy1,cy2,cy3}), max_wy = std::max({cy0,cy1,cy2,cy3});
          int mx0 = (min_wx - vox) / vr, mx1 = (max_wx - vox) / vr;
          int my0 = (min_wy - voy) / vr, my1 = (max_wy - voy) / vr;
          if (mx0 < 0) mx0 = 0; if (mx1 >= vw) mx1 = vw - 1;
          if (my0 < 0) my0 = 0; if (my1 >= vh) my1 = vh - 1;
          if (mx0 > mx1 || my0 > my1) return true;  // 完全越界
          for (int my = my0; my <= my1; ++my) {
            for (int mx = mx0; mx <= mx1; ++mx) {
              double wx = vox + (mx + 0.5) * vr, wy = voy + (my + 0.5) * vr;
              double dx = wx - cx, dy = wy - cy;
              double lx = dx * cc + dy * ss, ly = -dx * ss + dy * cc;
              if (lx >= -bw && lx <= fw && ly >= -rw && ly <= lw) {
                if (vd[my * vw + mx] >= 253) return true;
              }
            }
          }
          return false;
        };
        // 搜索无碰朝向: 24 个采样, 优先朝向路径前方
        auto vsrch = [&](double cx, double cy, double path_tangent, double &ot) -> bool {
          double best_score = 1e9; bool found = false;
          for (int i = 0; i < 24; ++i) {
            double t = i * 2.0 * M_PI / 24.0;
            if (!vfp_coll(cx, cy, t)) {
              double d_cur = std::abs(t - current_theta);
              if (d_cur > M_PI) d_cur = 2.0 * M_PI - d_cur;
              double d_path = std::abs(t - path_tangent);
              if (d_path > M_PI) d_path = 2.0 * M_PI - d_path;
              double score = d_cur + 1.5 * d_path;
              if (score < best_score) { best_score = score; ot = t; found = true; }
            }
          }
          return found;
        };

        // 两个虚拟机器人位置: 沿全局路径, 从 closest_idx 起向前取样
        // 超过终点时 clamp 到路径末尾，与主前瞻点行为一致
        double vx_arr[2], vy_arr[2];
        {
          const double LOOKAHEAD_DIST[2] = {0.20, 0.40};
          int last_pose_idx = static_cast<int>(global_plan_.poses.size()) - 1;
          double gx_end = global_plan_.poses[last_pose_idx].pose.position.x;
          double gy_end = global_plan_.poses[last_pose_idx].pose.position.y;
          // 兜底: 剩余路径不足时 clamp 到终点
          vx_arr[0] = gx_end; vy_arr[0] = gy_end;
          vx_arr[1] = gx_end; vy_arr[1] = gy_end;
          for (int vi = 0; vi < 2; ++vi) {
            double cum = 0;
            for (int pi = closest_idx; pi + 1 < (int)global_plan_.poses.size(); ++pi) {
              double ax = global_plan_.poses[pi].pose.position.x;
              double ay = global_plan_.poses[pi].pose.position.y;
              double bx = global_plan_.poses[pi + 1].pose.position.x;
              double by = global_plan_.poses[pi + 1].pose.position.y;
              double seg = std::hypot(bx - ax, by - ay);
              if (cum + seg >= LOOKAHEAD_DIST[vi]) {
                double t = (LOOKAHEAD_DIST[vi] - cum) / seg;
                if (t < 0) t = 0; if (t > 1) t = 1;
                vx_arr[vi] = ax + t * (bx - ax);
                vy_arr[vi] = ay + t * (by - ay);
                break;
              }
              cum += seg;
            }
            // 路径剩余不足 LOOKAHEAD_DIST[vi]: 保留 fallback 值 (已 clamp 到终点)
          }
        }
        double proj_arr[2] = {1.0, 1.0};
        std::vector<double> vtheta_arr = {vtheta, vtheta};  // 记录每个虚拟机器人的实际起始朝向

        for (int vi = 0; vi < 2; ++vi) {
          // 搜索无碰朝向 (同 SEARCH_BOX 逻辑), 朝向前瞻点方向
          double search_theta;
          double vpath_tangent = atan2(target_y - vy_arr[vi], target_x - vx_arr[vi]);
          if (vsrch(vx_arr[vi], vy_arr[vi], vpath_tangent, search_theta)) {
            vtheta_arr[vi] = search_theta;
          }
          // 生成噪声
          for (int i = 0; i < VN * VH; ++i) {
            vn_vx[i] = static_cast<float>(dist_vx_(generator_));
            vn_vy[i] = static_cast<float>(dist_vy_(generator_));
            vn_w[i]  = static_cast<float>(dist_w_(generator_));
          }
          // 虚拟MPPI的路径方向: 从虚拟机器人位置指向 lookahead
          double vpdx = target_x - vx_arr[vi], vpdy = target_y - vy_arr[vi];
          double vpnl = std::hypot(vpdx, vpdy);
          if (vpnl > 0.01) { vpdx /= vpnl; vpdy /= vpnl; }
          else { vpdx = 1.0; vpdy = 0.0; }
          int ret = mppi_gpu_sample_and_cost(
            vn_vx.data(), vn_vy.data(), vn_w.data(), vb.data(), vb.data(), vb.data(),
            static_cast<float>(vx_arr[vi]), static_cast<float>(vy_arr[vi]), static_cast<float>(vtheta_arr[vi]),
            static_cast<float>(target_x), static_cast<float>(target_y),
            vd, vw, vh, vr, vox, voy,
            static_cast<float>(dt_), static_cast<float>(min_v_),
            static_cast<float>(max_v_), static_cast<float>(max_vy_), static_cast<float>(max_w_),
            static_cast<float>(obstacle_weight_), static_cast<float>(heading_weight_),
            static_cast<float>(time_weight_),
            static_cast<float>(vpdx), static_cast<float>(vpdy), static_cast<float>(guidance_weight_),
            static_cast<float>(cross_track_noise_scale_), static_cast<float>(noise_decay_rate_),
            1.0f, static_cast<float>(spatial_decay_weight_),
            static_cast<float>(noise_scale_floor_vx_), static_cast<float>(noise_scale_floor_vy_),
            static_cast<float>(noise_scale_floor_w_),
            0.0f, 0, 1.0f, static_cast<float>(lateral_guidance_scale_), 0.0f,
            static_cast<float>(turn_lateral_boost_), static_cast<float>(turn_lateral_max_boost_),
            static_cast<float>(footprint_front_), static_cast<float>(footprint_back_),
            static_cast<float>(footprint_left_), static_cast<float>(footprint_right_),
            static_cast<float>(footprint_sample_spacing_), static_cast<float>(rear_obstacle_cost_),
            nullptr, nullptr, 0, 0.0f, 0.0f, 0.0f, 0, 0, VN, VH,
            d_noise_vx_, d_noise_vy_, d_noise_w_, d_base_vx_, d_base_vy_, d_base_w_,
            d_costmap_, d_path_x_, d_path_y_,
            d_costs_, d_sampled_vx_, d_sampled_vy_, d_sampled_w_, d_traj_x_, d_traj_y_, vs);
          if (ret != 0) continue;
          cudaMemcpyAsync(vc.data(), d_costs_, VN * sizeof(float), cudaMemcpyDeviceToHost, vs);
          cudaStreamSynchronize(vs);
          float min_c = vc[0];
          for (int i = 1; i < VN; ++i) if (vc[i] < min_c) min_c = vc[i];
          ret = mppi_gpu_weighted_sum(d_costs_, d_sampled_vx_, d_sampled_vy_, d_sampled_w_,
            d_result_seq_, min_c, static_cast<float>(lambda_), VN, VH, vs);
          if (ret != 0) continue;
          cudaStreamSynchronize(vs);
          cudaMemcpy(vres.data(), d_result_seq_, VH * 4 * sizeof(float), cudaMemcpyDeviceToHost);
          double wvx = 0, wvy = 0, wsum = 0;
          for (int t = 0; t < std::min(3, VH); ++t) {
            float sw = vres[t * 4 + 3];
            if (sw > 1e-6f) { wvx += vres[t * 4 + 0] / sw; wvy += vres[t * 4 + 1] / sw; wsum += 1.0; }
          }
          if (wsum > 0) { wvx /= wsum; wvy /= wsum; }
          // 投影到"虚拟机器人→前瞻点"方向 (世界系)
          // 先将body系速度转到世界系, 再投影, 确保判定的是"朝目标前进"
          double world_wvx = wvx * std::cos(vtheta_arr[vi]) - wvy * std::sin(vtheta_arr[vi]);
          double world_wvy = wvx * std::sin(vtheta_arr[vi]) + wvy * std::cos(vtheta_arr[vi]);
          proj_arr[vi] = world_wvx * vpdx + world_wvy * vpdy;
        }
        cudaStreamDestroy(vs);

        if (proj_arr[0] < 0.10 && proj_arr[1] < 0.10) hesitate_count_++;
        else hesitate_count_ = 0;
        if (hesitate_count_ >= 3) mppi_hesitate = true;
        // 可视化两个虚拟机器人位置 (近点蓝色, 远点红色)
        if (vis_pub_) {
          visualization_msgs::msg::MarkerArray ma; auto now = node_.lock()->now();
          for (int vi = 0; vi < 2; ++vi) {
            visualization_msgs::msg::Marker m;
            m.header.frame_id = "odom"; m.header.stamp = now;
            m.ns = "virtual_mppi"; m.id = vi;
            m.type = m.SPHERE; m.action = m.ADD;
            m.pose.position.x = vx_arr[vi]; m.pose.position.y = vy_arr[vi]; m.pose.position.z = 0.06;
            m.scale.x = 0.04; m.scale.y = 0.04; m.scale.z = 0.04;
            m.color.a = 0.7f;
            m.color.r = (vi == 0) ? 0.0f : 1.0f;
            m.color.g = (vi == 0) ? 0.5f : 0.2f;
            m.color.b = (vi == 0) ? 1.0f : 0.0f;
            m.lifetime = rclcpp::Duration::from_seconds(0.15);
            ma.markers.push_back(m);
          }
          vis_pub_->publish(ma);
        }
      }
    }
    // 前瞻点碰撞箱检测: 以路径方向为朝向, 检查 footprint 覆盖的所有 cell
    // 单 cell 查询有盲区 — 前瞻点 cell=0 但 footprint 边角可能压在障碍上
    bool lookahead_blocked = false;
    int lh_cost = -1;  // footprint 内最高代价 (-1=无效)
    {
      auto* lcm = costmap_ros_ ? costmap_ros_->getCostmap() : nullptr;
      if (lcm && lcm->getCharMap() && lcm->getSizeInCellsX() > 0) {
        double lh_theta = 0.0;
        // 使用路径方向作为 footprint 朝向
        if (!global_plan_.poses.empty() && closest_idx + 1 < (int)global_plan_.poses.size()) {
          double pdx = global_plan_.poses[closest_idx+1].pose.position.x - global_plan_.poses[closest_idx].pose.position.x;
          double pdy = global_plan_.poses[closest_idx+1].pose.position.y - global_plan_.poses[closest_idx].pose.position.y;
          double nl = std::hypot(pdx, pdy); if (nl > 0.01) { pdx /= nl; pdy /= nl; }
          lh_theta = std::atan2(pdy, pdx);
        }
        double cc = std::cos(lh_theta), ss = std::sin(lh_theta);
        double fw = footprint_front_, bw = footprint_back_, lw = footprint_left_, rw = footprint_right_;
        // 足迹四个角的世界坐标
        double cx0 = target_x - bw*cc + rw*ss, cy0 = target_y - bw*ss - rw*cc;
        double cx1 = target_x - bw*cc - lw*ss, cy1 = target_y - bw*ss + lw*cc;
        double cx2 = target_x + fw*cc - lw*ss, cy2 = target_y + fw*ss + lw*cc;
        double cx3 = target_x + fw*cc + rw*ss, cy3 = target_y + fw*ss - rw*cc;
        double min_wx = std::min({cx0,cx1,cx2,cx3}), max_wx = std::max({cx0,cx1,cx2,cx3});
        double min_wy = std::min({cy0,cy1,cy2,cy3}), max_wy = std::max({cy0,cy1,cy2,cy3});
        int mx0 = (min_wx - lcm->getOriginX()) / lcm->getResolution();
        int mx1 = (max_wx - lcm->getOriginX()) / lcm->getResolution();
        int my0 = (min_wy - lcm->getOriginY()) / lcm->getResolution();
        int my1 = (max_wy - lcm->getOriginY()) / lcm->getResolution();
        int cw = (int)lcm->getSizeInCellsX(), ch = (int)lcm->getSizeInCellsY();
        if (mx0 < 0) mx0 = 0; if (mx1 >= cw) mx1 = cw - 1;
        if (my0 < 0) my0 = 0; if (my1 >= ch) my1 = ch - 1;
        if (mx0 > mx1 || my0 > my1) { lookahead_blocked = true; lh_cost = 255; }  // 完全越界
        else {
          lh_cost = 0;
          const unsigned char* cmap = lcm->getCharMap();
          for (int my = my0; my <= my1; ++my) {
            for (int mx = mx0; mx <= mx1; ++mx) {
              double wx = lcm->getOriginX() + (mx + 0.5) * lcm->getResolution();
              double wy = lcm->getOriginY() + (my + 0.5) * lcm->getResolution();
              double dx = wx - target_x, dy = wy - target_y;
              double lx = dx * cc + dy * ss, ly = -dx * ss + dy * cc;  // 转到机器人系
              if (lx >= -bw && lx <= fw && ly >= -rw && ly <= lw) {
                unsigned char cv = cmap[my * cw + mx];
                if (cv > (unsigned char)lh_cost) lh_cost = cv;
                if (cv >= 253) lookahead_blocked = true;
              }
            }
          }
        }
      }
    }
    // 终端对齐区域: 无视 inflation, 避免253代价干扰终端对准和goal checker
    if (near_goal) { lh_cost = 0; lookahead_blocked = false; }
    prev_state_ = state_;
    state_ = determineState(target_x, target_y, final_goal_x_, final_goal_y_,
                            near_goal, h_err, at_end, all_at_end,
                            mppi_hesitate && lookahead_blocked, now_sec,
                            current_x, current_y);
    { static bool diag_init = false; if (!diag_init) { narrowDiagHdr(); diag_init = true; } }
    narrowDiagRow(now_sec, target_x, target_y, 0.0, at_end, mppi_hesitate?1.0:0.0, state_, current_x, current_y, final_goal_x_, final_goal_y_, std::hypot(final_goal_x_-current_x, final_goal_y_-current_y), near_goal?1:0, lh_cost);
    // 锁
    if (state_ == ControllerState::LATERAL_SHIFT) {
      if (!lateral_locked_) lateral_lock_start_ = now_sec; lateral_locked_ = true;
    } else if (lateral_locked_) {
      // 锁维持: 只在超时保护下退出 (12s, 足够以 0.15m/s 走 1.8m)
      // 不再因 heading_err 变化而退出 — 到达目标点才是唯一正常出口
      if (now_sec - lateral_lock_start_ > 12.0)
        { lateral_locked_ = false; lateral_target_x_ = 0; lateral_target_y_ = 0; }
      else state_ = ControllerState::LATERAL_SHIFT;
    }
    // NARROW 锁: 框体锁定后不允许退出
    if (state_ != ControllerState::NARROW_PASSAGE && narrow_box_locked_)
      state_ = ControllerState::NARROW_PASSAGE;
    // TERMINAL_ALIGN: 锁定 goal, 纯旋转+平移, 永不退出
    // TERMINAL_ALIGN: 先对齐后平移, 永不退出
    if (state_ == ControllerState::TERMINAL_ALIGN) {
      double goal_yaw = tf2::getYaw(global_plan_.poses.back().pose.orientation);
      double ye = goal_yaw - current_theta;
      while (ye > M_PI) ye -= 2.0*M_PI; while (ye < -M_PI) ye += 2.0*M_PI;
      // Phase 1: 旋转对齐 yaw
      if (std::abs(ye) > terminal_angle_tolerance_) {
        cmd_vel.twist.angular.z = std::max(-max_w_, std::min(max_w_, terminal_angle_kp_ * ye));
      } else {
        // Phase 2: yaw 对齐后, 平移逼近 goal (需检测致命障碍物)
        double tdx = final_goal_x_ - current_x, tdy = final_goal_y_ - current_y;
        double tdist = std::hypot(tdx, tdy);

        // ── 致命区域检测: 沿直线路径采样, 有致命障碍物则禁止平移 ──
        bool terminal_path_blocked = false;
        {
          auto* tcm = costmap_ros_ ? costmap_ros_->getCostmap() : nullptr;
          if (tcm && tcm->getCharMap() && tcm->getSizeInCellsX() > 0) {
            const unsigned char* td = tcm->getCharMap();
            int tw = tcm->getSizeInCellsX(), th = tcm->getSizeInCellsY();
            float tr = tcm->getResolution(), tox = tcm->getOriginX(), toy = tcm->getOriginY();
            // 沿直线密集采样 (每 2cm 一个点)
            int n_samples = static_cast<int>(tdist / 0.02) + 1;
            // 同时检查足迹: 在采样点处以 goal_yaw 为朝向检查 footprint
            double gc = std::cos(goal_yaw), gs = std::sin(goal_yaw);
            double fw = footprint_front_, bw = footprint_back_;
            double lw = footprint_left_, rw = footprint_right_;
            for (int k = 0; k <= n_samples && !terminal_path_blocked; ++k) {
              double alpha = (n_samples > 0) ? static_cast<double>(k) / n_samples : 0.0;
              double sx = current_x + tdx * alpha;
              double sy = current_y + tdy * alpha;
              // 足迹包围盒
              double cx0 = sx - bw*gc + rw*gs, cy0 = sy - bw*gs - rw*gc;
              double cx1 = sx - bw*gc - lw*gs, cy1 = sy - bw*gs + lw*gc;
              double cx2 = sx + fw*gc - lw*gs, cy2 = sy + fw*gs + lw*gc;
              double cx3 = sx + fw*gc + rw*gs, cy3 = sy + fw*gs - rw*gc;
              double min_wx = std::min({cx0,cx1,cx2,cx3}), max_wx = std::max({cx0,cx1,cx2,cx3});
              double min_wy = std::min({cy0,cy1,cy2,cy3}), max_wy = std::max({cy0,cy1,cy2,cy3});
              int mx0 = (min_wx - tox) / tr, mx1 = (max_wx - tox) / tr;
              int my0 = (min_wy - toy) / tr, my1 = (max_wy - toy) / tr;
              if (mx0 < 0) mx0 = 0; if (mx1 >= tw) mx1 = tw - 1;
              if (my0 < 0) my0 = 0; if (my1 >= th) my1 = th - 1;
              if (mx0 > mx1 || my0 > my1) { terminal_path_blocked = true; break; }
              for (int my = my0; my <= my1 && !terminal_path_blocked; ++my) {
                for (int mx = mx0; mx <= mx1; ++mx) {
                  // 检查 cell 是否在 footprint 内
                  double wx = tox + (mx + 0.5) * tr, wy = toy + (my + 0.5) * tr;
                  double dx = wx - sx, dy = wy - sy;
                  double lx = dx * gc + dy * gs, ly = -dx * gs + dy * gc;
                  if (lx >= -bw && lx <= fw && ly >= -rw && ly <= lw) {
                    if (td[my * tw + mx] >= nav2_costmap_2d::LETHAL_OBSTACLE) {
                      terminal_path_blocked = true; break;
                    }
                  }
                }
              }
            }
          }
        }

        if (terminal_path_blocked) {
          // 路径被致命障碍物阻塞: 禁止平移, 保持原地旋转等待 MPPI 重新接管
          cmd_vel.twist.linear.x = 0.0;
          cmd_vel.twist.linear.y = 0.0;
        } else {
          double c = std::cos(-current_theta), s = std::sin(-current_theta);
          double sp = max_v_;
          double raw_vx = ((tdx / tdist) * c - (tdy / tdist) * s) * sp;
          double raw_vy = ((tdx / tdist) * s + (tdy / tdist) * c) * sp;
          cmd_vel.twist.linear.x = std::max(min_v_, std::min(max_v_, raw_vx));
          cmd_vel.twist.linear.y = std::max(-max_vy_, std::min(max_vy_, raw_vy));
        }
      }
      if (vis_pub_) {
        visualization_msgs::msg::MarkerArray ma; auto now = node_.lock()->now();
        visualization_msgs::msg::Marker m; m.header.frame_id = "odom"; m.header.stamp = now;
        m.ns = "state"; m.id = 0; m.type = m.ARROW; m.action = m.ADD;
        m.pose.position.x = current_x; m.pose.position.y = current_y; m.pose.position.z = 0.08;
        tf2::Quaternion q; q.setRPY(0, 0, goal_yaw); m.pose.orientation = tf2::toMsg(q);
        m.scale.x = 0.35; m.scale.y = 0.03; m.scale.z = 0.03; m.color.a = 0.9f;
        m.color.r = 0.2f; m.color.g = 0.8f; m.color.b = 0.2f; m.lifetime = rclcpp::Duration::from_seconds(0.1);
        ma.markers.push_back(m);
        // 框体: goal 位置 footprint 方块
        visualization_msgs::msg::Marker mc; mc.header.frame_id = "odom"; mc.header.stamp = now;
        mc.ns = "terminal"; mc.id = 50; mc.type = mc.CUBE; mc.action = mc.ADD;
        mc.pose.position.x = final_goal_x_; mc.pose.position.y = final_goal_y_; mc.pose.position.z = 0.05;
        tf2::Quaternion qc; qc.setRPY(0, 0, goal_yaw); mc.pose.orientation = tf2::toMsg(qc);
        mc.scale.x = footprint_front_ + footprint_back_; mc.scale.y = footprint_left_ + footprint_right_;
        mc.scale.z = 0.05; mc.color.a = 0.5f;
        mc.color.r = 0.2f; mc.color.g = 0.8f; mc.color.b = 0.2f; mc.lifetime = rclcpp::Duration::from_seconds(0.1);
        ma.markers.push_back(mc); vis_pub_->publish(ma);
      }
      return cmd_vel;
    }
    // HEADING: 选短弧直接转, 框体保证了两侧安全
    if (state_ == ControllerState::HEADING_MISALIGN) {
      cmd_vel.twist.linear.x = 0; cmd_vel.twist.linear.y = 0;
      cmd_vel.twist.angular.z = std::copysign(max_w_, h_err);
      if (vis_pub_) { visualization_msgs::msg::MarkerArray ma; auto now=node_.lock()->now();
        visualization_msgs::msg::Marker m; m.header.frame_id="odom"; m.header.stamp=now;
        m.ns="state"; m.id=1; m.type=m.ARROW; m.action=m.ADD;
        m.pose.position.x=current_x; m.pose.position.y=current_y; m.pose.position.z=0.08;
        double ty=atan2(sin(current_theta+h_err),cos(current_theta+h_err));
        tf2::Quaternion q; q.setRPY(0,0,ty); m.pose.orientation=tf2::toMsg(q);
        m.scale.x=0.35; m.scale.y=0.03; m.scale.z=0.03; m.color.a=0.8f;
        m.color.r=1; m.color.g=0.8f; m.color.b=0; m.lifetime=rclcpp::Duration::from_seconds(0.1);
        ma.markers.push_back(m); vis_pub_->publish(ma); }
      return cmd_vel;
    }
    // LATERAL_SHIFT (精简版)
    if (state_ == ControllerState::LATERAL_SHIFT) {
      bool found = false; double bx = 0, by = 0;
      // 冷却期: 到达目标后 1s 内不再搜索, 防止原地重复搜索
      bool in_cooldown = (lateral_reach_time_ > 0 && now_sec - lateral_reach_time_ < 1.0);
      if (lateral_locked_ && lateral_target_x_ != 0) { bx = lateral_target_x_; by = lateral_target_y_; found = true; }
      else if (!in_cooldown) {
        auto* cm = costmap_ros_ ? costmap_ros_->getCostmap() : nullptr;
        if (cm && cm->getCharMap() && cm->getSizeInCellsX() > 0) {
          const unsigned char* d = cm->getCharMap();
          int w = cm->getSizeInCellsX(), h = cm->getSizeInCellsY();
          float r = cm->getResolution(), ox = cm->getOriginX(), oy = cm->getOriginY();
          double pdx, pdy;
          if (!global_plan_.poses.empty() && closest_idx+1 < (int)global_plan_.poses.size()) {
            pdx = global_plan_.poses[closest_idx+1].pose.position.x - global_plan_.poses[closest_idx].pose.position.x;
            pdy = global_plan_.poses[closest_idx+1].pose.position.y - global_plan_.poses[closest_idx].pose.position.y;
          } else { pdx = cos(current_theta); pdy = sin(current_theta); }
          double nl = std::hypot(pdx, pdy); if (nl > 0.01) { pdx /= nl; pdy /= nl; }
          double maxf = std::min(0.50, std::hypot(final_goal_x_-target_x, final_goal_y_-target_y));
          for (double dd = 0; dd <= maxf && !found; dd += 0.10) {
            double cx = target_x + dd*pdx, cy = target_y + dd*pdy;
            bool safe = true;
            double flx=footprint_front_+footprint_back_, fly=footprint_left_+footprint_right_;
            double sp=footprint_sample_spacing_;
            int nx=std::max(2,std::min(12,(int)std::ceil(flx/sp)+1)), ny=std::max(2,std::min(12,(int)std::ceil(fly/sp)+1));
            double sx=(nx>1)?flx/(nx-1):0, sy=(ny>1)?fly/(ny-1):0;
            for (int iy=0; iy<ny && safe; ++iy) { double fy=-footprint_right_+iy*sy;
              for (int ix=0; ix<nx && safe; ++ix) { double fx=-footprint_back_+ix*sx;
                double wx=cx+fx*cos(current_theta)-fy*sin(current_theta);
                double wy=cy+fx*sin(current_theta)+fy*cos(current_theta);
                int mx=(wx-ox)/r, my=(wy-oy)/r;
                if ((mx>=0&&mx<w&&my>=0&&my<h ? d[my*w+mx] : (unsigned char)255) >= nav2_costmap_2d::LETHAL_OBSTACLE) safe = false;
              }}
            if (safe) { bx = cx; by = cy; found = true; }
          }
          if (found && !lateral_locked_) { lateral_target_x_ = bx; lateral_target_y_ = by; }
        }
      }
      if (found) {
        double dx = bx - current_x, dy = by - current_y, dist = std::hypot(dx, dy);
        // 同时对齐 goal yaw
        double gy = tf2::getYaw(global_plan_.poses.back().pose.orientation);
        double ye = gy - current_theta;
        while (ye > M_PI) ye -= 2.0*M_PI; while (ye < -M_PI) ye += 2.0*M_PI;
        cmd_vel.twist.angular.z = std::max(-max_w_, std::min(max_w_, terminal_angle_kp_ * ye));
        if (dist < 0.05) {
          lateral_reach_time_ = now_sec;
          lateral_locked_ = false; lateral_target_x_ = 0; lateral_target_y_ = 0;
        }
        else { double c=cos(-current_theta), s=sin(-current_theta), sp=max_v_;
          double rvx = ((dx/dist)*c-(dy/dist)*s)*sp;
          double rvy = ((dx/dist)*s+(dy/dist)*c)*sp;
          cmd_vel.twist.linear.x = std::max(min_v_, std::min(max_v_, rvx));
          cmd_vel.twist.linear.y = std::max(-max_vy_, std::min(max_vy_, rvy)); }
      }
      if (found && vis_pub_) { visualization_msgs::msg::MarkerArray ma; auto now=node_.lock()->now();
        visualization_msgs::msg::Marker m; m.header.frame_id="odom"; m.header.stamp=now;
        m.ns="lateral"; m.id=400; m.type=m.CUBE; m.action=m.ADD;
        m.pose.position.x=bx; m.pose.position.y=by; m.pose.position.z=0.05;
        tf2::Quaternion q; q.setRPY(0,0,current_theta); m.pose.orientation=tf2::toMsg(q);
        m.scale.x=footprint_front_+footprint_back_; m.scale.y=footprint_left_+footprint_right_; m.scale.z=0.05;
        m.color.a=0.6f; m.color.r=0.5f; m.color.g=0; m.color.b=1; m.lifetime=rclcpp::Duration::from_seconds(0.1);
        ma.markers.push_back(m); vis_pub_->publish(ma); }
      return cmd_vel;
    }
    // NARROW_PASSAGE
    if (state_ == ControllerState::NARROW_PASSAGE) {
      nav2_costmap_2d::Costmap2D* cm = costmap_ros_ ? costmap_ros_->getCostmap() : nullptr;
      const unsigned char* ns_cm = nullptr; int ns_w=0, ns_h=0; float ns_r=0.05f, ns_ox=0, ns_oy=0;
      if (cm && cm->getCharMap()) { ns_cm=cm->getCharMap(); ns_w=cm->getSizeInCellsX(); ns_h=cm->getSizeInCellsY(); ns_r=cm->getResolution(); ns_ox=cm->getOriginX(); ns_oy=cm->getOriginY(); }
      double ns_pdx=0, ns_pdy=0;
      ns_pdx=final_goal_x_-current_x; ns_pdy=final_goal_y_-current_y;
      double nl=std::hypot(ns_pdx,ns_pdy); if(nl>0.01){ns_pdx/=nl;ns_pdy/=nl;} else{ns_pdx=cos(current_theta);ns_pdy=sin(current_theta);}
      double nh_lh_x = target_x, nh_lh_y = target_y;
      { int pl=global_plan_.poses.size()-1; for(int i=closest_idx; i<=pl; ++i){
          if(std::hypot(global_plan_.poses[i].pose.position.x-current_x,global_plan_.poses[i].pose.position.y-current_y)>=0.20)
          {nh_lh_x=global_plan_.poses[i].pose.position.x;nh_lh_y=global_plan_.poses[i].pose.position.y;break;}}}
      // ── 诊断计数器 (每帧 SEARCH_BOX 累计) ──
      int diag_fp_calls=0, diag_fp_oob=0, diag_fp_lethal=0;
      int diag_srch_calls=0, diag_srch_ok=0;
      unsigned char diag_worst_cell=0; // 遇到的最高 costmap cell 值 (>=254 即致命)
      bool   diag_worst_oob=false;
      double diag_worst_wx=0, diag_worst_wy=0;
      auto fp_coll=[&](double cx,double cy,double ct){diag_fp_calls++;if(!ns_cm)return false;double cc=cos(ct),ss=sin(ct);
        double flx=footprint_front_+footprint_back_,fly=footprint_left_+footprint_right_;
        // 遍历足迹包围盒内所有 costmap cell, 不遗漏任何致命 cell
        // 四个角的世界坐标
        double cx0=cx-footprint_back_*cc+footprint_right_*ss, cy0=cy-footprint_back_*ss-footprint_right_*cc;
        double cx1=cx-footprint_back_*cc-footprint_left_*ss,  cy1=cy-footprint_back_*ss+footprint_left_*cc;
        double cx2=cx+footprint_front_*cc-footprint_left_*ss, cy2=cy+footprint_front_*ss+footprint_left_*cc;
        double cx3=cx+footprint_front_*cc+footprint_right_*ss,cy3=cy+footprint_front_*ss-footprint_right_*cc;
        double min_wx=std::min({cx0,cx1,cx2,cx3}), max_wx=std::max({cx0,cx1,cx2,cx3});
        double min_wy=std::min({cy0,cy1,cy2,cy3}), max_wy=std::max({cy0,cy1,cy2,cy3});
        int mx0=(min_wx-ns_ox)/ns_r, mx1=(max_wx-ns_ox)/ns_r;
        int my0=(min_wy-ns_oy)/ns_r, my1=(max_wy-ns_oy)/ns_r;
        if(mx0<0)mx0=0;if(mx1>=ns_w)mx1=ns_w-1;if(my0<0)my0=0;if(my1>=ns_h)my1=ns_h-1;
        if(mx0>mx1||my0>my1){diag_fp_oob++; return true;}  // 完全越界
        // 逆变换: 把 cell 中心转到机器人系判断是否在矩形内
        double fw=footprint_front_, bw=footprint_back_, lw=footprint_left_, rw=footprint_right_;
        for(int my=my0;my<=my1;++my){
          for(int mx=mx0;mx<=mx1;++mx){
            double wx=ns_ox+(mx+0.5)*ns_r, wy=ns_oy+(my+0.5)*ns_r;
            double dx=wx-cx, dy=wy-cy;
            double lx=dx*cc+dy*ss, ly=-dx*ss+dy*cc;  // 转到机器人系
            if(lx>=-bw && lx<=fw && ly>=-rw && ly<=lw){
              unsigned char cv=ns_cm[my*ns_w+mx];
              if(cv>=nav2_costmap_2d::LETHAL_OBSTACLE){
                if(cv>diag_worst_cell){diag_worst_cell=cv;diag_worst_oob=false;diag_worst_wx=wx;diag_worst_wy=wy;}
                diag_fp_lethal++;
                return true;}}}}
        return false;};
      auto srch=[&](double cx,double cy,double path_tangent,double&ot){
        diag_srch_calls++;
        double best_score=1e9; bool found=false;
        for(int i=0;i<24;++i){double t=i*2.0*M_PI/24.0;if(!fp_coll(cx,cy,t)){
          double d_cur=std::abs(t-current_theta);if(d_cur>M_PI)d_cur=2.0*M_PI-d_cur;
          double d_path=std::abs(t-path_tangent);if(d_path>M_PI)d_path=2.0*M_PI-d_path;
          double score=d_cur+1.5*d_path;
          if(score<best_score){best_score=score;ot=t;found=true;}}}
        if(found) diag_srch_ok++;
        return found;};
      bool ndone=false;
      switch(narrow_sub_state_){
        case NarrowSubState::SEARCH_BOX:
          cmd_vel.twist.linear.x=0;cmd_vel.twist.linear.y=0;cmd_vel.twist.angular.z=0;
          if(narrow_box_locked_){narrow_sub_state_=NarrowSubState::ALIGN;break;}
          // ── 沿全局路径前向搜索 ──
          // 从 closest_idx+0.10m 起搜到 1.0m, 跳过脚下, 不超出合理范围
          {double ft;int pl=global_plan_.poses.size()-1;
          const double SEARCH_START=0.20, SEARCH_END=1.00;
          double next_d=SEARCH_START;
          // 第一遍: 路径中心线搜索
          {double cum=0;
          for(int pi=closest_idx;pi<pl&&!narrow_box_found_;++pi){
            double ax=global_plan_.poses[pi].pose.position.x,ay=global_plan_.poses[pi].pose.position.y;
            double bx=global_plan_.poses[pi+1].pose.position.x,by=global_plan_.poses[pi+1].pose.position.y;
            double seg=std::hypot(bx-ax,by-ay);
            while(cum+seg>=next_d&&next_d<=SEARCH_END&&!narrow_box_found_){
              double t=(next_d-cum)/seg;if(t<0)t=0;if(t>1)t=1;
              double cx=ax+t*(bx-ax),cy=ay+t*(by-ay);
              if((cx-current_x)*(bx-ax)+(cy-current_y)*(by-ay)>=-0.05){
                if(srch(cx,cy,atan2(by-ay,bx-ax),ft)){narrow_box_target_x_=cx;narrow_box_target_y_=cy;narrow_box_target_theta_=ft;narrow_box_found_=true;break;}}
              next_d+=0.10;}
            cum+=seg;if(next_d>SEARCH_END)break;}}
          // 第二遍: 中心线失败后, 横向偏移 ±6cm 搜索
          if(!narrow_box_found_){double ft;const double LA[]={0.06,-0.06};
          next_d=SEARCH_START;
          {double cum=0;
          for(int pi=closest_idx;pi<pl&&!narrow_box_found_;++pi){
            double ax=global_plan_.poses[pi].pose.position.x,ay=global_plan_.poses[pi].pose.position.y;
            double bx=global_plan_.poses[pi+1].pose.position.x,by=global_plan_.poses[pi+1].pose.position.y;
            double seg=std::hypot(bx-ax,by-ay);double lx=-(by-ay)/seg,ly=(bx-ax)/seg;
            while(cum+seg>=next_d&&next_d<=SEARCH_END&&!narrow_box_found_){
              double t=(next_d-cum)/seg;if(t<0)t=0;if(t>1)t=1;
              double cx=ax+t*(bx-ax),cy=ay+t*(by-ay);
              if((cx-current_x)*(bx-ax)+(cy-current_y)*(by-ay)>=-0.05){
                for(double lat:LA){double sx=cx+lat*lx,sy=cy+lat*ly;
                  if(srch(sx,sy,atan2(by-ay,bx-ax),ft)){narrow_box_target_x_=sx;narrow_box_target_y_=sy;narrow_box_target_theta_=ft;narrow_box_found_=true;break;}}}
              next_d+=0.10;}
            cum+=seg;if(next_d>SEARCH_END)break;}}}}
          if(narrow_box_found_){narrow_box_locked_=true;narrow_sub_state_=NarrowSubState::ALIGN;}
          if(enable_file_log_ && logger_.is_open()){std::ostringstream os;
            os<<(narrow_box_found_?"found":"failed")
              <<" lh=("<<nh_lh_x<<","<<nh_lh_y<<")"
              <<" box=("<<narrow_box_target_x_<<","<<narrow_box_target_y_<<") th="<<narrow_box_target_theta_
              <<" cur=("<<current_x<<","<<current_y<<")"
              <<" | cmap: w="<<ns_w<<" h="<<ns_h<<" res="<<ns_r<<" ox="<<ns_ox<<" oy="<<ns_oy
              <<" | diag: srch="<<diag_srch_calls<<" ok="<<diag_srch_ok
              <<" fp="<<diag_fp_calls<<" oob="<<diag_fp_oob<<" lethal="<<diag_fp_lethal;
            if(!narrow_box_found_ && diag_srch_calls>0 && diag_srch_ok==0){
              os<<" | worst_cell="<<(int)diag_worst_cell
                <<(diag_worst_oob?" OOB":" inmap")
                <<" wpos=("<<diag_worst_wx<<","<<diag_worst_wy<<")";
            }
            logger_.logState("narrow_box",os.str());}
          break;
        case NarrowSubState::ALIGN:
          {double ye=narrow_box_target_theta_-current_theta;while(ye>M_PI)ye-=2.0*M_PI;while(ye<-M_PI)ye+=2.0*M_PI;
          if(std::abs(ye)<0.05){narrow_sub_state_=NarrowSubState::ADVANCE;cmd_vel.twist.angular.z=0;}
          else cmd_vel.twist.angular.z=std::max(-max_w_,std::min(max_w_,1.5*ye));}
          break;
        case NarrowSubState::ADVANCE:
          {double dx=narrow_box_target_x_-current_x,dy=narrow_box_target_y_-current_y,dist=std::hypot(dx,dy);
          if(dist<0.05){
            // 到达框体, 用虚拟 MPPI 验证前瞻点是否恢复正常
            bool lh_ok=false;
            if(ns_cm){
              const int VN=num_samples_;int VH=prediction_horizon_;
              std::vector<float> vn_vx(VN*VH),vn_vy(VN*VH),vn_w(VN*VH),vb(VH,0),vc(VN);
              cudaStream_t vs;cudaStreamCreate(&vs);
              for(int i=0;i<VN*VH;++i){vn_vx[i]=(float)dist_vx_(generator_);vn_vy[i]=(float)dist_vy_(generator_);vn_w[i]=(float)dist_w_(generator_);}
              int ret=mppi_gpu_sample_and_cost(vn_vx.data(),vn_vy.data(),vn_w.data(),vb.data(),vb.data(),vb.data(),
                (float)narrow_box_target_x_,(float)narrow_box_target_y_,(float)narrow_box_target_theta_,
                (float)nh_lh_x,(float)nh_lh_y,ns_cm,ns_w,ns_h,ns_r,ns_ox,ns_oy,
                (float)dt_,(float)min_v_,(float)max_v_,(float)max_vy_,(float)max_w_,
                (float)obstacle_weight_,(float)heading_weight_,(float)time_weight_,
                1,0,(float)guidance_weight_,(float)cross_track_noise_scale_,(float)noise_decay_rate_,1,(float)spatial_decay_weight_,
                (float)noise_scale_floor_vx_,(float)noise_scale_floor_vy_,(float)noise_scale_floor_w_,
                0,0,1,(float)lateral_guidance_scale_,0,(float)turn_lateral_boost_,(float)turn_lateral_max_boost_,
                (float)footprint_front_,(float)footprint_back_,(float)footprint_left_,(float)footprint_right_,
                (float)footprint_sample_spacing_,(float)rear_obstacle_cost_,
                nullptr,nullptr,0,0,0,0,0,0,VN,VH,
                d_noise_vx_,d_noise_vy_,d_noise_w_,d_base_vx_,d_base_vy_,d_base_w_,
                d_costmap_,d_path_x_,d_path_y_,d_costs_,d_sampled_vx_,d_sampled_vy_,d_sampled_w_,d_traj_x_,d_traj_y_,vs);
              if(ret==0){cudaMemcpyAsync(vc.data(),d_costs_,VN*sizeof(float),cudaMemcpyDeviceToHost,vs);cudaStreamSynchronize(vs);
                float mc=vc[0];for(int i=1;i<VN;++i)if(vc[i]<mc)mc=vc[i];
                ret=mppi_gpu_weighted_sum(d_costs_,d_sampled_vx_,d_sampled_vy_,d_sampled_w_,d_result_seq_,mc,(float)lambda_,VN,VH,vs);
                if(ret==0){std::vector<float> vr(VH*4);cudaMemcpyAsync(vr.data(),d_result_seq_,VH*4*sizeof(float),cudaMemcpyDeviceToHost,vs);cudaStreamSynchronize(vs);
                  double wvx=0,wvy=0,ws=0;for(int t=0;t<std::min(3,VH);++t){float sw=vr[t*4+3];if(sw>1e-6f){wvx+=vr[t*4+0]/sw;wvy+=vr[t*4+1]/sw;ws+=1;}}
                  if(ws>0){wvx/=ws;wvy/=ws;}
                  double pdx=std::cos(narrow_box_target_theta_),pdy=std::sin(narrow_box_target_theta_);
                  narrow_verify_speed_ = wvx * pdx + wvy * pdy;  // 记录到 CSV
                  if(narrow_verify_speed_>=0.10)lh_ok=true;}}
              cudaStreamDestroy(vs);}
            if(lh_ok){
              if(!narrow_final_target_){narrow_final_target_=true;narrow_box_target_x_=nh_lh_x;narrow_box_target_y_=nh_lh_y;narrow_box_target_theta_=tf2::getYaw(global_plan_.poses.back().pose.orientation);narrow_sub_state_=NarrowSubState::ALIGN;}
              else ndone=true;
            }else{narrow_box_locked_=false;narrow_box_found_=false;narrow_sub_state_=NarrowSubState::SEARCH_BOX;narrow_verify_speed_=-1.0;}
            cmd_vel.twist.linear.x=0;cmd_vel.twist.linear.y=0;cmd_vel.twist.angular.z=0;}
          else{double c=cos(-current_theta),s=sin(-current_theta),sp=max_v_;
            double rvx=((dx/dist)*c-(dy/dist)*s)*sp,rvy=((dx/dist)*s+(dy/dist)*c)*sp;
            cmd_vel.twist.linear.x=std::max(min_v_,std::min(max_v_,rvx));
            cmd_vel.twist.linear.y=std::max(-max_vy_,std::min(max_vy_,rvy));cmd_vel.twist.angular.z=0;}}
          break;
      }
      if(ndone){state_=ControllerState::NORMAL;narrow_sub_state_=NarrowSubState::SEARCH_BOX;
        narrow_box_found_=false;narrow_box_locked_=false;narrow_final_target_=false;hesitate_count_=0;narrow_verify_speed_=-1.0;}
      if ((narrow_box_found_||narrow_box_locked_) && vis_pub_) {
        visualization_msgs::msg::MarkerArray ma; auto now=node_.lock()->now();
        visualization_msgs::msg::Marker m; m.header.frame_id="odom"; m.header.stamp=now;
        m.ns="narrow"; m.id=100; m.type=m.CUBE; m.action=m.ADD;
        m.pose.position.x=narrow_box_target_x_; m.pose.position.y=narrow_box_target_y_; m.pose.position.z=0.05;
        tf2::Quaternion q; q.setRPY(0,0,narrow_box_target_theta_); m.pose.orientation=tf2::toMsg(q);
        m.scale.x=footprint_front_+footprint_back_; m.scale.y=footprint_left_+footprint_right_; m.scale.z=0.05;
        m.color.a=0.6f; m.color.g=(narrow_sub_state_==NarrowSubState::ADVANCE)?1:0.5f;
        m.color.b=(narrow_sub_state_==NarrowSubState::ADVANCE)?0:1; m.lifetime=rclcpp::Duration::from_seconds(0.1);
        ma.markers.push_back(m); vis_pub_->publish(ma); }
      return cmd_vel;
    }
  }

  // ── 前瞻点堵塞程度 (0=通畅, 1=堵塞), 相对当前代价 ──
  float lookahead_blockage = 0.0f;
  {
    if (costmap_ros_ != nullptr) {
      auto* cm = costmap_ros_->getCostmap();
      if (cm != nullptr) {
        auto get_cost = [&](double wx, double wy) -> unsigned char {
          int mx = static_cast<int>((wx - cm->getOriginX()) / cm->getResolution());
          int my = static_cast<int>((wy - cm->getOriginY()) / cm->getResolution());
          if (mx >= 0 && mx < static_cast<int>(cm->getSizeInCellsX()) &&
              my >= 0 && my < static_cast<int>(cm->getSizeInCellsY()))
            return cm->getCharMap()[my * cm->getSizeInCellsX() + mx];
          return 255;
        };
        unsigned char lh_c = get_cost(target_x, target_y);
        unsigned char base_c = get_cost(current_x, current_y);
        int diff = static_cast<int>(lh_c) - static_cast<int>(base_c);
        if (diff > 60) lookahead_blockage = 1.0f;
        else if (diff > 20) lookahead_blockage = (diff - 20.0f) / 40.0f;
      }
    }
  }

  // 局部路径方向：取 closest_idx 处的一小段，用于 guidance 和噪声对齐
  double path_direction_x = 0.0;
  double path_direction_y = 0.0;
  if (!global_plan_.poses.empty() && closest_idx + 1 < static_cast<int>(global_plan_.poses.size())) {
    path_direction_x = global_plan_.poses[closest_idx + 1].pose.position.x
                     - global_plan_.poses[closest_idx].pose.position.x;
    path_direction_y = global_plan_.poses[closest_idx + 1].pose.position.y
                     - global_plan_.poses[closest_idx].pose.position.y;
    double path_len = std::hypot(path_direction_x, path_direction_y);
    if (path_len > 0.01) {
      path_direction_x /= path_len;
      path_direction_y /= path_len;
    }
  }

  // ── 路径转折角度计算 ──
  // 比较当前路径段方向与前方若干段方向之间的角度变化，
  // 用于 GPU 内核中根据转弯幅度动态增强横向运动。
  double path_turn_angle = 0.0;
  {
    int path_turn_lookahead = 6;  // 前瞻索引数，越大越远
    int idx1 = closest_idx;
    int idx2 = closest_idx + path_turn_lookahead;
    int end = static_cast<int>(global_plan_.poses.size()) - 1;
    if (idx2 >= end) idx2 = end - 1;
    if (idx1 + 1 < end && idx2 + 1 <= end && idx2 > idx1 + 1) {
      double dx1 = global_plan_.poses[idx1 + 1].pose.position.x
                 - global_plan_.poses[idx1].pose.position.x;
      double dy1 = global_plan_.poses[idx1 + 1].pose.position.y
                 - global_plan_.poses[idx1].pose.position.y;
      double dx2 = global_plan_.poses[idx2 + 1].pose.position.x
                 - global_plan_.poses[idx2].pose.position.x;
      double dy2 = global_plan_.poses[idx2 + 1].pose.position.y
                 - global_plan_.poses[idx2].pose.position.y;
      double angle1 = std::atan2(dy1, dx1);
      double angle2 = std::atan2(dy2, dx2);
      double diff = angle2 - angle1;
      diff = std::atan2(std::sin(diff), std::cos(diff));  // normalize to [-π, π]
      path_turn_angle = std::abs(diff);
    }
  }

  // ── 最终目标姿态朝向 ──
  // 用于余弦退火混合朝向代价：离目标越近，朝向偏好从路径方向过渡到 goal yaw
  double goal_yaw = 0.0;
  if (!global_plan_.poses.empty()) {
    goal_yaw = tf2::getYaw(global_plan_.poses.back().pose.orientation);
  }

  // ── 前瞻点最优摆放朝向搜索 ──
  // 在前瞻点处采样多个朝向，评估碰撞箱各向异性代价，
  // 选取代价最小的朝向作为前瞻点位姿接近奖励的目标朝向。
  // 全向底盘在窄通道中可能需要横着走 (窄边 0.4m 朝墙, 宽边 0.6m 沿通道)。
  // 特例: 前瞻点即终点时，直接使用目标姿态朝向，不搜索。
  {
    double dist_target_to_final = std::hypot(
        target_x - global_plan_.poses.back().pose.position.x,
        target_y - global_plan_.poses.back().pose.position.y);
    if (dist_target_to_final < 0.01) {
      lookahead_theta_ = goal_yaw;
    } else {
      lookahead_theta_ = std::atan2(path_direction_y, path_direction_x);  // 默认: 路径方向

      const unsigned char* search_costmap = nullptr;
      int search_w = 0, search_h = 0;
      float search_res = 0.05f;
      float search_ox = 0.0f, search_oy = 0.0f;

      if (costmap_ros_ != nullptr) {
        auto* cm = costmap_ros_->getCostmap();
        if (cm != nullptr) {
          search_costmap = cm->getCharMap();
          search_w = static_cast<int>(cm->getSizeInCellsX());
          search_h = static_cast<int>(cm->getSizeInCellsY());
          search_res = static_cast<float>(cm->getResolution());
          search_ox = static_cast<float>(cm->getOriginX());
          search_oy = static_cast<float>(cm->getOriginY());
        }
      }

      if (search_costmap != nullptr && search_w > 0 && search_h > 0) {
        const double fp_front = footprint_front_, fp_back = footprint_back_;
        const double fp_left = footprint_left_, fp_right = footprint_right_;
        const double fp_len_x = fp_front + fp_back;
        const double fp_len_y = fp_left + fp_right;
        const double FP_SPACING = 0.08;
        int nx = static_cast<int>(std::ceil(fp_len_x / FP_SPACING)) + 1;
        int ny = static_cast<int>(std::ceil(fp_len_y / FP_SPACING)) + 1;
        nx = std::max(2, std::min(nx, 10));
        ny = std::max(2, std::min(ny, 10));
        int n_pts = nx * ny;
        double step_x = (nx > 1) ? fp_len_x / (nx - 1) : 0.0;
        double step_y = (ny > 1) ? fp_len_y / (ny - 1) : 0.0;

        // 评估给定朝向的碰撞代价 (统一 norm^4, 与 GPU 内核一致)
        auto eval_theta = [&](double theta) -> double {
          double cos_t = std::cos(theta);
          double sin_t = std::sin(theta);
          double cost_acc = 0.0;
          for (int iy = 0; iy < ny; ++iy) {
            double fy = -fp_right + iy * step_y;
            for (int ix = 0; ix < nx; ++ix) {
              double fx = -fp_back + ix * step_x;
              double wx = target_x + fx * cos_t - fy * sin_t;
              double wy = target_y + fx * sin_t + fy * cos_t;
              int mx = static_cast<int>((wx - search_ox) / search_res);
              int my = static_cast<int>((wy - search_oy) / search_res);
              double val = 255.0;  // 越界视同致命
              if (mx >= 0 && mx < search_w && my >= 0 && my < search_h) {
                val = static_cast<double>(search_costmap[my * search_w + mx]);
              }
              double norm = val / 255.0;
              double norm2 = norm * norm;
              cost_acc += norm2 * norm2;  // norm^4
            }
          }
          return cost_acc / n_pts;  // 平均 norm^6 代价
        };

        // 评估路径方向代价作为基准
        double path_dir_theta = std::atan2(path_direction_y, path_direction_x);
        double path_cost = eval_theta(path_dir_theta);

        // 搜索最优朝向 — 以上一帧结果为起点，避免帧间跳变
        double seed_theta = has_prev_lookahead_theta_
            ? prev_lookahead_theta_ : path_dir_theta;
        double best_theta = seed_theta;
        double best_cost = eval_theta(seed_theta);

        // 同时评估路径方向
        if (path_cost < best_cost) {
          best_cost = path_cost;
          best_theta = path_dir_theta;
        }

        const int N_THETA = 24;
        for (int i = 0; i < N_THETA; ++i) {
          double theta = i * 2.0 * M_PI / N_THETA;
          double cost = eval_theta(theta);
          if (cost < best_cost) {
            best_cost = cost;
            best_theta = theta;
          }
        }

        // 融合路径方向与最优碰撞朝向
        double improvement = path_cost - best_cost;
        double blend_ratio = (improvement > 0.0)
            ? std::min(1.0, improvement / 0.3)
            : 0.0;
        double diff = best_theta - path_dir_theta;
        while (diff > M_PI) diff -= 2.0 * M_PI;
        while (diff < -M_PI) diff += 2.0 * M_PI;
        lookahead_theta_ = path_dir_theta + blend_ratio * diff;
      }
    }

    // ── 变化率限制: 跨路径重规划保持全局持久朝向，避免突变 ──
    // lookahead_theta_ 是全局持久变量，仅在每帧按最大速率逐步趋近 raw 值
    {
      double raw_theta = lookahead_theta_;  // 保留搜索+融合后的目标朝向
      auto now = node_.lock()->now();
      double dt = (last_theta_update_time_.nanoseconds() == 0)
          ? 0.0 : (now - last_theta_update_time_).seconds();
      last_theta_update_time_ = now;

      // 首帧直接采用 raw 值
      if (!has_prev_lookahead_theta_) {
        lookahead_theta_ = raw_theta;
        prev_lookahead_theta_ = raw_theta;
        has_prev_lookahead_theta_ = true;
      } else if (dt > 0.0 && lookahead_theta_rate_ > 0.0) {
        // 计算角度差（处理环绕）
        double delta = raw_theta - prev_lookahead_theta_;
        while (delta > M_PI) delta -= 2.0 * M_PI;
        while (delta < -M_PI) delta += 2.0 * M_PI;

        // 变化率限幅: |delta_max| = rate * dt
        double max_delta = lookahead_theta_rate_ * dt;
        if (std::abs(delta) > max_delta) {
          delta = std::copysign(max_delta, delta);
        }
        lookahead_theta_ = prev_lookahead_theta_ + delta;
        // 归一化到 [-π, π]
        while (lookahead_theta_ > M_PI) lookahead_theta_ -= 2.0 * M_PI;
        while (lookahead_theta_ < -M_PI) lookahead_theta_ += 2.0 * M_PI;
      } else {
        // rate=0 禁用限制，直接使用 raw
        lookahead_theta_ = raw_theta;
      }
      prev_lookahead_theta_ = lookahead_theta_;
    }
  }

  // 初始化或滚动最优控制序列 (MPPI 标准 warm-start)
  if (!initialized_) {
    double dx = target_x - current_x;
    double dy = target_y - current_y;
    double angle_to_goal = std::atan2(dy, dx);
    double angle_diff = angle_to_goal - current_theta;
    while (angle_diff > M_PI) angle_diff -= 2 * M_PI;
    while (angle_diff < -M_PI) angle_diff += 2 * M_PI;

    double dist_to_goal = std::hypot(dx, dy);
    double init_vx = std::min(max_v_, dist_to_goal / dt_);
    double init_vy = 0.0;
    double init_omega = std::max(-max_w_, std::min(max_w_, angle_diff / dt_));

    for (int i = 0; i < H; ++i) {
      optimal_vx_seq_[i] = init_vx;
      optimal_vy_seq_[i] = init_vy;
      optimal_omega_seq_[i] = init_omega;
    }
    initialized_ = true;
  } else {
    for (int i = 0; i < H - 1; ++i) {
      optimal_vx_seq_[i] = optimal_vx_seq_[i + 1];
      optimal_vy_seq_[i] = optimal_vy_seq_[i + 1];
      optimal_omega_seq_[i] = optimal_omega_seq_[i + 1];
    }
    // 尾部指数衰减而非直接清零，防止 base 序列逐帧塌缩
    const double TAIL_DECAY = 0.5;
    optimal_vx_seq_.back()     = optimal_vx_seq_[H - 2] * TAIL_DECAY;
    optimal_vy_seq_.back()     = optimal_vy_seq_[H - 2] * TAIL_DECAY;
    optimal_omega_seq_.back()  = optimal_omega_seq_[H - 2] * TAIL_DECAY;
  }

  // 预生成噪声序列（CPU 端，与原始 MPPI 一致）
  size_t noise_size = N * H;
  std::vector<float> noise_vx(noise_size);
  std::vector<float> noise_vy(noise_size);
  std::vector<float> noise_w(noise_size);

  // NLN 混合采样: 纯高斯 + 对数正态(重尾)混合
  for (int i = 0; i < N * H; ++i) {
    auto nln = [&](std::normal_distribution<>& nd, std::lognormal_distribution<>& ld) -> float {
      if (dist_mix_(generator_) < nln_ratio_) {
        // 对数正态: 重尾探索 → 偶尔产生大偏离值
        float mag = static_cast<float>(ld(generator_)) - 1.0f;
        return (dist_sign_(generator_) < 0.5f ? -mag : mag);
      }
      return static_cast<float>(nd(generator_));
    };
    noise_vx[i] = nln(dist_vx_, dist_ln_vx_);
    noise_vy[i] = nln(dist_vy_, dist_ln_vy_);
    noise_w[i]  = nln(dist_w_,  dist_ln_w_);
  }

  // 转换基控制序列为 float
  std::vector<float> base_vx_f(H), base_vy_f(H), base_w_f(H);
  for (int i = 0; i < H; ++i) {
    base_vx_f[i] = static_cast<float>(optimal_vx_seq_[i]);
    base_vy_f[i] = static_cast<float>(optimal_vy_seq_[i]);
    base_w_f[i]  = static_cast<float>(optimal_omega_seq_[i]);
  }

  // 获取 costmap 数据
  const unsigned char* costmap_data = nullptr;
  int costmap_w = 0, costmap_h = 0;
  float costmap_res = 0.05f;
  float costmap_origin_x = 0.0f, costmap_origin_y = 0.0f;

  // 可写的合并后 costmap（生命周期需覆盖后续所有 costmap_data 使用）
  std::vector<unsigned char> merged_costmap;

  if (costmap_ros_ != nullptr) {
    nav2_costmap_2d::Costmap2D* costmap = costmap_ros_->getCostmap();
    if (costmap != nullptr) {
      costmap_data = costmap->getCharMap();
      costmap_w = static_cast<int>(costmap->getSizeInCellsX());
      costmap_h = static_cast<int>(costmap->getSizeInCellsY());
      costmap_res = static_cast<float>(costmap->getResolution());
      costmap_origin_x = static_cast<float>(costmap->getOriginX());
      costmap_origin_y = static_cast<float>(costmap->getOriginY());

      // ── 合并全局代价地图到局部代价地图 ──
      // 对局部 costmap 每个 cell，通过 tf2 将世界坐标从 odom 帧
      // 变换到 map 帧，查询全局 costmap 对应值，取 max 合并。
      if (use_global_costmap_ && latest_global_costmap_ != nullptr) {
        const auto& global = *latest_global_costmap_;
        int gw = static_cast<int>(global.info.width);
        int gh = static_cast<int>(global.info.height);
        float g_res = global.info.resolution;
        float g_ox = static_cast<float>(global.info.origin.position.x);
        float g_oy = static_cast<float>(global.info.origin.position.y);

        // 仅分辨率相近时合并，避免栅格不对齐
        if (gw > 0 && gh > 0 && std::abs(g_res - costmap_res) < 1e-4f) {
          int total_cells = costmap_w * costmap_h;
          merged_costmap.assign(costmap_data, costmap_data + total_cells);

          try {
            auto tf_stamped = tf_->lookupTransform(
              "map", "odom", tf2::TimePointZero);

            double tx = tf_stamped.transform.translation.x;
            double ty = tf_stamped.transform.translation.y;
            double yaw = tf2::getYaw(tf_stamped.transform.rotation);
            double cos_t = std::cos(yaw);
            double sin_t = std::sin(yaw);

            int merged_count = 0;
            for (int my = 0; my < costmap_h; ++my) {
              for (int mx = 0; mx < costmap_w; ++mx) {
                // odom 帧下的世界坐标
                double wx = costmap_origin_x + (mx + 0.5) * costmap_res;
                double wy = costmap_origin_y + (my + 0.5) * costmap_res;

                // 变换到 map 帧
                double wx_map = tx + cos_t * wx - sin_t * wy;
                double wy_map = ty + sin_t * wx + cos_t * wy;

                // 查询全局 costmap
                int gx = static_cast<int>((wx_map - g_ox) / g_res);
                int gy = static_cast<int>((wy_map - g_oy) / g_res);
                if (gx >= 0 && gx < gw && gy >= 0 && gy < gh) {
                  unsigned char g_val = global.data[gy * gw + gx];
                  unsigned char& local_val = merged_costmap[my * costmap_w + mx];
                  // 取 max：不遗漏全局已知障碍物
                  if (g_val == 255 && local_val == 0) {
                    // 全局未知但局部空闲 → 适度提示（局部可能未探索此处）
                    local_val = 128;
                  } else if (g_val > local_val) {
                    local_val = g_val;
                  }
                  merged_count++;
                }
              }
            }
            costmap_data = merged_costmap.data();
            RCLCPP_DEBUG(node_.lock()->get_logger(),
              "全局代价地图合并: %d/%d cells 有覆盖", merged_count, total_cells);
          } catch (const tf2::TransformException& e) {
            RCLCPP_WARN_THROTTLE(node_.lock()->get_logger(),
              *node_.lock()->get_clock(), 5000,
              "odom→map 变换失败，跳过全局代价地图合并: %s", e.what());
          }
        }
      }
    }
  }

  // 按需分配/重分配 costmap GPU 缓冲区
  if (costmap_data != nullptr && costmap_w > 0 && costmap_h > 0) {
    if (costmap_w != costmap_w_ || costmap_h != costmap_h_) {
      cudaFree(d_costmap_);
      cudaError_t err = cudaMalloc(&d_costmap_, costmap_w * costmap_h * sizeof(unsigned char));
      if (err != cudaSuccess) {
        RCLCPP_ERROR(node_.lock()->get_logger(), "cudaMalloc(costmap) 失败: %s", cudaGetErrorString(err));
        d_costmap_ = nullptr;
        costmap_w_ = 0;
        costmap_h_ = 0;
      } else {
        costmap_w_ = costmap_w;
        costmap_h_ = costmap_h;
      }
    }
  }

  // 如果没有 costmap，确保 d_costmap_ 为 nullptr（内核需检查）
  if (costmap_data == nullptr) {
    costmap_w = 0;
    costmap_h = 0;
  }

  // ── 横向偏好分析：打破对称障碍物的左右抉择困境 ──
  // 在前瞻点处沿路径法向扫描 costmap，判断哪侧空闲更多，
  // 将 path_direction 微旋向空闲侧，使 MPPI 噪声采样/guidance 偏向该侧
  // 可通过 enable_lateral_bias 参数关闭
  if (enable_lateral_bias_ && costmap_data != nullptr && costmap_w > 0 && costmap_h > 0
      && std::hypot(path_direction_x, path_direction_y) > 0.01) {
    // 路径法向（全局坐标系）：左 = +90° 旋转
    double perp_x = -path_direction_y;
    double perp_y =  path_direction_x;

    auto sample_cost = [&](double wx, double wy) -> double {
      int mx = static_cast<int>((wx - costmap_origin_x) / costmap_res);
      int my = static_cast<int>((wy - costmap_origin_y) / costmap_res);
      if (mx >= 0 && mx < costmap_w && my >= 0 && my < costmap_h) {
        return costmap_data[my * costmap_w + mx] / 255.0;  // 归一化到 [0, 1]
      }
      return 1.0;  // 越界视为障碍
    };

    // 在前瞻点处沿法向采样左右两侧 (0.3m ~ 1.8m, 步长 0.3m)
    double left_cost = 0.0, right_cost = 0.0;
    int n_samples = 0;
    for (double d = 0.3; d <= 1.8; d += 0.3) {
      left_cost  += sample_cost(target_x + d * perp_x, target_y + d * perp_y);
      right_cost += sample_cost(target_x - d * perp_x, target_y - d * perp_y);
      n_samples++;
    }
    if (n_samples > 0) {
      left_cost /= n_samples;
      right_cost /= n_samples;
    }

    double cost_diff = right_cost - left_cost;  // >0 → 左侧更空闲 → 偏好左绕
    const double HYSTERESIS_MARGIN = 0.25;       // 迟滞区间，costmap 噪声下不易翻转

    if (preferred_lateral_dir_ < -0.5) {
      // 当前偏好左绕，需要右侧显著更空闲才切换
      if (cost_diff < -HYSTERESIS_MARGIN) {
        preferred_lateral_dir_ = 1.0;
      }
    } else if (preferred_lateral_dir_ > 0.5) {
      // 当前偏好右绕，需要左侧显著更空闲才切换
      if (cost_diff > HYSTERESIS_MARGIN) {
        preferred_lateral_dir_ = -1.0;
      }
    } else {
      // 无偏好，任一方向有显著优势即采纳
      if (cost_diff > HYSTERESIS_MARGIN) {
        preferred_lateral_dir_ = -1.0;
      } else if (cost_diff < -HYSTERESIS_MARGIN) {
        preferred_lateral_dir_ = 1.0;
      }
    }

    // 若前瞻点前方空旷且两侧对称，重置偏好（已通过障碍）
    double ahead_cost = sample_cost(target_x, target_y);
    if (ahead_cost < 0.05 && std::abs(cost_diff) < 0.08) {
      preferred_lateral_dir_ = 0.0;
    }

    // 将 path_direction 向偏好侧微旋（±6°），避免过度偏转引起摆动
    if (std::abs(preferred_lateral_dir_) > 0.1) {
      const double MAX_BIAS_ANGLE = 6.0 * M_PI / 180.0;
      double bias_angle = preferred_lateral_dir_ * MAX_BIAS_ANGLE;
      double cos_ba = std::cos(bias_angle);
      double sin_ba = std::sin(bias_angle);
      double biased_x = path_direction_x * cos_ba - path_direction_y * sin_ba;
      double biased_y = path_direction_x * sin_ba + path_direction_y * cos_ba;
      path_direction_x = biased_x;
      path_direction_y = biased_y;
    }
  }

  // ── 探索范围距离衰减 ──
  // 越远越衰减: 远距离 → floor (跟随路径), 近距离 → 1.0 (充分探索精细定位)
  float dist_to_goal = std::hypot(target_x - current_x, target_y - current_y);
  float exploration_range_scale = 1.0f;
  if (dist_to_goal < exploration_decay_start_) {
    float t = (dist_to_goal - exploration_decay_end_)
            / (exploration_decay_start_ - exploration_decay_end_);
    t = std::max(0.0f, std::min(1.0f, t));
    // 反转: t=1(远) → floor, t=0(近) → 1.0
    exploration_range_scale = static_cast<float>(exploration_decay_floor_)
                            + (1.0f - static_cast<float>(exploration_decay_floor_)) * (1.0f - t);
  }

  // ── 动态探索调整: 前瞻点堵塞时扩大采样和噪声 (仅调整采样侧, 不影响代价权重) ──
  float dyn_guidance       = guidance_weight_ * (1.0f - lookahead_blockage * 0.9f);
  float dyn_explore_scale  = std::max(exploration_range_scale,
                                      0.3f + lookahead_blockage * 0.7f);
  float dyn_noise_vx       = noise_scale_floor_vx_ + lookahead_blockage * 0.4f;
  float dyn_noise_vy       = noise_scale_floor_vy_ + lookahead_blockage * 0.5f;
  float dyn_noise_w        = noise_scale_floor_w_  + lookahead_blockage * 0.4f;

  // 全局路径重采样 (供 GPU cross-track 计算)
  std::vector<float> host_path_x, host_path_y;
  int num_path_pts = 0;
  if (!global_plan_.poses.empty()) {
    int start_i = closest_idx;
    int end_i = static_cast<int>(global_plan_.poses.size()) - 1;
    int n_pts = std::min(MAX_PATH_POINTS, end_i - start_i + 1);
    if (n_pts >= 2) {
      double step = static_cast<double>(end_i - start_i) / (n_pts - 1);
      for (int k = 0; k < n_pts; ++k) {
        int idx = start_i + static_cast<int>(k * step);
        if (idx > end_i) idx = end_i;
        host_path_x.push_back(static_cast<float>(global_plan_.poses[idx].pose.position.x));
        host_path_y.push_back(static_cast<float>(global_plan_.poses[idx].pose.position.y));
      }
      num_path_pts = n_pts;
    }
  }
  float goal_yaw_f = 0.0f;
  if (!global_plan_.poses.empty())
    goal_yaw_f = static_cast<float>(tf2::getYaw(global_plan_.poses.back().pose.orientation));

  // TERMINAL_ALIGN: 近距离只需短时域预测
  if (state_ == ControllerState::TERMINAL_ALIGN) { H = 3; }

  // 创建 CUDA stream
  cudaStream_t stream;
  cudaStreamCreate(&stream);

  // GPU 采样 + 三组件代价计算
  int ret = mppi_gpu_sample_and_cost(
      noise_vx.data(), noise_vy.data(), noise_w.data(),
      base_vx_f.data(), base_vy_f.data(), base_w_f.data(),
      static_cast<float>(current_x), static_cast<float>(current_y), static_cast<float>(current_theta),
      static_cast<float>(target_x), static_cast<float>(target_y),
      costmap_data,
      costmap_w, costmap_h,
      costmap_res, costmap_origin_x, costmap_origin_y,
      static_cast<float>(dt_), static_cast<float>(min_v_), static_cast<float>(max_v_), static_cast<float>(max_vy_), static_cast<float>(max_w_),
      static_cast<float>(obstacle_weight_),
      static_cast<float>(heading_weight_),
      static_cast<float>(time_weight_),
      static_cast<float>(path_direction_x), static_cast<float>(path_direction_y),
      dyn_guidance,
      static_cast<float>(cross_track_noise_scale_),
      static_cast<float>(noise_decay_rate_),
      dyn_explore_scale,
      static_cast<float>(spatial_decay_weight_),
      dyn_noise_vx,
      dyn_noise_vy,
      dyn_noise_w,
      static_cast<float>(pure_rotation_ratio_),
      static_cast<int>(pure_rotation_steps_),
      static_cast<float>(pure_rotation_w_boost_),
      static_cast<float>(lateral_guidance_scale_),
      static_cast<float>(path_turn_angle),
      static_cast<float>(turn_lateral_boost_),
      static_cast<float>(turn_lateral_max_boost_),
      static_cast<float>(footprint_front_), static_cast<float>(footprint_back_),
      static_cast<float>(footprint_left_), static_cast<float>(footprint_right_),
      static_cast<float>(footprint_sample_spacing_),
      static_cast<float>(rear_obstacle_cost_),
      host_path_x.data(), host_path_y.data(), num_path_pts,
      goal_yaw_f,
      static_cast<float>(final_goal_x_),
      static_cast<float>(final_goal_y_),
      global_horizon_,
      static_cast<int>(N * global_trajectory_ratio_),
      N, H,
      d_noise_vx_, d_noise_vy_, d_noise_w_,
      d_base_vx_, d_base_vy_, d_base_w_,
      d_costmap_,
      d_path_x_, d_path_y_,
      d_costs_, d_sampled_vx_, d_sampled_vy_, d_sampled_w_,
      d_traj_x_, d_traj_y_,
      stream);

  bool gpu_success = true;

  if (ret != 0) {
    RCLCPP_ERROR(node_.lock()->get_logger(), "GPU 采样核函数失败，错误码: %d", ret);
    cudaStreamDestroy(stream);
    gpu_success = false;
  }

  std::vector<float> host_costs(N);
  std::vector<double> u_star_vx(H, 0.0), u_star_vy(H, 0.0), u_star_w(H, 0.0);
  double best_vx = 0.0, best_vy = 0.0, best_omega = 0.0;

  if (gpu_success) {
    cudaMemcpyAsync(host_costs.data(), d_costs_, N * sizeof(float), cudaMemcpyDeviceToHost, stream);
    cudaError_t e = cudaStreamSynchronize(stream);
    if (e != cudaSuccess) {
      RCLCPP_ERROR(node_.lock()->get_logger(), "cost sync fail: %s", cudaGetErrorString(e));
      cudaStreamDestroy(stream);
      gpu_success = false;
    }
  }

  float min_cost = std::numeric_limits<float>::max();
  if (gpu_success) {
    for (int i = 0; i < N; ++i) { if (host_costs[i] < min_cost) min_cost = host_costs[i]; }
    ret = mppi_gpu_weighted_sum(d_costs_, d_sampled_vx_, d_sampled_vy_, d_sampled_w_,
        d_result_seq_, min_cost, static_cast<float>(lambda_), N, H, stream);
    if (ret != 0) {
      RCLCPP_ERROR(node_.lock()->get_logger(), "weighted sum fail: %d", ret);
      cudaStreamDestroy(stream);
      gpu_success = false;
    }
  }

  if (gpu_success) {
    std::vector<float> host_result_seq(H * 4);
    cudaMemcpyAsync(host_result_seq.data(), d_result_seq_, H * 4 * sizeof(float), cudaMemcpyDeviceToHost, stream);
    cudaError_t e = cudaStreamSynchronize(stream);
    if (e != cudaSuccess) {
      RCLCPP_ERROR(node_.lock()->get_logger(), "result sync fail: %s", cudaGetErrorString(e));
      cudaStreamDestroy(stream);
      gpu_success = false;
    }
    if (gpu_success) {
      for (int t = 0; t < H; ++t) {
        float sw = host_result_seq[t * 4 + 3];
        if (sw > 1e-6f) {
          u_star_vx[t] = host_result_seq[t * 4 + 0] / sw;
          u_star_vy[t] = host_result_seq[t * 4 + 1] / sw;
          u_star_w[t] = host_result_seq[t * 4 + 2] / sw;
        }
      }
      best_vx = u_star_vx[0]; best_vy = u_star_vy[0]; best_omega = u_star_w[0];
    }
  }

  // Clamp
  best_vx = std::max(min_v_, std::min(max_v_, best_vx));
  best_vy = std::max(-max_vy_, std::min(max_vy_, best_vy));
  best_omega = std::max(-max_w_, std::min(max_w_, best_omega));

  // ── 输出 EMA 低通滤波：抑制帧间控制量跳变 ──
  // ema = α·current + (1-α)·prev
  // α=0: 完全冻结, α=1: 无平滑 (raw MPPI)
  // enable_ema=false: 完全跳过 EMA，直接输出 MPPI 原始控制量
  if (enable_ema_) {
    if (!ema_initialized_) {
      ema_cmd_vx_ = best_vx;
      ema_cmd_vy_ = best_vy;
      ema_cmd_w_  = best_omega;
      ema_initialized_ = true;
    } else {
      ema_cmd_vx_ = ema_alpha_ * best_vx + (1.0 - ema_alpha_) * ema_cmd_vx_;
      ema_cmd_vy_ = ema_alpha_ * best_vy + (1.0 - ema_alpha_) * ema_cmd_vy_;
      ema_cmd_w_  = ema_alpha_ * best_omega + (1.0 - ema_alpha_) * ema_cmd_w_;
    }
    best_vx = ema_cmd_vx_;
    best_vy = ema_cmd_vy_;
    best_omega = ema_cmd_w_;
  }

  // MPPI 滚动优化：直接存储当前帧的全时域最优序列 u*
  // 下一帧开头会执行一次移位 (optimal[i] = optimal[i+1]) 将时间对齐到新时刻。
  // 此处不再移位，避免与开头的移位叠加形成"双重移位"导致 u*[1] 被跳过。
  for (int i = 0; i < H; ++i) {
    optimal_vx_seq_[i] = u_star_vx[i];
    optimal_vy_seq_[i] = u_star_vy[i];
    optimal_omega_seq_[i] = u_star_w[i];
  }

  // ── 前瞻点 KP 减速 ──
  // 读取代价地图在前瞻点 (target_x, target_y) 处的代价，
  // 代价越高速度越低: scale = 1 - cost/254 * (1 - kp)
  if (costmap_data != nullptr && costmap_w > 0 && costmap_h > 0) {
    int mx = static_cast<int>((target_x - costmap_origin_x) / costmap_res);
    int my = static_cast<int>((target_y - costmap_origin_y) / costmap_res);
    if (mx >= 0 && mx < costmap_w && my >= 0 && my < costmap_h) {
      unsigned char cost = costmap_data[my * costmap_w + mx];
      if (cost > 0) {
        double norm_cost = static_cast<double>(cost) / 254.0;
        if (norm_cost > 1.0) norm_cost = 1.0;
        double scale = 1.0 - norm_cost * (1.0 - lookahead_kp_);
        best_vx *= scale;
        best_vy *= scale;
        RCLCPP_DEBUG(node_.lock()->get_logger(),
          "前瞻点 KP: cost=%d, scale=%.2f, vx=%.2f", cost, scale, best_vx);
      }
    }
  }



  // 朝向限速已由状态机 HEADING_MISALIGN 处理

  // ── 死区保护 ──
  {
    double df = 0.0;
    if (!global_plan_.poses.empty()) {
      df = std::hypot(global_plan_.poses.back().pose.position.x - current_x,
                      global_plan_.poses.back().pose.position.y - current_y);
    }
    if (std::hypot(best_vx, best_vy) < 0.005 && df > 0.02 && df < min_lookahead_dist_) {
      double dxf = final_goal_x_ - current_x, dyf = final_goal_y_ - current_y;
      double dist = std::hypot(dxf, dyf);
      if (dist > 0.01) {
        double c = std::cos(-current_theta), s = std::sin(-current_theta);
        double aspd = std::min(0.25, 1.2 * dist);
        best_vx = ((dxf / dist) * c - (dyf / dist) * s) * aspd;
        best_vy = ((dxf / dist) * s + (dyf / dist) * c) * aspd;
      }
    }
  }

  // ── 最终钳位: 每个指令独立，绝对值钳位保证后处理不越界 ──
  if (best_vx > 0.0) {
    best_vx = std::min(best_vx, max_v_);
  } else {
    best_vx = std::max(best_vx, min_v_);
  }
  best_vy    = std::copysign(std::min(std::abs(best_vy), max_vy_), best_vy);
  best_omega = std::copysign(std::min(std::abs(best_omega), max_w_), best_omega);

  cmd_vel.twist.linear.x = best_vx;
  cmd_vel.twist.linear.y = best_vy;
  cmd_vel.twist.angular.z = best_omega;

  if (gpu_success) {
    // 可视化：挑选代价最低的 10 条轨迹，从 GPU 拷贝
    const int TOP_K = 10;

    // 找出代价最低的 TOP_K 个索引
    std::vector<int> top_indices;
    {
      std::vector<std::pair<float, int>> cost_idx_pairs;
      cost_idx_pairs.reserve(N);
      for (int i = 0; i < N; ++i) {
        cost_idx_pairs.emplace_back(host_costs[i], i);
      }
      int n_select = std::min(TOP_K, N);
      std::partial_sort(cost_idx_pairs.begin(),
                        cost_idx_pairs.begin() + n_select,
                        cost_idx_pairs.end());
      for (int k = 0; k < n_select; ++k) {
        top_indices.push_back(cost_idx_pairs[k].second);
      }
    }

    int vis_samples = static_cast<int>(top_indices.size());
    std::vector<float> vis_traj_x(vis_samples * H);
    std::vector<float> vis_traj_y(vis_samples * H);

    // 按索引逐条拷贝（同一 stream 保证顺序）
    for (int k = 0; k < vis_samples; ++k) {
      int idx = top_indices[k];
      cudaMemcpyAsync(vis_traj_x.data() + k * H, d_traj_x_ + idx * H,
                      H * sizeof(float), cudaMemcpyDeviceToHost, stream);
      cudaMemcpyAsync(vis_traj_y.data() + k * H, d_traj_y_ + idx * H,
                      H * sizeof(float), cudaMemcpyDeviceToHost, stream);
    }

    // 最后一次同步：确保所有 stream 操作（含可视数据拷贝）完成
    cudaError_t vis_sync_err = cudaStreamSynchronize(stream);
    cudaStreamDestroy(stream);

    if (vis_sync_err != cudaSuccess) {
      RCLCPP_ERROR(node_.lock()->get_logger(),
        "cudaStreamSynchronize 失败 (vis): %s", cudaGetErrorString(vis_sync_err));
    }

    // top_indices[0] 代价最低，为最优轨迹（绿色），其余按代价升序排列
    int vis_best_idx = 0;

    publishVisualization(current_x, current_y, current_theta,
                         target_x, target_y,
                         vis_traj_x, vis_traj_y,
                         vis_samples, vis_best_idx,
                         "map");
  }

  // ── 运行时统计数据采集 ──
  if (enable_stats_) {
    // cross-track error: 当前位置到全局路径的最短距离
    double cross_track_err = std::numeric_limits<double>::max();
    if (!global_plan_.poses.empty() && closest_idx + 1 < static_cast<int>(global_plan_.poses.size())) {
      for (size_t i = 0; i + 1 < global_plan_.poses.size(); ++i) {
        double ax = global_plan_.poses[i].pose.position.x;
        double ay = global_plan_.poses[i].pose.position.y;
        double bx = global_plan_.poses[i + 1].pose.position.x;
        double by = global_plan_.poses[i + 1].pose.position.y;
        double abx = bx - ax, aby = by - ay;
        double ab_len_sq = abx * abx + aby * aby;
        double t = 0.0;
        if (ab_len_sq > 1e-9) {
          t = ((current_x - ax) * abx + (current_y - ay) * aby) / ab_len_sq;
          t = std::max(0.0, std::min(1.0, t));
        }
        double cx = ax + t * abx;
        double cy = ay + t * aby;
        double err = std::hypot(current_x - cx, current_y - cy);
        if (err < cross_track_err) cross_track_err = err;
      }
    }
    if (cross_track_err == std::numeric_limits<double>::max()) cross_track_err = 0.0;

    // heading error: 机器人朝向与路径方向之差
    double path_heading = std::atan2(path_direction_y, path_direction_x);
    double heading_err = path_heading - current_theta;
    while (heading_err > M_PI) heading_err -= 2.0 * M_PI;
    while (heading_err < -M_PI) heading_err += 2.0 * M_PI;

    // 到前瞻点距离
    double dist_to_goal = std::hypot(target_x - current_x, target_y - current_y);

    double now_sec = node_.lock()->now().seconds();

    recordStatsFrame(best_vx, best_vy, best_omega,
                     u_star_vx[0], u_star_vy[0], u_star_w[0],
                     cross_track_err, heading_err, dist_to_goal,
                     min_cost, now_sec);
  }

  // ── 帧日志 + 停滞检测 ──
  if (enable_file_log_ && logger_.is_open()) {
    double dfv = 0.0;
    int til = 0;
    if (!global_plan_.poses.empty()) {
      dfv = std::hypot(global_plan_.poses.back().pose.position.x - current_x,
                       global_plan_.poses.back().pose.position.y - current_y);
      double md = 1e9;
      for (size_t i = 0; i < global_plan_.poses.size(); ++i) {
        double d = std::hypot(global_plan_.poses[i].pose.position.x - target_x,
                              global_plan_.poses[i].pose.position.y - target_y);
        if (d < md) { md = d; til = static_cast<int>(i); }
      }
    }
    double he = std::atan2(path_direction_y, path_direction_x) - current_theta;
    logger_.logFrame(stats_frame_count_, best_vx, best_vy, best_omega,
      std::hypot(target_x - current_x, target_y - current_y), dfv,
      (enable_stats_ && !stats_frames_.empty()) ? stats_frames_.back().cross_track_err : 0.0,
      he, gpu_success ? min_cost : -1.0f, gpu_success ? min_cost : -1.0f,
      closest_idx, til, terminal_angle_active_, false, gpu_success,
      path_direction_x, path_direction_y);

    double cur_spd = std::hypot(best_vx, best_vy);
    double now_sec = node_.lock()->now().seconds();
    if (cur_spd < stall_speed_threshold_) {
      if (stall_start_time_ < 0.0) { stall_start_time_ = now_sec; last_stall_report_time_ = -1.0; }
      double sdur = now_sec - stall_start_time_;
      if (sdur >= 0.5 && (last_stall_report_time_ < 0.0 || now_sec - last_stall_report_time_ >= stall_report_interval_)) {
        unsigned char lhc = 0, ahc = 0;
        if (costmap_data != nullptr && costmap_w > 0 && costmap_h > 0) {
          int mx = (target_x - costmap_origin_x) / costmap_res, my = (target_y - costmap_origin_y) / costmap_res;
          if (mx >= 0 && mx < costmap_w && my >= 0 && my < costmap_h) lhc = costmap_data[my * costmap_w + mx];
          double ax = current_x + std::cos(current_theta) * 0.3, ay = current_y + std::sin(current_theta) * 0.3;
          mx = (ax - costmap_origin_x) / costmap_res; my = (ay - costmap_origin_y) / costmap_res;
          if (mx >= 0 && mx < costmap_w && my >= 0 && my < costmap_h) ahc = costmap_data[my * costmap_w + mx];
        }
        const char* reason = "unknown";
        if (!gpu_success) reason = "gpu_error";
        else if (terminal_angle_active_) {
          if (dfv <= 0.005) reason = "terminal_arrived";
          else reason = "terminal_aligning";
        }
        else if (dfv < 0.02) reason = "at_goal_not_fired";
        else if (lhc >= 150) reason = "lookahead_in_obstacle";
        else if (ahc >= 150) reason = "ahead_blocked";
        else if (std::hypot(target_x - current_x, target_y - current_y) < min_lookahead_dist_ * 0.4) reason = "lookahead_too_close";
        else if (gpu_success && min_cost > 50.0f) reason = "high_mppi_cost";
        else {
          double bs = 0;
          for (int i = 0; i < H; ++i) bs += std::hypot(optimal_vx_seq_[i], optimal_vy_seq_[i]);
          bs /= H;
          if (bs < 0.01) reason = "base_collapsed";
          else if (std::hypot(path_direction_x, path_direction_y) < 0.001) reason = "path_dir_zero";
          else if (!global_plan_.poses.empty() && closest_idx >= static_cast<int>(global_plan_.poses.size()) - 1) reason = "at_last_pt";
          else reason = "mppi_low_output";
        }
        logger_.logStall(sdur, cur_spd, std::hypot(target_x - current_x, target_y - current_y), dfv,
          static_cast<int>(lhc), static_cast<int>(ahc), closest_idx, til,
          static_cast<int>(global_plan_.poses.size()), terminal_angle_active_, false, gpu_success,
          gpu_success ? min_cost : -1.0f, reason);
        last_stall_report_time_ = now_sec;
      }
    } else {
      if (stall_start_time_ >= 0.0) stall_start_time_ = -1.0;
    }
  }

  // ── 推头诊断可视化: 前瞻点连线 + 速度指令方向 ──
  if (vis_pub_) {
    visualization_msgs::msg::MarkerArray diag_ma;
    auto diag_now = node_.lock()->now();
    auto diag_lifetime = rclcpp::Duration::from_seconds(0.15);

    // 1. 当前位置 → 前瞻点连线 (橙黄色)
    {
      visualization_msgs::msg::Marker m;
      m.header.frame_id = "map";
      m.header.stamp = diag_now;
      m.ns = "push_diag";
      m.id = 0;
      m.type = visualization_msgs::msg::Marker::LINE_STRIP;
      m.action = visualization_msgs::msg::Marker::ADD;
      m.points.resize(2);
      m.points[0].x = current_x; m.points[0].y = current_y; m.points[0].z = 0.12;
      m.points[1].x = target_x;  m.points[1].y = target_y;  m.points[1].z = 0.12;
      m.scale.x = 0.03;
      m.color.r = 1.0f; m.color.g = 0.8f; m.color.b = 0.0f; m.color.a = 0.9f;
      m.lifetime = diag_lifetime;
      diag_ma.markers.push_back(m);
    }

    // 2. 速度指令方向箭头 (品红色, 固定 0.5m 长)
    {
      double spd = std::hypot(best_vx, best_vy);
      double arrow_len = 0.5;
      if (spd > 0.05) {
        double c = std::cos(current_theta), s = std::sin(current_theta);
        double world_vx = best_vx * c - best_vy * s;
        double world_vy = best_vx * s + best_vy * c;
        double v_norm = std::hypot(world_vx, world_vy);
        if (v_norm > 1e-6) { world_vx /= v_norm; world_vy /= v_norm; }
        double tip_x = current_x + world_vx * arrow_len;
        double tip_y = current_y + world_vy * arrow_len;

        visualization_msgs::msg::Marker m;
        m.header.frame_id = "map";
        m.header.stamp = diag_now;
        m.ns = "push_diag";
        m.id = 1;
        m.type = visualization_msgs::msg::Marker::ARROW;
        m.action = visualization_msgs::msg::Marker::ADD;
        m.points.resize(2);
        m.points[0].x = current_x; m.points[0].y = current_y; m.points[0].z = 0.10;
        m.points[1].x = tip_x;     m.points[1].y = tip_y;     m.points[1].z = 0.10;
        m.scale.x = 0.04; m.scale.y = 0.08;
        m.color.r = 1.0f; m.color.g = 0.0f; m.color.b = 0.6f; m.color.a = 0.9f;
        m.lifetime = diag_lifetime;
        diag_ma.markers.push_back(m);
      }
    }

    // 3. 偏差指示箭头: 速度方向末端 → 前瞻点方向 (绿=对齐, 红=偏差大)
    {
      double c = std::cos(current_theta), s = std::sin(current_theta);
      double world_vx = best_vx * c - best_vy * s;
      double world_vy = best_vx * s + best_vy * c;
      double v_angle = std::atan2(world_vy, world_vx);
      double lh_angle = std::atan2(target_y - current_y, target_x - current_x);
      double angle_diff = lh_angle - v_angle;
      while (angle_diff > M_PI) angle_diff -= 2.0 * M_PI;
      while (angle_diff < -M_PI) angle_diff += 2.0 * M_PI;

      double base_x = current_x + std::cos(v_angle) * 0.4;
      double base_y = current_y + std::sin(v_angle) * 0.4;
      double tip2_x = base_x + std::cos(lh_angle) * 0.2;
      double tip2_y = base_y + std::sin(lh_angle) * 0.2;

      visualization_msgs::msg::Marker m;
      m.header.frame_id = "map";
      m.header.stamp = diag_now;
      m.ns = "push_diag";
      m.id = 2;
      m.type = visualization_msgs::msg::Marker::ARROW;
      m.action = visualization_msgs::msg::Marker::ADD;
      m.points.resize(2);
      m.points[0].x = base_x;  m.points[0].y = base_y;  m.points[0].z = 0.11;
      m.points[1].x = tip2_x;  m.points[1].y = tip2_y;  m.points[1].z = 0.11;
      m.scale.x = 0.03; m.scale.y = 0.06;
      double severity = std::min(1.0, std::abs(angle_diff) / 1.0);
      m.color.r = severity;
      m.color.g = 1.0f - severity;
      m.color.b = 0.0f;
      m.color.a = 0.8f;
      m.lifetime = diag_lifetime;
      diag_ma.markers.push_back(m);
    }

    vis_pub_->publish(diag_ma);
  }

  return cmd_vel;
}

void MPPIGPUController::publishVisualization(
  double robot_x, double robot_y, double robot_theta,
  double target_x, double target_y,
  const std::vector<float>& traj_data_x,
  const std::vector<float>& traj_data_y,
  int vis_samples,
  int best_idx,
  const std::string& frame_id)
{
  auto node_ptr = node_.lock();
  if (!node_ptr || !vis_pub_) return;

  int H = prediction_horizon_;

  visualization_msgs::msg::MarkerArray marker_array;
  rclcpp::Time now = node_ptr->now();
  // 设置生命周期为控制器周期的 2 倍，确保每帧刷新、旧 marker 自动过期
  auto marker_lifetime = rclcpp::Duration::from_seconds(0.1);

  // 机器人位置（红色）
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id;
    m.header.stamp = now;
    m.ns = "mppi_gpu";
    m.id = 0;
    m.type = visualization_msgs::msg::Marker::SPHERE;
    m.action = visualization_msgs::msg::Marker::ADD;
    m.lifetime = marker_lifetime;
    m.pose.position.x = robot_x;
    m.pose.position.y = robot_y;
    m.pose.position.z = 0.1;
    m.scale.x = 0.3;
    m.scale.y = 0.3;
    m.scale.z = 0.3;
    m.color.r = 1.0;
    m.color.g = 0.0;
    m.color.b = 0.0;
    m.color.a = 1.0;
    marker_array.markers.push_back(m);
  }

  // 机器人朝向（黄色箭头）
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id;
    m.header.stamp = now;
    m.ns = "mppi_gpu";
    m.id = 2;
    m.type = visualization_msgs::msg::Marker::LINE_STRIP;
    m.action = visualization_msgs::msg::Marker::ADD;
    m.lifetime = marker_lifetime;
    m.points.resize(2);
    m.points[0].x = robot_x;
    m.points[0].y = robot_y;
    m.points[0].z = 0.15;
    m.points[1].x = robot_x + std::cos(robot_theta) * 0.8;
    m.points[1].y = robot_y + std::sin(robot_theta) * 0.8;
    m.points[1].z = 0.15;
    m.scale.x = 0.08;
    m.color.r = 1.0;
    m.color.g = 1.0;
    m.color.b = 0.0;
    m.color.a = 1.0;
    marker_array.markers.push_back(m);
  }

  // ── 前瞻点最优摆放碰撞箱（青色矩形 + 朝向箭头）──
  {
    double cos_lh = std::cos(lookahead_theta_);
    double sin_lh = std::sin(lookahead_theta_);

    // 碰撞箱四个角点（机器人坐标系 → 世界坐标系）
    auto corner = [&](double lx, double ly) -> geometry_msgs::msg::Point {
      geometry_msgs::msg::Point p;
      p.x = target_x + lx * cos_lh - ly * sin_lh;
      p.y = target_y + lx * sin_lh + ly * cos_lh;
      p.z = 0.12;
      return p;
    };

    visualization_msgs::msg::Marker footprint;
    footprint.header.frame_id = frame_id;
    footprint.header.stamp = now;
    footprint.ns = "mppi_gpu";
    footprint.id = 3;
    footprint.type = visualization_msgs::msg::Marker::LINE_STRIP;
    footprint.action = visualization_msgs::msg::Marker::ADD;
    footprint.lifetime = marker_lifetime;
    footprint.scale.x = 0.04;
    footprint.color.r = 0.0;
    footprint.color.g = 1.0;
    footprint.color.b = 1.0;  // 青色
    footprint.color.a = 0.9;

    double fb = footprint_back_, ff = footprint_front_;
    double fl = footprint_left_, fr = footprint_right_;
    footprint.points.push_back(corner(-fb, -fl));
    footprint.points.push_back(corner( ff, -fl));
    footprint.points.push_back(corner( ff,  fr));
    footprint.points.push_back(corner(-fb,  fr));
    footprint.points.push_back(corner(-fb, -fl));  // 闭合
    marker_array.markers.push_back(footprint);

    // 朝向箭头（从中心向前 0.2m）
    visualization_msgs::msg::Marker heading_arrow;
    heading_arrow.header.frame_id = frame_id;
    heading_arrow.header.stamp = now;
    heading_arrow.ns = "mppi_gpu";
    heading_arrow.id = 4;
    heading_arrow.type = visualization_msgs::msg::Marker::LINE_STRIP;
    heading_arrow.action = visualization_msgs::msg::Marker::ADD;
    heading_arrow.lifetime = marker_lifetime;
    heading_arrow.scale.x = 0.06;
    heading_arrow.color.r = 0.0;
    heading_arrow.color.g = 1.0;
    heading_arrow.color.b = 1.0;
    heading_arrow.color.a = 0.9;
    {
      geometry_msgs::msg::Point p;
      p.x = target_x; p.y = target_y; p.z = 0.13;
      heading_arrow.points.push_back(p);
      p.x = target_x + 0.2 * cos_lh;
      p.y = target_y + 0.2 * sin_lh;
      heading_arrow.points.push_back(p);
    }
    marker_array.markers.push_back(heading_arrow);
  }

  // 采样轨迹
  for (int i = 0; i < vis_samples; ++i) {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id;
    m.header.stamp = now;
    m.ns = "samples_gpu";
    m.id = i;
    m.type = visualization_msgs::msg::Marker::LINE_STRIP;
    m.action = visualization_msgs::msg::Marker::ADD;
    m.lifetime = marker_lifetime;

    // 起点：机器人当前位置（轨迹从这里开始）
    {
      geometry_msgs::msg::Point p;
      p.x = robot_x;
      p.y = robot_y;
      p.z = 0.05;
      m.points.push_back(p);
    }

    // GPU 预测的 H 步位置，到达目标附近时截断
    const float GOAL_REACHED_DIST = 0.25f;
    for (int t = 0; t < H; ++t) {
      float px = traj_data_x[i * H + t];
      float py = traj_data_y[i * H + t];
      geometry_msgs::msg::Point p;
      p.x = px;
      p.y = py;
      p.z = 0.05;
      m.points.push_back(p);
      float dx_g = static_cast<float>(target_x) - px;
      float dy_g = static_cast<float>(target_y) - py;
      if (std::hypot(dx_g, dy_g) < GOAL_REACHED_DIST) break;
    }

    if (i == best_idx) {
      m.color.r = 0.0;
      m.color.g = 1.0;
      m.color.b = 0.0;
      m.color.a = 0.8;
      m.scale.x = 0.06;
    } else {
      m.color.r = 0.7;
      m.color.g = 0.7;
      m.color.b = 0.7;
      m.color.a = 0.5;
      m.scale.x = 0.04;
    }
    marker_array.markers.push_back(m);
  }

  vis_pub_->publish(marker_array);
}

void MPPIGPUController::recordStatsFrame(
  double vx, double vy, double omega,
  double vx_raw, double vy_raw, double omega_raw,
  double cross_track_err, double heading_err,
  double dist_to_goal, float best_cost,
  double current_time)
{
  StatsFrame f;
  f.frame = stats_frame_count_++;
  f.time_s = current_time - stats_start_time_;
  f.vx = vx;
  f.vy = vy;
  f.omega = omega;
  f.vx_raw = vx_raw;
  f.vy_raw = vy_raw;
  f.omega_raw = omega_raw;
  f.cross_track_err = cross_track_err;
  f.heading_err = heading_err;
  f.dist_to_goal = dist_to_goal;
  f.best_cost = best_cost;

  // 突变检测：相邻帧间原始控制量跳变超过阈值
  if (has_prev_stats_) {
    f.mutation_vx = std::abs(vx_raw - prev_vx_raw_) > mutation_thresh_vx_;
    f.mutation_vy = std::abs(vy_raw - prev_vy_raw_) > mutation_thresh_vy_;
    f.mutation_w  = std::abs(omega_raw - prev_omega_raw_) > mutation_thresh_w_;
  } else {
    f.mutation_vx = false;
    f.mutation_vy = false;
    f.mutation_w = false;
  }

  prev_vx_raw_ = vx_raw;
  prev_vy_raw_ = vy_raw;
  prev_omega_raw_ = omega_raw;
  has_prev_stats_ = true;

  stats_frames_.push_back(f);
}

void MPPIGPUController::writeStatsToFile()
{
  if (!enable_stats_ || stats_frames_.empty()) return;

  std::ofstream ofs(stats_file_path_);
  if (!ofs.is_open()) {
    RCLCPP_ERROR(node_.lock()->get_logger(),
      "无法打开统计数据文件: %s", stats_file_path_.c_str());
    return;
  }

  ofs << std::fixed << std::setprecision(4);

  // 表头
  ofs << "# MPPI GPU Controller Statistics\n";
  ofs << "# frame,time_s,vx,vy,omega,vx_raw,vy_raw,omega_raw,"
      << "cross_track_err,heading_err,dist_to_goal,"
      << "mutation_vx,mutation_vy,mutation_w,best_cost\n";

  // 逐帧数据
  for (const auto& f : stats_frames_) {
    ofs << f.frame << "," << f.time_s << ","
        << f.vx << "," << f.vy << "," << f.omega << ","
        << f.vx_raw << "," << f.vy_raw << "," << f.omega_raw << ","
        << f.cross_track_err << "," << f.heading_err << "," << f.dist_to_goal << ","
        << (f.mutation_vx ? 1 : 0) << ","
        << (f.mutation_vy ? 1 : 0) << ","
        << (f.mutation_w ? 1 : 0) << ","
        << f.best_cost << "\n";
  }

  // ── 汇总统计 ──
  int n = static_cast<int>(stats_frames_.size());
  if (n == 0) { ofs.close(); return; }

  auto compute_stats = [&](const auto& getter) -> std::tuple<double, double, double, double> {
    double sum = 0.0, sq_sum = 0.0, min_v = 1e18, max_v = -1e18;
    for (const auto& f : stats_frames_) {
      double v = getter(f);
      sum += v;
      sq_sum += v * v;
      if (v < min_v) min_v = v;
      if (v > max_v) max_v = v;
    }
    double mean = sum / n;
    double stddev = std::sqrt(sq_sum / n - mean * mean);
    return {mean, stddev, min_v, max_v};
  };

  auto [avg_vx, std_vx, min_vx, max_vx] = compute_stats([](const StatsFrame& f) { return f.vx; });
  auto [avg_vy, std_vy, min_vy, max_vy] = compute_stats([](const StatsFrame& f) { return f.vy; });
  auto [avg_w, std_w, min_w, max_w] = compute_stats([](const StatsFrame& f) { return f.omega; });
  auto [avg_cte, std_cte, min_cte, max_cte] = compute_stats([](const StatsFrame& f) { return f.cross_track_err; });
  auto [avg_he, std_he, min_he, max_he] = compute_stats([](const StatsFrame& f) { return std::abs(f.heading_err); });

  int mut_vx_cnt = 0, mut_vy_cnt = 0, mut_w_cnt = 0;
  for (const auto& f : stats_frames_) {
    if (f.mutation_vx) mut_vx_cnt++;
    if (f.mutation_vy) mut_vy_cnt++;
    if (f.mutation_w)  mut_w_cnt++;
  }

  ofs << "#\n# ── 汇总统计 (n=" << n << ") ──\n";
  ofs << "# avg_vx: " << avg_vx << ", std_vx: " << std_vx
      << ", min_vx: " << min_vx << ", max_vx: " << max_vx << "\n";
  ofs << "# avg_vy: " << avg_vy << ", std_vy: " << std_vy
      << ", min_vy: " << min_vy << ", max_vy: " << max_vy << "\n";
  ofs << "# avg_omega: " << avg_w << ", std_omega: " << std_w
      << ", min_omega: " << min_w << ", max_omega: " << max_w << "\n";
  ofs << "# avg_cross_track_err: " << avg_cte << ", max_cross_track_err: " << max_cte << "\n";
  ofs << "# avg_abs_heading_err: " << avg_he << ", max_abs_heading_err: " << max_he << "\n";
  ofs << "# mutation_rate_vx: " << (static_cast<double>(mut_vx_cnt) / n)
      << " (" << mut_vx_cnt << "/" << n << "), thresh=" << mutation_thresh_vx_ << "\n";
  ofs << "# mutation_rate_vy: " << (static_cast<double>(mut_vy_cnt) / n)
      << " (" << mut_vy_cnt << "/" << n << "), thresh=" << mutation_thresh_vy_ << "\n";
  ofs << "# mutation_rate_w: " << (static_cast<double>(mut_w_cnt) / n)
      << " (" << mut_w_cnt << "/" << n << "), thresh=" << mutation_thresh_w_ << "\n";

  ofs.close();

  RCLCPP_INFO(node_.lock()->get_logger(),
    "统计数据已写入: %s (%d 帧)", stats_file_path_.c_str(), n);
}

}  // namespace nav2_custom_plugins

PLUGINLIB_EXPORT_CLASS(nav2_custom_plugins::MPPIGPUController, nav2_core::Controller)
