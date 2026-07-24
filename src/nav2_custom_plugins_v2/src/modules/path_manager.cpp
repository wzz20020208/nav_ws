/**
 * @file path_manager.cpp
 * @brief PathManager 实现 — 路径预处理 + 查询
 */

#include "nav2_custom_plugins_v2/modules/path_manager.hpp"
#include <cmath>
#include <limits>
#include <algorithm>

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════════
// setPath
// ═══════════════════════════════════════════════════════════════════════════════

void PathManager::setPath(const nav_msgs::msg::Path &path)
{
  plan_ = path;
  prev_closest_idx_ = 0;  // 新路径, 重置搜索起点
}

void PathManager::refreshTransform(const nav_msgs::msg::Path &transformed)
{
  plan_ = transformed;
  // 注意: 不重置 prev_closest_idx_, 保留路径跟踪连续性
}

// ═══════════════════════════════════════════════════════════════════════════════
// findClosestIndex — 增量搜索, 防止 closest_idx 跳变
// ═══════════════════════════════════════════════════════════════════════════════
//
// 从上一帧的最近点出发, 分别向前/后搜索, 找到离机器人最近的路径点。
// 增量搜索防止 closest_idx 在两条路径段之间来回跳动 (尤其在转弯处)。
//
// "只进不退": 搜索结果不能小于 prev_closest_idx_, 确保沿路径正向推进。

int PathManager::findClosestIndex(double robot_x, double robot_y) const
{
  if (!valid()) return 0;

  int n = static_cast<int>(plan_.poses.size());
  double min_sq = std::numeric_limits<double>::max();
  int best = prev_closest_idx_;

  // 从 prev_closest_idx_ 向后搜索 (前进方向)
  for (int i = prev_closest_idx_; i < n; ++i) {
    double dx = plan_.poses[i].pose.position.x - robot_x;
    double dy = plan_.poses[i].pose.position.y - robot_y;
    double d2 = dx * dx + dy * dy;
    if (d2 < min_sq) { min_sq = d2; best = i; }
  }

  prev_closest_idx_ = best;
  return best;
}

// ═══════════════════════════════════════════════════════════════════════════════
// computeLookahead — 1:1 THEMIS: Euclidean 距离 ≥ min_dist 的首个点, 默认终点兜底
// ═══════════════════════════════════════════════════════════════════════════════

LookaheadResult PathManager::computeLookahead(
    int closest_idx, double min_dist, double robot_x, double robot_y) const
{
  LookaheadResult result;
  if (!valid()) return result;

  int last_idx = static_cast<int>(plan_.poses.size()) - 1;
  int lh_idx = last_idx;  // 默认终点, 未达标时兜底

  for (int i = closest_idx; i <= last_idx; ++i) {
    double dx = plan_.poses[i].pose.position.x - robot_x;
    double dy = plan_.poses[i].pose.position.y - robot_y;
    if (std::hypot(dx, dy) >= min_dist) { lh_idx = i; break; }
  }

  result.idx  = lh_idx;
  result.wx   = plan_.poses[lh_idx].pose.position.x;
  result.wy   = plan_.poses[lh_idx].pose.position.y;
  result.dist = std::hypot(result.wx - robot_x, result.wy - robot_y);

  return result;
}

// ═══════════════════════════════════════════════════════════════════════════════
// getPlannerYaw — 方式 1: 使用 planner 给的 pose.orientation
// ═══════════════════════════════════════════════════════════════════════════════

double PathManager::getPlannerYaw(int idx) const
{
  if (!valid()) return 0.0;
  int i = std::clamp(idx, 0, static_cast<int>(plan_.poses.size()) - 1);
  auto &ori = plan_.poses[i].pose.orientation;
  return 2.0 * std::atan2(ori.z, ori.w);
}

// ═══════════════════════════════════════════════════════════════════════════════
// getTangentYaw — 方式 2: 自算切线 atan2(dy, dx)
// ═══════════════════════════════════════════════════════════════════════════════
//
// 最后一点复刻倒数第二段的切线方向。

double PathManager::getTangentYaw(int idx) const
{
  if (!valid()) return 0.0;
  int n = static_cast<int>(plan_.poses.size());
  int i0 = std::clamp(idx, 0, n - 1);
  int i1 = std::min(i0 + 1, n - 1);
  if (i0 == n - 1) { i0 = n - 2; i1 = n - 1; }

  double dx = plan_.poses[i1].pose.position.x - plan_.poses[i0].pose.position.x;
  double dy = plan_.poses[i1].pose.position.y - plan_.poses[i0].pose.position.y;
  return std::atan2(dy, dx);
}

// ═══════════════════════════════════════════════════════════════════════════════
// buildPathInfo — 填充 PathInfo (不含 GPU 指针)
// ═══════════════════════════════════════════════════════════════════════════════

void PathManager::buildPathInfo(PathInfo &info, double path_tangent) const
{
  info.num_pts      = static_cast<int>(plan_.poses.size());
  info.path_tangent = static_cast<float>(path_tangent);

  // goal_yaw: 终点朝向 = 最后一点的 orientation
  if (!plan_.poses.empty()) {
    auto &ori = plan_.poses.back().pose.orientation;
    info.goal_yaw = static_cast<float>(2.0 * std::atan2(ori.z, ori.w));
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// buildGoalInfo — 填充 GoalInfo
// ═══════════════════════════════════════════════════════════════════════════════

void PathManager::buildGoalInfo(GoalInfo &info, double target_yaw,
                                  double max_v, double max_vy) const
{
  // 期望速度方向 (机器人 body 系, 与 rollout vx/vy 同系)
  float tvx = static_cast<float>(std::cos(target_yaw));
  float tvy = static_cast<float>(std::sin(target_yaw));
  info.target_vx_r = tvx;
  info.target_vy_r = tvy;

  // 该方向矩形速度包络可达最大速度: min(max_v / |tvx|, max_vy / |tvy|)
  // 纯前向 → max_v, 纯侧向 → max_vy, 斜向自动取紧的一边
  float ax = std::abs(tvx) > 1e-6f ? static_cast<float>(max_v) / std::abs(tvx) : static_cast<float>(max_vy);
  float ay = std::abs(tvy) > 1e-6f ? static_cast<float>(max_vy) / std::abs(tvy) : static_cast<float>(max_v);
  info.max_feasible_v = std::min(ax, ay);

  // 终点坐标
  if (!plan_.poses.empty()) {
    auto &goal_pos = plan_.poses.back().pose.position;
    info.goal_x = static_cast<float>(goal_pos.x);
    info.goal_y = static_cast<float>(goal_pos.y);
  }
}

// ═══════════════════════════════════════════════════════════════════════════════
// extractPathArrays — 提取整条路径的 float 数组 (供 GPU 上传)
// ═══════════════════════════════════════════════════════════════════════════════

void PathManager::extractPathArrays(
    std::vector<float> &xs, std::vector<float> &ys) const
{
  int n = static_cast<int>(plan_.poses.size());
  xs.resize(n);
  ys.resize(n);
  for (int i = 0; i < n; ++i) {
    xs[i] = static_cast<float>(plan_.poses[i].pose.position.x);
    ys[i] = static_cast<float>(plan_.poses[i].pose.position.y);
  }
}

}  // namespace nav2_custom_plugins_v2
