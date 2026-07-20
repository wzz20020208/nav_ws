// is_path_obstructed.hpp
#pragma once
#include <nav_msgs/msg/path.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <behaviortree_cpp_v3/condition_node.h>
#include <tf2_ros/buffer.h>
#include <tf2/exceptions.h>
#include <rclcpp/rclcpp.hpp>
#include <cmath>

namespace nav2_custom_bt_nodes
{

class IsPathObstructed : public BT::ConditionNode
{
public:
  IsPathObstructed(const std::string & name, const BT::NodeConfiguration & config)
  : BT::ConditionNode(name, config)
  {
    auto node = config.blackboard->get<rclcpp::Node::SharedPtr>("node");

    std::string costmap_topic = "local_costmap/costmap";
    getInput("costmap_topic", costmap_topic);

    costmap_sub_ = node->create_subscription<nav_msgs::msg::OccupancyGrid>(
      costmap_topic, rclcpp::QoS(1).transient_local(),
      [this](nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
        costmap_ = msg;
      });
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<nav_msgs::msg::Path>("path", "current planned path"),
      BT::InputPort<std::string>("costmap_topic", "local_costmap/costmap", "costmap topic"),
      BT::InputPort<double>("lookahead_dist", 2.0, "lookahead distance in meters"),
      BT::InputPort<int>("lethal_threshold", 253, "lethal cost threshold"),
    };
  }

  BT::NodeStatus tick() override
  {
    nav_msgs::msg::Path path;
    double lookahead = 2.0;
    int lethal_thresh = 253;

    if (!getInput("path", path) || path.poses.empty()) {
      return BT::NodeStatus::SUCCESS;
    }

    getInput("lookahead_dist", lookahead);
    getInput("lethal_threshold", lethal_thresh);

    // costmap 还没收到，不阻塞
    if (!costmap_) {
      return BT::NodeStatus::FAILURE;
    }

    auto node = config().blackboard->get<rclcpp::Node::SharedPtr>("node");
    auto tf   = config().blackboard->get<std::shared_ptr<tf2_ros::Buffer>>("tf_buffer");

    geometry_msgs::msg::Point robot_pos;
    try {
      auto t = tf->lookupTransform(
        path.header.frame_id, "BASE_LINK", tf2::TimePointZero);
      robot_pos.x = t.transform.translation.x;
      robot_pos.y = t.transform.translation.y;
    } catch (const tf2::TransformException & ex) {
      RCLCPP_WARN(node->get_logger(), "IsPathObstructed TF error: %s", ex.what());
      return BT::NodeStatus::FAILURE;
    }

    for (const auto & pose : path.poses) {
      double dx = pose.pose.position.x - robot_pos.x;
      double dy = pose.pose.position.y - robot_pos.y;
      if (std::hypot(dx, dy) > lookahead) break;

      int mx = static_cast<int>(
        (pose.pose.position.x - costmap_->info.origin.position.x) / costmap_->info.resolution);
      int my = static_cast<int>(
        (pose.pose.position.y - costmap_->info.origin.position.y) / costmap_->info.resolution);

      if (mx < 0 || my < 0 ||
          mx >= static_cast<int>(costmap_->info.width) ||
          my >= static_cast<int>(costmap_->info.height)) continue;

      int cost = costmap_->data[my * costmap_->info.width + mx];
      if (cost >= lethal_thresh) {
        return BT::NodeStatus::SUCCESS;
      }
    }

    return BT::NodeStatus::FAILURE;
  }

private:
  nav_msgs::msg::OccupancyGrid::SharedPtr costmap_{nullptr};
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr costmap_sub_;
};

}  // namespace nav2_custom_bt_nodes