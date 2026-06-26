#include "nav2_custom_plugins/mppi_gpu_controller.hpp"
#include <limits>
#include <cmath>
#include <algorithm>
#include <random>
#include <cstring>
#include <fstream>
#include <iomanip>
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
    float costmap_weight,
    float path_dir_x, float path_dir_y,
    float guidance_weight,
    float cross_track_noise_scale,
    float noise_decay_rate,
    float exploration_range_scale,
    float spatial_decay_weight,
    float noise_scale_floor_vx,
    float noise_scale_floor_vy,
    float noise_scale_floor_w,
    float pure_rotation_ratio,
    int pure_rotation_steps,
    float pure_rotation_w_boost,
    float vel_direction_weight,
    float speed_reward_weight,
    float heading_weight,
    float cost_vy_threshold,
    float lateral_guidance_scale,
    float path_turn_angle,
    float turn_lateral_boost,
    float lookahead_proximity_weight,
    float lookahead_proximity_decay,
    float lookahead_theta,
    float fp_front, float fp_back, float fp_left, float fp_right,
    const float* path_x, const float* path_y,
    int num_path_pts,
    float path_attraction_weight,
    // ── 代价函数内部参数（原硬编码常量） ──
    float terminal_dist_weight,
    float path_length_weight,
    float goal_attraction_weight,
    float path_follow_scale_increment,
    float goal_soft_radius,
    float base_speed_floor_ratio,
    float turn_lateral_max_boost,
    float footprint_sample_spacing,
    float rear_obstacle_cost,

    // ── 分层规划 ──
    float final_goal_x, float final_goal_y, float final_goal_yaw,
    int global_horizon, int num_global_trajs,

    int num_samples, int horizon,
    float* d_noise_vx, float* d_noise_vy, float* d_noise_w,
    float* d_base_vx, float* d_base_vy, float* d_base_w,
    unsigned char* d_costmap,
    float* d_path_x, float* d_path_y,
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
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "path_attraction_weight",       path_attraction_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lookahead_time",               lookahead_time_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "min_lookahead_dist",           min_lookahead_dist_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "guidance_weight",              guidance_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "cross_track_noise_scale",      cross_track_noise_scale_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "noise_decay_rate",             noise_decay_rate_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "vel_direction_weight",         vel_direction_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "speed_reward_weight",          speed_reward_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "heading_weight",               heading_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lateral_guidance_scale",       lateral_guidance_scale_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "turn_lateral_boost",           turn_lateral_boost_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "ema_alpha",                    ema_alpha_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "enable_ema",                   enable_ema_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lookahead_proximity_weight",   lookahead_proximity_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "lookahead_proximity_decay",    lookahead_proximity_decay_);
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
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "collision_cost",               costmap_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "enable_lateral_bias",          enable_lateral_bias_);

  // ── 代价函数内部参数（原硬编码常量，现暴露为可配置参数） ──
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "cost_vy_threshold",             cost_vy_threshold_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "terminal_dist_weight",         terminal_dist_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "path_length_weight",           path_length_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "goal_attraction_weight",       goal_attraction_weight_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "path_follow_scale_increment",  path_follow_scale_increment_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "goal_soft_radius",             goal_soft_radius_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "base_speed_floor_ratio",       base_speed_floor_ratio_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "turn_lateral_max_boost",       turn_lateral_max_boost_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "footprint_sample_spacing",     footprint_sample_spacing_);
  DECLARE_GET_PARAM(node_ptr, plugin_name_, "rear_obstacle_cost",           rear_obstacle_cost_);

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

  allocateGPUBuffers();

  RCLCPP_INFO(node_.lock()->get_logger(), "配置 MPPIGPUController (GPU加速) 成功！");
}

void MPPIGPUController::cleanup()
{
  RCLCPP_INFO(node_.lock()->get_logger(), "清理 MPPIGPUController");
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
  global_plan_ = path;
  // 新路径到达时重置终端状态，但保留 lookahead_theta_ 全局持久朝向
  // (朝向变化由变化率限制器控制，跨路径重规划不突变)
  terminal_angle_active_ = false;
}

void MPPIGPUController::setSpeedLimit(const double &, const bool &)
{
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

    const double TERMINAL_HYSTERESIS = 0.10;  // 退出迟滞，进入 0.10m，退出 0.20m
    if (!terminal_angle_active_ && dist_to_final < terminal_angle_dist_) {
      terminal_angle_active_ = true;
    } else if (terminal_angle_active_ && dist_to_final > terminal_angle_dist_ + TERMINAL_HYSTERESIS) {
      terminal_angle_active_ = false;
    }

    if (terminal_angle_active_) {
      double goal_yaw = tf2::getYaw(global_plan_.poses.back().pose.orientation);
      double yaw_err = goal_yaw - current_theta;
      while (yaw_err > M_PI) yaw_err -= 2.0 * M_PI;
      while (yaw_err < -M_PI) yaw_err += 2.0 * M_PI;

      cmd_vel.twist.linear.x = 0.0;
      cmd_vel.twist.linear.y = 0.0;
      cmd_vel.twist.angular.z = std::max(-max_w_, std::min(max_w_,
        terminal_angle_kp_ * yaw_err));

      // 朝向到位 → 完全停止，重置状态
      if (std::abs(yaw_err) < terminal_angle_tolerance_) {
        cmd_vel.twist.angular.z = 0.0;
        terminal_angle_active_ = false;
      }

      return cmd_vel;
    }

    // 前瞻点：沿路径插值取精确的 min_lookahead_dist_ 米处
    // 在路径段上线性插值，而非直接取离散路径点，避免前瞻点跳变
    int last_idx = static_cast<int>(global_plan_.poses.size()) - 1;
    double raw_lookahead_x = target_x;
    double raw_lookahead_y = target_y;
    bool found = false;
    int found_seg_end = closest_idx;  // 记录前瞻点所在段的终点索引，用于到达后推进

    // ── 到达推进：若机器人已接近前瞻点，增大有效前瞻距离 ──
    // 避免机器人"追上"前瞻点后停滞不前
    double effective_lookahead = min_lookahead_dist_;
    {
      double dx_to_prev = prev_lookahead_x_ - current_x;
      double dy_to_prev = prev_lookahead_y_ - current_y;
      double dist_to_prev = std::hypot(dx_to_prev, dy_to_prev);
      // 上一帧前瞻点足够近 → 机器人已到达，本帧推进
      const double ARRIVE_THRESHOLD = min_lookahead_dist_ * 0.5;
      if (has_prev_lookahead_ && dist_to_prev < ARRIVE_THRESHOLD) {
        effective_lookahead = min_lookahead_dist_ * 2.0;
      }
    }

    // ── 第一次搜索：从 closest_idx 出发累积弧长 ──
    double accumulated = 0.0;
    for (int i = closest_idx; i < last_idx; ++i) {
      double dx = global_plan_.poses[i + 1].pose.position.x
                - global_plan_.poses[i].pose.position.x;
      double dy = global_plan_.poses[i + 1].pose.position.y
                - global_plan_.poses[i].pose.position.y;
      double seg_len = std::hypot(dx, dy);

      if (accumulated + seg_len >= effective_lookahead) {
        double remaining = effective_lookahead - accumulated;
        double alpha = (seg_len > 1e-9) ? remaining / seg_len : 0.0;
        alpha = std::max(0.0, std::min(1.0, alpha));
        raw_lookahead_x = global_plan_.poses[i].pose.position.x + alpha * dx;
        raw_lookahead_y = global_plan_.poses[i].pose.position.y + alpha * dy;
        found_seg_end = i + 1;
        found = true;
        break;
      }
      accumulated += seg_len;
    }

    // 路径不够长时，退化为终点
    if (!found && last_idx >= closest_idx) {
      raw_lookahead_x = global_plan_.poses[last_idx].pose.position.x;
      raw_lookahead_y = global_plan_.poses[last_idx].pose.position.y;
      found_seg_end = last_idx;
    }

    // 直接使用插值结果（始终在路径上），不做笛卡尔空间 EMA 平滑。
    // 增量式 closest_idx 搜索已消除索引跳变，不再需要平滑来抑制抖动。
    target_x = raw_lookahead_x;
    target_y = raw_lookahead_y;
    if (!has_prev_lookahead_) {
      has_prev_lookahead_ = true;
    }
    prev_lookahead_x_ = target_x;
    prev_lookahead_y_ = target_y;
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

  // 提取全局路径点并均匀重采样（用于 GPU 内核的 cross-track error 计算）
  std::vector<float> host_path_x, host_path_y;
  int num_path_pts = 0;

  if (!global_plan_.poses.empty() && closest_idx < static_cast<int>(global_plan_.poses.size())) {
    // 第一步：计算从 closest_idx 到末尾的累计距离
    std::vector<double> cum_dist;
    cum_dist.push_back(0.0);
    double total_dist = 0.0;

    for (int i = closest_idx + 1; i < static_cast<int>(global_plan_.poses.size()); ++i) {
      double dx = global_plan_.poses[i].pose.position.x - global_plan_.poses[i - 1].pose.position.x;
      double dy = global_plan_.poses[i].pose.position.y - global_plan_.poses[i - 1].pose.position.y;
      total_dist += std::hypot(dx, dy);
      cum_dist.push_back(total_dist);
    }

    if (total_dist > 0.05 && cum_dist.size() >= 2) {
      // 第二步：按均匀步长采样 MAX_PATH_POINTS 个点
      double sample_dist = total_dist / (MAX_PATH_POINTS - 1);
      int seg_idx = 0;  // 当前所在的路径段索引

      for (int k = 0; k < MAX_PATH_POINTS; ++k) {
        double target_dist = k * sample_dist;

        // 找到包含 target_dist 的段
        while (seg_idx + 1 < static_cast<int>(cum_dist.size()) &&
               cum_dist[seg_idx + 1] < target_dist) {
          seg_idx++;
        }

        int i0 = closest_idx + seg_idx;
        int i1 = i0 + 1;
        if (i1 >= static_cast<int>(global_plan_.poses.size())) {
          i1 = static_cast<int>(global_plan_.poses.size()) - 1;
        }

        double seg_start = cum_dist[seg_idx];
        double seg_end = (seg_idx + 1 < static_cast<int>(cum_dist.size()))
                             ? cum_dist[seg_idx + 1]
                             : total_dist;
        double seg_len = seg_end - seg_start;
        double alpha = (seg_len > 1e-9) ? (target_dist - seg_start) / seg_len : 0.0;
        alpha = std::max(0.0, std::min(1.0, alpha));

        double px = global_plan_.poses[i0].pose.position.x +
                    alpha * (global_plan_.poses[i1].pose.position.x -
                             global_plan_.poses[i0].pose.position.x);
        double py = global_plan_.poses[i0].pose.position.y +
                    alpha * (global_plan_.poses[i1].pose.position.y -
                             global_plan_.poses[i0].pose.position.y);

        host_path_x.push_back(static_cast<float>(px));
        host_path_y.push_back(static_cast<float>(py));
      }
      num_path_pts = MAX_PATH_POINTS;
    }
  }

  // 初始化或滚动最优控制序列
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
    // 连续 H 帧清零会导致整条序列归零，轨迹失去前向引导而四散
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

  for (int i = 0; i < N * H; ++i) {
    noise_vx[i] = static_cast<float>(dist_vx_(generator_));
    noise_vy[i] = static_cast<float>(dist_vy_(generator_));
    noise_w[i]  = static_cast<float>(dist_w_(generator_));
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

  // ── 前瞻点接近奖励衰减自适应 ──
  // 确保奖励梯度覆盖从机器人当前位置到前瞻点的完整距离:
  //   effective_decay = max(配置下限, 实际前瞻距离)
  // 远距离 → 平缓梯度引导接近; 近距离 → 保持配置的锐利衰减精细定位
  float effective_proximity_decay = std::max(
      static_cast<float>(lookahead_proximity_decay_),
      dist_to_goal);

  // 创建 CUDA stream
  cudaStream_t stream;
  cudaStreamCreate(&stream);

  // GPU 采样 + 代价计算
  int ret = mppi_gpu_sample_and_cost(
      noise_vx.data(), noise_vy.data(), noise_w.data(),
      base_vx_f.data(), base_vy_f.data(), base_w_f.data(),
      static_cast<float>(current_x), static_cast<float>(current_y), static_cast<float>(current_theta),
      static_cast<float>(target_x), static_cast<float>(target_y),
      costmap_data,
      costmap_w, costmap_h,
      costmap_res, costmap_origin_x, costmap_origin_y,
      static_cast<float>(dt_), static_cast<float>(min_v_), static_cast<float>(max_v_), static_cast<float>(max_vy_), static_cast<float>(max_w_),
      static_cast<float>(costmap_weight_),
      static_cast<float>(path_direction_x), static_cast<float>(path_direction_y),
      static_cast<float>(guidance_weight_),
      static_cast<float>(cross_track_noise_scale_),
      static_cast<float>(noise_decay_rate_),
      exploration_range_scale,
      static_cast<float>(spatial_decay_weight_),
      static_cast<float>(noise_scale_floor_vx_),
      static_cast<float>(noise_scale_floor_vy_),
      static_cast<float>(noise_scale_floor_w_),
      static_cast<float>(pure_rotation_ratio_),
      static_cast<int>(pure_rotation_steps_),
      static_cast<float>(pure_rotation_w_boost_),
      static_cast<float>(vel_direction_weight_),
      static_cast<float>(speed_reward_weight_),
      static_cast<float>(heading_weight_),
      static_cast<float>(cost_vy_threshold_),
      static_cast<float>(lateral_guidance_scale_),
      static_cast<float>(path_turn_angle),
      static_cast<float>(turn_lateral_boost_),
      static_cast<float>(lookahead_proximity_weight_),
      effective_proximity_decay,
      static_cast<float>(lookahead_theta_),
      static_cast<float>(footprint_front_), static_cast<float>(footprint_back_),
      static_cast<float>(footprint_left_), static_cast<float>(footprint_right_),
      host_path_x.data(), host_path_y.data(),
      num_path_pts,
      static_cast<float>(path_attraction_weight_),
      static_cast<float>(terminal_dist_weight_),
      static_cast<float>(path_length_weight_),
      static_cast<float>(goal_attraction_weight_),
      static_cast<float>(path_follow_scale_increment_),
      static_cast<float>(goal_soft_radius_),
      static_cast<float>(base_speed_floor_ratio_),
      static_cast<float>(turn_lateral_max_boost_),
      static_cast<float>(footprint_sample_spacing_),
      static_cast<float>(rear_obstacle_cost_),
      // ── 分层规划参数 ──
      static_cast<float>(final_goal_x_),
      static_cast<float>(final_goal_y_),
      static_cast<float>(final_goal_yaw_),
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

  if (ret != 0) {
    RCLCPP_ERROR(node_.lock()->get_logger(), "GPU 采样核函数失败，错误码: %d", ret);
    cudaStreamDestroy(stream);
    cmd_vel.twist.linear.x = 0.0;
    cmd_vel.twist.linear.y = 0.0;
    cmd_vel.twist.angular.z = 0.0;
    return cmd_vel;
  }

  // 拷贝代价回 CPU 以查找最小值
  std::vector<float> host_costs(N);
  cudaMemcpyAsync(host_costs.data(), d_costs_, N * sizeof(float), cudaMemcpyDeviceToHost, stream);
  cudaError_t sync_err = cudaStreamSynchronize(stream);
  if (sync_err != cudaSuccess) {
    RCLCPP_ERROR(node_.lock()->get_logger(),
      "cudaStreamSynchronize 失败 (代价拷贝): %s", cudaGetErrorString(sync_err));
    cudaStreamDestroy(stream);
    cmd_vel.twist.linear.x = 0.0;
    cmd_vel.twist.linear.y = 0.0;
    cmd_vel.twist.angular.z = 0.0;
    return cmd_vel;
  }

  float min_cost = std::numeric_limits<float>::max();
  for (int i = 0; i < N; ++i) {
    if (host_costs[i] < min_cost) {
      min_cost = host_costs[i];
    }
  }

  // GPU 加权求和 —— 对全部 H 个 timestep 独立计算
  ret = mppi_gpu_weighted_sum(
      d_costs_, d_sampled_vx_, d_sampled_vy_, d_sampled_w_,
      d_result_seq_,
      min_cost,
      static_cast<float>(lambda_),
      N, H,
      stream);

  if (ret != 0) {
    RCLCPP_ERROR(node_.lock()->get_logger(), "GPU 加权求和核函数失败，错误码: %d", ret);
    cudaStreamDestroy(stream);
    cmd_vel.twist.linear.x = 0.0;
    cmd_vel.twist.linear.y = 0.0;
    cmd_vel.twist.angular.z = 0.0;
    return cmd_vel;
  }

  // 拷贝全时域加权结果回 CPU（H × 4 个 float）
  std::vector<float> host_result_seq(H * 4);
  cudaMemcpyAsync(host_result_seq.data(), d_result_seq_, H * 4 * sizeof(float),
                  cudaMemcpyDeviceToHost, stream);
  sync_err = cudaStreamSynchronize(stream);
  if (sync_err != cudaSuccess) {
    RCLCPP_ERROR(node_.lock()->get_logger(),
      "cudaStreamSynchronize 失败 (结果拷贝): %s", cudaGetErrorString(sync_err));
    cudaStreamDestroy(stream);
    cmd_vel.twist.linear.x = 0.0;
    cmd_vel.twist.linear.y = 0.0;
    cmd_vel.twist.angular.z = 0.0;
    return cmd_vel;
  }

  // 解析全时域加权结果: u*_t = Σ w_k · sampled_u[k,t] / Σ w_k
  std::vector<double> u_star_vx(H, 0.0);
  std::vector<double> u_star_vy(H, 0.0);
  std::vector<double> u_star_w(H, 0.0);

  for (int t = 0; t < H; ++t) {
    float sum_w = host_result_seq[t * 4 + 3];
    if (sum_w > 1e-6f) {
      u_star_vx[t] = static_cast<double>(host_result_seq[t * 4 + 0] / sum_w);
      u_star_vy[t] = static_cast<double>(host_result_seq[t * 4 + 1] / sum_w);
      u_star_w[t]  = static_cast<double>(host_result_seq[t * 4 + 2] / sum_w);
    }
  }

  // 当前执行的控制量 = 优化序列的第一步 u*[0]
  double best_vx = u_star_vx[0];
  double best_vy = u_star_vy[0];
  double best_omega = u_star_w[0];

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



  // ── 朝向偏差限速: 防止机器人在严重偏航时高速移动，确保先转向再前进 ──
  if (enable_heading_speed_limit_) {
    double dx_target = target_x - current_x;
    double dy_target = target_y - current_y;
    double target_dist = std::hypot(dx_target, dy_target);
    if (target_dist > 0.05) {
      double target_dir = std::atan2(dy_target, dx_target);
      double heading_err = target_dir - current_theta;
      while (heading_err > M_PI) heading_err -= 2.0 * M_PI;
      while (heading_err < -M_PI) heading_err += 2.0 * M_PI;

      if (std::abs(heading_err) > heading_misalign_threshold_) {
        double speed = std::hypot(best_vx, best_vy);
        if (speed > heading_misalign_max_speed_) {
          double scale = heading_misalign_max_speed_ / speed;
          best_vx *= scale;
          best_vy *= scale;
        }
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
                       pose.header.frame_id);

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
