// Copyright (c) 2025
// IsProgressDiagnosis — BT condition: check /progress_checker_diagnosis

#ifndef NAV2_CUSTOM_PLUGINS__IS_PROGRESS_DIAGNOSIS_HPP_
#define NAV2_CUSTOM_PLUGINS__IS_PROGRESS_DIAGNOSIS_HPP_

#include <string>
#include "behaviortree_cpp_v3/condition_node.h"
#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

namespace nav2_custom_plugins
{

/**
 * @class IsProgressDiagnosis
 * @brief BT Condition 节点: 订阅 /progress_checker_diagnosis，
 *        对比 expected 输入端口，匹配返回 SUCCESS。
 *
 * 用法 (BT XML):
 *   <IsProgressDiagnosis expected="oscillation"/>
 *
 * 配合 Fallback 实现诊断驱动 recovery:
 *   <Fallback>
 *     <Sequence>
 *       <IsProgressDiagnosis expected="oscillation"/>
 *       <Spin spin_dist="0.785"/>
 *     </Sequence>
 *     <Sequence>
 *       <IsProgressDiagnosis expected="retreating"/>
 *       <BackUp backup_dist="0.50"/>
 *     </Sequence>
 *     ...
 *   </Fallback>
 */
class IsProgressDiagnosis : public BT::ConditionNode
{
public:
  IsProgressDiagnosis(const std::string & name,
                      const BT::NodeConfiguration & config);
  ~IsProgressDiagnosis() override = default;

  static BT::PortsList providedPorts();

  BT::NodeStatus tick() override;

private:
  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr sub_;
  std::string last_diag_{"ok"};

  void diagnosisCallback(const std_msgs::msg::String::SharedPtr msg);
};

}  // namespace nav2_custom_plugins

#endif  // NAV2_CUSTOM_PLUGINS__IS_PROGRESS_DIAGNOSIS_HPP_
