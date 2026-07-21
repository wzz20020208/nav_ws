#ifndef NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_

/**
 * @file velocity_postprocessor.hpp
 * @brief VelocityPostProcessor — 加权结果提取 + 限幅 + omega→delta 转换
 *
 * 内部: [vx, vy, omega] — 全部 body 系 (omega 为角速度, warm-start 用)
 *   global 模式   → vx/vy 旋转 body→odom, omega→delta = yaw + omega·dt (目标角度)
 *   base_link 模式 → vx/vy/omega 直出 (body 系角速度)
 */

#include "nav2_custom_plugins_v2/core/mppi_core.hpp"
#include <vector>

namespace nav2_custom_plugins_v2 {

class VelocityPostProcessor {
public:
  explicit VelocityPostProcessor(const MPPIParams &params);

  struct ProcessedCommand {
    Control control;         ///< vx, vy, omega — 用于 warm-start (始终 body 系角速度)
    double  vx_out = 0.0;   ///< 输出线速度 x (body 系或 odom 系, 视模式)
    double  vy_out = 0.0;   ///< 输出线速度 y
    double  omega_out = 0.0;///< 输出: base_link=角速度, global=目标角度 delta
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
