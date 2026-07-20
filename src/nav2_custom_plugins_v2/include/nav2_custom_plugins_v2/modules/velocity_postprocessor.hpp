#ifndef NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_

/**
 * @file velocity_postprocessor.hpp
 * @brief VelocityPostProcessor — 加权结果提取 + 限幅 + δ→ω 转换
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 职责 (解耦 steering_controller 末尾的控制量后处理)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ① 从加权结果数组 [H×4] 提取 step-0 控制量 (vx_sum/w, vy_sum/w, delta_sum/w)
 *   ② clamp 到物理限幅 (max_v, max_vy, max_steering_angle)
 *   ③ 输出:
 *        base_link 模式 → δ→ω 转换 (deltaToOmega)
 *        global 模式   → vx/vy 直出 (运动模型 x+=vx*dt, 已是全局量) + delta 作为目标角度
 *
 * 搁置功能: EMA 平滑、前瞻减速、heading 限速 (后续按需加回)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 使用
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   auto proc = postprocessor_->process(result, current_yaw);
 *   // proc.control: 用于 warm-start 更新
 *   // proc.omega:   填入 TwistStamped.angular.z
 */

#include "nav2_custom_plugins_v2/core/mppi_core.hpp"
#include <vector>

namespace nav2_custom_plugins_v2 {

class VelocityPostProcessor {
public:
  explicit VelocityPostProcessor(const MPPIParams &params);

  /// 后处理产出
  struct ProcessedCommand {
    Control control;       ///< vx, vy, delta — 用于 warm-start 更新 (始终初始朝向系)
    double  omega = 0.0;   ///< body 系角速度 (δ→ω 转换后), 仅 base_link 模式使用
    double  vx_out = 0.0;  ///< 输出线速度 x (已转换到目标系)
    double  vy_out = 0.0;  ///< 输出线速度 y
    double  delta_out = 0.0; ///< 输出角度 (已转换到目标系)
    bool    global_mode = false;
  };

  /// 完整后处理: 提取 → clamp → (δ→ω 或 TF 旋转)
  /// @param result       GPU 加权结果 [H×4]
  /// @param current_yaw  当前机器人朝向 (rad)
  /// @param global_mode  true=odom 系输出 (vx/vy 旋转 + delta 直出), false=base_link 系 (δ→ω)
  ProcessedCommand process(const std::vector<float> &result,
                           double current_yaw, bool global_mode);

private:
  const MPPIParams &p_;

  /// 提取 step-0 加权控制量: best[c] = result[t*4+c] / result[t*4+3]
  Control extractStep0(const std::vector<float> &result) const;

  /// clamp vx/vy/delta 到 [min, max]
  Control clamp(const Control &raw) const;

  /// δ→ω: dtheta = clamp(normalize(delta - theta), ±max_w*dt)
  double deltaToOmega(double delta, double current_theta) const;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__VELOCITY_POSTPROCESSOR_HPP_
