#ifndef NAV2_CUSTOM_PLUGINS_V2__PATH_MANAGER_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__PATH_MANAGER_HPP_

#include <vector>
#include "nav_msgs/msg/path.hpp"
#include "nav2_custom_plugins_v2/mppi_core.hpp"

namespace nav2_custom_plugins_v2 {

/// 路径管理器: 解析全局路径、查找最近点、计算前瞻点
class PathManager {
public:
  PathManager() = default;

  /// 设置全局路径 (每帧由 Nav2 调用)
  void setPath(const nav_msgs::msg::Path& path);

  /// 路径是否有效
  bool valid() const { return !plan_.poses.empty(); }
  int numPoints() const { return static_cast<int>(plan_.poses.size()); }

  /// 增量搜索最近路径点 (防止 closest_idx 跳变)
  int findClosestIndex(double robot_x, double robot_y) const;

  /// 计算前瞻点: 沿路径推进直到累计距离 >= lookahead_dist
  LookaheadPoint computeLookahead(
      double robot_x, double robot_y, double robot_theta,
      double lookahead_dist, double lookahead_time,
      double current_speed) const;

  /// 获取某路径点的 SE2 朝向
  double getPathYaw(int idx) const;

  /// 提取局部路径段 (供 GPU 上传, 最多 max_pts 个点)
  void extractLocalPath(int closest_idx, int max_pts,
                        std::vector<float>& xs, std::vector<float>& ys) const;

private:
  nav_msgs::msg::Path plan_;
  mutable int prev_closest_idx_ = 0;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__PATH_MANAGER_HPP_
