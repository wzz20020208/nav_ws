#ifndef NAV2_CUSTOM_PLUGINS_V2__VISUALIZATION_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__VISUALIZATION_HPP_

#include <memory>
#include <vector>
#include "nav2_custom_plugins_v2/mppi_core.hpp"

namespace rclcpp { class Node; }
namespace visualization_msgs { namespace msg { class MarkerArray; } }

namespace nav2_custom_plugins_v2 {

/// RViz 可视化发布器: 机器人位姿、前瞻点、采样轨迹、速度指令
class VisualizationPublisher {
public:
  VisualizationPublisher() = default;

  /// 初始化发布者
  void init(rclcpp::Node* node);

  /// 发布可视化 markers
  void publish(
      const RobotState& robot,
      const LookaheadPoint& lh,
      const Control& cmd,
      const std::vector<float>& traj_x,   // N×H 轨迹 X 坐标
      const std::vector<float>& traj_y,   // N×H 轨迹 Y 坐标
      int best_idx, int N, int H);

private:
  std::shared_ptr<rclcpp::Publisher<visualization_msgs::msg::MarkerArray>> pub_;
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__VISUALIZATION_HPP_
