#ifndef NAV2_CUSTOM_PLUGINS_V2__PATH_MANAGER_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__PATH_MANAGER_HPP_

/**
 * @file path_manager.hpp
 * @brief PathManager — 路径预处理 + 查询, 解耦 steering_controller 中的路径逻辑
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 职责
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ① 持有全局路径 (setPath)
 *   ② 最近点查询 (findClosestIndex) — 增量搜索, 防止 closest_idx 回跳
 *   ③ 前瞻点计算 (computeLookahead) — 沿路径前推 lookahead_dist
 *   ④ 推荐朝向 (getYaw / getTangentYaw) — 两种来源, 开关在 computeVelocityCommands
 *   ⑤ 产出 GPU 数据 (buildPathInfo / buildGoalInfo / extractLocalPath)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 在 computeVelocityCommands 中的使用
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   path_mgr_.setPath(global_plan_);
 *   int closest = path_mgr_.findClosestIndex(start.x, start.y);
 *   auto lp = path_mgr_.computeLookahead(closest, min_lookahead_dist);
 *
 *   // yaw 来源切换
 *   double yaw = params_.use_planner_yaw
 *     ? path_mgr_.getPlannerYaw(lp.idx)
 *     : path_mgr_.getTangentYaw(lp.idx);
 *
 *   // GPU 数据
 *   PathInfo pi; GoalInfo gi;
 *   path_mgr_.buildPathInfo(pi);
 *   path_mgr_.buildGoalInfo(gi, yaw);
 */

#include <vector>
#include "nav_msgs/msg/path.hpp"
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// 前瞻点结果
struct LookaheadResult {
  int    idx = 0;       ///< 前瞻点在路径上的索引
  double wx = 0.0;      ///< 世界坐标 x
  double wy = 0.0;      ///< 世界坐标 y
  double dist = 0.0;    ///< 沿路径累计距离
};

class PathManager {
public:
  PathManager() = default;

  // ═════════════════════════════════════════════════════════════════════════
  // 路径设置
  // ═════════════════════════════════════════════════════════════════════════

  /// 设置全局路径 (每帧由 Nav2 setPlan 回调 → 此处存储)
  void setPath(const nav_msgs::msg::Path &path);

  /// 刷新路径坐标 (重定位后 map→odom 会变, 每帧用最新 tf 更新)
  /// 仅替换 plan_ 内容, 不重置 prev_closest_idx_
  void refreshTransform(const nav_msgs::msg::Path &transformed);

  /// 路径是否有效
  bool valid() const { return !plan_.poses.empty(); }
  int  numPoints() const { return static_cast<int>(plan_.poses.size()); }

  // ═════════════════════════════════════════════════════════════════════════
  // 最近点查询
  // ═════════════════════════════════════════════════════════════════════════

  /// 增量搜索最近路径点 (从 prev_closest_idx_ 出发, 防止跳变)
  int findClosestIndex(double robot_x, double robot_y) const;

  // ═════════════════════════════════════════════════════════════════════════
  // 前瞻点
  // ═════════════════════════════════════════════════════════════════════════

  /// 1:1 THEMIS: 沿路径扫描, Euclidean 距离 ≥ min_dist 的首个点, 默认终点兜底
  LookaheadResult computeLookahead(int closest_idx, double min_dist,
                                    double robot_x, double robot_y) const;

  // ═════════════════════════════════════════════════════════════════════════
  // 推荐朝向 (两种来源)
  // ═════════════════════════════════════════════════════════════════════════

  /// 方式 1: planner 给的 pose.orientation (含 SE2 优化)
  double getPlannerYaw(int idx) const;

  /// 方式 2: 自算切线 atan2(dy, dx)
  double getTangentYaw(int idx) const;

  // ═════════════════════════════════════════════════════════════════════════
  // GPU 数据产出
  // ═════════════════════════════════════════════════════════════════════════

  /// 填充 PathInfo (不含 GPU 指针, 由调用者上传后再填)
  void buildPathInfo(PathInfo &info, double path_tangent) const;

  /// 填充 GoalInfo (target_yaw=body系期望方向, max_v/max_vy=矩形速度包络)
  void buildGoalInfo(GoalInfo &info, double target_yaw,
                     double max_v, double max_vy) const;

  /// 提取整条路径的 (x, y) 浮点数组 (供 GPU 上传)
  void extractPathArrays(std::vector<float> &xs, std::vector<float> &ys) const;

private:
  nav_msgs::msg::Path plan_;
  mutable int prev_closest_idx_ = 0;   ///< 上一帧最近点, 增量搜索起点
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__PATH_MANAGER_HPP_
