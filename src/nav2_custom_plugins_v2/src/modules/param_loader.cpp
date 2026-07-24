/**
 * @file param_loader.cpp
 * @brief ParamLoader 实现 — YAML → MPPIParams, MPPIParams → GPU structs
 */

#include "nav2_custom_plugins_v2/modules/param_loader.hpp"

#include "nav2_util/node_utils.hpp"    // declare_parameter_if_not_declared
#include "rclcpp/rclcpp.hpp"           // ParameterValue
#include "rclcpp_lifecycle/lifecycle_node.hpp"

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════════
// 参数表 — 新增参数只需在此加一行, controller 无需修改
// ═══════════════════════════════════════════════════════════════════════════════

namespace
{

struct IntParam   { const char *name; int MPPIParams::*ptr; };
struct DblParam   { const char *name; double MPPIParams::*ptr; };
struct BoolParam  { const char *name; bool MPPIParams::*ptr; };
struct StrParam   { const char *name; std::string MPPIParams::*ptr; };

constexpr IntParam kIntParams[] = {
  {"num_samples",        &MPPIParams::num_samples},
  {"prediction_horizon", &MPPIParams::prediction_horizon},
};

constexpr DblParam kDblParams[] = {
  {"dt",                 &MPPIParams::dt},
  {"max_v",              &MPPIParams::max_v},
  {"min_v",              &MPPIParams::min_v},
  {"max_vy",             &MPPIParams::max_vy},
  {"max_w",              &MPPIParams::max_w},
  {"action_std_v",       &MPPIParams::action_std_v},
  {"action_std_vy",      &MPPIParams::action_std_vy},
  {"action_std_w",       &MPPIParams::action_std_w},
  {"lambda",             &MPPIParams::lambda},
  {"cost_scale",         &MPPIParams::cost_scale},
  {"obstacle_ratio",     &MPPIParams::obstacle_ratio},
  {"tracking_ratio",     &MPPIParams::tracking_ratio},
  {"speed_ratio",        &MPPIParams::speed_ratio},
  {"footprint_front",    &MPPIParams::footprint_front},
  {"footprint_back",     &MPPIParams::footprint_back},
  {"footprint_left",     &MPPIParams::footprint_left},
  {"footprint_right",    &MPPIParams::footprint_right},
  {"footprint_weight",       &MPPIParams::footprint_weight},
  {"path_align_weight",      &MPPIParams::path_align_weight},
  {"path_angle_weight",      &MPPIParams::path_angle_weight},
  {"path_deviation_weight",  &MPPIParams::path_deviation_weight},
  {"speed_reward_weight",      &MPPIParams::speed_reward_weight},
  {"base_similarity_weight",   &MPPIParams::base_similarity_weight},
  {"min_lookahead_dist",          &MPPIParams::min_lookahead_dist},
  {"lookahead_kp",               &MPPIParams::lookahead_kp},
  {"lookahead_decel_dist",       &MPPIParams::lookahead_decel_dist},
  {"lookahead_overshoot_weight", &MPPIParams::lookahead_overshoot_weight},
  {"terminal_angle_dist",      &MPPIParams::terminal_angle_dist},
  {"terminal_angle_tolerance", &MPPIParams::terminal_angle_tolerance},
  {"ema_alpha",          &MPPIParams::ema_alpha},
};

constexpr BoolParam kBoolParams[] = {
  {"use_planner_yaw",             &MPPIParams::use_planner_yaw},
  {"use_global_mode",             &MPPIParams::use_global_mode},
  {"enable_heading_speed_limit",  &MPPIParams::enable_heading_speed_limit},
  {"enable_narrow_passage",       &MPPIParams::enable_narrow_passage},
  {"enable_ema",                  &MPPIParams::enable_ema},
  {"enable_file_log",             &MPPIParams::enable_file_log},
};

}  // anonymous namespace

// ═══════════════════════════════════════════════════════════════════════════════
// configure — ROS node → MPPIParams
// ═══════════════════════════════════════════════════════════════════════════════

void ParamLoader::configure(
    std::shared_ptr<rclcpp_lifecycle::LifecycleNode> node,
    const std::string &plugin_name,
    MPPIParams &params)
{
  for (auto &p : kIntParams) {
    nav2_util::declare_parameter_if_not_declared(
        node, plugin_name + "." + p.name,
        rclcpp::ParameterValue(params.*p.ptr));
    node->get_parameter(plugin_name + "." + p.name, params.*p.ptr);
  }
  for (auto &p : kDblParams) {
    nav2_util::declare_parameter_if_not_declared(
        node, plugin_name + "." + p.name,
        rclcpp::ParameterValue(params.*p.ptr));
    node->get_parameter(plugin_name + "." + p.name, params.*p.ptr);
  }
  for (auto &p : kBoolParams) {
    nav2_util::declare_parameter_if_not_declared(
        node, plugin_name + "." + p.name,
        rclcpp::ParameterValue(params.*p.ptr));
    node->get_parameter(plugin_name + "." + p.name, params.*p.ptr);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// buildFootprint — MPPIParams → Footprint
// ═══════════════════════════════════════════════════════════════════════════════

Footprint ParamLoader::buildFootprint(const MPPIParams &p)
{
  Footprint fp;
  fp.front   = static_cast<float>(p.footprint_front);
  fp.back    = static_cast<float>(p.footprint_back);
  fp.left    = static_cast<float>(p.footprint_left);
  fp.right   = static_cast<float>(p.footprint_right);
  // sample_spacing 和 rear_obstacle_cost 保留默认值, 待后续暴露
  return fp;
}

// ═══════════════════════════════════════════════════════════════════════════════
// buildCriticParams — MPPIParams → CriticParams
// ═══════════════════════════════════════════════════════════════════════════════

CriticParams ParamLoader::buildCriticParams(const MPPIParams &p)
{
  CriticParams cp;
  cp.obstacle_ratio = static_cast<float>(p.obstacle_ratio);
  cp.tracking_ratio = static_cast<float>(p.tracking_ratio);
  cp.speed_ratio    = static_cast<float>(p.speed_ratio);

  cp.footprint_weight       = static_cast<float>(p.footprint_weight);
  cp.path_align_weight      = static_cast<float>(p.path_align_weight);
  cp.path_angle_weight      = static_cast<float>(p.path_angle_weight);
  cp.path_deviation_weight  = static_cast<float>(p.path_deviation_weight);
  cp.speed_reward_weight      = static_cast<float>(p.speed_reward_weight);
  cp.base_similarity_weight   = static_cast<float>(p.base_similarity_weight);
  return cp;
}

}  // namespace nav2_custom_plugins_v2
