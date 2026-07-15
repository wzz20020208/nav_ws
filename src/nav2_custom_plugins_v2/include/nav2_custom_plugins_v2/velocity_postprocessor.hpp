#ifndef NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_

#include "nav2_custom_plugins_v2/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// 速度后处理器: EMA 滤波 → 前瞻减速 → 钳位 → δ→ω 转换
class VelocityPostProcessor {
public:
  explicit VelocityPostProcessor(const MPPIParams& params);

  /// 后处理结果: 新格式 + 兼容旧格式
  struct ProcessedCommand {
    Control steering_cmd;     // vx, vy, delta — 发布到 /cmd_vel_steering
    double twist_omega = 0.0; // angular.z — 填入 TwistStamped
  };

  /// 完整后处理管线
  ProcessedCommand process(
      const Control& raw_control,     // MPPI 加权平均原始值
      double current_theta,           // 当前机器人朝向
      double dist_to_goal);           // 距目标距离

  /// 重置 EMA 状态 (新路径开始时调用)
  void reset();

private:
  const MPPIParams& params_;

  Control applyEMA(const Control& raw);
  Control applyDeceleration(const Control& cmd, double dist_to_goal);
  Control applyHeadingSpeedLimit(const Control& cmd, double heading_err);
  Control applyClamp(const Control& cmd);
  double deltaToOmega(double delta, double current_theta) const;

  Control ema_control_{0.0, 0.0, 0.0};
  bool ema_initialized_ = false;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_
