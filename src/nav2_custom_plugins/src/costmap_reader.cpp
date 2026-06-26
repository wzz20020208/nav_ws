/**
 * @file costmap_reader.cpp
 * @brief 代价地图读取器 - 独立可执行工具
 *
 * 这是一个独立的ROS2节点，用于：
 * - 订阅代价地图话题 (nav_msgs/OccupancyGrid)
 * - 监听控制点并进行碰撞检测
 * - 发布可视化标记到RViz
 *
 * 使用方法:
 *   ros2 run nav2_custom_plugins costmap_reader_tool
 *
 * @author nav2_custom_plugins
 * @date 2026
 */

#include <memory>
#include <string>
#include <cmath>
#include <vector>
#include "rclcpp/rclcpp.hpp"
#include "nav_msgs/msg/occupancy_grid.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "geometry_msgs/msg/point_stamped.hpp"
#include "nav2_costmap_2d/costmap_2d.hpp"
#include "nav2_costmap_2d/cost_values.hpp"

/**
 * @class CostmapReaderNode
 * @brief 代价地图读取器节点
 *
 * 独立的ROS2节点，提供代价地图订阅和控制点碰撞检测功能。
 */
class CostmapReaderNode : public rclcpp::Node
{
public:
  /**
   * @brief 构造函数
   * @param options ROS2节点选项
   */
  CostmapReaderNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
    : Node("costmap_reader_tool", options), initialized_(false)
  {
    // 声明参数
    this->declare_parameter<std::string>("costmap_topic", "/local_costmap/costmap");
    this->declare_parameter<std::string>("control_point_topic", "control_point");
    this->declare_parameter<double>("visualization_resolution", 0.2);

    // 获取参数
    std::string costmap_topic = this->get_parameter("costmap_topic").as_string();
    std::string control_point_topic = this->get_parameter("control_point_topic").as_string();
    resolution_ = this->get_parameter("visualization_resolution").as_double();

    // 创建代价地图订阅者
    costmap_sub_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
      costmap_topic,
      rclcpp::SensorDataQoS(),
      [this](const nav_msgs::msg::OccupancyGrid::SharedPtr msg) {
        costmapCallback(msg);
      });

    // 创建控制点订阅者（用于碰撞检测查询）
    control_point_sub_ = this->create_subscription<geometry_msgs::msg::PointStamped>(
      control_point_topic,
      rclcpp::SensorDataQoS(),
      [this](const geometry_msgs::msg::PointStamped::SharedPtr msg) {
        controlPointCallback(msg);
      });

    // 创建可视化发布者
    marker_pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>(
      "costmap_visualization", 10);

    // 创建碰撞检测结果发布者
    collision_pub_ = this->create_publisher<geometry_msgs::msg::PointStamped>(
      "collision_result", 10);

    // 创建定时器用于定期发布可视化
    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(500),
      [this]() { publishVisualization(); });

    RCLCPP_INFO(this->get_logger(), "代价地图读取器已启动");
    RCLCPP_INFO(this->get_logger(), "  - 代价地图话题: %s", costmap_topic.c_str());
    RCLCPP_INFO(this->get_logger(), "  - 控制点话题: %s", control_point_topic.c_str());
  }

private:
  /**
   * @brief 代价地图回调函数
   * @param msg 收到的OccupancyGrid消息
   *
   * 将OccupancyGrid数据转换为Costmap2D格式并存储。
   */
  void costmapCallback(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
  {
    // 解析地图元信息
    unsigned int width = msg->info.width;
    unsigned int height = msg->info.height;
    double resolution = msg->info.resolution;
    double origin_x = msg->info.origin.position.x;
    double origin_y = msg->info.origin.position.y;

    // 保存全局坐标系ID
    global_frame_id_ = msg->header.frame_id;

    // 创建代价地图
    costmap_ = std::make_shared<nav2_costmap_2d::Costmap2D>(
      width, height, resolution, origin_x, origin_y);

    // 复制地图数据并进行转换
    const auto & map_data = msg->data;
    for (unsigned int y = 0; y < height; ++y) {
      for (unsigned int x = 0; x < width; ++x) {
        unsigned int index = y * width + x;
        signed char occupancy_value = map_data[index];

        unsigned char cost_value;
        if (occupancy_value < 0) {
          cost_value = nav2_costmap_2d::NO_INFORMATION;
        } else if (occupancy_value >= 90) {
          cost_value = nav2_costmap_2d::LETHAL_OBSTACLE;
        } else if (occupancy_value >= 50) {
          cost_value = nav2_costmap_2d::INSCRIBED_INFLATED_OBSTACLE;
        } else {
          cost_value = static_cast<unsigned char>(occupancy_value * 2.55);
        }

        costmap_->setCost(x, y, cost_value);
      }
    }

    initialized_ = true;
    RCLCPP_DEBUG(this->get_logger(), "代价地图已更新: %ux%u", width, height);
  }

  /**
   * @brief 控制点回调函数
   * @param msg 收到的控制点消息
   *
   * 检查控制点是否与障碍物碰撞，并发布检测结果。
   */
  void controlPointCallback(const geometry_msgs::msg::PointStamped::SharedPtr msg)
  {
    if (!initialized_) {
      RCLCPP_WARN(this->get_logger(), "代价地图未初始化，无法进行碰撞检测");
      return;
    }

    double wx = msg->point.x;
    double wy = msg->point.y;

    // 获取费用值
    unsigned int mx, my;
    if (!costmap_->worldToMap(wx, wy, mx, my)) {
      RCLCPP_WARN(this->get_logger(), "控制点超出地图范围");
      return;
    }

    unsigned char cost = costmap_->getCost(mx, my);

    // 发布碰撞检测结果
    geometry_msgs::msg::PointStamped result;
    result.header = msg->header;
    result.point.x = static_cast<double>(cost);
    result.point.y = (cost >= nav2_costmap_2d::LETHAL_OBSTACLE) ? 1.0 : 0.0;
    result.point.z = 0.0;
    collision_pub_->publish(result);

    RCLCPP_INFO(this->get_logger(), "控制点 (%.2f, %.2f) - 费用值: %u, 碰撞: %s",
                wx, wy, cost,
                (cost >= nav2_costmap_2d::LETHAL_OBSTACLE) ? "是" : "否");
  }

  /**
   * @brief 发布可视化标记
   *
   * 将代价地图以三维方块形式发布到RViz。
   * 红色 = 致命障碍, 橙色 = 膨胀障碍, 绿色 = 可通行区域
   */
  void publishVisualization()
  {
    if (!initialized_ || !marker_pub_) {
      return;
    }

    visualization_msgs::msg::MarkerArray marker_array;
    unsigned int id = 0;

    double origin_x = costmap_->getOriginX();
    double origin_y = costmap_->getOriginY();
    double width = costmap_->getSizeInMetersX();
    double height = costmap_->getSizeInMetersY();
    double step = resolution_;

    for (double wx = origin_x; wx < origin_x + width; wx += step) {
      for (double wy = origin_y; wy < origin_y + height; wy += step) {
        unsigned int mx, my;
        if (!costmap_->worldToMap(wx, wy, mx, my)) {
          continue;
        }

        unsigned char cost = costmap_->getCost(mx, my);

        visualization_msgs::msg::Marker marker;
        marker.header.frame_id = "MAP";
        marker.header.stamp = this->now();
        marker.ns = "costmap_reader";
        marker.id = id++;
        marker.type = visualization_msgs::msg::Marker::CUBE;
        marker.action = visualization_msgs::msg::Marker::ADD;
        marker.pose.position.x = wx;
        marker.pose.position.y = wy;
        marker.pose.position.z = 0.05;
        marker.pose.orientation.w = 1.0;
        marker.scale.x = step * 0.8;
        marker.scale.y = step * 0.8;
        marker.scale.z = 0.1;

        if (cost >= nav2_costmap_2d::LETHAL_OBSTACLE) {
          marker.color.r = 1.0;
          marker.color.g = 0.0;
          marker.color.b = 0.0;
          marker.color.a = 0.8;
        } else if (cost >= nav2_costmap_2d::INSCRIBED_INFLATED_OBSTACLE) {
          marker.color.r = 1.0;
          marker.color.g = 0.5;
          marker.color.b = 0.0;
          marker.color.a = 0.6;
        } else if (cost >= nav2_costmap_2d::FREE_SPACE) {
          marker.color.r = 0.0;
          marker.color.g = 1.0;
          marker.color.b = 0.0;
          marker.color.a = 0.3;
        } else {
          continue;
        }

        marker_array.markers.push_back(marker);
      }
    }

    marker_pub_->publish(marker_array);
  }

  // 订阅者和发布者
  rclcpp::Subscription<nav_msgs::msg::OccupancyGrid>::SharedPtr costmap_sub_;
  rclcpp::Subscription<geometry_msgs::msg::PointStamped>::SharedPtr control_point_sub_;
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr marker_pub_;
  rclcpp::Publisher<geometry_msgs::msg::PointStamped>::SharedPtr collision_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  // 代价地图
  std::shared_ptr<nav2_costmap_2d::Costmap2D> costmap_;
  std::string global_frame_id_;
  bool initialized_;
  double resolution_;
};

/**
 * @brief 主函数 - 程序入口
 * @param argc 命令行参数个数
 * @param argv 命令行参数指针数组
 */
int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<CostmapReaderNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}