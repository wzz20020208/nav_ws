#ifndef NAV2_CUSTOM_PLUGINS_V2__MPPI_CORE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MPPI_CORE_HPP_

#include <vector>
#include <string>

namespace rclcpp { class Node; }

namespace nav2_custom_plugins_v2 {

// ═══════════════════════════════════════════════════════════════════════════
// 参数结构体
// ═══════════════════════════════════════════════════════════════════════════

struct MPPIParams {
  // ── 控制空间 ──
  int num_samples = 8000;
  int prediction_horizon = 5;
  double dt = 0.1;
  double max_v = 0.4;
  double min_v = -0.4;
  double max_vy = 0.2;
  double max_steering_angle = 0.785;   // ±45° (rad)
  double max_w = 0.6;                  // 朝向跟踪角速度上限 (rad/s)

  // ── 噪声 ──
  double action_std_v = 0.5;
  double action_std_vy = 0.5;
  double action_std_delta = 0.3;
  double noise_scale_floor_vx = 0.35;
  double noise_scale_floor_vy = 0.30;
  double noise_scale_floor_delta = 0.15;
  double nln_ratio = 0.35;             // 对数正态混合比
  double nln_sigma_mult = 3.0;         // 对数正态 sigma 倍数
  double noise_decay_rate = 0.55;

  // ── 探索衰减 ──
  double exploration_decay_start = 3.0;
  double exploration_decay_end = 0.5;
  double exploration_decay_floor = 0.3;
  double spatial_decay_weight = 0.5;

  // ── MPPI 代价 ──
  double lambda = 4.0;
  double cost_scale = 50.0;
  double obstacle_ratio = 0.7;
  double tracking_ratio = 0.2;
  double speed_ratio = 0.05;
  double path_deviation_weight = 0.0;
  double path_corridor = 0.15;

  // ── 碰撞检测 ──
  double footprint_front = 0.17;
  double footprint_back = 0.17;
  double footprint_left = 0.28;
  double footprint_right = 0.28;
  double footprint_sample_spacing = 0.04;
  double rear_obstacle_cost = 0.0;

  // ── 前瞻 ──
  double min_lookahead_dist = 0.3;
  double lookahead_kp = 1.0;
  double lookahead_decel_dist = 0.5;
  double cost_discount = 0.9;

  // ── EMA ──
  bool enable_ema = false;
  double ema_alpha = 0.1;

  // ── 分层规划 ──
  double global_trajectory_ratio = 0.3;
  int global_horizon = 50;

  // ── 朝向偏差限速 ──
  bool enable_heading_speed_limit = true;
  double heading_misalign_threshold = 1.047;    // ~60°
  double heading_misalign_max_speed = 0.1;

  // ── 状态机 ──
  bool enable_narrow_passage = false;

  // ── 可视化 & 日志 ──
  bool enable_visualization = true;
  bool enable_stats = false;
  bool enable_file_log = true;
  std::string stats_file_path = "/tmp/mppi_gpu_stats.csv";
  std::string log_file_path = "/tmp/mppi_steering_controller.log";

  // ── 停滞检测 ──
  double stall_speed_threshold = 0.05;
  double stall_report_interval = 2.0;

  // 从 ROS 参数服务器加载所有参数
  void loadFromROS(rclcpp::Node* node, const std::string& plugin_name);
};

// ═══════════════════════════════════════════════════════════════════════════
// 基础类型
// ═══════════════════════════════════════════════════════════════════════════

/// 单帧控制指令: [vx, vy, delta]
struct Control {
  double vx = 0.0;
  double vy = 0.0;
  double delta = 0.0;

  Control() = default;
  Control(double vx_, double vy_, double delta_) : vx(vx_), vy(vy_), delta(delta_) {}
};

/// H 步控制序列 (供 GPU warm-start)
struct ControlSequence {
  std::vector<double> vx;
  std::vector<double> vy;
  std::vector<double> delta;

  void resize(int H);
  void shiftAndDecay(double decay = 0.5);
  Control step(int t) const;
  Control step0() const { return step(0); }
};

/// 机器人状态
struct RobotState {
  double x = 0.0;
  double y = 0.0;
  double theta = 0.0;
};

/// 前瞻点
struct LookaheadPoint {
  double wx = 0.0;           // 世界坐标 X
  double wy = 0.0;           // 世界坐标 Y
  double dist = 0.0;         // 距机器人距离
  double recommended_yaw = 0.0;  // SE2 路径推荐朝向
};

// ═══════════════════════════════════════════════════════════════════════════
// 运动学模型 (位置解耦 + 朝向限速)
// ═══════════════════════════════════════════════════════════════════════════

class KinematicModel {
public:
  explicit KinematicModel(const MPPIParams& params);

  /// 单步前向积分: vx,vy 在全局系直接累加, theta 以 max_w 速率向 delta 靠拢
  RobotState integrate(const RobotState& state, const Control& u, double dt) const;

  /// δ → ω 转换 (用于 TwistStamped 向后兼容)
  double deltaToOmega(double delta, double current_theta, double dt) const;

private:
  double max_w_;
  double max_steering_angle_;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__MPPI_CORE_HPP_
