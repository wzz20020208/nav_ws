#ifndef NAV2_CUSTOM_PLUGINS__MPPI_GPU_CONTROLLER_HPP_
#define NAV2_CUSTOM_PLUGINS__MPPI_GPU_CONTROLLER_HPP_

#include <string>
#include <memory>
#include <vector>
#include <random>
#include <cstdint>

#include "nav2_core/controller.hpp"
#include "rclcpp/rclcpp.hpp"
#include "pluginlib/class_list_macros.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/path.hpp"
#include "nav2_costmap_2d/costmap_2d_ros.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "nav2_custom_plugins/mppi_gpu_logger.hpp"

namespace nav2_custom_plugins
{

/// 单帧统计数据，用于运行时性能分析与参数调优
struct StatsFrame
{
  int frame;                       // 帧序号
  double time_s;                   // 自首帧起的相对时间 (s)
  double vx, vy, omega;            // EMA 平滑后的输出控制量
  double vx_raw, vy_raw, omega_raw;// EMA 平滑前的原始 MPPI 输出
  double cross_track_err;          // 到全局路径的横向误差 (m)
  double heading_err;              // 机器人朝向与路径方向的偏差 (rad)
  double dist_to_goal;             // 到前瞻点的距离 (m)
  bool mutation_vx, mutation_vy, mutation_w; // 本帧是否发生突变
  float best_cost;                 // 最优轨迹代价
};

/// 控制器状态机 (优先级: TERMINAL > LATERAL > HEADING > NARROW > NORMAL)
enum class ControllerState : uint8_t
{
  NORMAL = 0,
  HEADING_MISALIGN = 1,
  NARROW_PASSAGE = 2,
  TERMINAL_ALIGN = 3
};

/// 窄道子状态: 侧身沿路径脱离
enum class NarrowSubState : uint8_t
{
  ROTATE_TO_SIDE = 0,  // 旋转至路径切线方向 (侧身姿态)
  MOVE_ALONG_PATH       // 逐帧沿路径侧移, 每 10cm 检测碰撞
};

/**
 * @class MPPIGPUController
 * @brief GPU 加速的 MPPI 控制器
 *
 * 将 MPPI 控制器的采样、轨迹滚动和代价计算卸载到 GPU 上并行执行。
 * CUDA 线程一对一映射到采样轨迹，实现大规模并行加速。
 * 在 Jetson Orin 等嵌入式 GPU 平台上可获得显著性能提升。
 */
class MPPIGPUController : public nav2_core::Controller
{
public:
  MPPIGPUController() = default;
  ~MPPIGPUController() override = default;

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

  // 全局代价地图订阅（map 帧，全图尺寸，补充局部代价地图覆盖不到的区域）
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr global_costmap_sub_;
  nav_msgs::msg::OccupancyGrid::SharedPtr latest_global_costmap_;
  bool use_global_costmap_ = true;
  std::string global_costmap_topic_ = "/global_costmap/costmap";

  // MPPI 参数
  int num_samples_ = 1000;
  int prediction_horizon_ = 15;
  double dt_ = 0.3;
  double max_v_ = 2.0;
  double min_v_ = -2.0;
  double max_vy_ = 2.0;
  double max_w_ = 0.5;
  // 各维度采样噪声下限 (替代单一 noise_scale_floor_, 可独立设置)
  double noise_scale_floor_vx_ = 0.15; // vx 噪声尺度下限 (0~1)
  double noise_scale_floor_vy_ = 0.15; // vy 噪声尺度下限 (0~1)
  double noise_scale_floor_w_  = 0.15; // omega 噪声尺度下限 (0~1)
  // 纯角速度采样: 部分轨迹先原地旋转对准路径，再平移前进
  double pure_rotation_ratio_ = 0.15;   // 纯旋转轨迹占比 (0~1)
  int pure_rotation_steps_ = 3;         // 纯旋转持续步数
  double pure_rotation_w_boost_ = 1.5;  // 旋转步 omega 噪声增强倍数
  double action_std_v_ = 0.2;
  double action_std_vy_ = 0.15;
  double action_std_w_ = 0.2;
  // ── NLN 混合采样 (Log-MPPI) ──
  double nln_ratio_ = 0.3;    // 对数正态采样占比 (0=纯高斯, 1=纯对数正态)
  double nln_sigma_mult_ = 3.0; // 对数正态 sigma 倍数 (越大尾越重)
  double lambda_ = 50.0;

  // ── 归一化代价参数: 占比 (推荐 0.4+0.3+0.3=1.0) + 全局缩放 ──
  double cost_scale_ = 10.0;        // 全局代价缩放 (调大=更敏感)
  double obstacle_ratio_ = 0.4;     // 障碍物代价占比 40%
  double tracking_ratio_ = 0.3;     // 路径跟踪代价占比 30% (PathAlign + PathAngle)
  double speed_ratio_ = 0.3;        // 速度/进度代价占比 30% (SpeedReward + GoalCritic)
  double path_deviation_weight_ = 150.0;  // 偏离路径软墙权重 (独立于 cost_scale, 0=禁用)
  double path_corridor_ = 0.15;     // 偏离免罚回廊半宽 [m], 超出后按超出量² 陡增惩罚

  double lookahead_distance_ = 1.0;
  double lookahead_time_ = 2.0;
  double min_lookahead_dist_ = 2.0;
  double cross_track_noise_scale_ = 0.3;
  double noise_decay_rate_ = 0.7;
  double lateral_guidance_scale_ = 0.2; // 横向 guidance 缩放 (0~1)，越小越抑制侧移
  double turn_lateral_boost_ = 4.0;       // 转弯横向增强: 急弯时 vy 放大倍数 (0=无增强, 4=急弯×5)
  double ema_alpha_ = 0.2;               // 输出 EMA 平滑系数: 0=完全冻结, 1=无平滑
  bool enable_ema_ = true;               // 是否启用 EMA 输出平滑，关闭则直接输出原始 MPPI 控制量
  double lookahead_theta_ = 0.0;              // 全局持久的前瞻点推荐朝向 (rad)，跨路径重规划保持
  double prev_lookahead_theta_ = 0.0;          // 上一帧的 lookahead_theta_（用于变化率限制）
  bool has_prev_lookahead_theta_ = false;      // 是否有上一帧朝向数据
  double lookahead_theta_rate_ = 0.5;          // 最大朝向变化率 (rad/s)，每帧限幅 dt*(此值)，默认≈28°/s
  rclcpp::Time last_theta_update_time_;  // 上一帧朝向更新时间 (默认构造为 0)
  double exploration_decay_start_ = 3.0;   // 探索衰减起始距离 (m), >此距离探索范围=100%
  double exploration_decay_end_ = 0.5;     // 探索衰减结束距离 (m), <此距离探索范围=floor
  double exploration_decay_floor_ = 0.3;   // 探索衰减下限 (0~1), 最小保留比例
  double spatial_decay_weight_ = 0.5;      // 空间衰减权重: 0=纯时间步, 1=纯空间距离
  double footprint_front_ = 0.3;   // 碰撞箱前向半尺寸 (m)
  double footprint_back_ = 0.3;    // 碰撞箱后向半尺寸 (m)
  double footprint_left_ = 0.4;    // 碰撞箱左向半尺寸 (m)
  double footprint_right_ = 0.4;   // 碰撞箱右向半尺寸 (m)
  double terminal_angle_dist_ = 0.05;      // 终端对准距离 (m)
  double terminal_angle_kp_ = 1.5;         // 终端角度 P 控制器增益
  double terminal_angle_tolerance_ = 0.07; // 终端角度容忍度 (rad), ≈4°, 留裕量给 5° goal checker
  // ── 障碍物代价内部参数 ──
  double rear_obstacle_cost_ = 160.0;      // 后方隐形障碍代价地板 (0~255), 0=禁用
  double footprint_sample_spacing_ = 0.08; // 足迹碰撞检测采样间距 (m)

  // ── 时步折扣 ──
  double cost_discount_ = 0.9;            // 代价时序折扣 γ, step t 权重=γ^t

  // ── 行为参数 (非代价权重) ──
  double turn_lateral_max_boost_ = 6.0;    // 转弯横向最大放大倍数上限

  // ── 分层规划: 全局轨迹直接预测到终点 ──
  double global_trajectory_ratio_ = 0.2;    // 全局轨迹占比 (0~1), 0=纯局部, 1=纯全局
  int global_horizon_ = 30;                 // 全局轨迹预测步数最大值
  double final_goal_x_ = 0.0;               // 最终目标点 x（每帧更新）
  double final_goal_y_ = 0.0;               // 最终目标点 y
  double final_goal_yaw_ = 0.0;             // 最终目标姿态

  // ── 前瞻点 KP 减速 ──
  // 读取代价地图在前瞻点处的代价，代价越高速度越低
  // scale = 1 - cost/254 * (1 - kp)
  double lookahead_kp_ = 0.3;         // 靠近前瞻点时最低速度比例 (0~1, 越小减速越猛)
  double lookahead_decel_dist_ = 0.5; // 前瞻减速起始距离 (m), 距目标 < 此值时线性减速

  // ── 朝向偏差限速 ──
  // 当机器人朝向与目标方向偏差超过阈值时，限制矢量速度
  bool enable_heading_speed_limit_ = true;   // 是否启用朝向偏差限速
  double heading_misalign_threshold_ = M_PI_2; // 偏差阈值 (rad), 默认 90°
  double heading_misalign_max_speed_ = 0.1;    // 超阈值时的最大矢量速度 (m/s)

  static constexpr int MAX_PATH_POINTS = 30;

  // 最优控制序列记忆（滚动窗口）
  std::vector<double> optimal_vx_seq_;
  std::vector<double> optimal_vy_seq_;
  std::vector<double> optimal_omega_seq_;
  bool initialized_ = false;

  // 上一帧最近路径点索引，用于增量搜索避免 closest_idx 跳变
  int prev_closest_idx_ = 0;

  // 上一帧前瞻点位置，用于到达推进检测
  double prev_lookahead_x_ = 0.0;
  double prev_lookahead_y_ = 0.0;
  bool has_prev_lookahead_ = false;

  // ── 终端角度对准（靠近目标时纯旋转对齐 goal yaw）──
  bool terminal_angle_active_ = false;  // 终端对准已激活（带迟滞，防边界抖动）

  // 横向偏好方向：打破对称障碍物的左右抉择困境
  // -1.0 = 偏好左绕, 0.0 = 无偏好, +1.0 = 偏好右绕
  // 每帧基于 costmap 分析更新，带迟滞避免振荡
  double preferred_lateral_dir_ = 0.0;
  bool enable_lateral_bias_ = true;  // 是否启用横向偏好分析

  // ── 状态机 ──
  ControllerState state_ = ControllerState::NORMAL;
  ControllerState prev_state_ = ControllerState::NORMAL;  // 上一帧状态, 用于禁止特殊状态间直接跳转
  NarrowSubState narrow_sub_state_ = NarrowSubState::ROTATE_TO_SIDE;
  bool enable_narrow_passage_ = true;     // 是否启用窄道踌躇检测与侧身脱离


  // 窄道侧身脱离
  double side_yaw_ = 0.0;              // 侧身朝向 (路径切线 ± π/2)
  double narrow_verify_speed_ = -1.0;  // 诊断用

  // 终端对齐
  double terminal_exit_time_ = -1.0;

  ControllerState determineState(
    double lh_wx, double lh_wy, double goal_wx, double goal_wy,
    bool near_goal, double heading_err, bool at_end, bool all_at_end,
    bool mppi_hesitate, double now,
    double cur_x, double cur_y);

  int hesitate_count_ = 0;                  // 连续踌躇帧数
  std::ofstream narrow_diag_log_;
  void narrowDiagHdr();
  void narrowDiagRow(double t, double lh_wx, double lh_wy, double moved, bool at_end, double stuck_dur, ControllerState s, double rx, double ry, double gx, double gy, double df, int near, int lh_cost);

  // ── 运行时统计数据采集 ──
  bool enable_stats_ = false;
  std::string stats_file_path_ = "/tmp/mppi_gpu_stats.csv";
  std::vector<StatsFrame> stats_frames_;
  double stats_start_time_ = 0.0;
  int stats_frame_count_ = 0;
  double prev_vx_raw_ = 0.0, prev_vy_raw_ = 0.0, prev_omega_raw_ = 0.0;
  bool has_prev_stats_ = false;
  float prev_bc_ = 0.0f;  // 上一帧主 MPPI best cost, 供 vmppi 使用
  double mutation_thresh_vx_ = 0.3;   // vx 突变阈值 (m/s)
  double mutation_thresh_vy_ = 0.15;  // vy 突变阈值 (m/s)
  double mutation_thresh_w_ = 0.3;    // omega 突变阈值 (rad/s)

  // 输出 EMA 平滑状态（帧间低通滤波，抑制控制量跳变）
  double ema_cmd_vx_ = 0.0;
  double ema_cmd_vy_ = 0.0;
  double ema_cmd_w_ = 0.0;
  bool ema_initialized_ = false;

  // 随机数生成器（噪声预生成）
  std::mt19937 generator_;
  std::normal_distribution<> dist_vx_;
  std::normal_distribution<> dist_vy_;
  std::normal_distribution<> dist_w_;
  // NLN 混合采样: 对数正态分布 + 均匀分布 (符号)
  std::lognormal_distribution<> dist_ln_vx_;
  std::lognormal_distribution<> dist_ln_vy_;
  std::lognormal_distribution<> dist_ln_w_;
  std::uniform_real_distribution<> dist_sign_{0.0, 1.0};
  std::uniform_real_distribution<> dist_mix_{0.0, 1.0};

  // GPU 缓冲区（持久分配，避免反复 cudaMalloc）
  float *d_noise_vx_ = nullptr;
  float *d_noise_vy_ = nullptr;
  float *d_noise_w_ = nullptr;
  float *d_base_vx_ = nullptr;
  float *d_base_vy_ = nullptr;
  float *d_base_w_ = nullptr;
  float *d_costs_ = nullptr;
  float *d_sampled_vx_ = nullptr;   // N × H: 每条采样轨迹每步的实际 vx
  float *d_sampled_vy_ = nullptr;   // N × H: 每条采样轨迹每步的实际 vy
  float *d_sampled_w_  = nullptr;   // N × H: 每条采样轨迹每步的实际 omega
  float *d_result_seq_ = nullptr;   // H × 4: 每个 timestep 的 [vx, vy, w, weight_sum]
  unsigned char *d_costmap_ = nullptr;
  float *d_path_x_ = nullptr;
  float *d_path_y_ = nullptr;
  float *d_traj_x_ = nullptr;
  float *d_traj_y_ = nullptr;
  int costmap_w_ = 0;
  int costmap_h_ = 0;
  bool gpu_buffers_allocated_ = false;

  void allocateGPUBuffers();
  void freeGPUBuffers();

  // 可视化发布者
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr vis_pub_;

  void publishVisualization(
    double robot_x, double robot_y, double robot_theta,
    double target_x, double target_y,
    const std::vector<float>& traj_data_x,
    const std::vector<float>& traj_data_y,
    int vis_samples,
    int best_idx,
    const std::string& frame_id);

  /// 全局代价地图回调：缓存最新的 OccupancyGrid 消息
  void globalCostmapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg);

  /// 记录一帧运行时统计数据（在 computeVelocityCommands 末尾调用）
  void recordStatsFrame(double vx, double vy, double omega,
                        double vx_raw, double vy_raw, double omega_raw,
                        double cross_track_err, double heading_err,
                        double dist_to_goal, float best_cost,
                        double current_time);

  /// 将累积的统计数据写入 CSV 文件（在 cleanup 中调用）
  void writeStatsToFile();

  // ── 文件日志 ──
  MPPIGPULogger logger_;
  bool enable_file_log_ = true;
  std::string log_file_path_ = "/tmp/mppi_gpu_controller.log";

  // ── 停滞检测 ──
  double stall_speed_threshold_ = 0.05;
  double stall_report_interval_ = 2.0;
  double stall_start_time_ = -1.0;
  double last_stall_report_time_ = -1.0;
};

}  // namespace nav2_custom_plugins

#endif  // NAV2_CUSTOM_PLUGINS__MPPI_GPU_CONTROLLER_HPP_
