#include "nav2_custom_plugins/mppi_controller.hpp"
#include <limits>
#include <cmath>
#include <algorithm>
#include <random>
#include "tf2/utils.h"

namespace nav2_custom_plugins
{

struct RobotState {
  double x, y, theta;
  double vx, vy;
};

struct TrajectorySample {
  std::vector<double> vx_seq;
  std::vector<double> vy_seq;
  std::vector<double> omega_seq;
  double total_cost;
  double weight;
};

RobotState predictNextState(
  const RobotState& current,
  double vx, double vy, double omega,
  double dt)
{
  RobotState next;
  next.x = current.x + (vx * std::cos(current.theta) - vy * std::sin(current.theta)) * dt;
  next.y = current.y + (vx * std::sin(current.theta) + vy * std::cos(current.theta)) * dt;
  next.theta = current.theta + omega * dt;

  while (next.theta > M_PI) next.theta -= 2 * M_PI;
  while (next.theta < -M_PI) next.theta += 2 * M_PI;

  return next;
}

double computeTrajectoryCost(
  const std::vector<RobotState>& trajectory,
  double target_x, double target_y,
  double robot_x, double robot_y,
  const nav2_costmap_2d::Costmap2D* costmap,
  double collision_cost,
  double heading_weight)
{
  if (trajectory.empty()) {
    return std::numeric_limits<double>::max();
  }

  double cost = 0.0;
  const RobotState& final_state = trajectory.back();

  double dist_to_goal = std::hypot(target_x - final_state.x, target_y - final_state.y);
  cost += dist_to_goal * 2.0;

  double path_cost = 0.0;
  double heading_cost = 0.0;
  for (size_t i = 0; i < trajectory.size(); ++i) {
    double progress = static_cast<double>(i) / trajectory.size();
    double dist = std::hypot(target_x - trajectory[i].x, target_y - trajectory[i].y);
    path_cost += dist * (1.0 - progress);

    // 朝向奖励：鼓励机器人面朝行进方向（位置增量方向）
    double dx_travel, dy_travel;
    if (i == 0) {
      dx_travel = trajectory[i].x - robot_x;
      dy_travel = trajectory[i].y - robot_y;
    } else {
      dx_travel = trajectory[i].x - trajectory[i - 1].x;
      dy_travel = trajectory[i].y - trajectory[i - 1].y;
    }
    double travel_dir = std::atan2(dy_travel, dx_travel);
    double heading_error = travel_dir - trajectory[i].theta;
    while (heading_error > M_PI) heading_error -= 2 * M_PI;
    while (heading_error < -M_PI) heading_error += 2 * M_PI;
    heading_cost += std::abs(heading_error);
  }
  cost += path_cost * 0.5;
  cost += heading_cost * heading_weight;

  if (costmap != nullptr) {
    unsigned int mx, my;
    for (const auto& state : trajectory) {
      if (costmap->worldToMap(state.x, state.y, mx, my)) {
        unsigned char cost_val = costmap->getCost(mx, my);

        if (cost_val >= nav2_costmap_2d::LETHAL_OBSTACLE) {
          cost += collision_cost;
        } else if (cost_val >= nav2_costmap_2d::INSCRIBED_INFLATED_OBSTACLE) {
          cost += collision_cost * 0.5;
        }
      }
    }
  }

  for (size_t i = 1; i < trajectory.size(); ++i) {
    double dx = trajectory[i].x - trajectory[i-1].x;
    double dy = trajectory[i].y - trajectory[i-1].y;
    cost += std::sqrt(dx*dx + dy*dy) * 0.1;
  }

  return cost;
}

void MPPIController::configure(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
  std::string name,
  std::shared_ptr<tf2_ros::Buffer> tf,
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros)
{
  node_ = parent;
  plugin_name_ = name;
  tf_ = tf;
  costmap_ros_ = costmap_ros;

  auto node_ptr = node_.lock();
  vis_pub_ = node_ptr->create_publisher<visualization_msgs::msg::MarkerArray>(
    "/mppi_visualization", 10);

  optimal_vx_seq_.resize(prediction_horizon_, 0.0);
  optimal_vy_seq_.resize(prediction_horizon_, 0.0);
  optimal_omega_seq_.resize(prediction_horizon_, 0.0);

  std::random_device rd;
  generator_ = std::mt19937(rd());
  dist_vx_ = std::normal_distribution<>(0.0, action_std_v_);
  dist_vy_ = std::normal_distribution<>(0.0, action_std_v_);
  dist_w_ = std::normal_distribution<>(0.0, action_std_w_);

  RCLCPP_INFO(node_.lock()->get_logger(), "配置 MPPIController 成功！");
}

void MPPIController::cleanup()
{
  RCLCPP_INFO(node_.lock()->get_logger(), "清理 MPPIController");
}

void MPPIController::activate()
{
  RCLCPP_INFO(node_.lock()->get_logger(), "激活 MPPIController");
}

void MPPIController::deactivate()
{
  RCLCPP_INFO(node_.lock()->get_logger(), "停用 MPPIController");
}

void MPPIController::setPlan(const nav_msgs::msg::Path & path)
{
  global_plan_ = path;
}

void MPPIController::setSpeedLimit(const double &, const bool &)
{
}

geometry_msgs::msg::TwistStamped MPPIController::computeVelocityCommands(
  const geometry_msgs::msg::PoseStamped & pose,
  const geometry_msgs::msg::Twist &,
  nav2_core::GoalChecker *)
{
  geometry_msgs::msg::TwistStamped cmd_vel;
  cmd_vel.header.frame_id = "base_link";
  cmd_vel.header.stamp = node_.lock()->now();

  RobotState current_state;
  current_state.x = pose.pose.position.x;
  current_state.y = pose.pose.position.y;
  current_state.theta = tf2::getYaw(pose.pose.orientation);
  current_state.vx = 0.0;
  current_state.vy = 0.0;

  double target_x = current_state.x;
  double target_y = current_state.y;
  double target_idx = 0;

  if (!global_plan_.poses.empty()) {
    double min_dist = std::numeric_limits<double>::max();
    int closest_idx = 0;

    for (size_t i = 0; i < global_plan_.poses.size(); ++i) {
      double dx = global_plan_.poses[i].pose.position.x - current_state.x;
      double dy = global_plan_.poses[i].pose.position.y - current_state.y;
      double dist = std::hypot(dx, dy);
      if (dist < min_dist) {
        min_dist = dist;
        closest_idx = static_cast<int>(i);
      }
    }

    double accumulated_dist = 0.0;
    target_idx = closest_idx;

    for (int i = closest_idx + 1; i < static_cast<int>(global_plan_.poses.size()); ++i) {
      double dx = global_plan_.poses[i].pose.position.x - global_plan_.poses[i-1].pose.position.x;
      double dy = global_plan_.poses[i].pose.position.y - global_plan_.poses[i-1].pose.position.y;
      double segment_dist = std::hypot(dx, dy);
      accumulated_dist += segment_dist;

      if (accumulated_dist >= lookahead_distance_) {
        target_idx = i;
        break;
      }
    }

    // 当剩余路径长度不足 lookahead_distance 时，使用最终目标点
    if (target_idx == closest_idx && !global_plan_.poses.empty()) {
      target_idx = static_cast<int>(global_plan_.poses.size()) - 1;
    }

    target_x = global_plan_.poses[target_idx].pose.position.x;
    target_y = global_plan_.poses[target_idx].pose.position.y;
  }

  // ========== 计算全局路径在目标点的方向（用于引导采样）==========
  double path_direction_x = 0.0;
  double path_direction_y = 0.0;
  if (!global_plan_.poses.empty() && target_idx + 1 < static_cast<int>(global_plan_.poses.size())) {
    path_direction_x = global_plan_.poses[target_idx + 1].pose.position.x - global_plan_.poses[target_idx].pose.position.x;
    path_direction_y = global_plan_.poses[target_idx + 1].pose.position.y - global_plan_.poses[target_idx].pose.position.y;
    double path_len = std::hypot(path_direction_x, path_direction_y);
    if (path_len > 0.01) {
      path_direction_x /= path_len;
      path_direction_y /= path_len;
    }
  }

  nav2_costmap_2d::Costmap2D* costmap = nullptr;
  if (costmap_ros_ != nullptr) {
    costmap = costmap_ros_->getCostmap();
  }

  // ========== MPPI 记忆机制 ==========
  if (!initialized_) {
    double dx = target_x - current_state.x;
    double dy = target_y - current_state.y;
    double angle_to_goal = std::atan2(dy, dx);
    double angle_diff = angle_to_goal - current_state.theta;
    while (angle_diff > M_PI) angle_diff -= 2 * M_PI;
    while (angle_diff < -M_PI) angle_diff += 2 * M_PI;

    double init_vx = std::min(max_v_, lookahead_distance_ / dt_);
    double init_vy = 0.0;
    double init_omega = angle_diff / dt_;
    init_omega = std::max(-max_w_, std::min(max_w_, init_omega));

    for (int i = 0; i < prediction_horizon_; ++i) {
      optimal_vx_seq_[i] = init_vx;
      optimal_vy_seq_[i] = init_vy;
      optimal_omega_seq_[i] = init_omega;
    }
    initialized_ = true;
  } else {
    for (int i = 0; i < prediction_horizon_ - 1; ++i) {
      optimal_vx_seq_[i] = optimal_vx_seq_[i + 1];
      optimal_vy_seq_[i] = optimal_vy_seq_[i + 1];
      optimal_omega_seq_[i] = optimal_omega_seq_[i + 1];
    }
    optimal_vx_seq_.back() = 0.0;
    optimal_vy_seq_.back() = 0.0;
    optimal_omega_seq_.back() = 0.0;
  }

  std::vector<TrajectorySample> samples(num_samples_);

  for (int s = 0; s < num_samples_; ++s) {
    TrajectorySample& sample = samples[s];
    sample.vx_seq.reserve(prediction_horizon_);
    sample.vy_seq.reserve(prediction_horizon_);
    sample.omega_seq.reserve(prediction_horizon_);
    sample.total_cost = 0.0;
    sample.weight = 0.0;

    RobotState sim_state = current_state;
    std::vector<RobotState> trajectory;
    trajectory.reserve(prediction_horizon_);

    for (int t = 0; t < prediction_horizon_; ++t) {
      // 基于路径方向的采样：主要沿路径方向添加噪声
      double base_vx = optimal_vx_seq_[t];
      double base_vy = optimal_vy_seq_[t];
      double base_omega = optimal_omega_seq_[t];

      // 世界坐标系下的路径方向转机器人坐标系
      double world_to_robot_cos = std::cos(-current_state.theta);
      double world_to_robot_sin = std::sin(-current_state.theta);
      double path_vx_robot = path_direction_x * world_to_robot_cos - path_direction_y * world_to_robot_sin;
      double path_vy_robot = path_direction_x * world_to_robot_sin + path_direction_y * world_to_robot_cos;

      // 将路径方向速度作为主要趋势，叠加较小噪声
      double guidance_weight = 0.7;  // 路径引导权重
      double vx = (1.0 - guidance_weight) * (base_vx + dist_vx_(generator_))
                 + guidance_weight * path_vx_robot * std::hypot(base_vx, base_vy);
      double vy = (1.0 - guidance_weight) * (base_vy + dist_vy_(generator_))
                 + guidance_weight * path_vy_robot * std::hypot(base_vx, base_vy);
      double omega = base_omega + dist_w_(generator_) * 0.5;  // 角速度噪声减小

      vx = std::max(min_v_, std::min(max_v_, vx));
      vy = std::max(min_v_, std::min(max_v_, vy));
      omega = std::max(-max_w_, std::min(max_w_, omega));

      sample.vx_seq.push_back(vx);
      sample.vy_seq.push_back(vy);
      sample.omega_seq.push_back(omega);

      sim_state = predictNextState(sim_state, vx, vy, omega, dt_);
      trajectory.push_back(sim_state);

      // 预测超出终点时截断（保证至少 1 步）
      if (std::hypot(target_x - sim_state.x, target_y - sim_state.y) < 0.25) break;
    }

    sample.total_cost = computeTrajectoryCost(trajectory, target_x, target_y,
                                               current_state.x, current_state.y,
                                               costmap, collision_cost_, heading_weight_);
  }

  double sum_weights = 0.0;
  double min_cost = std::numeric_limits<double>::max();

  for (int s = 0; s < num_samples_; ++s) {
    if (samples[s].total_cost < min_cost) {
      min_cost = samples[s].total_cost;
    }
  }

  for (int s = 0; s < num_samples_; ++s) {
    samples[s].weight = std::exp(-(samples[s].total_cost - min_cost) / lambda_);
    sum_weights += samples[s].weight;
  }

  for (int s = 0; s < num_samples_; ++s) {
    samples[s].weight /= std::max(sum_weights, 1e-6);
  }

  double best_vx = 0.0, best_vy = 0.0, best_omega = 0.0;

  for (int s = 0; s < num_samples_; ++s) {
    if (samples[s].vx_seq.empty()) continue;
    best_vx += samples[s].weight * samples[s].vx_seq[0];
    best_vy += samples[s].weight * samples[s].vy_seq[0];
    best_omega += samples[s].weight * samples[s].omega_seq[0];
  }

  best_vx = std::max(min_v_, std::min(max_v_, best_vx));
  best_vy = std::max(min_v_, std::min(max_v_, best_vy));
  best_omega = std::max(-max_w_, std::min(max_w_, best_omega));

  optimal_vx_seq_.back() = best_vx;
  optimal_vy_seq_.back() = best_vy;
  optimal_omega_seq_.back() = best_omega;

  double min_output = 0.01;
  if (std::abs(best_vx) < min_output && std::abs(best_vy) < min_output && std::abs(best_omega) < min_output) {
    double dx = target_x - current_state.x;
    double dy = target_y - current_state.y;
    double angle_to_goal = std::atan2(dy, dx);
    double angle_diff = angle_to_goal - current_state.theta;
    while (angle_diff > M_PI) angle_diff -= 2 * M_PI;
    while (angle_diff < -M_PI) angle_diff += 2 * M_PI;
    best_vx = 0.3;
    best_omega = angle_diff * 0.5;
  }

  cmd_vel.twist.linear.x = best_vx;
  cmd_vel.twist.linear.y = best_vy;
  cmd_vel.twist.angular.z = best_omega;

  // ========== 发布可视化 ==========
  // 仅绘制 MPPI 预测时域内的轨迹（vx, vy, omega 按 dt 组合的预测），不进行外推
  // 挑选代价最低的 10 条轨迹显示
  const int TOP_K = 10;
  std::vector<std::vector<double>> trajectories;
  std::vector<double> costs;

  // 找出代价最低的 TOP_K 条轨迹的索引
  std::vector<int> top_indices;
  {
    std::vector<std::pair<double, int>> cost_idx_pairs;
    cost_idx_pairs.reserve(num_samples_);
    for (int s = 0; s < num_samples_; ++s) {
      cost_idx_pairs.emplace_back(samples[s].total_cost, s);
    }
    int n_select = std::min(TOP_K, num_samples_);
    std::partial_sort(cost_idx_pairs.begin(),
                      cost_idx_pairs.begin() + n_select,
                      cost_idx_pairs.end());
    for (int k = 0; k < n_select; ++k) {
      top_indices.push_back(cost_idx_pairs[k].second);
    }
  }

  const double GOAL_REACHED_DIST = 0.25;

  for (int k = 0; k < static_cast<int>(top_indices.size()); ++k) {
    int s = top_indices[k];
    std::vector<double> traj;
    RobotState st = current_state;
    for (int t = 0; t < prediction_horizon_; ++t) {
      traj.push_back(st.x);
      traj.push_back(st.y);
      st.x += (samples[s].vx_seq[t] * std::cos(st.theta) - samples[s].vy_seq[t] * std::sin(st.theta)) * dt_;
      st.y += (samples[s].vx_seq[t] * std::sin(st.theta) + samples[s].vy_seq[t] * std::cos(st.theta)) * dt_;
      st.theta += samples[s].omega_seq[t] * dt_;
      // 预测超出终点时截断
      if (std::hypot(target_x - st.x, target_y - st.y) < GOAL_REACHED_DIST) break;
    }
    trajectories.push_back(traj);
    costs.push_back(samples[s].total_cost);
  }

  int best_idx = 0;  // top_indices[0] 代价最低，为最优轨迹（绿色）

  publishVisualization(current_state.x, current_state.y, current_state.theta,
                       target_x, target_y, trajectories, costs, best_idx);

  return cmd_vel;
}

void MPPIController::publishVisualization(
  double robot_x, double robot_y, double robot_theta,
  double target_x, double target_y,
  const std::vector<std::vector<double>>& trajectories,
  const std::vector<double>& trajectory_costs,
  int best_idx)
{
  auto node_ptr = node_.lock();
  if (!node_ptr || !vis_pub_) return;

  visualization_msgs::msg::MarkerArray marker_array;
  rclcpp::Time now = node_ptr->now();

  // 机器人位置（红色）
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = "map";
    m.header.stamp = now;
    m.ns = "mppi";
    m.id = 0;
    m.type = visualization_msgs::msg::Marker::SPHERE;
    m.action = visualization_msgs::msg::Marker::ADD;
    m.pose.position.x = robot_x;
    m.pose.position.y = robot_y;
    m.pose.position.z = 0.1;
    m.scale.x = 0.3;
    m.scale.y = 0.3;
    m.scale.z = 0.3;
    m.color.r = 1.0;
    m.color.g = 0.0;
    m.color.b = 0.0;
    m.color.a = 1.0;
    marker_array.markers.push_back(m);
  }

  // 目标点（绿色）
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = "map";
    m.header.stamp = now;
    m.ns = "mppi";
    m.id = 1;
    m.type = visualization_msgs::msg::Marker::SPHERE;
    m.action = visualization_msgs::msg::Marker::ADD;
    m.pose.position.x = target_x;
    m.pose.position.y = target_y;
    m.pose.position.z = 0.1;
    m.scale.x = 0.4;
    m.scale.y = 0.4;
    m.scale.z = 0.4;
    m.color.r = 0.0;
    m.color.g = 1.0;
    m.color.b = 0.0;
    m.color.a = 1.0;
    marker_array.markers.push_back(m);
  }

  // 机器人朝向（黄色箭头）
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = "map";
    m.header.stamp = now;
    m.ns = "mppi";
    m.id = 2;
    m.type = visualization_msgs::msg::Marker::LINE_STRIP;
    m.action = visualization_msgs::msg::Marker::ADD;
    m.points.resize(2);
    m.points[0].x = robot_x;
    m.points[0].y = robot_y;
    m.points[0].z = 0.15;
    m.points[1].x = robot_x + std::cos(robot_theta) * 0.8;
    m.points[1].y = robot_y + std::sin(robot_theta) * 0.8;
    m.points[1].z = 0.15;
    m.scale.x = 0.08;
    m.color.r = 1.0;
    m.color.g = 1.0;
    m.color.b = 0.0;
    m.color.a = 1.0;
    marker_array.markers.push_back(m);
  }

  // 采样轨迹
  for (size_t i = 0; i < trajectories.size(); ++i) {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = "map";
    m.header.stamp = now;
    m.ns = "samples";
    m.id = static_cast<int>(i);
    m.type = visualization_msgs::msg::Marker::LINE_STRIP;
    m.action = visualization_msgs::msg::Marker::ADD;

    const auto& traj = trajectories[i];
    for (size_t j = 0; j < traj.size(); j += 2) {
      geometry_msgs::msg::Point p;
      p.x = traj[j];
      p.y = traj[j + 1];
      p.z = 0.05;
      m.points.push_back(p);
    }

    if (static_cast<int>(i) == best_idx) {
      m.color.r = 0.0;
      m.color.g = 1.0;
      m.color.b = 0.0;
      m.color.a = 0.8;
      m.scale.x = 0.06;
    } else {
      m.color.r = 0.7;
      m.color.g = 0.7;
      m.color.b = 0.7;
      m.color.a = 0.5;
      m.scale.x = 0.04;
    }
    marker_array.markers.push_back(m);
  }

  vis_pub_->publish(marker_array);
}

}  // namespace nav2_custom_plugins

PLUGINLIB_EXPORT_CLASS(nav2_custom_plugins::MPPIController, nav2_core::Controller)