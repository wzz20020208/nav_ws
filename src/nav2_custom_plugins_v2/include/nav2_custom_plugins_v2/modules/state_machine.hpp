#ifndef NAV2_CUSTOM_PLUGINS_V2__STATE_MACHINE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__STATE_MACHINE_HPP_

/**
 * @file state_machine.hpp
 * @brief StateMachine — 特殊状态判定 & 避险控制
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 当前实现: HEADING 状态机 (原地旋转/前进切换)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   参考: nav2_custom_plugins/src/mppi_gpu_controller.cpp determineState()
 *
 *   进入 HEADING_MISALIGN:
 *     abs(heading_err) > heading_misalign_threshold
 *     AND dist(lookahead, goal) >= 0.5m  (不干扰终端对准)
 *
 *   退出 HEADING_MISALIGN (迟滞):
 *     abs(heading_err) <= heading_misalign_threshold * 0.5
 *     (退出阈值 = 进入阈值的一半, 防止边界振荡)
 *
 *   旋转指令:
 *     vx=0, vy=0, omega=±max_w (参考原版)
 *     或 delta=lookahead_yaw (本系统 delta 模式)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 搁置: NARROW_PASSAGE, TERMINAL_ALIGN
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 在 computeVelocityCommands 中的使用
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   auto dec = state_machine_->evaluateHeading(
 *       current_yaw, lookahead_yaw, dist_to_goal, params_);
 *   if (dec.rotate_in_place) {
 *     // 跳过 MPPI, 直接发旋转指令
 *     return cmd;  // vx=0, vy=0, delta=lookahead_yaw
 *   }
 *   // else: 正常 MPPI 流程
 */

namespace nav2_custom_plugins_v2 {

struct MPPIParams;

/// 朝向判定结果
struct HeadingDecision {
  bool   rotate_in_place = false;  ///< 是否需要原地旋转
  double omega = 0.0;              ///< 目标朝向角 (rad), 已弃用, 保留兼容
  double omega_sign = 0.0;         ///< 旋转方向 ±1 (参考原版 max_w 旋转)
};

class StateMachine {
public:
  StateMachine() = default;

  /// 朝向偏差判定 (每帧调用, 在 MPPI 优化之前)
  /// @param current_yaw        当前机器人朝向 (rad)
  /// @param lookahead_yaw      前瞻点推荐朝向 (rad)
  /// @param dist_lh_to_goal    前瞻点到终点距离 (m), 用于防止终端附近误触发
  /// @param params             MPPI 参数
  HeadingDecision evaluateHeading(double current_yaw, double lookahead_yaw,
                                  double dist_lh_to_goal,
                                  const MPPIParams &params);

  /// 是否处于旋转状态 (调试/日志用)
  bool isRotating() const { return rotating_; }

  /// 重置状态 (新路径时调用)
  void reset() { rotating_ = false; }

private:
  bool rotating_ = false;  ///< 上一帧是否在旋转 (迟滞退出用)
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__STATE_MACHINE_HPP_
