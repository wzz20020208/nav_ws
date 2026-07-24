/**
 * @file hybrid_a_star.cpp
 * @brief HybridAStar — 连续状态空间 Hybrid-A* 搜索算法实现
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 算法流程
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ① computeObstacleHeuristic: 2D Dijkstra 从 goal 反向传播
 *   ② generateMotionPrimitives: 运动原语生成
 *   ③ A* 主循环:
 *        ├── 弹出最小 f_cost 节点
 *        ├── visited 去重检查
 *        ├── 到达判定 (距离 + 角度)
 *        ├── 分析扩张 (每 N 次): RS → 碰撞检测 → 成功返回
 *        └── 运动原语展开 → push open_set
 *   ④ 路径重建 + 简化 + 朝向赋值
 */

#include "nav2_hybrid_a_star_planner/hybrid_a_star.hpp"

#include <queue>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstring>

#include "nav2_costmap_2d/costmap_2d.hpp"

namespace nav2_hybrid_a_star_planner
{

// ═══════════════════════════════════════════════════════════════════════════════
// 构造 / 析构
// ═══════════════════════════════════════════════════════════════════════════════

HybridAStar::HybridAStar() = default;
HybridAStar::~HybridAStar() = default;

// ═══════════════════════════════════════════════════════════════════════════════
// 初始化
// ═══════════════════════════════════════════════════════════════════════════════

void HybridAStar::initialize(
    const PlannerParams & params,
    unsigned int size_x, unsigned int size_y,
    double resolution, double origin_x, double origin_y)
{
  params_ = params;
  map_size_x_ = size_x;
  map_size_y_ = size_y;
  map_resolution_ = resolution;
  map_origin_x_ = origin_x;
  map_origin_y_ = origin_y;

  // 分配障碍物启发式数组
  obstacle_heuristic_.resize(map_size_x_ * map_size_y_);

  // 分配 visited 数组: 2D cell + heading bin
  visited_.resize(map_size_x_ * map_size_y_ * params_.heading_bins);

  // 生成运动原语
  generateMotionPrimitives();
}

// ═══════════════════════════════════════════════════════════════════════════════
// 主入口
// ═══════════════════════════════════════════════════════════════════════════════

bool HybridAStar::createPlan(
    const SE2State & start, const SE2State & goal,
    const nav2_costmap_2d::Costmap2D * costmap,
    std::vector<SE2State> & plan)
{
  plan.clear();
  closed_list_.clear();
  std::fill(visited_.begin(), visited_.end(), 0);

  GridCoord start_cell = worldToGrid(start.x, start.y);
  GridCoord goal_cell = worldToGrid(goal.x, goal.y);

  // ── ① 障碍物距离启发式 ──
  computeObstacleHeuristic(goal_cell, costmap);

  // 起点被障碍物包围 → 直接失败
  if (getObstacleHeuristic(start.x, start.y) > 1e6f) {
    return false;
  }

  // ── ② 初始化 open set ──
  using OpenQueue = std::priority_queue<
      HybridNode, std::vector<HybridNode>, NodeComparator>;

  OpenQueue open_set;

  HybridNode start_node;
  start_node.state = start;
  start_node.g_cost = 0.0;
  start_node.h_cost = std::max(
      getObstacleHeuristic(start.x, start.y),
      getNonHolonomicHeuristic(start, goal));
  start_node.parent_idx = -1;
  start_node.prim_idx = -1;

  open_set.push(start_node);

  int start_bin = headingBin(start.theta);
  visited_[visitedIndex(start_cell, start_bin)] = 1;

  int iterations = 0;

  // ── ③ A* 主循环 ──
  while (!open_set.empty() && iterations < params_.max_iterations) {
    HybridNode current = open_set.top();
    open_set.pop();
    iterations++;

    // ── 到达判定 ──
    double dist_to_goal = distance(current.state.x, current.state.y,
                                   goal.x, goal.y);
    double angle_diff = std::abs(angleDiff(current.state.theta, goal.theta));

    if (dist_to_goal < params_.goal_tolerance &&
        angle_diff < params_.goal_angle_tolerance) {
      // 达到目标: 重建路径
      closed_list_.push_back(current);
      plan = reconstructPath(static_cast<int>(closed_list_.size()) - 1);

      // 追加精确 goal 作为最后一个点
      plan.push_back(goal);

      // 后处理
      plan = simplifyPath(plan, costmap);
      assignOrientations(plan);
      return true;
    }

    // ── 分析扩张 (RS 直连) ──
    if (iterations % params_.analytic_expansion_interval == 0 &&
        dist_to_goal < 10.0)  // 只在距离合理时尝试 RS
    {
      RSPath rs = reedsShepp(current.state, goal);
      if (rs.valid && rs.total_length < 20.0) {
        auto rs_states = sampleRSPath(current.state, rs,
                                      map_resolution_ * 0.5);
        if (isRSPathCollisionFree(rs_states, costmap)) {
          // RS 路径有效: 直接构建最终路径
          closed_list_.push_back(current);
          int current_idx = static_cast<int>(closed_list_.size()) - 1;

          // 从当前节点重建到起点
          plan = reconstructPath(current_idx);

          // 追加 RS 采样点
          for (const auto & s : rs_states) {
            plan.push_back(s);
          }
          plan.push_back(goal);

          plan = simplifyPath(plan, costmap);
          assignOrientations(plan);
          return true;
        }
      }
    }

    // ── 加入 closed list ──
    int current_idx = static_cast<int>(closed_list_.size());
    closed_list_.push_back(current);

    // ── 运动原语展开 ──
    for (size_t p = 0; p < primitives_.size(); p++) {
      const auto & prim = primitives_[p];

      SE2State next_state = expand(current.state, prim);

      // 边界检查
      GridCoord next_cell = worldToGrid(next_state.x, next_state.y);
      if (next_cell.x < 0 || next_cell.y < 0 ||
          static_cast<unsigned int>(next_cell.x) >= map_size_x_ ||
          static_cast<unsigned int>(next_cell.y) >= map_size_y_) {
        continue;
      }

      // visited 去重
      int next_bin = headingBin(next_state.theta);
      int v_idx = visitedIndex(next_cell, next_bin);
      if (visited_[v_idx]) {
        continue;
      }

      // 碰撞检测
      if (!isArcCollisionFree(current.state, prim, costmap)) {
        continue;
      }

      visited_[v_idx] = 1;

      // 计算代价
      double step_cost = prim.cost_multiplier * prim.arc_length;
      double g = current.g_cost + step_cost;
      double h = std::max(
          getObstacleHeuristic(next_state.x, next_state.y),
          getNonHolonomicHeuristic(next_state, goal));

      HybridNode next_node;
      next_node.state = next_state;
      next_node.g_cost = g;
      next_node.h_cost = h;
      next_node.parent_idx = current_idx;
      next_node.prim_idx = static_cast<int>(p);

      open_set.push(next_node);
    }
  }

  // 未找到路径
  return false;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 障碍物距离启发式 — 2D Dijkstra
// ═══════════════════════════════════════════════════════════════════════════════

void HybridAStar::computeObstacleHeuristic(
    const GridCoord & goal_cell,
    const nav2_costmap_2d::Costmap2D * costmap)
{
  const unsigned int total_cells = map_size_x_ * map_size_y_;

  // 初始化为 "未访问" (infinity)
  std::fill(obstacle_heuristic_.begin(), obstacle_heuristic_.end(),
            std::numeric_limits<float>::infinity());

  // 8-connected neighbors: dx, dy, cost multiplier (√2 for diagonals)
  static const int dx[8] = {-1,  0,  1, -1, 1, -1, 0, 1};
  static const int dy[8] = {-1, -1, -1,  0, 0,  1, 1, 1};
  static const float dc[8] = {
    1.414f, 1.0f, 1.414f,
    1.0f,          1.0f,
    1.414f, 1.0f, 1.414f
  };

  // priority_queue: (distance, flat_index)
  using PQElement = std::pair<float, unsigned int>;
  auto cmp = [](const PQElement & a, const PQElement & b) {
    return a.first > b.first;
  };
  std::priority_queue<PQElement, std::vector<PQElement>, decltype(cmp)> pq(cmp);

  // 从 goal cell 开始
  unsigned int goal_idx = goal_cell.y * map_size_x_ + goal_cell.x;
  if (goal_idx < total_cells) {
    obstacle_heuristic_[goal_idx] = 0.0f;
    pq.push({0.0f, goal_idx});
  }

  while (!pq.empty()) {
    auto [dist, idx] = pq.top();
    pq.pop();

    if (dist > obstacle_heuristic_[idx]) {
      continue;  // 已找到更短路径
    }

    int cx = static_cast<int>(idx % map_size_x_);
    int cy = static_cast<int>(idx / map_size_x_);

    for (int k = 0; k < 8; k++) {
      int nx = cx + dx[k];
      int ny = cy + dy[k];

      if (nx < 0 || ny < 0 ||
          static_cast<unsigned int>(nx) >= map_size_x_ ||
          static_cast<unsigned int>(ny) >= map_size_y_) {
        continue;
      }

      unsigned int n_idx = ny * map_size_x_ + nx;

      // 障碍物检查
      unsigned char cost = costmap->getCost(
          static_cast<unsigned int>(nx), static_cast<unsigned int>(ny));

      if (cost >= LETHAL_OBSTACLE) {
        continue;  // 不可通行
      }
      if (!params_.allow_unknown && cost == NO_INFORMATION) {
        continue;
      }

      // traversal cost 加权 (高 cost → 更高 traversal cost)
      float traversal = 1.0f + static_cast<float>(cost) / 252.0f;
      float new_dist = dist + dc[k] * map_resolution_ * traversal;

      if (new_dist < obstacle_heuristic_[n_idx]) {
        obstacle_heuristic_[n_idx] = new_dist;
        pq.push({new_dist, n_idx});
      }
    }
  }
}

double HybridAStar::getObstacleHeuristic(double wx, double wy) const
{
  GridCoord cell = worldToGrid(wx, wy);
  if (cell.x < 0 || cell.y < 0 ||
      static_cast<unsigned int>(cell.x) >= map_size_x_ ||
      static_cast<unsigned int>(cell.y) >= map_size_y_) {
    return std::numeric_limits<double>::infinity();
  }
  return static_cast<double>(
      obstacle_heuristic_[cell.y * map_size_x_ + cell.x]);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 非完整启发式 — Reeds-Shepp 无碰撞最短路径长度
// ═══════════════════════════════════════════════════════════════════════════════

double HybridAStar::getNonHolonomicHeuristic(
    const SE2State & from, const SE2State & to) const
{
  RSPath rs = reedsShepp(from, to);
  if (rs.valid) {
    return rs.total_length;
  }
  // RS 无解时退化为欧氏距离 (弱启发式但可采纳)
  return distance(from.x, from.y, to.x, to.y);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 运动原语
// ═══════════════════════════════════════════════════════════════════════════════

void HybridAStar::generateMotionPrimitives()
{
  primitives_.clear();

  const double k_max = params_.max_curvature;
  const double L = params_.arc_length;

  // 曲率集合: 0 (直行), ±k_max/n, ±2*k_max/n, ..., ±k_max
  int num_levels = params_.num_curvature_levels;
  if (num_levels < 1) num_levels = 1;

  for (int sign = -1; sign <= 1; sign += 2) {
    for (int level = 0; level <= num_levels; level++) {
      double k = sign * k_max * static_cast<double>(level) /
                 static_cast<double>(num_levels);
      if (level == 0) {
        k = 0.0;  // 直行, sign 无意义
      }

      double dtheta = k * L;
      double cost_mult = 1.0;

      // 转向代价: 曲率越大代价越高
      cost_mult += params_.steering_cost_multiplier *
                   std::abs(k) / k_max;

      MotionPrimitive prim;
      prim.curvature = k;
      prim.arc_length = L;
      prim.delta_theta = dtheta;
      prim.cost_multiplier = cost_mult;
      prim.is_reverse = false;
      primitives_.push_back(prim);

      // 后退版本
      MotionPrimitive prim_rev = prim;
      prim_rev.is_reverse = true;
      prim_rev.delta_theta = -dtheta;  // 后退时朝向变化方向翻转? 不变: 差速底盘后退转向时朝向变化方向与前进相同
      // wait: for diff drive, turning direction is the same regardless of forward/reverse
      // if turning left while backing up, the heading still changes CCW
      prim_rev.delta_theta = dtheta;  // same heading change
      prim_rev.cost_multiplier =
          cost_mult * params_.reverse_cost_multiplier;
      primitives_.push_back(prim_rev);

      if (level == 0) break;  // k=0 只加一次
    }
  }
}

SE2State HybridAStar::expand(const SE2State & state,
                             const MotionPrimitive & prim) const
{
  double c = std::cos(state.theta);
  double s = std::sin(state.theta);

  double dx_body, dy_body;

  if (std::abs(prim.curvature) < 1e-9) {
    // 直线
    double effective_len = prim.is_reverse ? -prim.arc_length : prim.arc_length;
    dx_body = effective_len;
    dy_body = 0.0;
  } else {
    // 弧线: dx = R·sin(dθ), dy = R·(1-cos(dθ))
    double R = 1.0 / prim.curvature;  // signed radius
    double dtheta = prim.delta_theta;
    dx_body = R * std::sin(dtheta);
    dy_body = R * (1.0 - std::cos(dtheta));
  }

  SE2State next;
  next.x = state.x + dx_body * c - dy_body * s;
  next.y = state.y + dx_body * s + dy_body * c;
  next.theta = normalizeAngle(state.theta + prim.delta_theta);
  return next;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 碰撞检测
// ═══════════════════════════════════════════════════════════════════════════════

bool HybridAStar::isStateValid(
    const SE2State & state,
    const nav2_costmap_2d::Costmap2D * costmap) const
{
  GridCoord cell = worldToGrid(state.x, state.y);
  if (cell.x < 0 || cell.y < 0 ||
      static_cast<unsigned int>(cell.x) >= map_size_x_ ||
      static_cast<unsigned int>(cell.y) >= map_size_y_) {
    return false;
  }

  unsigned char cost = costmap->getCost(
      static_cast<unsigned int>(cell.x),
      static_cast<unsigned int>(cell.y));

  if (cost >= LETHAL_OBSTACLE) return false;
  if (!params_.allow_unknown && cost == NO_INFORMATION) return false;

  return true;
}

bool HybridAStar::isArcCollisionFree(
    const SE2State & from, const MotionPrimitive & prim,
    const nav2_costmap_2d::Costmap2D * costmap) const
{
  // 沿弧线采样, 间距 = map_resolution_ / 2
  double sample_spacing = map_resolution_ * 0.5;
  double total_dist = prim.arc_length;
  int num_samples = std::max(1, static_cast<int>(total_dist / sample_spacing));

  for (int i = 0; i <= num_samples; i++) {
    // 采样点位置: 沿 primitive 前进 i/num_samples 比例
    double frac = static_cast<double>(i) / static_cast<double>(num_samples);

    // 构建部分 primitive
    MotionPrimitive partial = prim;
    partial.arc_length = prim.arc_length * frac;
    partial.delta_theta = prim.delta_theta * frac;

    SE2State sample = expand(from, partial);

    if (!isFootprintCollisionFree(sample, costmap)) {
      return false;
    }
  }

  return true;
}

bool HybridAStar::isFootprintCollisionFree(
    const SE2State & state,
    const nav2_costmap_2d::Costmap2D * costmap) const
{
  double half_l = params_.robot_length * 0.5;
  double half_w = params_.robot_width * 0.5;
  double spacing = params_.collision_sample_spacing;

  double c = std::cos(state.theta);
  double s_ = std::sin(state.theta);

  // 在机器人外包矩形内采样点
  for (double dx = -half_l; dx <= half_l + 1e-6; dx += spacing) {
    for (double dy = -half_w; dy <= half_w + 1e-6; dy += spacing) {
      // body → world
      double wx = state.x + dx * c - dy * s_;
      double wy = state.y + dx * s_ + dy * c;

      GridCoord cell = worldToGrid(wx, wy);
      if (cell.x < 0 || cell.y < 0 ||
          static_cast<unsigned int>(cell.x) >= map_size_x_ ||
          static_cast<unsigned int>(cell.y) >= map_size_y_) {
        return false;  // 出界
      }

      unsigned char cost = costmap->getCost(
          static_cast<unsigned int>(cell.x),
          static_cast<unsigned int>(cell.y));

      if (cost >= LETHAL_OBSTACLE) return false;
      if (!params_.allow_unknown && cost == NO_INFORMATION) return false;
    }
  }

  return true;
}

bool HybridAStar::isRSPathCollisionFree(
    const std::vector<SE2State> & rs_states,
    const nav2_costmap_2d::Costmap2D * costmap) const
{
  for (const auto & state : rs_states) {
    if (!isFootprintCollisionFree(state, costmap)) {
      return false;
    }
  }
  return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// Reeds-Shepp 曲线
// ═══════════════════════════════════════════════════════════════════════════════

namespace
{

/// normalize angle to [0, 2π)
double mod2pi(double theta)
{
  while (theta < 0.0) theta += 2.0 * M_PI;
  while (theta >= 2.0 * M_PI) theta -= 2.0 * M_PI;
  return theta;
}

/// polar coordinates: (r, theta) with theta in (-π, π]
std::pair<double, double> cartesianToPolar(double x, double y)
{
  double r = std::sqrt(x * x + y * y);
  double theta = std::atan2(y, x);
  return {r, theta};
}

/// 构建一个 RS 路径
RSPath makeRSPath(const std::vector<RSSegment> & segs)
{
  RSPath path;
  path.segments = segs;
  path.total_length = 0.0;
  path.valid = true;
  for (const auto & seg : segs) {
    if (seg.length < 0.0) {
      path.valid = false;
      return path;
    }
    path.total_length += seg.length;
  }
  return path;
}

/// CSC 类型: L+S+L+, L+S+R+, R+S+L+, R+S+R+
RSPath cscPath(double t, double u, double v, bool left_first, bool left_last)
{
  RSSegment s1, s2, s3;
  s1.steer = left_first ? SteerDir::LEFT : SteerDir::RIGHT;
  s1.gear = GearDir::FORWARD;
  s1.length = t;

  s2.steer = SteerDir::STRAIGHT;
  s2.gear = GearDir::FORWARD;
  s2.length = u;

  s3.steer = left_last ? SteerDir::LEFT : SteerDir::RIGHT;
  s3.gear = GearDir::FORWARD;
  s3.length = v;

  return makeRSPath({s1, s2, s3});
}

/// CCC 类型: L+R-L+, R+L-R+
RSPath cccPath(double t, double u, double v, bool left_first)
{
  RSSegment s1, s2, s3;
  s1.steer = left_first ? SteerDir::LEFT : SteerDir::RIGHT;
  s1.gear = GearDir::FORWARD;
  s1.length = t;

  s2.steer = left_first ? SteerDir::RIGHT : SteerDir::LEFT;
  s2.gear = GearDir::BACKWARD;   // CCC 中间段 cusp 换向
  s2.length = u;

  s3.steer = left_first ? SteerDir::LEFT : SteerDir::RIGHT;
  s3.gear = GearDir::FORWARD;
  s3.length = v;

  return makeRSPath({s1, s2, s3});
}

}  // anonymous namespace

RSPath HybridAStar::reedsShepp(const SE2State & from, const SE2State & to) const
{
  double R = params_.turning_radius;

  // 变换 goal 到 start 坐标系
  double dx = to.x - from.x;
  double dy = to.y - from.y;
  double c = std::cos(from.theta);
  double s_ = std::sin(from.theta);

  // goal in start frame, normalized by turning radius
  double x = (dx * c + dy * s_) / R;
  double y = (-dx * s_ + dy * c) / R;
  double phi = normalizeAngle(to.theta - from.theta);

  RSPath best_path;
  best_path.valid = false;
  best_path.total_length = std::numeric_limits<double>::infinity();

  // ═══════════════════════════════════════════════════════════════════════════
  // 辅助: 尝试一个 RS formula, 有效则更新 best_path
  // ═══════════════════════════════════════════════════════════════════════════
  auto tryPath = [&best_path](const RSPath & candidate) {
    if (candidate.valid && candidate.total_length < best_path.total_length) {
      best_path = candidate;
    }
  };

  // ═══════════════════════════════════════════════════════════════════════════
  // CSC formulas: LSL, LSR, RSL, RSR
  // ═══════════════════════════════════════════════════════════════════════════

  // ── L+S+L+ ──
  {
    auto [u, t] = cartesianToPolar(x - std::sin(phi), y - 1.0 + std::cos(phi));
    double v = mod2pi(phi - t);
    tryPath(cscPath(t, u, v, true, true));
  }

  // ── L+S+R+ ──
  {
    auto [u_sq_base, t1] = cartesianToPolar(
        x + std::sin(phi), y - 1.0 - std::cos(phi));
    double d2 = u_sq_base * u_sq_base;
    if (d2 >= 4.0) {
      double u = std::sqrt(d2 - 4.0);
      double theta = std::atan2(2.0, u);
      double t = mod2pi(t1 + theta);
      double v = mod2pi(t - phi);
      tryPath(cscPath(t, u, v, true, false));
    }
  }

  // ── R+S+L+ ──
  {
    auto [u_sq_base, t1] = cartesianToPolar(
        x - std::sin(phi), y + 1.0 + std::cos(phi));
    double d2 = u_sq_base * u_sq_base;
    if (d2 >= 4.0) {
      double u = std::sqrt(d2 - 4.0);
      double theta = std::atan2(2.0, u);
      double t = mod2pi(t1 - theta);
      double v = mod2pi(t - phi);
      tryPath(cscPath(t, u, v, false, true));
    }
  }

  // ── R+S+R+ ──
  {
    auto [u, t] = cartesianToPolar(
        x + std::sin(phi), y + 1.0 - std::cos(phi));
    double v = mod2pi(phi - t);
    tryPath(cscPath(t, u, v, false, false));
  }

  // ═══════════════════════════════════════════════════════════════════════════
  // CCC formulas: L+R-L+, R+L-R+
  // ═══════════════════════════════════════════════════════════════════════════

  // ── L+R-L+ ──
  // 条件: 两个同向圆的圆心距离 ≤ 4R (归一化后 ≤ 4)
  {
    auto [u, t1] = cartesianToPolar(
        x - std::sin(phi), y - 1.0 + std::cos(phi));
    if (u <= 4.0) {
      double A = std::acos(u / 4.0);           // A ∈ [0, π/2]
      double t = mod2pi(t1 + M_PI / 2.0 + A);
      double u2 = 2.0 * A;                      // 中间后退段 = 2A
      double v = mod2pi(phi - t + u2);
      tryPath(cccPath(t, u2, v, true));
    }
  }

  // ── R+L-R+ ──
  {
    auto [u, t1] = cartesianToPolar(
        x + std::sin(phi), y + 1.0 - std::cos(phi));
    if (u <= 4.0) {
      double A = std::acos(u / 4.0);
      double t = mod2pi(t1 + M_PI / 2.0 + A);
      double u2 = 2.0 * A;
      double v = mod2pi(phi - t + u2);
      tryPath(cccPath(t, u2, v, false));
    }
  }

  // 去归一化 (乘以 R)
  if (best_path.valid) {
    for (auto & seg : best_path.segments) {
      seg.length *= R;
    }
    best_path.total_length *= R;
  }

  return best_path;
}

std::vector<SE2State> HybridAStar::sampleRSPath(
    const SE2State & from, const RSPath & rs_path,
    double sample_spacing) const
{
  std::vector<SE2State> states;
  SE2State current = from;

  for (const auto & seg : rs_path.segments) {
    int num_samples = std::max(1,
        static_cast<int>(seg.length / sample_spacing));

    // 构建运动原语
    MotionPrimitive prim;
    prim.arc_length = seg.length;
    prim.is_reverse = (seg.gear == GearDir::BACKWARD);

    switch (seg.steer) {
      case SteerDir::LEFT:
        prim.curvature = params_.max_curvature;
        break;
      case SteerDir::RIGHT:
        prim.curvature = -params_.max_curvature;
        break;
      case SteerDir::STRAIGHT:
        prim.curvature = 0.0;
        break;
    }
    prim.delta_theta = prim.curvature * (prim.is_reverse ? -seg.length : seg.length);
    prim.cost_multiplier = 1.0;

    for (int i = 0; i < num_samples; i++) {
      double frac = static_cast<double>(i + 1) /
                    static_cast<double>(num_samples);

      MotionPrimitive partial = prim;
      partial.arc_length = seg.length * frac;
      partial.delta_theta = prim.delta_theta * frac;

      SE2State sample = expand(current, partial);
      states.push_back(sample);
    }

    current = expand(current, prim);
  }

  return states;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 路径后处理
// ═══════════════════════════════════════════════════════════════════════════════

std::vector<SE2State> HybridAStar::reconstructPath(int goal_idx) const
{
  std::vector<SE2State> path;

  int idx = goal_idx;
  while (idx >= 0 && idx < static_cast<int>(closed_list_.size())) {
    path.push_back(closed_list_[idx].state);
    idx = closed_list_[idx].parent_idx;
  }

  // 当前是 goal→start, 反转
  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<SE2State> HybridAStar::simplifyPath(
    const std::vector<SE2State> & raw_path,
    const nav2_costmap_2d::Costmap2D * costmap) const
{
  if (!params_.enable_path_simplification || raw_path.size() <= 2) {
    return raw_path;
  }

  // 贪心可见性简化: 从起点出发, 尽量连最远的可见点
  std::vector<SE2State> simplified;
  simplified.push_back(raw_path.front());

  size_t anchor = 0;
  while (anchor < raw_path.size() - 1) {
    size_t furthest = anchor + 1;

    // 从后往前找最远的无碰撞直连点
    for (size_t i = raw_path.size() - 1; i > anchor; i--) {
      // 线段碰撞检测
      bool collision_free = true;
      double d = distance(raw_path[anchor].x, raw_path[anchor].y,
                          raw_path[i].x, raw_path[i].y);
      int num_checks = std::max(1, static_cast<int>(d / (map_resolution_ * 0.5)));

      for (int j = 1; j <= num_checks && collision_free; j++) {
        double frac = static_cast<double>(j) / static_cast<double>(num_checks);
        double wx = raw_path[anchor].x + frac *
            (raw_path[i].x - raw_path[anchor].x);
        double wy = raw_path[anchor].y + frac *
            (raw_path[i].y - raw_path[anchor].y);

        GridCoord cell = worldToGrid(wx, wy);
        if (cell.x < 0 || cell.y < 0 ||
            static_cast<unsigned int>(cell.x) >= map_size_x_ ||
            static_cast<unsigned int>(cell.y) >= map_size_y_) {
          collision_free = false;
          break;
        }

        unsigned char cost = costmap->getCost(
            static_cast<unsigned int>(cell.x),
            static_cast<unsigned int>(cell.y));
        if (cost >= LETHAL_OBSTACLE) collision_free = false;
        if (!params_.allow_unknown && cost == NO_INFORMATION)
          collision_free = false;
      }

      if (collision_free) {
        furthest = i;
        break;
      }
    }

    simplified.push_back(raw_path[furthest]);
    anchor = furthest;
  }

  return simplified;
}

void HybridAStar::assignOrientations(std::vector<SE2State> & path) const
{
  if (path.size() < 2) return;

  for (size_t i = 0; i < path.size(); i++) {
    if (i == 0 && path.size() > 1) {
      // 第一个点: 指向下一个点
      path[i].theta = std::atan2(
          path[i + 1].y - path[i].y,
          path[i + 1].x - path[i].x);
    } else if (i == path.size() - 1 && path.size() > 1) {
      // 最后一个点: 保持与前一个点相同
      path[i].theta = path[i - 1].theta;
    } else if (path.size() > 1) {
      // 中间点: 切线方向 (前后点的平均)
      double dy = path[i + 1].y - path[i - 1].y;
      double dx = path[i + 1].x - path[i - 1].x;
      path[i].theta = std::atan2(dy, dx);
    }
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// 坐标变换
// ═══════════════════════════════════════════════════════════════════════════════

GridCoord HybridAStar::worldToGrid(double wx, double wy) const
{
  GridCoord cell;
  cell.x = static_cast<int>(
      std::round((wx - map_origin_x_) / map_resolution_));
  cell.y = static_cast<int>(
      std::round((wy - map_origin_y_) / map_resolution_));
  return cell;
}

void HybridAStar::gridToWorld(int mx, int my, double & wx, double & wy) const
{
  wx = map_origin_x_ + static_cast<double>(mx) * map_resolution_;
  wy = map_origin_y_ + static_cast<double>(my) * map_resolution_;
}

int HybridAStar::headingBin(double theta) const
{
  double normalized = normalizeAngle(theta);
  // 映射 [−π, π) → [0, heading_bins)
  int bin = static_cast<int>(
      std::round((normalized + M_PI) / (2.0 * M_PI) *
                 static_cast<double>(params_.heading_bins)));
  if (bin >= params_.heading_bins) bin = 0;
  if (bin < 0) bin = params_.heading_bins - 1;
  return bin;
}

int HybridAStar::visitedIndex(GridCoord cell, int heading_bin) const
{
  return (cell.y * static_cast<int>(map_size_x_) + cell.x) *
         params_.heading_bins + heading_bin;
}

}  // namespace nav2_hybrid_a_star_planner
