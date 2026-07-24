/**
 * @file param_loader.hpp
 * @brief ParamLoader — YAML 参数加载 + GPU struct 构建
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 职责
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   1. configure() — 从 ROS node 加载全部 MPPI 参数 → MPPIParams
 *   2. buildFootprint()    — MPPIParams → Footprint (碰撞箱体/采样间距)
 *   3. buildCriticParams() — MPPIParams → CriticParams (大类权重)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 使用
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   // configure 阶段
 *   ParamLoader loader;
 *   loader.configure(node, plugin_name, params_);
 *
 *   // computeVelocityCommands 阶段
 *   Footprint fp = ParamLoader::buildFootprint(params_);
 *   CriticParams cp = ParamLoader::buildCriticParams(params_);
 */

#ifndef NAV2_CUSTOM_PLUGINS_V2__MODULES__PARAM_LOADER_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__MODULES__PARAM_LOADER_HPP_

#include <memory>
#include <string>
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"  // MPPIParams, Footprint, CriticParams

namespace rclcpp_lifecycle { class LifecycleNode; }

namespace nav2_custom_plugins_v2
{

class ParamLoader
{
public:
  /// 从 ROS node 加载全部 YAML 参数 → MPPIParams
  ///
  /// 参数前缀: node.get_parameter(plugin_name + "." + name, value)
  /// 新增参数只需在 .cpp 的 kDblParams[] 等表中加一行。
  ///
  /// @param node        ROS 2 LifecycleNode (shared_ptr, 与 nav2_util 接口兼容)
  /// @param plugin_name 参数名前缀 (e.g. "FollowPath")
  /// @param params      输出: 填充后的 MPPIParams
  void configure(std::shared_ptr<rclcpp_lifecycle::LifecycleNode> node,
                 const std::string &plugin_name,
                 MPPIParams &params);

  /// 从 MPPIParams 构建 GPU 侧足迹参数
  static Footprint buildFootprint(const MPPIParams &p);

  /// 从 MPPIParams 构建 GPU 侧代价参数 (大类权重)
  static CriticParams buildCriticParams(const MPPIParams &p);
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__MODULES__PARAM_LOADER_HPP_
