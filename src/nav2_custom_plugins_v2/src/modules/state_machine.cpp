/**
 * @file state_machine.cpp
 * @brief StateMachine 实现 — heading 原地旋转判定
 *
 * 参考: nav2_custom_plugins/src/mppi_gpu_controller.cpp determineState()
 *       lines 382-386 (任务锁), 399-402 (候选状态), 924-939 (旋转指令)
 */

#include "nav2_custom_plugins_v2/modules/state_machine.hpp"
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"
#include <cmath>

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════════
// evaluateHeading — 朝向偏差判定
// ═══════════════════════════════════════════════════════════════════════════════
//
// 参考原版逻辑:
//   进入: abs(err) > threshold 且 dist(lh,goal) >= 0.5
//   退出: abs(err) <= threshold * 0.5  (迟滞, 防止振荡)
//   指令: vx=vy=0, omega = sign(err) * max_w (或 delta=lookahead_yaw)

HeadingDecision StateMachine::evaluateHeading(
    double current_yaw, double lookahead_yaw,
    double dist_lh_to_goal, const MPPIParams &params)
{
  HeadingDecision dec;

  if (!params.enable_heading_speed_limit) {
    rotating_ = false;
    return dec;
  }

  double err = lookahead_yaw - current_yaw;
  while (err > M_PI)  err -= 2.0 * M_PI;
  while (err < -M_PI) err += 2.0 * M_PI;

  // ── 任务锁: 已进入旋转状态 → 迟滞退出 ──
  // 退出阈值 = 进入阈值 × 0.5, 防止在阈值边界来回振荡
  if (rotating_) {
    if (std::abs(err) <= params.heading_misalign_threshold * 0.5) {
      rotating_ = false;  // 对准完成, 退出旋转
    } else {
      // 继续旋转
      dec.rotate_in_place = true;
      dec.omega = current_yaw + err;
      if (dec.omega > M_PI)       dec.omega -= 2.0 * M_PI;
      else if (dec.omega < -M_PI) dec.omega += 2.0 * M_PI;
      dec.omega_sign = (err > 0) ? 1.0 : -1.0;
    }
    return dec;
  }

  // ── 候选判定: 首次进入 ──
  // 偏差超阈值即触发, 直接发前瞻点推荐朝向
  if (std::abs(err) > params.heading_misalign_threshold)
  {
    dec.rotate_in_place = true;
    dec.omega = current_yaw + err;
    if (dec.omega > M_PI)       dec.omega -= 2.0 * M_PI;
    else if (dec.omega < -M_PI) dec.omega += 2.0 * M_PI;
    dec.omega_sign = (err > 0) ? 1.0 : -1.0;
    rotating_ = true;
  }

  return dec;
}

}  // namespace nav2_custom_plugins_v2
