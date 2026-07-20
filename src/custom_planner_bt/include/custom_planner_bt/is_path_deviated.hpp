// is_path_deviated.hpp
#pragma once
#include <nav_msgs/msg/path.hpp>
#include <nav2_behavior_tree/bt_action_node.hpp>
#include <behaviortree_cpp_v3/condition_node.h>
#include <tf2_ros/buffer.h>

namespace nav2_custom_bt_nodes
{

class IsPathDeviated : public BT::ConditionNode
{
public:
  IsPathDeviated(const std::string & name, const BT::NodeConfiguration & config)
  : BT::ConditionNode(name, config) {}

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<nav_msgs::msg::Path>("path", "current planned path"),
      BT::InputPort<double>("deviation_threshold", 0.5, "deviation threshold in meters"),
    };
  }

  BT::NodeStatus tick() override
  {
    nav_msgs::msg::Path path;
    double threshold;
    if (!getInput("path", path) || path.poses.empty()) {
      return BT::NodeStatus::SUCCESS;
    }
    getInput("deviation_threshold", threshold);

    auto node = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
    auto tf = config().blackboard->get<std::shared_ptr<tf2_ros::Buffer>>("tf_buffer");

    geometry_msgs::msg::PoseStamped robot_pose;
    try {
      auto transform = tf->lookupTransform(
        path.header.frame_id, "BASE_LINK", tf2::TimePointZero);
      robot_pose.pose.position.x = transform.transform.translation.x;
      robot_pose.pose.position.y = transform.transform.translation.y;
    } catch (...) {
      return BT::NodeStatus::FAILURE;
    }

    double min_dist = std::numeric_limits<double>::max();
    for (const auto & pose : path.poses) {
      double dx = robot_pose.pose.position.x - pose.pose.position.x;
      double dy = robot_pose.pose.position.y - pose.pose.position.y;
      double dist = std::hypot(dx, dy);
      min_dist = std::min(min_dist, dist);
    }

    return (min_dist > threshold) ?
      BT::NodeStatus::SUCCESS : BT::NodeStatus::FAILURE;
  }
};

}  // namespace nav2_custom_bt_nodes