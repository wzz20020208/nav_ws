#ifndef NAV2_CUSTOM_PLUGINS_V2__MPPI_CORE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MPPI_CORE_HPP_

#include <vector>
#include <string>

namespace rclcpp { class Node; }

namespace nav2_custom_plugins_v2 {

// ═══════════════════════════════════════════════════════════════════════════
// 参数
// ═══════════════════════════════════════════════════════════════════════════

struct MPPIParams {
  // 控制空间
  int num_samples = 8000;
  int prediction_horizon = 5;
  double dt = 0.1;
  double max_v = 0.4;
  double min_v = -0.4;
  double max_vy = 0.2;
  double max_steering_angle = 0.785;
  double max_w = 0.6;

  // 噪声
  double action_std_v = 0.5;
  double action_std_vy = 0.5;
  double action_std_delta = 0.3;

  // 代价
  double lambda = 4.0;
  double cost_scale = 50.0;

  // 碰撞
  double footprint_front = 0.17;
  double footprint_back = 0.17;
  double footprint_left = 0.28;
  double footprint_right = 0.28;

  // 前瞻
  double min_lookahead_dist = 0.3;
  double lookahead_decel_dist = 0.5;

  // EMA
  bool enable_ema = false;
  double ema_alpha = 0.1;

  // 状态机
  bool enable_heading_speed_limit = true;
  double heading_misalign_threshold = 1.047;
  bool enable_narrow_passage = false;

  // 日志
  bool enable_file_log = true;
  std::string log_file_path = "/tmp/mppi_steering_controller.log";

  void loadFromROS(rclcpp::Node* node, const std::string& plugin_name);
};

// ═══════════════════════════════════════════════════════════════════════════
// 基础类型
// ═══════════════════════════════════════════════════════════════════════════

struct Control {
  double vx = 0.0;
  double vy = 0.0;
  double delta = 0.0;
};

struct ControlSequence {
  std::vector<double> vx;
  std::vector<double> vy;
  std::vector<double> delta;

  void resize(int H);
  void shiftAndDecay(double decay = 0.5);
  Control step(int t) const;
  Control step0() const { return step(0); }
};

struct RobotState {
  double x = 0.0;
  double y = 0.0;
  double theta = 0.0;
};

struct LookaheadPoint {
  double wx = 0.0;
  double wy = 0.0;
  double dist = 0.0;
  double recommended_yaw = 0.0;
};

// ═══════════════════════════════════════════════════════════════════════════
// 运动学: 位置全局系直接累加, 朝向以 max_w 限速向 δ 靠拢
// ═══════════════════════════════════════════════════════════════════════════

class KinematicModel {
public:
  explicit KinematicModel(const MPPIParams& params);

  RobotState integrate(const RobotState& state, const Control& u, double dt) const;
  double deltaToOmega(double delta, double current_theta, double dt) const;

private:
  double max_w_;
  double max_steering_angle_;
};

}  // namespace nav2_custom_plugins_v2

#endif
