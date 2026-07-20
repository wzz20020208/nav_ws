/**
 * @file velocity_postprocessor.cpp
 * @brief VelocityPostProcessor — 提取 + clamp (omega 即角速度, 无需转换)
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
  out.control     = clamp(extractStep0(result));
  out.global_mode = global_mode;

  if (global_mode) {
    double c = std::cos(current_yaw), s = std::sin(current_yaw);
    out.vx_out    = out.control.vx * c - out.control.vy * s;
    out.vy_out    = out.control.vx * s + out.control.vy * c;
    out.omega_out = out.control.omega;
  } else {
    out.vx_out    = out.control.vx;
    out.vy_out    = out.control.vy;
    out.omega_out = out.control.omega;
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
