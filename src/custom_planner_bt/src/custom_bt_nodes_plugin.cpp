#include "custom_planner_bt/should_replan.hpp"
#include <behaviortree_cpp_v3/bt_factory.h>

extern "C" void BT_RegisterNodesFromPlugin(BT::BehaviorTreeFactory & factory)
{
  BT::NodeBuilder builder_deviated =
    [](const std::string & name, const BT::NodeConfiguration & config)
    {
      return std::make_unique<nav2_custom_bt_nodes::ShouldReplan>(name, config);
    };
  factory.registerBuilder<nav2_custom_bt_nodes::ShouldReplan>(
    "ShouldReplan", builder_deviated);
}