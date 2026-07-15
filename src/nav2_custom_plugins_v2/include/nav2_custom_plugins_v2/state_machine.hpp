#ifndef NAV2_CUSTOM_PLUGINS_V2__STATE_MACHINE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__STATE_MACHINE_HPP_

#include <cstdint>
#include "nav2_custom_plugins_v2/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// 控制器状态
enum class ControllerState : uint8_t {
  NORMAL = 0,
  HEADING_MISALIGN = 1,
  NARROW_PASSAGE = 2
};

/// 窄道子状态
enum class NarrowSubState : uint8_t {
  ROTATE_TO_SIDE = 0,
  MOVE_ALONG_PATH = 1
};

/// 状态机: 特殊场景判定 & 避险控制指令生成
class StateMachine {
public:
  StateMachine() = default;

  /// 判定当前状态 (每帧调用, 优先级: HEADING > NARROW > NORMAL)
  ControllerState determineState(
      const RobotState& robot,
      const LookaheadPoint& lh,
      const Control& mppi_output,
      const MPPIParams& params);

  /// 特殊状态下直接生成控制指令 (跳过 MPPI 优化)
  /// 仅 HEADING_MISALIGN 和 NARROW_PASSAGE 时调用
  Control computeSpecialCommand(
      ControllerState state,
      const RobotState& robot,
      const LookaheadPoint& lh,
      const MPPIParams& params);

  /// 获取当前状态
  ControllerState currentState() const { return state_; }

  /// 重置状态
  void reset();

private:
  ControllerState state_ = ControllerState::NORMAL;
  NarrowSubState narrow_sub_ = NarrowSubState::ROTATE_TO_SIDE;

  // 状态持久数据
  double side_yaw_ = 0.0;
  int hesitate_count_ = 0;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__STATE_MACHINE_HPP_
