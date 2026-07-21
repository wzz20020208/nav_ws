/**
 * @file velocity_postprocessor.cpp
 * @brief VelocityPostProcessor — 提取 + clamp + omega→delta 转换
 *
 * 内部: vx/vy/omega 全部为 body 系角速度 (用于 warm-start 和 MPPI rollout)
 * 输出:
 *   global    → vx/vy 旋转到 odom 系, omega→delta = normalize_angle(yaw + omega*dt) (目标角度)
 *   base_link → vx/vy/omega 直出 (body 系角速度)
 */

#include "nav2_custom_plugins_v2/modules/velocity_postprocessor.hpp"
#include <algorithm>
#include <cmath>

namespace nav2_custom_plugins_v2
{

VelocityPostProcessor::VelocityPostProcessor(const MPPIParams &params)
  : p_(params)
{}

VelocityPostProcessor::ProcessedCommand
VelocityPostProcessor::process(const std::vector<float> &result,
                               double current_yaw, bool global_mode)
{
  ProcessedCommand out;
  out.global_mode = global_mode;
  Control raw = extractStep0(result);

  // vx, vy 始终是 body 系线速度, 始终限幅
  raw.vx = std::clamp(raw.vx, p_.min_v, p_.max_v);
  raw.vy = std::clamp(raw.vy, -p_.max_vy, p_.max_vy);
  // omega 是 inner 角速度, clamp 到物理极限 (warm-start 用)
  raw.omega = std::clamp(raw.omega, -p_.max_w, p_.max_w);

  out.control = raw;  // warm-start: 始终 body 系角速度

  if (global_mode) {
    // global: body→odom 旋转 + omega→delta 目标角度
    double c = std::cos(current_yaw), s = std::sin(current_yaw);
    out.vx_out    = raw.vx * c - raw.vy * s;
    out.vy_out    = raw.vx * s + raw.vy * c;
    double delta  = current_yaw + raw.omega * p_.dt;
    out.omega_out = std::atan2(std::sin(delta), std::cos(delta));
  } else {
    // base_link: body 系直出
    out.vx_out    = raw.vx;
    out.vy_out    = raw.vy;
    out.omega_out = raw.omega;
  }

  return out;
}

Control VelocityPostProcessor::extractStep0(
    const std::vector<float> &result) const
{
  Control c;
  float w_sum = result[3];
  if (w_sum > 0.0f) {
    c.vx    = static_cast<double>(result[0] / w_sum);
    c.vy    = static_cast<double>(result[1] / w_sum);
    c.omega = static_cast<double>(result[2] / w_sum);
  }
  return c;
}

Control VelocityPostProcessor::clamp(const Control &raw) const
{
  Control c;
  c.vx    = std::clamp(raw.vx,    p_.min_v,  p_.max_v);
  c.vy    = std::clamp(raw.vy,   -p_.max_vy, p_.max_vy);
  c.omega = std::clamp(raw.omega, -p_.max_w, p_.max_w);
  return c;
}

}  // namespace nav2_custom_plugins_v2
