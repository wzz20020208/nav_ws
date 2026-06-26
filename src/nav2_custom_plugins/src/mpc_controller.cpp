/**
 * @file mpc_controller.cpp
 * @brief MPC (模型预测控制) 控制器实现
 *
 * 本文件实现了基于模型预测控制 (Model Predictive Control, MPC) 的路径跟踪控制器。
 * 通过采样控制空间并预测未来状态来选择最优控制指令，使机器人能够跟踪全局路径。
 *
 * 核心算法：
 * 1. 从当前状态出发，在控制空间中进行网格采样
 * 2. 对每组采样控制量，模拟预测未来N步的系统状态
 * 3. 计算每组控制量对应的代价值（与目标点的距离）
 * 4. 选择代价值最小的控制量作为输出
 */

#include "nav2_custom_plugins/mpc_controller.hpp"
#include <limits>
#include <cmath>
#include <algorithm>
#include "tf2/utils.h"

namespace nav2_custom_plugins
{

/**
 * @struct RobotState
 * @brief 机器人状态结构体
 *
 * 存储机器人在二维平面中的位姿信息：
 * - x, y: 机器人当前位置的笛卡尔坐标
 * - theta: 机器人朝向角（弧度）
 */
struct RobotState { double x, y, theta; };

/**
 * @struct ControlCmd
 * @brief 控制指令结构体
 *
 * 定义机器人的基本运动控制量：
 * - v: 线速度 (m/s)
 * - w: 角速度 (rad/s)
 */
struct ControlCmd { double v, w; };

/**
 * @brief 根据当前状态和控制指令预测下一时刻状态
 * @param current 当前机器人状态
 * @param cmd 控制指令（线速度、角速度）
 * @param dt 时间步长
 * @return 预测的下一时刻机器人状态
 *
 * 使用简化的运动模型：
 * x_{t+1} = x_t + v * cos(theta) * dt
 * y_{t+1} = y_t + v * sin(theta) * dt
 * theta_{t+1} = theta_t + w * dt
 */
RobotState predictNextState(const RobotState& current, const ControlCmd& cmd, double dt) {
  RobotState next;
  next.x = current.x + cmd.v * cos(current.theta) * dt;
  next.y = current.y + cmd.v * sin(current.theta) * dt;
  next.theta = current.theta + cmd.w * dt;
  return next;
}

/**
 * @brief 配置控制器参数
 * @param parent 父节点指针（弱指针，避免循环引用）
 * @param name 插件名称
 * @param tf TF变换缓冲器，用于坐标变换
 * @param costmap_ros 代价地图接口
 *
 * 在 Nav2 生命周期节点管理下初始化控制器所需的各种资源。
 * 存储插件名称、TF缓冲器和代价地图的引用。
 */
void MPCController::configure(
  const rclcpp_lifecycle::LifecycleNode::WeakPtr & parent,
  std::string name, std::shared_ptr<tf2_ros::Buffer> tf,
  std::shared_ptr<nav2_costmap_2d::Costmap2DROS> costmap_ros)
{
  node_ = parent;
  plugin_name_ = name;
  tf_ = tf;
  costmap_ros_ = costmap_ros;
  RCLCPP_INFO(node_.lock()->get_logger(), "配置 MPCController 成功！");
}

/**
 * @brief 清理资源
 *
 * 在插件停用时被调用，执行必要的资源释放和清理工作。
 */
void MPCController::cleanup() { RCLCPP_INFO(node_.lock()->get_logger(), "清理 MPCController"); }

/**
 * @brief 激活插件
 *
 * 将插件状态设置为活跃，开始处理控制请求。
 */
void MPCController::activate() { RCLCPP_INFO(node_.lock()->get_logger(), "激活 MPCController"); }

/**
 * @brief 停用插件
 *
 * 将插件状态设置为非活跃，暂停处理控制请求。
 */
void MPCController::deactivate() { RCLCPP_INFO(node_.lock()->get_logger(), "停用 MPCController"); }

/**
 * @brief 设置全局路径
 * @param path Nav2 规划出的全局路径
 *
 * 接收并存储全局路径，控制器将基于此路径生成速度指令。
 */
void MPCController::setPlan(const nav_msgs::msg::Path & path)
{
  global_plan_ = path;
}

/**
 * @brief 计算速度指令（核心 MPC 算法）
 * @param pose 当前机器人位姿
 * @param velocity 当前机器人速度（未使用）
 * @param goal_checker 目标检查器（未使用）
 * @return TwistStamped 速度指令
 *
 * 实现 MPC 控制器的核心逻辑：
 * 1. 从全局路径中选取前向目标点
 * 2. 在控制空间（v, w）中进行网格采样
 * 3. 对每组采样进行预测，计算代价值
 * 4. 返回最优控制量（最小代价值对应的速度指令）
 *
 * @note 当前实现为简化版 MPC，仅考虑距离代价，未考虑障碍物避障
 */
geometry_msgs::msg::TwistStamped MPCController::computeVelocityCommands(
  const geometry_msgs::msg::PoseStamped & pose,
  const geometry_msgs::msg::Twist & /*velocity*/,
  nav2_core::GoalChecker * /*goal_checker*/)
{
  geometry_msgs::msg::TwistStamped cmd_vel;
  cmd_vel.header.frame_id = "base_link";
  cmd_vel.header.stamp = node_.lock()->now();

  // ========== MPC 参数配置 ==========
  // 全方向移动：小车可前后、斜向运动，故线速度范围对称
  double dt = 0.1;              // 时间步长 (s)
  int prediction_horizon = 10;  // 预测时域（向前模拟的步数）
  double max_v = 0.5, min_v = -0.5;  // 线速度限制 (m/s)，支持全方向
  double max_w = 1.0;          // 角速度限制 (rad/s)
  int v_samples = 11, w_samples = 15;  // 控制空间采样分辨率

  // ========== 获取当前状态 ==========
  RobotState current_state;
  current_state.x = pose.pose.position.x;
  current_state.y = pose.pose.position.y;
  current_state.theta = tf2::getYaw(pose.pose.orientation);

  // ========== 从全局路径中选择目标点 ==========
  // 选取路径上第20个点（或最后一个点）作为预测目标
  double target_x = current_state.x, target_y = current_state.y;
  if (!global_plan_.poses.empty()) {
    int target_idx = std::min(20, (int)global_plan_.poses.size() - 1);
    target_x = global_plan_.poses[target_idx].pose.position.x;
    target_y = global_plan_.poses[target_idx].pose.position.y;
  }

  // ========== 初始化最优控制量搜索 ==========
  double min_cost = std::numeric_limits<double>::max();
  ControlCmd best_cmd = {0.0, 0.0};

  // 计算控制空间采样步长
  double dv = (max_v - min_v) / std::max(1, v_samples - 1);
  double dw = (2.0 * max_w) / std::max(1, w_samples - 1);

  // ========== 网格采样搜索最优控制量 ==========
  for (int i = 0; i < v_samples; ++i) {
    for (int j = 0; j < w_samples; ++j) {
      // 生成待测试的控制指令
      ControlCmd test_cmd = {min_v + i * dv, -max_w + j * dw};
      RobotState sim_state = current_state;
      double cost = 0.0;

      // ========== 预测并计算代价值 ==========
      // 在预测时域内模拟系统状态演变
      for (int k = 0; k < prediction_horizon; ++k) {
        sim_state = predictNextState(sim_state, test_cmd, dt);
        // 累加每一步与目标点的距离作为代价
        cost += std::hypot(target_x - sim_state.x, target_y - sim_state.y);
      }
      // 额外增加终点距离的权重（终端代价）
      cost += 0.5 * std::hypot(target_x - sim_state.x, target_y - sim_state.y);

      // 更新最优控制量
      if (cost < min_cost) {
        min_cost = cost;
        best_cmd = test_cmd;
      }
    }
  }

  // ========== 输出最优控制指令 ==========
  cmd_vel.twist.linear.x = best_cmd.v;
  cmd_vel.twist.angular.z = best_cmd.w;
  return cmd_vel;
}

/**
 * @brief 设置速度限制
 * @param speed_limit 速度限制值
 * @param percentage 是否为百分比模式
 *
 * 该方法目前为空实现。用于动态调整机器人的最大速度限制。
 */
void MPCController::setSpeedLimit(const double & /*speed_limit*/, const bool & /*percentage*/) {}

}  // namespace nav2_custom_plugins

// 注册插件，暴露给 Nav2
// PLUGINLIB_EXPORT_CLASS 宏将此类导出为 Nav2 可加载的插件
PLUGINLIB_EXPORT_CLASS(nav2_custom_plugins::MPCController, nav2_core::Controller)