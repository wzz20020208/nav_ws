// Copyright (c) 2025
// IsProgressDiagnosis — 订阅 /progress_checker_diagnosis，BT 路由用

#include "nav2_custom_plugins/is_progress_diagnosis.hpp"
#include "behaviortree_cpp_v3/bt_factory.h"

namespace nav2_custom_plugins
{

IsProgressDiagnosis::IsProgressDiagnosis(
  const std::string & name,
  const BT::NodeConfiguration & config)
: BT::ConditionNode(name, config)
{
  // 创建一个独立的 rclcpp Node 用于订阅
  // 注意: 这是 BT 树外部的独立节点，生命周期跟随 BT
  node_ = std::make_shared<rclcpp::Node>(
    std::string("progress_diag_bt_") + name);

  sub_ = node_->create_subscription<std_msgs::msg::String>(
    "/progress_checker_diagnosis", 10,
    std::bind(&IsProgressDiagnosis::diagnosisCallback, this,
              std::placeholders::_1));
}

BT::PortsList IsProgressDiagnosis::providedPorts()
{
  return { BT::InputPort<std::string>("expected",
    "ok", "Expected diagnosis: oscillation | retreating | timeout | ok") };
}

void IsProgressDiagnosis::diagnosisCallback(
  const std_msgs::msg::String::SharedPtr msg)
{
  last_diag_ = msg->data;
}

BT::NodeStatus IsProgressDiagnosis::tick()
{
  // 处理 ROS2 消息队列
  rclcpp::spin_some(node_);

  auto expected = getInput<std::string>("expected");
  if (!expected) {
    return BT::NodeStatus::FAILURE;
  }

  return (last_diag_ == expected.value())
    ? BT::NodeStatus::SUCCESS
    : BT::NodeStatus::FAILURE;
}

}  // namespace nav2_custom_plugins

// ── 注册到 BehaviorTree.CPP 工厂 (v3 API) ──
BT_REGISTER_NODES(factory)
{
  BT::PortsList ports = {
    BT::InputPort<std::string>("expected", "ok",
      "Expected diagnosis: oscillation | retreating | timeout | ok")
  };
  factory.registerBuilder(
    BT::CreateManifest<nav2_custom_plugins::IsProgressDiagnosis>(
      "IsProgressDiagnosis", ports),
    BT::CreateBuilder<nav2_custom_plugins::IsProgressDiagnosis>());
}
