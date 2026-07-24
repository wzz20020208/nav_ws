#ifndef NAV2_CUSTOM_PLUGINS_V2__VISUALIZATION_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__VISUALIZATION_HPP_

/**
 * @file visualization.hpp
 * @brief VisualizationPublisher — RViz Marker 发布 (轨迹/前瞻/速度指令)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 发布内容 (topic: /mppi_visualization, type: MarkerArray)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ① 机器人朝向箭头 (青色)
 *   ② 前瞻点小球 (黄色)
 *   ③ 机器人→前瞻点连线 (橙色)
 *   ④ 最优轨迹 LINE_STRIP (绿色)
 *   ⑤ 采样轨迹散布 (浅蓝色, 稀疏采样 num_vis_trajs 条)
 *   ⑥ 速度指令箭头 (品红色)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 使用
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   vis_pub_->publish(robot, lookahead, batch, costs, best_idx, N, H, cmd, global_mode);
 */

#include <memory>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"

namespace rclcpp_lifecycle { class LifecycleNode; }

namespace nav2_custom_plugins_v2 {

struct LookaheadResult;

class VisualizationPublisher {
public:
  VisualizationPublisher() = default;

  /// 初始化 publisher (configure 时调用)
  void init(rclcpp_lifecycle::LifecycleNode *node);

  /// 发布全部 markers
  /// @param robot       机器人当前位姿
  /// @param lh          前瞻点 (世界坐标)
  /// @param batch       N×H 轨迹数据 (CPU 侧)
  /// @param costs       N 条轨迹代价
  /// @param best_idx    最优轨迹索引
  /// @param N, H        轨迹数 / 步数
  /// @param cmd         输出控制量
  /// @param global_mode 输出模式 (影响速度箭头方向)
  /// @param frame_id    marker 坐标系 (与路径数据一致)
  /// @param heading_mode 是否为 heading 原地旋转模式 (影响前瞻点颜色)
  void publish(const RobotState &robot,
               const LookaheadResult &lh,
               const BatchTrajectories &batch,
               const std::vector<float> &costs,
               int best_idx, int N, int H,
               const Control &cmd, bool global_mode,
               const std::string &frame_id,
               bool heading_mode = false);

private:
  rclcpp_lifecycle::LifecycleNode *node_ = nullptr;
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr pub_;
  int num_vis_trajs_ = 20;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__VISUALIZATION_HPP_
