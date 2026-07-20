#ifndef NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_

/**
 * @file velocity_postprocessor.hpp
 * @brief VelocityPostProcessor — 加权结果提取 + 限幅
 *
 * 控制空间 [vx, vy, omega], omega 即角速度, 无需 δ→ω 转换.
 *
 *   base_link 模式 → vx/vy/omega 直出
 *   global 模式   → vx/vy 旋转到 odom 系, omega 直出
 */

#include "nav2_custom_plugins_v2/core/mppi_core.hpp"
#include <vector>

namespace nav2_custom_plugins_v2 {

class VelocityPostProcessor {
public:
  explicit VelocityPostProcessor(const MPPIParams &params);

  struct ProcessedCommand {
    Control control;         ///< vx, vy, omega — 用于 warm-start 更新 (始终初始朝向系)
    double  vx_out = 0.0;   ///< 输出线速度 x (已转换到目标系)
    double  vy_out = 0.0;   ///< 输出线速度 y
    double  omega_out = 0.0;///< 输出角速度
    bool    global_mode = false;
  };

  ProcessedCommand process(const std::vector<float> &result,
                           double current_yaw, bool global_mode);

private:
  const MPPIParams &p_;

  Control extractStep0(const std::vector<float> &result) const;
  Control clamp(const Control &raw) const;
};

}  // namespace nav2_custom_plugins_v2

#endif
