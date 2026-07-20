// Copyright (c) 2018 Intel Corporation
// Copyright (c) 2018 Simbe Robotics
// Copyright (c) 2019 Samsung Research America
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Navigation Strategy based on:
// Brock, O. and Oussama K. (1999). High-Speed Navigation Using
// the Global Dynamic Window Approach. IEEE.
// https://cs.stanford.edu/group/manips/publications/pdfs/Brock_1999_ICRA.pdf

// #define BENCHMARK_TESTING

#include "nav2_navfn_planner/navfn_planner.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <queue>

#include "builtin_interfaces/msg/duration.hpp"
#include "nav2_navfn_planner/navfn.hpp"
#include "nav2_util/costmap.hpp"
#include "nav2_util/node_utils.hpp"
#include "nav2_costmap_2d/cost_values.hpp"

using namespace std::chrono_literals;
using namespace std::chrono;  // NOLINT
using nav2_util::declare_parameter_if_not_declared;
using rcl_interfaces::msg::ParameterType;
using std::placeholders::_1;

namespace nav2_navfn_planner
{

NavfnPlanner::NavfnPlanner()
: tf_(nullptr), costmap_(nullptr)
{
}

NavfnPlanner::~NavfnPlanner()
{
  RCLCPP_INFO(
    logger_, "Destroying plugin %s of type NavfnPlanner",
    name_.c_str());
}

void
NavfnPlanner::configure(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
  std::string name, std::shared_ptr<tf2_ros::Buffer> tf,
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros)
{
  tf_ = tf;
  name_ = name;
  costmap_ = costmap_ros->getCostmap();
  global_frame_ = costmap_ros->getGlobalFrameID();

  node_ = parent;
  auto node = parent.lock();
  clock_ = node->get_clock();
  logger_ = node->get_logger();

  RCLCPP_INFO(
    logger_, "Configuring plugin %s of type NavfnPlanner",
    name_.c_str());

  // Initialize parameters
  // Declare this plugin's parameters
  declare_parameter_if_not_declared(node, name + ".tolerance", rclcpp::ParameterValue(0.5));
  node->get_parameter(name + ".tolerance", tolerance_);
  declare_parameter_if_not_declared(node, name + ".use_astar", rclcpp::ParameterValue(false));
  node->get_parameter(name + ".use_astar", use_astar_);
  declare_parameter_if_not_declared(node, name + ".allow_unknown", rclcpp::ParameterValue(true));
  node->get_parameter(name + ".allow_unknown", allow_unknown_);
  declare_parameter_if_not_declared(
    node, name + ".use_final_approach_orientation", rclcpp::ParameterValue(false));
  node->get_parameter(name + ".use_final_approach_orientation", use_final_approach_orientation_);

  declare_parameter_if_not_declared(node, name + ".enable_se2_optimization", rclcpp::ParameterValue(true));
  declare_parameter_if_not_declared(node, name + ".se2_resample_resolution", rclcpp::ParameterValue(0.10));
  declare_parameter_if_not_declared(node, name + ".robot_length", rclcpp::ParameterValue(0.40));
  declare_parameter_if_not_declared(node, name + ".robot_width", rclcpp::ParameterValue(0.60));
  declare_parameter_if_not_declared(node, name + ".robot_safe_distance", rclcpp::ParameterValue(0.03));
  declare_parameter_if_not_declared(node, name + ".se2_w_obs", rclcpp::ParameterValue(80.0));
  declare_parameter_if_not_declared(node, name + ".se2_w_ref", rclcpp::ParameterValue(1.0));
  declare_parameter_if_not_declared(node, name + ".se2_w_smooth", rclcpp::ParameterValue(8.0));
  declare_parameter_if_not_declared(node, name + ".se2_w_yaw_smooth", rclcpp::ParameterValue(1.0));
  declare_parameter_if_not_declared(node, name + ".se2_w_align", rclcpp::ParameterValue(0.4));
  declare_parameter_if_not_declared(node, name + ".se2_w_tangent", rclcpp::ParameterValue(6.0));
  declare_parameter_if_not_declared(node, name + ".se2_w_temporal_yaw", rclcpp::ParameterValue(12.0));
  declare_parameter_if_not_declared(
    node, name + ".se2_temporal_match_distance", rclcpp::ParameterValue(0.45));
  declare_parameter_if_not_declared(node, name + ".se2_w_step", rclcpp::ParameterValue(2.0));
  declare_parameter_if_not_declared(node, name + ".se2_max_step", rclcpp::ParameterValue(0.20));
  declare_parameter_if_not_declared(node, name + ".se2_max_yaw_delta", rclcpp::ParameterValue(0.45));
  declare_parameter_if_not_declared(node, name + ".se2_iterations", rclcpp::ParameterValue(35));
  declare_parameter_if_not_declared(node, name + ".se2_learning_rate", rclcpp::ParameterValue(0.04));
  declare_parameter_if_not_declared(node, name + ".se2_fd_eps", rclcpp::ParameterValue(0.02));
  declare_parameter_if_not_declared(
    node, name + ".se2_max_optimization_points", rclcpp::ParameterValue(160));
  declare_parameter_if_not_declared(node, name + ".se2_post_smooth_passes", rclcpp::ParameterValue(2));
  declare_parameter_if_not_declared(node, name + ".se2_post_smooth_alpha", rclcpp::ParameterValue(0.35));

  node->get_parameter(name + ".enable_se2_optimization", enable_se2_optimization_);
  node->get_parameter(name + ".se2_resample_resolution", se2_resample_resolution_);
  node->get_parameter(name + ".robot_length", robot_length_);
  node->get_parameter(name + ".robot_width", robot_width_);
  node->get_parameter(name + ".robot_safe_distance", robot_safe_distance_);
  node->get_parameter(name + ".se2_w_obs", se2_w_obs_);
  node->get_parameter(name + ".se2_w_ref", se2_w_ref_);
  node->get_parameter(name + ".se2_w_smooth", se2_w_smooth_);
  node->get_parameter(name + ".se2_w_yaw_smooth", se2_w_yaw_smooth_);
  node->get_parameter(name + ".se2_w_align", se2_w_align_);
  node->get_parameter(name + ".se2_w_tangent", se2_w_tangent_);
  node->get_parameter(name + ".se2_w_temporal_yaw", se2_w_temporal_yaw_);
  node->get_parameter(name + ".se2_temporal_match_distance", se2_temporal_match_distance_);
  node->get_parameter(name + ".se2_w_step", se2_w_step_);
  node->get_parameter(name + ".se2_max_step", se2_max_step_);
  node->get_parameter(name + ".se2_max_yaw_delta", se2_max_yaw_delta_);
  node->get_parameter(name + ".se2_iterations", se2_iterations_);
  node->get_parameter(name + ".se2_learning_rate", se2_learning_rate_);
  node->get_parameter(name + ".se2_fd_eps", se2_fd_eps_);
  node->get_parameter(name + ".se2_max_optimization_points", se2_max_optimization_points_);
  node->get_parameter(name + ".se2_post_smooth_passes", se2_post_smooth_passes_);
  node->get_parameter(name + ".se2_post_smooth_alpha", se2_post_smooth_alpha_);

  // Create a planner based on the new costmap size
  planner_ = std::make_unique<NavFn>(
    costmap_->getSizeInCellsX(),
    costmap_->getSizeInCellsY());
}

void
NavfnPlanner::activate()
{
  RCLCPP_INFO(
    logger_, "Activating plugin %s of type NavfnPlanner",
    name_.c_str());
  // Add callback for dynamic parameters
  auto node = node_.lock();
  dyn_params_handler_ = node->add_on_set_parameters_callback(
    std::bind(&NavfnPlanner::dynamicParametersCallback, this, _1));
}

void
NavfnPlanner::deactivate()
{
  RCLCPP_INFO(
    logger_, "Deactivating plugin %s of type NavfnPlanner",
    name_.c_str());
  dyn_params_handler_.reset();
}

void
NavfnPlanner::cleanup()
{
  RCLCPP_INFO(
    logger_, "Cleaning up plugin %s of type NavfnPlanner",
    name_.c_str());
  planner_.reset();
  has_last_se2_path_ = false;
  has_last_goal_ = false;
  last_se2_path_.clear();
}

nav_msgs::msg::Path NavfnPlanner::createPlan(
  const geometry_msgs::msg::PoseStamped & start,
  const geometry_msgs::msg::PoseStamped & goal)
{
#ifdef BENCHMARK_TESTING
  steady_clock::time_point a = steady_clock::now();
#endif

  // Update planner based on the new costmap size
  if (isPlannerOutOfDate()) {
    planner_->setNavArr(
      costmap_->getSizeInCellsX(),
      costmap_->getSizeInCellsY());
  }

  nav_msgs::msg::Path path;

  // Corner case of the start(x,y) = goal(x,y)
  if (start.pose.position.x == goal.pose.position.x &&
    start.pose.position.y == goal.pose.position.y)
  {
    unsigned int mx, my;
    costmap_->worldToMap(start.pose.position.x, start.pose.position.y, mx, my);
    if (costmap_->getCost(mx, my) == nav2_costmap_2d::LETHAL_OBSTACLE) {
      RCLCPP_WARN(logger_, "Failed to create a unique pose path because of obstacles");
      return path;
    }
    path.header.stamp = clock_->now();
    path.header.frame_id = global_frame_;
    geometry_msgs::msg::PoseStamped pose;
    pose.header = path.header;
    pose.pose.position.z = 0.0;

    pose.pose = start.pose;
    // if we have a different start and goal orientation, set the unique path pose to the goal
    // orientation, unless use_final_approach_orientation=true where we need it to be the start
    // orientation to avoid movement from the local planner
    if (start.pose.orientation != goal.pose.orientation && !use_final_approach_orientation_) {
      pose.pose.orientation = goal.pose.orientation;
    }
    path.poses.push_back(pose);
    return path;
  }

  if (!makePlan(start.pose, goal.pose, tolerance_, path)) {
    RCLCPP_WARN(
      logger_, "%s: failed to create plan with "
      "tolerance %.2f.", name_.c_str(), tolerance_);
  }

  if (enable_se2_optimization_ && path.poses.size() >= 3) {
    path = optimizeSE2Path(path, start, goal);
  } else if (!path.poses.empty()) {
    setPathOrientationsFromTangent(path);
  }


#ifdef BENCHMARK_TESTING
  steady_clock::time_point b = steady_clock::now();
  duration<double> time_span = duration_cast<duration<double>>(b - a);
  std::cout << "It took " << time_span.count() * 1000 << std::endl;
#endif

  return path;
}

bool
NavfnPlanner::isPlannerOutOfDate()
{
  if (!planner_.get() ||
    planner_->nx != static_cast<int>(costmap_->getSizeInCellsX()) ||
    planner_->ny != static_cast<int>(costmap_->getSizeInCellsY()))
  {
    return true;
  }
  return false;
}

bool
NavfnPlanner::makePlan(
  const geometry_msgs::msg::Pose & start,
  const geometry_msgs::msg::Pose & goal, double tolerance,
  nav_msgs::msg::Path & plan)
{
  // clear the plan, just in case
  plan.poses.clear();

  plan.header.stamp = clock_->now();
  plan.header.frame_id = global_frame_;

  double wx = start.position.x;
  double wy = start.position.y;

  RCLCPP_DEBUG(
    logger_, "Making plan from (%.2f,%.2f) to (%.2f,%.2f)",
    start.position.x, start.position.y, goal.position.x, goal.position.y);

  unsigned int mx, my;
  if (!worldToMap(wx, wy, mx, my)) {
    RCLCPP_WARN(
      logger_,
      "Cannot create a plan: the robot's start position is off the global"
      " costmap. Planning will always fail, are you sure"
      " the robot has been properly localized?");
    return false;
  }

  // clear the starting cell within the costmap because we know it can't be an obstacle
  clearRobotCell(mx, my);

  std::unique_lock<nav2_costmap_2d::Costmap2D::mutex_t> lock(*(costmap_->getMutex()));

  // make sure to resize the underlying array that Navfn uses
  planner_->setNavArr(
    costmap_->getSizeInCellsX(),
    costmap_->getSizeInCellsY());

  planner_->setCostmap(costmap_->getCharMap(), true, allow_unknown_);

  lock.unlock();

  int map_start[2];
  map_start[0] = mx;
  map_start[1] = my;

  wx = goal.position.x;
  wy = goal.position.y;

  if (!worldToMap(wx, wy, mx, my)) {
    RCLCPP_WARN(
      logger_,
      "The goal sent to the planner is off the global costmap."
      " Planning will always fail to this goal.");
    return false;
  }

  int map_goal[2];
  map_goal[0] = mx;
  map_goal[1] = my;

  planner_->setStart(map_goal);
  planner_->setGoal(map_start);
  if (use_astar_) {
    planner_->calcNavFnAstar();
  } else {
    planner_->calcNavFnDijkstra(true);
  }

  double resolution = costmap_->getResolution();
  geometry_msgs::msg::Pose p, best_pose;

  bool found_legal = false;

  p = goal;
  double potential = getPointPotential(p.position);
  if (potential < POT_HIGH) {
    // Goal is reachable by itself
    best_pose = p;
    found_legal = true;
  } else {
    // Goal is not reachable. Trying to find nearest to the goal
    // reachable point within its tolerance region
    double best_sdist = std::numeric_limits<double>::max();

    p.position.y = goal.position.y - tolerance;
    while (p.position.y <= goal.position.y + tolerance) {
      p.position.x = goal.position.x - tolerance;
      while (p.position.x <= goal.position.x + tolerance) {
        potential = getPointPotential(p.position);
        double sdist = squared_distance(p, goal);
        if (potential < POT_HIGH && sdist < best_sdist) {
          best_sdist = sdist;
          best_pose = p;
          found_legal = true;
        }
        p.position.x += resolution;
      }
      p.position.y += resolution;
    }
  }

  if (found_legal) {
    // extract the plan
    if (getPlanFromPotential(best_pose, plan)) {
      smoothApproachToGoal(best_pose, plan);

      // If use_final_approach_orientation=true, interpolate the last pose orientation from the
      // previous pose to set the orientation to the 'final approach' orientation of the robot so
      // it does not rotate.
      // And deal with corner case of plan of length 1
      if (use_final_approach_orientation_) {
        size_t plan_size = plan.poses.size();
        if (plan_size == 1) {
          plan.poses.back().pose.orientation = start.orientation;
        } else if (plan_size > 1) {
          double dx, dy, theta;
          auto last_pose = plan.poses.back().pose.position;
          auto approach_pose = plan.poses[plan_size - 2].pose.position;
          // Deal with the case of NavFn producing a path with two equal last poses
          if (std::abs(last_pose.x - approach_pose.x) < 0.0001 &&
            std::abs(last_pose.y - approach_pose.y) < 0.0001 && plan_size > 2)
          {
            approach_pose = plan.poses[plan_size - 3].pose.position;
          }
          dx = last_pose.x - approach_pose.x;
          dy = last_pose.y - approach_pose.y;
          theta = atan2(dy, dx);
          plan.poses.back().pose.orientation =
            nav2_util::geometry_utils::orientationAroundZAxis(theta);
        }
      }
    } else {
      RCLCPP_ERROR(
        logger_,
        "Failed to create a plan from potential when a legal"
        " potential was found. This shouldn't happen.");
    }
  }

  return !plan.poses.empty();
}

void
NavfnPlanner::smoothApproachToGoal(
  const geometry_msgs::msg::Pose & goal,
  nav_msgs::msg::Path & plan)
{
  // Replace the last pose of the computed path if it's actually further away
  // to the second to last pose than the goal pose.
  if (plan.poses.size() >= 2) {
    auto second_to_last_pose = plan.poses.end()[-2];
    auto last_pose = plan.poses.back();
    if (
      squared_distance(last_pose.pose, second_to_last_pose.pose) >
      squared_distance(goal, second_to_last_pose.pose))
    {
      plan.poses.back().pose = goal;
      return;
    }
  }
  geometry_msgs::msg::PoseStamped goal_copy;
  goal_copy.pose = goal;
  goal_copy.header = plan.header;
  plan.poses.push_back(goal_copy);
}

bool
NavfnPlanner::getPlanFromPotential(
  const geometry_msgs::msg::Pose & goal,
  nav_msgs::msg::Path & plan)
{
  // clear the plan, just in case
  plan.poses.clear();

  // Goal should be in global frame
  double wx = goal.position.x;
  double wy = goal.position.y;

  // the potential has already been computed, so we won't update our copy of the costmap
  unsigned int mx, my;
  if (!worldToMap(wx, wy, mx, my)) {
    RCLCPP_WARN(
      logger_,
      "The goal sent to the navfn planner is off the global costmap."
      " Planning will always fail to this goal.");
    return false;
  }

  int map_goal[2];
  map_goal[0] = mx;
  map_goal[1] = my;

  planner_->setStart(map_goal);

  const int & max_cycles = (costmap_->getSizeInCellsX() >= costmap_->getSizeInCellsY()) ?
    (costmap_->getSizeInCellsX() * 4) : (costmap_->getSizeInCellsY() * 4);

  int path_len = planner_->calcPath(max_cycles);
  if (path_len == 0) {
    return false;
  }

  auto cost = planner_->getLastPathCost();
  RCLCPP_DEBUG(
    logger_,
    "Path found, %d steps, %f cost\n", path_len, cost);

  // extract the plan
  float * x = planner_->getPathX();
  float * y = planner_->getPathY();
  int len = planner_->getPathLen();

  for (int i = len - 1; i >= 0; --i) {
    // convert the plan to world coordinates
    double world_x, world_y;
    mapToWorld(x[i], y[i], world_x, world_y);

    geometry_msgs::msg::PoseStamped pose;
    pose.header = plan.header;
    pose.pose.position.x = world_x;
    pose.pose.position.y = world_y;
    pose.pose.position.z = 0.0;
    pose.pose.orientation.x = 0.0;
    pose.pose.orientation.y = 0.0;
    pose.pose.orientation.z = 0.0;
    pose.pose.orientation.w = 1.0;
    plan.poses.push_back(pose);
  }

  return !plan.poses.empty();
}

double
NavfnPlanner::getPointPotential(const geometry_msgs::msg::Point & world_point)
{
  unsigned int mx, my;
  if (!worldToMap(world_point.x, world_point.y, mx, my)) {
    return std::numeric_limits<double>::max();
  }

  unsigned int index = my * planner_->nx + mx;
  return planner_->potarr[index];
}

void
NavfnPlanner::setPathOrientationsFromTangent(nav_msgs::msg::Path & path) const
{
  if (path.poses.size() < 2) {
    return;
  }

  for (size_t i = 0; i < path.poses.size(); ++i) {
    size_t j0 = i;
    size_t j1 = std::min(i + 1, path.poses.size() - 1);
    if (i == path.poses.size() - 1) {
      j0 = i - 1;
      j1 = i;
    }

    const auto & p0 = path.poses[j0].pose.position;
    const auto & p1 = path.poses[j1].pose.position;
    const double yaw = std::atan2(p1.y - p0.y, p1.x - p0.x);
    path.poses[i].pose.orientation = nav2_util::geometry_utils::orientationAroundZAxis(yaw);
  }
}

double
NavfnPlanner::wrapAngle(double a) const
{
  return std::atan2(std::sin(a), std::cos(a));
}

double
NavfnPlanner::angleDiff(double a, double b) const
{
  return wrapAngle(a - b);
}

bool
NavfnPlanner::getTemporalYawReference(const SE2Pose & pose, double & yaw) const
{
  if (!has_last_se2_path_ || last_se2_path_.empty() || se2_w_temporal_yaw_ <= 0.0) {
    return false;
  }

  const double max_match_dist = std::max(0.05, se2_temporal_match_distance_);
  const double max_match_dist_sq = max_match_dist * max_match_dist;
  double best_dist_sq = max_match_dist_sq;
  bool found = false;

  for (const auto & previous : last_se2_path_) {
    const double dx = pose.x - previous.x;
    const double dy = pose.y - previous.y;
    const double dist_sq = dx * dx + dy * dy;
    if (dist_sq <= best_dist_sq) {
      best_dist_sq = dist_sq;
      yaw = previous.yaw;
      found = true;
    }
  }

  return found;
}

std::vector<NavfnPlanner::SE2Pose>
NavfnPlanner::resamplePathForOptimization(
  const nav_msgs::msg::Path & path,
  double spacing)
{
  std::vector<SE2Pose> out;
  if (path.poses.empty()) {
    return out;
  }
  if (path.poses.size() == 1) {
    out.push_back({path.poses.front().pose.position.x, path.poses.front().pose.position.y, 0.0});
    return out;
  }

  spacing = std::max(0.03, spacing);

  std::vector<double> acc(path.poses.size(), 0.0);
  for (size_t i = 1; i < path.poses.size(); ++i) {
    const auto & a = path.poses[i - 1].pose.position;
    const auto & b = path.poses[i].pose.position;
    acc[i] = acc[i - 1] + std::hypot(b.x - a.x, b.y - a.y);
  }

  const double total = acc.back();
  if (total < 1e-6) {
    out.push_back({path.poses.front().pose.position.x, path.poses.front().pose.position.y, 0.0});
    return out;
  }

  const int n = std::max(2, static_cast<int>(std::ceil(total / spacing)) + 1);
  out.reserve(n);

  size_t seg = 1;
  for (int k = 0; k < n; ++k) {
    const double s = std::min(total, k * total / static_cast<double>(n - 1));
    while (seg < acc.size() - 1 && acc[seg] < s) {
      ++seg;
    }

    const double s0 = acc[seg - 1];
    const double s1 = acc[seg];
    const double t = (s1 - s0 > 1e-9) ? (s - s0) / (s1 - s0) : 0.0;
    const auto & p0 = path.poses[seg - 1].pose.position;
    const auto & p1 = path.poses[seg].pose.position;

    SE2Pose p;
    p.x = p0.x + t * (p1.x - p0.x);
    p.y = p0.y + t * (p1.y - p0.y);
    p.yaw = std::atan2(p1.y - p0.y, p1.x - p0.x);
    out.push_back(p);
  }

  for (size_t i = 0; i < out.size(); ++i) {
    if (i + 1 < out.size()) {
      out[i].yaw = std::atan2(out[i + 1].y - out[i].y, out[i + 1].x - out[i].x);
    } else if (i > 0) {
      out[i].yaw = out[i - 1].yaw;
    }
  }

  return out;
}

NavfnPlanner::ESDFGrid
NavfnPlanner::buildESDFGrid()
{
  ESDFGrid esdf;
  esdf.size_x = costmap_->getSizeInCellsX();
  esdf.size_y = costmap_->getSizeInCellsY();
  esdf.resolution = costmap_->getResolution();
  esdf.origin_x = costmap_->getOriginX();
  esdf.origin_y = costmap_->getOriginY();
  esdf.dist.assign(esdf.size_x * esdf.size_y, std::numeric_limits<double>::infinity());

  using Node = std::pair<double, unsigned int>;
  std::priority_queue<Node, std::vector<Node>, std::greater<Node>> q;

  std::unique_lock<nav2_costmap_2d::Costmap2D::mutex_t> lock(*(costmap_->getMutex()));
  for (unsigned int y = 0; y < esdf.size_y; ++y) {
    for (unsigned int x = 0; x < esdf.size_x; ++x) {
      const unsigned int idx = y * esdf.size_x + x;
      const unsigned char c = costmap_->getCost(x, y);
      if (c >= nav2_costmap_2d::LETHAL_OBSTACLE) {
        esdf.dist[idx] = 0.0;
        q.emplace(0.0, idx);
      }
    }
  }
  lock.unlock();

  const int dx[8] = {1, -1, 0, 0, 1, 1, -1, -1};
  const int dy[8] = {0, 0, 1, -1, 1, -1, 1, -1};

  while (!q.empty()) {
    const auto [d, idx] = q.top();
    q.pop();
    if (d > esdf.dist[idx]) {
      continue;
    }

    const unsigned int x = idx % esdf.size_x;
    const unsigned int y = idx / esdf.size_x;

    for (int k = 0; k < 8; ++k) {
      const int nx = static_cast<int>(x) + dx[k];
      const int ny = static_cast<int>(y) + dy[k];
      if (nx < 0 || ny < 0 || nx >= static_cast<int>(esdf.size_x) || ny >= static_cast<int>(esdf.size_y)) {
        continue;
      }
      const unsigned int nidx = static_cast<unsigned int>(ny) * esdf.size_x + static_cast<unsigned int>(nx);
      const double step = ((dx[k] == 0 || dy[k] == 0) ? 1.0 : std::sqrt(2.0)) * esdf.resolution;
      const double nd = d + step;
      if (nd < esdf.dist[nidx]) {
        esdf.dist[nidx] = nd;
        q.emplace(nd, nidx);
      }
    }
  }

  return esdf;
}

double
NavfnPlanner::queryESDF(const ESDFGrid & esdf, double wx, double wy) const
{
  const double mx = (wx - esdf.origin_x) / esdf.resolution;
  const double my = (wy - esdf.origin_y) / esdf.resolution;

  if (mx < 0.0 || my < 0.0 || mx >= static_cast<double>(esdf.size_x - 1) ||
    my >= static_cast<double>(esdf.size_y - 1))
  {
    return 0.0;
  }

  const unsigned int x0 = static_cast<unsigned int>(std::floor(mx));
  const unsigned int y0 = static_cast<unsigned int>(std::floor(my));
  const unsigned int x1 = x0 + 1;
  const unsigned int y1 = y0 + 1;
  const double tx = mx - x0;
  const double ty = my - y0;

  const auto at = [&](unsigned int x, unsigned int y) -> double {
    return esdf.dist[y * esdf.size_x + x];
  };

  const double d00 = at(x0, y0);
  const double d10 = at(x1, y0);
  const double d01 = at(x0, y1);
  const double d11 = at(x1, y1);

  const double d0 = d00 * (1.0 - tx) + d10 * tx;
  const double d1 = d01 * (1.0 - tx) + d11 * tx;
  return d0 * (1.0 - ty) + d1 * ty;
}

double
NavfnPlanner::computeFootprintObstacleCost(
  const SE2Pose & pose,
  const ESDFGrid & esdf) const
{
  const double hx = 0.5 * robot_length_;
  const double hy = 0.5 * robot_width_;
  const double clearance_influence =
    robot_safe_distance_ + std::max(0.15, 3.0 * esdf.resolution);
  constexpr int samples_x = 5;
  constexpr int samples_y = 5;

  double cost = 0.0;
  const double c = std::cos(pose.yaw);
  const double s = std::sin(pose.yaw);

  for (int ix = 0; ix < samples_x; ++ix) {
    const double bx = -hx + 2.0 * hx * static_cast<double>(ix) /
      static_cast<double>(samples_x - 1);
    for (int iy = 0; iy < samples_y; ++iy) {
      const bool boundary =
        ix == 0 || ix == samples_x - 1 || iy == 0 || iy == samples_y - 1;
      const bool center = ix == samples_x / 2 && iy == samples_y / 2;
      if (!boundary && !center) {
        continue;
      }

      const double by = -hy + 2.0 * hy * static_cast<double>(iy) /
        static_cast<double>(samples_y - 1);
      const double wx = pose.x + c * bx - s * by;
      const double wy = pose.y + s * bx + c * by;
      const double d = queryESDF(esdf, wx, wy);
      const double hard = std::max(0.0, robot_safe_distance_ - d);
      const double soft = std::max(0.0, clearance_influence - d);
      cost += se2_w_obs_ * (4.0 * hard * hard + 0.08 * soft * soft);
    }
  }

  return cost;
}

void
NavfnPlanner::assignContinuousYaw(
  std::vector<SE2Pose> & poses,
  const ESDFGrid & esdf,
  const geometry_msgs::msg::PoseStamped & start,
  const geometry_msgs::msg::PoseStamped & goal) const
{
  if (poses.size() < 2) {
    return;
  }

  constexpr double pi = 3.14159265358979323846;
  constexpr double half_pi = 1.57079632679489661923;

  auto yawFromQuaternion = [](const geometry_msgs::msg::Quaternion & q) {
      return std::atan2(
        2.0 * (q.w * q.z + q.x * q.y),
        1.0 - 2.0 * (q.y * q.y + q.z * q.z));
    };

  const auto tangentYaw = [&](size_t i) {
      size_t i0 = i;
      size_t i1 = std::min(i + 1, poses.size() - 1);
      if (i == poses.size() - 1) {
        i0 = i - 1;
        i1 = i;
      } else if (i > 0) {
        i0 = i - 1;
        i1 = i + 1;
      }
      return std::atan2(poses[i1].y - poses[i0].y, poses[i1].x - poses[i0].x);
    };

  std::vector<std::vector<double>> candidates(poses.size());
  for (size_t i = 0; i < poses.size(); ++i) {
    auto addCandidate = [&](double yaw) {
        yaw = wrapAngle(yaw);
        for (const auto & existing : candidates[i]) {
          if (std::abs(angleDiff(yaw, existing)) < 1e-3) {
            return;
          }
        }
        candidates[i].push_back(yaw);
      };

    if (i == 0) {
      addCandidate(yawFromQuaternion(start.pose.orientation));
      continue;
    }
    if (i + 1 == poses.size()) {
      addCandidate(yawFromQuaternion(goal.pose.orientation));
      continue;
    }

    const double phi = tangentYaw(i);
    addCandidate(phi);
    addCandidate(phi + half_pi);
    addCandidate(phi - half_pi);
    addCandidate(phi + pi);
    addCandidate(poses[i].yaw);
    addCandidate(poses[i].yaw + pi);
  }

  std::vector<std::vector<double>> dp(poses.size());
  std::vector<std::vector<int>> parent(poses.size());
  for (size_t i = 0; i < poses.size(); ++i) {
    dp[i].assign(candidates[i].size(), std::numeric_limits<double>::infinity());
    parent[i].assign(candidates[i].size(), -1);
  }

  const double continuity_weight = std::max(4.0, 2.0 * se2_w_yaw_smooth_);
  const double flip_penalty = std::max(20.0, 8.0 * se2_w_yaw_smooth_);

  for (size_t i = 0; i < poses.size(); ++i) {
    const double phi = tangentYaw(i);
    for (size_t c = 0; c < candidates[i].size(); ++c) {
      SE2Pose candidate_pose = poses[i];
      candidate_pose.yaw = candidates[i][c];
      const double yaw_error = angleDiff(candidate_pose.yaw, phi);
      const double tangent_preference =
        se2_w_tangent_ * (1.0 - std::cos(yaw_error));
      double temporal_preference = 0.0;
      double temporal_yaw = 0.0;
      if (getTemporalYawReference(candidate_pose, temporal_yaw)) {
        const double temporal_error = angleDiff(candidate_pose.yaw, temporal_yaw);
        temporal_preference = se2_w_temporal_yaw_ * temporal_error * temporal_error;
      }
      const double state_cost =
        computeFootprintObstacleCost(candidate_pose, esdf) + tangent_preference +
        temporal_preference +
        0.25 * se2_w_align_ * yaw_error * yaw_error;

      if (i == 0) {
        dp[i][c] = 0.0;
        continue;
      }

      for (size_t pc = 0; pc < candidates[i - 1].size(); ++pc) {
        const double dyaw = std::abs(angleDiff(candidate_pose.yaw, candidates[i - 1][pc]));
        double transition_cost = continuity_weight * dyaw * dyaw;
        if (dyaw > half_pi) {
          transition_cost += flip_penalty;
        }
        const double total = dp[i - 1][pc] + state_cost + transition_cost;
        if (total < dp[i][c]) {
          dp[i][c] = total;
          parent[i][c] = static_cast<int>(pc);
        }
      }
    }
  }

  size_t best = 0;
  for (size_t c = 1; c < dp.back().size(); ++c) {
    if (dp.back()[c] < dp.back()[best]) {
      best = c;
    }
  }

  for (int i = static_cast<int>(poses.size()) - 1; i >= 0; --i) {
    poses[static_cast<size_t>(i)].yaw = candidates[static_cast<size_t>(i)][best];
    const int previous = parent[static_cast<size_t>(i)][best];
    if (previous < 0) {
      break;
    }
    best = static_cast<size_t>(previous);
  }
}

double
NavfnPlanner::computeTrajectoryCost(
  const std::vector<SE2Pose> & poses,
  const std::vector<SE2Pose> & ref,
  const ESDFGrid & esdf) const
{
  if (poses.empty()) {
    return 0.0;
  }

  double cost = 0.0;

  for (size_t i = 0; i < poses.size(); ++i) {
    const auto & p = poses[i];
    cost += computeFootprintObstacleCost(p, esdf);

    const double ex = p.x - ref[i].x;
    const double ey = p.y - ref[i].y;
    cost += se2_w_ref_ * (ex * ex + ey * ey);

    double phi = ref[i].yaw;
    if (i + 1 < poses.size()) {
      phi = std::atan2(poses[i + 1].y - poses[i].y, poses[i + 1].x - poses[i].x);
    } else if (i > 0) {
      phi = std::atan2(poses[i].y - poses[i - 1].y, poses[i].x - poses[i - 1].x);
    }
    const double ay = angleDiff(p.yaw, phi);
    cost += se2_w_align_ * ay * ay;
    cost += se2_w_tangent_ * (1.0 - std::cos(ay));

    double temporal_yaw = 0.0;
    if (getTemporalYawReference(p, temporal_yaw)) {
      const double temporal_error = angleDiff(p.yaw, temporal_yaw);
      cost += se2_w_temporal_yaw_ * temporal_error * temporal_error;
    }
  }

  for (size_t i = 1; i + 1 < poses.size(); ++i) {
    const double ax = poses[i + 1].x - 2.0 * poses[i].x + poses[i - 1].x;
    const double ay = poses[i + 1].y - 2.0 * poses[i].y + poses[i - 1].y;
    const double ath = wrapAngle(poses[i + 1].yaw - 2.0 * poses[i].yaw + poses[i - 1].yaw);
    cost += se2_w_smooth_ * (ax * ax + ay * ay);
    cost += se2_w_yaw_smooth_ * ath * ath;
  }

  for (size_t i = 1; i < poses.size(); ++i) {
    const double ds = std::hypot(poses[i].x - poses[i - 1].x, poses[i].y - poses[i - 1].y);
    const double dth = std::abs(angleDiff(poses[i].yaw, poses[i - 1].yaw));
    const double rs = std::max(0.0, ds - se2_max_step_);
    const double rt = std::max(0.0, dth - se2_max_yaw_delta_);
    cost += 0.25 * se2_w_yaw_smooth_ * dth * dth;
    cost += se2_w_step_ * (rs * rs + rt * rt);
  }

  return cost;
}


bool
NavfnPlanner::validateOptimizedPath(
  const std::vector<SE2Pose> & poses,
  const ESDFGrid & esdf) const
{
  const double hx = 0.5 * robot_length_;
  const double hy = 0.5 * robot_width_;
  const std::array<std::array<double, 2>, 8> footprint = {{
    {{ hx,  hy}}, {{ hx, 0.0}}, {{ hx, -hy}},
    {{0.0,  hy}}, {{0.0, -hy}},
    {{-hx,  hy}}, {{-hx, 0.0}}, {{-hx, -hy}}
  }};

  for (const auto & p : poses) {
    const double c = std::cos(p.yaw);
    const double s = std::sin(p.yaw);
    for (const auto & b : footprint) {
      const double wx = p.x + c * b[0] - s * b[1];
      const double wy = p.y + s * b[0] + c * b[1];
      if (queryESDF(esdf, wx, wy) < std::max(0.0, robot_safe_distance_ * 0.5)) {
        return false;
      }
    }
  }
  return true;
}

nav_msgs::msg::Path
NavfnPlanner::optimizeSE2Path(
  const nav_msgs::msg::Path & raw_path,
  const geometry_msgs::msg::PoseStamped & start,
  const geometry_msgs::msg::PoseStamped & goal)
{
  auto poses = resamplePathForOptimization(raw_path, se2_resample_resolution_);
  if (poses.size() < 3) {
    nav_msgs::msg::Path fallback = raw_path;
    setPathOrientationsFromTangent(fallback);
    has_last_se2_path_ = false;
    last_se2_path_.clear();
    return fallback;
  }

  const size_t max_points =
    static_cast<size_t>(std::max(3, se2_max_optimization_points_));
  if (poses.size() > max_points) {
    std::vector<SE2Pose> reduced;
    reduced.reserve(max_points);
    for (size_t k = 0; k < max_points; ++k) {
      const double t = static_cast<double>(k) / static_cast<double>(max_points - 1);
      const size_t idx = static_cast<size_t>(
        std::round(t * static_cast<double>(poses.size() - 1)));
      if (reduced.empty() ||
        reduced.back().x != poses[idx].x ||
        reduced.back().y != poses[idx].y)
      {
        reduced.push_back(poses[idx]);
      }
    }
    if (reduced.size() >= 3) {
      poses = reduced;
    }
  }

  std::vector<SE2Pose> ref = poses;
  poses.front().x = start.pose.position.x;
  poses.front().y = start.pose.position.y;
  poses.back().x = goal.pose.position.x;
  poses.back().y = goal.pose.position.y;

  ESDFGrid esdf = buildESDFGrid();

  auto yawFromQuaternion = [](const geometry_msgs::msg::Quaternion & q) {
      return std::atan2(
        2.0 * (q.w * q.z + q.x * q.y),
        1.0 - 2.0 * (q.y * q.y + q.z * q.z));
    };
  auto pinEndpoints = [&](std::vector<SE2Pose> & candidate) {
      candidate.front().x = start.pose.position.x;
      candidate.front().y = start.pose.position.y;
      candidate.front().yaw = yawFromQuaternion(start.pose.orientation);
      candidate.back().x = goal.pose.position.x;
      candidate.back().y = goal.pose.position.y;
      candidate.back().yaw = yawFromQuaternion(goal.pose.orientation);
    };

  if (has_last_goal_) {
    const double goal_dx = goal.pose.position.x - last_goal_.pose.position.x;
    const double goal_dy = goal.pose.position.y - last_goal_.pose.position.y;
    const double reset_dist = std::max(0.75, 2.0 * se2_temporal_match_distance_);
    if (std::hypot(goal_dx, goal_dy) > reset_dist) {
      has_last_se2_path_ = false;
      last_se2_path_.clear();
    }
  }

  pinEndpoints(poses);
  assignContinuousYaw(poses, esdf, start, goal);

  double best_cost = computeTrajectoryCost(poses, ref, esdf);

  const double eps_xy = std::max(0.005, se2_fd_eps_);
  auto computePositionGradient = [&](size_t i) -> SE2Pose {
      SE2Pose grad{0.0, 0.0, 0.0};

      SE2Pose tmp = poses[i];
      tmp.x += eps_xy;
      const double cxp = computeFootprintObstacleCost(tmp, esdf);
      tmp.x = poses[i].x - eps_xy;
      const double cxm = computeFootprintObstacleCost(tmp, esdf);
      tmp.x = poses[i].x;

      tmp.y += eps_xy;
      const double cyp = computeFootprintObstacleCost(tmp, esdf);
      tmp.y = poses[i].y - eps_xy;
      const double cym = computeFootprintObstacleCost(tmp, esdf);
      tmp.y = poses[i].y;

      grad.x += (cxp - cxm) / (2.0 * eps_xy);
      grad.y += (cyp - cym) / (2.0 * eps_xy);

      grad.x += 2.0 * se2_w_ref_ * (poses[i].x - ref[i].x);
      grad.y += 2.0 * se2_w_ref_ * (poses[i].y - ref[i].y);

      const auto addSmoothGradient = [&](size_t j, double coeff) {
          const double ax = poses[j + 1].x - 2.0 * poses[j].x + poses[j - 1].x;
          const double ay = poses[j + 1].y - 2.0 * poses[j].y + poses[j - 1].y;
          grad.x += 2.0 * se2_w_smooth_ * coeff * ax;
          grad.y += 2.0 * se2_w_smooth_ * coeff * ay;
        };

      if (i >= 2) {
        addSmoothGradient(i - 1, 1.0);
      }
      if (i >= 1 && i + 1 < poses.size()) {
        addSmoothGradient(i, -2.0);
      }
      if (i + 2 < poses.size()) {
        addSmoothGradient(i + 1, 1.0);
      }

      const auto addStepGradient = [&](size_t i0, size_t i1, bool point_is_end) {
          const double dx = poses[i1].x - poses[i0].x;
          const double dy = poses[i1].y - poses[i0].y;
          const double ds = std::hypot(dx, dy);
          const double rs = std::max(0.0, ds - se2_max_step_);
          if (rs <= 0.0 || ds < 1e-6) {
            return;
          }

          const double coeff = 2.0 * se2_w_step_ * rs / ds;
          const double sign = point_is_end ? 1.0 : -1.0;
          grad.x += sign * coeff * dx;
          grad.y += sign * coeff * dy;
        };

      if (i > 0) {
        addStepGradient(i - 1, i, true);
      }
      if (i + 1 < poses.size()) {
        addStepGradient(i, i + 1, false);
      }

      return grad;
    };

  for (int iter = 0; iter < se2_iterations_; ++iter) {
    std::vector<SE2Pose> grads(poses.size(), SE2Pose{0.0, 0.0, 0.0});
    const double alpha = se2_learning_rate_ / (1.0 + 0.05 * static_cast<double>(iter));

    for (size_t i = 1; i + 1 < poses.size(); ++i) {
      grads[i] = computePositionGradient(i);
    }

    bool accepted = false;
    double trial_alpha = alpha;
    for (int line_search = 0; line_search < 8; ++line_search) {
      std::vector<SE2Pose> next = poses;
      const double max_xy_update = std::min(0.05, 0.5 * se2_resample_resolution_);

      for (size_t i = 1; i + 1 < poses.size(); ++i) {
        const double tx = std::cos(ref[i].yaw);
        const double ty = std::sin(ref[i].yaw);
        const double nx = -ty;
        const double ny = tx;
        const double tangent_grad = grads[i].x * tx + grads[i].y * ty;
        const double normal_grad = grads[i].x * nx + grads[i].y * ny;
        const double projected_grad_x = normal_grad * nx + 0.15 * tangent_grad * tx;
        const double projected_grad_y = normal_grad * ny + 0.15 * tangent_grad * ty;

        const double ux =
          std::clamp(-trial_alpha * projected_grad_x, -max_xy_update, max_xy_update);
        const double uy =
          std::clamp(-trial_alpha * projected_grad_y, -max_xy_update, max_xy_update);

        next[i].x += ux;
        next[i].y += uy;
      }

      pinEndpoints(next);
      assignContinuousYaw(next, esdf, start, goal);
      const double next_cost = computeTrajectoryCost(next, ref, esdf);
      if (std::isfinite(next_cost) && next_cost <= best_cost) {
        poses = next;
        best_cost = next_cost;
        accepted = true;
        break;
      }
      trial_alpha *= 0.5;
    }

    if (!accepted) {
      RCLCPP_DEBUG(logger_, "SE2 optimization stopped at iteration %d: no decreasing step", iter);
      break;
    }
  }

  assignContinuousYaw(poses, esdf, start, goal);

  const int smooth_passes = std::max(0, se2_post_smooth_passes_);
  const double smooth_alpha = std::clamp(se2_post_smooth_alpha_, 0.0, 1.0);
  for (int pass = 0; pass < smooth_passes; ++pass) {
    bool accepted = false;
    double trial_alpha = smooth_alpha;
    for (int attempt = 0; attempt < 5; ++attempt) {
      std::vector<SE2Pose> smoothed = poses;
      for (size_t i = 1; i + 1 < poses.size(); ++i) {
        smoothed[i].x =
          (1.0 - trial_alpha) * poses[i].x +
          0.5 * trial_alpha * (poses[i - 1].x + poses[i + 1].x);
        smoothed[i].y =
          (1.0 - trial_alpha) * poses[i].y +
          0.5 * trial_alpha * (poses[i - 1].y + poses[i + 1].y);
      }

      pinEndpoints(smoothed);
      assignContinuousYaw(smoothed, esdf, start, goal);
      const double smooth_cost = computeTrajectoryCost(smoothed, ref, esdf);
      if (std::isfinite(smooth_cost) && validateOptimizedPath(smoothed, esdf) &&
        smooth_cost <= best_cost)
      {
        poses = smoothed;
        best_cost = smooth_cost;
        accepted = true;
        break;
      }

      trial_alpha *= 0.5;
    }

    if (!accepted) {
      break;
    }
  }

  auto posesToPath = [&](const std::vector<SE2Pose> & path_poses) {
      nav_msgs::msg::Path output;
      output.header = raw_path.header;
      output.poses.reserve(path_poses.size());

      for (const auto & p : path_poses) {
        geometry_msgs::msg::PoseStamped ps;
        ps.header = output.header;
        ps.pose.position.x = p.x;
        ps.pose.position.y = p.y;
        ps.pose.position.z = 0.0;
        ps.pose.orientation = nav2_util::geometry_utils::orientationAroundZAxis(p.yaw);
        output.poses.push_back(ps);
      }

      return output;
    };
  auto finalizePath = [&](const std::vector<SE2Pose> & path_poses) {
      last_se2_path_ = path_poses;
      has_last_se2_path_ = true;
      last_goal_ = goal;
      has_last_goal_ = true;
      return posesToPath(path_poses);
    };

  const bool valid = validateOptimizedPath(poses, esdf);
  if (!valid) {
    std::vector<SE2Pose> fallback_poses = ref;
    pinEndpoints(fallback_poses);
    assignContinuousYaw(fallback_poses, esdf, start, goal);

    if (validateOptimizedPath(fallback_poses, esdf)) {
      RCLCPP_WARN(
        logger_,
        "SE2 optimized path failed footprint validation; returning footprint-oriented NavFn path");
      return finalizePath(fallback_poses);
    }

    RCLCPP_WARN(
      logger_,
      "SE2 optimized path and footprint-oriented NavFn fallback failed footprint validation; "
      "returning best-effort SE2 path");
    return finalizePath(poses);
  }

  return finalizePath(poses);
}

bool
NavfnPlanner::worldToMap(double wx, double wy, unsigned int & mx, unsigned int & my)
{
  if (wx < costmap_->getOriginX() || wy < costmap_->getOriginY()) {
    return false;
  }

  mx = static_cast<int>(
    std::round((wx - costmap_->getOriginX()) / costmap_->getResolution()));
  my = static_cast<int>(
    std::round((wy - costmap_->getOriginY()) / costmap_->getResolution()));

  if (mx < costmap_->getSizeInCellsX() && my < costmap_->getSizeInCellsY()) {
    return true;
  }

  RCLCPP_ERROR(
    logger_,
    "worldToMap failed: mx,my: %d,%d, size_x,size_y: %d,%d", mx, my,
    costmap_->getSizeInCellsX(), costmap_->getSizeInCellsY());

  return false;
}

void
NavfnPlanner::mapToWorld(double mx, double my, double & wx, double & wy)
{
  wx = costmap_->getOriginX() + mx * costmap_->getResolution();
  wy = costmap_->getOriginY() + my * costmap_->getResolution();
}

void
NavfnPlanner::clearRobotCell(unsigned int mx, unsigned int my)
{
  // TODO(orduno): check usage of this function, might instead be a request to
  //               world_model / map server
  costmap_->setCost(mx, my, nav2_costmap_2d::FREE_SPACE);
}

rcl_interfaces::msg::SetParametersResult
NavfnPlanner::dynamicParametersCallback(std::vector<rclcpp::Parameter> parameters)
{
  rcl_interfaces::msg::SetParametersResult result;
  for (auto parameter : parameters) {
    const auto & type = parameter.get_type();
    const auto & name = parameter.get_name();

    if (type == ParameterType::PARAMETER_DOUBLE) {
      if (name == name_ + ".tolerance") {
        tolerance_ = parameter.as_double();
      } else if (name == name_ + ".se2_resample_resolution") {
        se2_resample_resolution_ = parameter.as_double();
      } else if (name == name_ + ".robot_length") {
        robot_length_ = parameter.as_double();
      } else if (name == name_ + ".robot_width") {
        robot_width_ = parameter.as_double();
      } else if (name == name_ + ".robot_safe_distance") {
        robot_safe_distance_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_obs") {
        se2_w_obs_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_ref") {
        se2_w_ref_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_smooth") {
        se2_w_smooth_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_yaw_smooth") {
        se2_w_yaw_smooth_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_align") {
        se2_w_align_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_tangent") {
        se2_w_tangent_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_temporal_yaw") {
        se2_w_temporal_yaw_ = parameter.as_double();
      } else if (name == name_ + ".se2_temporal_match_distance") {
        se2_temporal_match_distance_ = parameter.as_double();
      } else if (name == name_ + ".se2_w_step") {
        se2_w_step_ = parameter.as_double();
      } else if (name == name_ + ".se2_max_step") {
        se2_max_step_ = parameter.as_double();
      } else if (name == name_ + ".se2_max_yaw_delta") {
        se2_max_yaw_delta_ = parameter.as_double();
      } else if (name == name_ + ".se2_learning_rate") {
        se2_learning_rate_ = parameter.as_double();
      } else if (name == name_ + ".se2_fd_eps") {
        se2_fd_eps_ = parameter.as_double();
      } else if (name == name_ + ".se2_post_smooth_alpha") {
        se2_post_smooth_alpha_ = parameter.as_double();
      }
    } else if (type == ParameterType::PARAMETER_BOOL) {
      if (name == name_ + ".use_astar") {
        use_astar_ = parameter.as_bool();
      } else if (name == name_ + ".allow_unknown") {
        allow_unknown_ = parameter.as_bool();
      } else if (name == name_ + ".use_final_approach_orientation") {
        use_final_approach_orientation_ = parameter.as_bool();
      } else if (name == name_ + ".enable_se2_optimization") {
        enable_se2_optimization_ = parameter.as_bool();
      }
    } else if (type == ParameterType::PARAMETER_INTEGER) {
      if (name == name_ + ".se2_iterations") {
        se2_iterations_ = static_cast<int>(parameter.as_int());
      } else if (name == name_ + ".se2_max_optimization_points") {
        se2_max_optimization_points_ = static_cast<int>(parameter.as_int());
      } else if (name == name_ + ".se2_post_smooth_passes") {
        se2_post_smooth_passes_ = static_cast<int>(parameter.as_int());
      }
    }
  }
  result.successful = true;
  return result;
}

}  // namespace nav2_navfn_planner

#include "pluginlib/class_list_macros.hpp"
PLUGINLIB_EXPORT_CLASS(nav2_navfn_planner::NavfnPlanner, nav2_core::GlobalPlanner)
