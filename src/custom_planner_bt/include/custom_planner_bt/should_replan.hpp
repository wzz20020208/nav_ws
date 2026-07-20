#pragma once

#include <nav_msgs/msg/path.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>
#include <behaviortree_cpp_v3/condition_node.h>
#include <tf2_ros/buffer.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#include <rclcpp/rclcpp.hpp>
#include <thread>
#include <mutex>

namespace nav2_custom_bt_nodes
{

class ShouldReplan : public BT::ConditionNode
{
public:
  ShouldReplan(const std::string & name, const BT::NodeConfiguration & config)
  : BT::ConditionNode(name, config)
  {
    // nav2's BT executor never spins the shared node's callbacks, so we
    // create a dedicated single-threaded executor on its own thread purely
    // for the costmap subscription.
    if (!getInput("costmap_topic", costmap_topic_)) {
      costmap_topic_ = "local_costmap/costmap";
    }

    costmap_node_ = std::make_shared<rclcpp::Node>("should_replan_costmap_node");

    costmap_sub_ = costmap_node_->create_subscription<nav_msgs::msg::OccupancyGrid>(
      costmap_topic_, rclcpp::SystemDefaultsQoS(),
      [this](nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
        std::lock_guard<std::mutex> lock(costmap_mutex_);
        costmap_ = msg;
      });

    executor_ = std::make_shared<rclcpp::executors::SingleThreadedExecutor>();
    executor_->add_node(costmap_node_);
    spin_thread_ = std::thread([this]() { executor_->spin(); });
  }

  ~ShouldReplan()
  {
    executor_->cancel();
    if (spin_thread_.joinable()) spin_thread_.join();
  }

  static BT::PortsList providedPorts()
  {
    return {
      BT::InputPort<nav_msgs::msg::Path>("path", "Current planned path"),
      BT::InputPort<geometry_msgs::msg::PoseStamped>("goal", "Current goal pose"),
      BT::InputPort<double>("deviation_threshold", 0.5, "Max allowed distance from path"),
      BT::InputPort<double>("lookahead_dist", 2.0, "Distance to check for obstacles"),
      BT::InputPort<int>("lethal_threshold", 90, "Lethal cost value (0-100 for OccupancyGrid)"),
      BT::InputPort<std::string>("costmap_topic", "local_costmap/costmap", "Costmap topic name")
    };
  }

  BT::NodeStatus tick() override
  {
    nav_msgs::msg::Path path;
    geometry_msgs::msg::PoseStamped current_goal;

    if (!getInput("path", path) || path.poses.empty()) return BT::NodeStatus::SUCCESS;
    if (!getInput("goal", current_goal)) return BT::NodeStatus::FAILURE;

    if (isGoalChanged(current_goal)) {
      last_goal_ = current_goal;
      return BT::NodeStatus::SUCCESS;
    }

    auto tf = config().blackboard->get<std::shared_ptr<tf2_ros::Buffer>>("tf_buffer");
    geometry_msgs::msg::PoseStamped robot_pose;
    try {
      auto transform = tf->lookupTransform(path.header.frame_id, "BASE_LINK", tf2::TimePointZero);
      robot_pose.pose.position.x = transform.transform.translation.x;
      robot_pose.pose.position.y = transform.transform.translation.y;
      robot_pose.header.frame_id = path.header.frame_id;
    } catch (tf2::TransformException & ex) {
      RCLCPP_WARN(rclcpp::get_logger("ShouldReplan"), "TF lookup failed: %s", ex.what());
      return BT::NodeStatus::FAILURE;
    }

    double dev_threshold;
    getInput("deviation_threshold", dev_threshold);
    if (isDeviated(robot_pose, path, dev_threshold)) {
      return BT::NodeStatus::SUCCESS;
    }

    // Take a snapshot of the latest costmap under lock
    nav_msgs::msg::OccupancyGrid::SharedPtr costmap_snapshot;
    {
      std::lock_guard<std::mutex> lock(costmap_mutex_);
      costmap_snapshot = costmap_;
    }

    if (!costmap_snapshot) {
      RCLCPP_WARN_THROTTLE(rclcpp::get_logger("ShouldReplan"),
        *costmap_node_->get_clock(), 5000,
        "No costmap received yet on '%s'", costmap_topic_.c_str());
      return BT::NodeStatus::FAILURE;
    }

    double lookahead;
    int lethal_thresh;
    getInput("lookahead_dist", lookahead);
    getInput("lethal_threshold", lethal_thresh);

    if (isObstructed(robot_pose, path, *costmap_snapshot, lookahead, lethal_thresh)) {
      RCLCPP_INFO(rclcpp::get_logger("ShouldReplan"), "Path obstructed — triggering replan");
      return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::FAILURE;
  }

private:
  bool isGoalChanged(const geometry_msgs::msg::PoseStamped & current_goal)
  {
    double dx = current_goal.pose.position.x - last_goal_.pose.position.x;
    double dy = current_goal.pose.position.y - last_goal_.pose.position.y;
    return std::hypot(dx, dy) > 0.10;
  }

  bool isDeviated(
    const geometry_msgs::msg::PoseStamped & robot,
    const nav_msgs::msg::Path & path,
    double thresh)
  {
    double min_dist = std::numeric_limits<double>::max();
    for (const auto & p : path.poses) {
      double d = std::hypot(
        robot.pose.position.x - p.pose.position.x,
        robot.pose.position.y - p.pose.position.y);
      if (d < min_dist) min_dist = d;
    }
    return min_dist > thresh;
  }

  bool isObstructed(
    const geometry_msgs::msg::PoseStamped & robot,
    const nav_msgs::msg::Path & path,
    const nav_msgs::msg::OccupancyGrid & costmap,
    double lookahead,
    int thresh)
  {
    auto tf = config().blackboard->get<std::shared_ptr<tf2_ros::Buffer>>("tf_buffer");
    geometry_msgs::msg::TransformStamped path_to_costmap;
    try {
      path_to_costmap = tf->lookupTransform(
        costmap.header.frame_id, path.header.frame_id, tf2::TimePointZero);
    } catch (tf2::TransformException & ex) {
      RCLCPP_WARN(rclcpp::get_logger("ShouldReplan"),
        "TF lookup failed ('%s' -> '%s'): %s",
        path.header.frame_id.c_str(), costmap.header.frame_id.c_str(), ex.what());
      return false;
    }

    // Find the path point closest to the robot
    size_t closest_idx = 0;
    double min_dist = std::numeric_limits<double>::max();
    for (size_t i = 0; i < path.poses.size(); ++i) {
      double d = std::hypot(
        path.poses[i].pose.position.x - robot.pose.position.x,
        path.poses[i].pose.position.y - robot.pose.position.y);
      if (d < min_dist) { min_dist = d; closest_idx = i; }
    }

    for (size_t i = closest_idx; i < path.poses.size(); ++i) {
      double dist_to_robot = std::hypot(
        path.poses[i].pose.position.x - robot.pose.position.x,
        path.poses[i].pose.position.y - robot.pose.position.y);
      if (dist_to_robot > lookahead) break;

      // path.poses[i].header.frame_id is empty by ROS convention;
      // copy path.header so doTransform has a valid source frame.
      geometry_msgs::msg::PoseStamped p_in;
      p_in.header = path.header;
      p_in.pose   = path.poses[i].pose;

      geometry_msgs::msg::PoseStamped p_out;
      tf2::doTransform(p_in, p_out, path_to_costmap);

      int mx = static_cast<int>(
        (p_out.pose.position.x - costmap.info.origin.position.x) / costmap.info.resolution);
      int my = static_cast<int>(
        (p_out.pose.position.y - costmap.info.origin.position.y) / costmap.info.resolution);

      if (mx < 0 || my < 0 ||
          mx >= static_cast<int>(costmap.info.width) ||
          my >= static_cast<int>(costmap.info.height))
      {
        continue;
      }

      int8_t cost = costmap.data[my * costmap.info.width + mx];
      if (cost >= static_cast<int8_t>(thresh) || cost == -1) {
        return true;
      }
    }
    return false;
  }

  std::string costmap_topic_;
  geometry_msgs::msg::PoseStamped last_goal_;

  rclcpp::Node::SharedPtr costmap_node_;
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr costmap_sub_;
  rclcpp::executors::SingleThreadedExecutor::SharedPtr executor_;
  std::thread spin_thread_;

  std::mutex costmap_mutex_;
  nav_msgs::msg::OccupancyGrid::SharedPtr costmap_{nullptr};
};

}  // namespace nav2_custom_bt_nodes
