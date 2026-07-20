/**
 * @file velocity_postprocessor.cpp
 * @brief VelocityPostProcessor 实现 — 提取 + clamp + δ→ω
 */

#include "nav2_custom_plugins_v2/modules/velocity_postprocessor.hpp"
#include <algorithm>
#include <cmath>

namespace nav2_custom_plugins_v2
{

VelocityPostProcessor::VelocityPostProcessor(const MPPIParams &params)
  : p_(params)
{}

// ═══════════════════════════════════════════════════════════════════════════════
// process — 完整后处理管线
// ═══════════════════════════════════════════════════════════════════════════════
//
// base_link 模式: vx/vy 原样输出, δ→ω
// global 模式:   vx/vy 旋转到 odom 系, delta 直接作为目标角度

VelocityPostProcessor::ProcessedCommand
VelocityPostProcessor::process(const std::vector<float> &result,
                               double current_yaw, bool global_mode)
{
  ProcessedCommand out;
  out.control     = clamp(extractStep0(result));
  out.global_mode = global_mode;

  if (global_mode) {
    // ── global 模式: 初始朝向系 → odom 系 ──
    double c = std::cos(current_yaw), s = std::sin(current_yaw);
    out.vx_out   = out.control.vx * c - out.control.vy * s;
    out.vy_out   = out.control.vx * s + out.control.vy * c;
    out.delta_out = out.control.delta + current_yaw;
    while (out.delta_out > M_PI)  out.delta_out -= 2.0 * M_PI;
    while (out.delta_out < -M_PI) out.delta_out += 2.0 * M_PI;
    out.omega = 0.0;
  } else {
    // ── base_link 模式: vx/vy 原样, δ→ω ──
    out.vx_out   = out.control.vx;
    out.vy_out   = out.control.vy;
    out.delta_out = out.control.delta;
    out.omega    = deltaToOmega(out.control.delta, current_yaw);
  }
  return out;
}

// ═══════════════════════════════════════════════════════════════════════════════
// extractStep0 — 从 [H×4] 加权结果提取 step 0
// ═══════════════════════════════════════════════════════════════════════════════
//
// result 布局: [vx_sum, vy_sum, delta_sum, weight_sum] × H
// step-0 = result[0..3] / result[3]

Control VelocityPostProcessor::extractStep0(
    const std::vector<float> &result) const
{
  Control c;
  float w_sum = result[3];
  if (w_sum > 0.0f) {
    c.vx    = static_cast<double>(result[0] / w_sum);
    c.vy    = static_cast<double>(result[1] / w_sum);
    c.delta = static_cast<double>(result[2] / w_sum);
  }
  // else: 全零 (w_sum=0 表示所有轨迹代价极大, 安全停车)
  return c;
}

// ═══════════════════════════════════════════════════════════════════════════════
// clamp — 钳位到 MPPIParams 物理限幅
// ═══════════════════════════════════════════════════════════════════════════════

Control VelocityPostProcessor::clamp(const Control &raw) const
{
  Control c;
  c.vx    = std::clamp(raw.vx,    p_.min_v,  p_.max_v);
  c.vy    = std::clamp(raw.vy,   -p_.max_vy, p_.max_vy);
  c.delta = std::clamp(raw.delta, -p_.max_steering_angle, p_.max_steering_angle);
  return c;
}

// ═══════════════════════════════════════════════════════════════════════════════
// deltaToOmega — δ→ω 限速转换
// ═══════════════════════════════════════════════════════════════════════════════
//
// ω = clamp(normalize(delta - theta) / dt, ±max_w)
// 归一化角度差然后除以 dt 得到角速度, 再 clamp 到 [−max_w, +max_w]

double VelocityPostProcessor::deltaToOmega(
    double delta, double current_theta) const
{
  double dtheta = delta - current_theta;
  while (dtheta > M_PI)  dtheta -= 2.0 * M_PI;
  while (dtheta < -M_PI) dtheta += 2.0 * M_PI;
  dtheta = std::clamp(dtheta, -p_.max_w * p_.dt, p_.max_w * p_.dt);
  return dtheta / p_.dt;
}

}  // namespace nav2_custom_plugins_v2
