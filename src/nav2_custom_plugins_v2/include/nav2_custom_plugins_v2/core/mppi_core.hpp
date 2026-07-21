#ifndef NAV2_CUSTOM_PLUGINS_V2__MPPI_CORE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MPPI_CORE_HPP_

#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

// ═══════════════════════════════════════════════════════════════════════════
// GPU 共享结构体 (host + device, 不含 __device__ 函数)
// ═══════════════════════════════════════════════════════════════════════════

/// costmap 元数据 + GPU 数据指针
struct CostmapInfo {
  const unsigned char *data = nullptr;
  int   w = 0, h = 0;
  float res = 0.05f;
  float origin_x = 0.0f, origin_y = 0.0f;
};

/// 机器人外形 + 碰撞检测参数
struct Footprint {
  float front = 0.3f, back = 0.3f;
  float left = 0.4f, right = 0.4f;
  float sample_spacing = 0.08f;
  float rear_obstacle_cost = 160.0f;
};

/// 全局路径数据 — 供 HEADING 大类代价函数使用 (GPU-compatible)
/// 路径点数组存储在 GPU 显存, PathInfo 持有其指针 + 元数据
struct PathInfo {
  const float *x = nullptr;      ///< 路径点 x 坐标数组 (GPU 显存)
  const float *y = nullptr;      ///< 路径点 y 坐标数组 (GPU 显存)
  int   num_pts = 0;             ///< 路径点数量
  float path_tangent = 0.0f;     ///< 当前前瞻点路径切线方向 (rad), 供 PathAngleCritic
  float goal_yaw = 0.0f;         ///< 终点期望朝向 (rad), 供 PathAngleCritic 余弦退火
  float goal_x = 0.0f;           ///< 终点世界坐标 x, 供 PathAngleCritic 算 dist_to_final
  float goal_y = 0.0f;           ///< 终点世界坐标 y
};

/// 目标/速度参考 — 供 SPEED 大类代价函数使用 (GPU-compatible)
struct GoalInfo {
  float target_vx_r = 0.0f;      ///< 期望速度 x 分量 (机器人坐标系), 供 SpeedRewardCritic
  float target_vy_r = 0.0f;      ///< 期望速度 y 分量 (机器人坐标系)
  float goal_x = 0.0f;           ///< 终点世界坐标 x, 供 TerminalDistCritic
  float goal_y = 0.0f;           ///< 终点世界坐标 y
  float lookahead_x = 0.0f;      ///< 前瞻点世界坐标 x, 供 kernel overshoot 检测
  float lookahead_y = 0.0f;      ///< 前瞻点世界坐标 y
  float lookahead_overshoot_weight = 5.0f; ///< 越界惩罚系数 (× overshoot²)
};

namespace nav2_custom_plugins_v2 {

// ═══════════════════════════════════════════════════════════════════════════
// 参数 (YAML → configure 时读入)
// ═══════════════════════════════════════════════════════════════════════════

struct MPPIParams {
  // 控制空间
  int num_samples = 8000;
  int prediction_horizon = 5;
  double dt = 0.05;                  // 单步积分时长 (s)
  double max_v = 0.4;
  double min_v = -0.4;
  double max_vy = 0.2;
  double max_w = 0.6;

  // 噪声
  double action_std_v = 0.5;
  double action_std_vy = 0.5;
  double action_std_w = 0.4;

  // 代价 (1:1 THEMIS)
  double lambda = 0.05;
  double cost_scale = 50.0;
  double obstacle_ratio = 0.70;
  double tracking_ratio = 0.20;
  double speed_ratio = 0.05;

  // 碰撞
  double footprint_front = 0.17;
  double footprint_back = 0.17;
  double footprint_left = 0.28;
  double footprint_right = 0.28;

  // 前瞻
  double min_lookahead_dist = 0.8;
  double lookahead_kp = 0.3;           ///< 前瞻减速最低比例 (0=完全停止, 1=不减速)
  double lookahead_decel_dist = 0.5;
  double lookahead_overshoot_weight = 5.0;  ///< 前瞻点越界惩罚系数

  // 终端朝向: 距终点此距离内直接发 goal_yaw 为目标角度
  double terminal_angle_dist = 0.3;      ///< 触发距离 (m)
  double terminal_angle_tolerance = 0.17;///< 对准容差 (rad)

  // EMA
  bool enable_ema = false;
  double ema_alpha = 0.1;

  // 路径
  bool use_planner_yaw = true;  ///< true=使用 planner 给的 pose.orientation, false=自算切线

  // 输出模式
  bool use_global_mode = true;  ///< true=odom 系输出 (delta 直出), false=base_link 系 (δ→ω)

  // 状态机
  bool enable_heading_speed_limit = true;
  double heading_misalign_threshold = 1.047;
  bool enable_narrow_passage = false;

  // 日志
  bool enable_file_log = true;
  std::string log_file_path = "/tmp/mppi_steering_controller.log";
};

/// YAML 参数表 — 新增参数只需在此加一行
struct IntParam   { const char *name; int MPPIParams::*ptr; };
struct DblParam   { const char *name; double MPPIParams::*ptr; };
struct BoolParam  { const char *name; bool MPPIParams::*ptr; };
struct StrParam   { const char *name; std::string MPPIParams::*ptr; };

inline constexpr IntParam kIntParams[] = {
  {"num_samples",        &MPPIParams::num_samples},
  {"prediction_horizon", &MPPIParams::prediction_horizon},
};

inline constexpr DblParam kDblParams[] = {
  {"dt",                 &MPPIParams::dt},
  {"max_v",              &MPPIParams::max_v},
  {"min_v",              &MPPIParams::min_v},
  {"max_vy",             &MPPIParams::max_vy},
  {"max_w",              &MPPIParams::max_w},
  {"action_std_v",       &MPPIParams::action_std_v},
  {"action_std_vy",      &MPPIParams::action_std_vy},
  {"action_std_w",       &MPPIParams::action_std_w},
  {"lambda",             &MPPIParams::lambda},
  {"cost_scale",         &MPPIParams::cost_scale},
  {"obstacle_ratio",     &MPPIParams::obstacle_ratio},
  {"tracking_ratio",     &MPPIParams::tracking_ratio},
  {"speed_ratio",        &MPPIParams::speed_ratio},
  {"footprint_front",    &MPPIParams::footprint_front},
  {"footprint_back",     &MPPIParams::footprint_back},
  {"footprint_left",     &MPPIParams::footprint_left},
  {"footprint_right",    &MPPIParams::footprint_right},
  {"min_lookahead_dist",          &MPPIParams::min_lookahead_dist},
  {"lookahead_kp",               &MPPIParams::lookahead_kp},
  {"lookahead_decel_dist",       &MPPIParams::lookahead_decel_dist},
  {"lookahead_overshoot_weight", &MPPIParams::lookahead_overshoot_weight},
  {"terminal_angle_dist",      &MPPIParams::terminal_angle_dist},
  {"terminal_angle_tolerance", &MPPIParams::terminal_angle_tolerance},
  {"ema_alpha",          &MPPIParams::ema_alpha},
};

inline constexpr BoolParam kBoolParams[] = {
  {"use_planner_yaw",             &MPPIParams::use_planner_yaw},
  {"use_global_mode",             &MPPIParams::use_global_mode},
  {"enable_heading_speed_limit",  &MPPIParams::enable_heading_speed_limit},
  {"enable_narrow_passage",       &MPPIParams::enable_narrow_passage},
  {"enable_ema",                  &MPPIParams::enable_ema},
  {"enable_file_log",             &MPPIParams::enable_file_log},
};

// ═══════════════════════════════════════════════════════════════════════════
// 基础类型
// ═══════════════════════════════════════════════════════════════════════════

/// 控制量 [vx, vy, omega]
struct Control {
  double vx = 0.0;
  double vy = 0.0;
  double omega = 0.0;
};

/// warm-start 基序列: 上一帧最优控制量, 左移一位后作为本帧采样基线
struct ControlSequence {
  std::vector<double> vx;
  std::vector<double> vy;
  std::vector<double> omega;

  void resize(int H);
  void shiftAndDecay(double decay = 0.5);
  Control step(int t) const;
  Control step0() const { return step(0); }
};

/// 机器人状态 (全局坐标系)
struct RobotState {
  double x = 0.0;
  double y = 0.0;
  double theta = 0.0;
};

/// 路径前瞻点
struct LookaheadPoint {
  double wx = 0.0;           // 世界坐标 x
  double wy = 0.0;           // 世界坐标 y
  double dist = 0.0;         // 沿路径距机器人距离
  double recommended_yaw = 0.0;  // 路径在该点的切线方向
};

// ═══════════════════════════════════════════════════════════════════════════
// 噪声生成: N 条轨迹 × H 步, 分布向路径切线方向偏置, 不跑散
// ═══════════════════════════════════════════════════════════════════════════

/// 噪声生成器: 在 CPU 侧预生成全部 N×H 带偏置噪声
///
/// 偏置策略:
///   噪声向 lookahead 方向偏移, 采样族围绕路径切线展开.
///   noise_vx += bias * cos(lookahead_in_robot)
///   noise_vy += bias * sin(lookahead_in_robot)
///   noise_w += 0.5 * heading_err
class NoiseGenerator {
public:
  explicit NoiseGenerator(const MPPIParams &params);

  /// 生成 N 条轨迹 × H 步的噪声 (每帧调用一次)
  void generate(int N, int H, double path_yaw, double current_yaw);

  const std::vector<float> &noise_vx()    const { return noise_vx_; }
  const std::vector<float> &noise_vy()    const { return noise_vy_; }
  const std::vector<float> &noise_w()     const { return noise_w_; }

private:
  const MPPIParams &p_;
  std::vector<float> noise_vx_;
  std::vector<float> noise_vy_;
  std::vector<float> noise_w_;
};

// ═══════════════════════════════════════════════════════════════════════════
// 采样: 官方 MPPI — u = base + noise * noise_scale
// ═══════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════
// 轨迹 rollout: 从起点递推 H 步, 批量产出 N 条轨迹
// ═══════════════════════════════════════════════════════════════════════════

/// N 条轨迹的展平数组, 直接可 cudaMemcpy 上传 GPU
struct BatchTrajectories {
  std::vector<float> x;           // [N×H] 轨迹点 x 坐标
  std::vector<float> y;           // [N×H] 轨迹点 y 坐标
  std::vector<float> theta;       // [N×H] 轨迹点朝向 (rad), 代价计算用
  std::vector<float> vx;          // [N×H] 实际采用的控制量 vx
  std::vector<float> vy;          // [N×H] 实际采用的控制量 vy
  std::vector<float> omega;       // [N×H] 实际采用的控制量 omega
};

/// 运动学积分: vx/vy 为 body 系 (前/左), 需旋转变换到世界系
inline RobotState kinematic_integrate(
    const RobotState &state, const Control &u, double dt, double max_w)
{
  RobotState next = state;
  double c = std::cos(state.theta), s = std::sin(state.theta);
  next.x += (u.vx * c - u.vy * s) * dt;
  next.y += (u.vx * s + u.vy * c) * dt;
  next.theta += std::clamp(u.omega, -max_w, max_w) * dt;
  return next;
}

/// 批量 rollout: 官方 MPPI — u = base + noise * noise_scale, 代价函数驱动选择
/// noise_gen 须已调用 generate(N, H).
BatchTrajectories batch_rollout(
    const RobotState &start,
    const ControlSequence &base_seq,
    const NoiseGenerator &noise_gen,
    const MPPIParams &params,
    int N, int H);

// ═══════════════════════════════════════════════════════════════════════════
// 运动学 (保留原有接口, 内部委托给 inline 函数)
// ═══════════════════════════════════════════════════════════════════════════

class KinematicModel {
public:
  explicit KinematicModel(const MPPIParams& params);

  RobotState integrate(const RobotState& state, const Control& u, double dt) const;

private:
  double max_w_;
};

}  // namespace nav2_custom_plugins_v2

#endif
