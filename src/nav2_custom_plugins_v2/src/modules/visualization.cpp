/**
 * @file visualization.cpp
 * @brief VisualizationPublisher 实现 — RViz MarkerArray 发布
 */

#include "nav2_custom_plugins_v2/modules/visualization.hpp"
#include "nav2_custom_plugins_v2/modules/path_manager.hpp"  // LookaheadResult
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "visualization_msgs/msg/marker.hpp"
#include <cmath>
#include <algorithm>

namespace nav2_custom_plugins_v2
{

void VisualizationPublisher::init(rclcpp_lifecycle::LifecycleNode *node)
{
  node_ = node;
  pub_ = node->create_publisher<visualization_msgs::msg::MarkerArray>(
      "/mppi_visualization", 10);
}

void VisualizationPublisher::publish(
    const RobotState &robot,
    const LookaheadResult &lh,
    const BatchTrajectories &batch,
    const std::vector<float> &costs,
    int best_idx, int N, int H,
    const Control &cmd, bool global_mode,
    const std::string &frame_id,
    bool heading_mode)
{
  if (!pub_) return;

  auto now = node_->now();
  auto lifetime = rclcpp::Duration::from_seconds(0.5);
  visualization_msgs::msg::MarkerArray ma;

  float rx = static_cast<float>(robot.x);
  float ry = static_cast<float>(robot.y);
  float rt = static_cast<float>(robot.theta);

  // ── ① 机器人朝向箭头 (青色) ──
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id; m.header.stamp = now;
    m.ns = "mppi"; m.id = 0; m.type = m.ARROW; m.action = m.ADD;
    m.pose.position.x = rx; m.pose.position.y = ry; m.pose.position.z = 0.08;
    m.pose.orientation.z = std::sin(rt * 0.5); m.pose.orientation.w = std::cos(rt * 0.5);
    m.scale.x = 0.25; m.scale.y = 0.04; m.scale.z = 0.04;
    m.color.r = 0.0f; m.color.g = 1.0f; m.color.b = 1.0f; m.color.a = 0.8f;
    m.lifetime = lifetime;
    ma.markers.push_back(m);
  }

  // ── ② 前瞻点小球 (黄色=正常, 红色=heading 模式) ──
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id; m.header.stamp = now;
    m.ns = "mppi"; m.id = 1; m.type = m.SPHERE; m.action = m.ADD;
    m.pose.position.x = static_cast<float>(lh.wx);
    m.pose.position.y = static_cast<float>(lh.wy);
    m.pose.position.z = 0.06;
    m.scale.x = 0.10; m.scale.y = 0.10; m.scale.z = 0.10;
    if (heading_mode) {
      m.color.r = 1.0f; m.color.g = 0.2f; m.color.b = 0.0f; m.color.a = 0.95f;  // 红色醒目
    } else {
      m.color.r = 1.0f; m.color.g = 1.0f; m.color.b = 0.0f; m.color.a = 0.9f;   // 黄色
    }
    m.lifetime = lifetime;
    ma.markers.push_back(m);
  }

  // ── ③ 机器人→前瞻点连线 (橙色=正常, 红色=heading 模式) ──
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id; m.header.stamp = now;
    m.ns = "mppi"; m.id = 2; m.type = m.LINE_STRIP; m.action = m.ADD;
    m.points.resize(2);
    m.points[0].x = rx;                        m.points[0].y = ry;                        m.points[0].z = 0.10;
    m.points[1].x = static_cast<float>(lh.wx); m.points[1].y = static_cast<float>(lh.wy); m.points[1].z = 0.10;
    m.scale.x = 0.03;
    if (heading_mode) {
      m.color.r = 1.0f; m.color.g = 0.2f; m.color.b = 0.0f; m.color.a = 0.8f;  // 红色
    } else {
      m.color.r = 1.0f; m.color.g = 0.6f; m.color.b = 0.0f; m.color.a = 0.7f;  // 橙色
    }
    m.lifetime = lifetime;
    ma.markers.push_back(m);
  }

  // ── ④ 最优轨迹 (绿色 LINE_STRIP) ──
  if (best_idx >= 0 && best_idx < N && !batch.x.empty()) {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id; m.header.stamp = now;
    m.ns = "mppi"; m.id = 3; m.type = m.LINE_STRIP; m.action = m.ADD;
    m.points.resize(H);
    for (int t = 0; t < H; ++t) {
      int idx = best_idx * H + t;
      m.points[t].x = batch.x[idx];
      m.points[t].y = batch.y[idx];
      m.points[t].z = 0.04;
    }
    m.scale.x = 0.03;
    m.color.r = 0.0f; m.color.g = 1.0f; m.color.b = 0.0f; m.color.a = 0.9f;
    m.lifetime = lifetime;
    ma.markers.push_back(m);
  }

  // ── ⑤ 采样轨迹散布 (浅蓝色, num_vis_trajs_ 条均匀采样) ──
  if (!batch.x.empty()) {
    int step = std::max(1, N / num_vis_trajs_);
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id; m.header.stamp = now;
    m.ns = "mppi"; m.id = 4; m.type = m.LINE_LIST; m.action = m.ADD;
    int n_segments = 0;
    for (int s = 0; s < N; s += step) {
      n_segments += (H - 1);
    }
    m.points.resize(n_segments * 2);
    int pi = 0;
    for (int s = 0; s < N; s += step) {
      for (int t = 0; t < H - 1; ++t) {
        int idx0 = s * H + t, idx1 = s * H + t + 1;
        m.points[pi].x = batch.x[idx0]; m.points[pi].y = batch.y[idx0]; m.points[pi].z = 0.03;
        pi++;
        m.points[pi].x = batch.x[idx1]; m.points[pi].y = batch.y[idx1]; m.points[pi].z = 0.03;
        pi++;
      }
    }
    m.scale.x = 0.01;
    m.color.r = 0.3f; m.color.g = 0.6f; m.color.b = 1.0f; m.color.a = 0.15f;
    m.lifetime = lifetime;
    ma.markers.push_back(m);
  }

  // ── ⑥ 速度指令箭头 (品红色) ──
  {
    double spd = std::hypot(cmd.vx, cmd.vy);
    if (spd > 0.02) {
      double vx_world, vy_world;
      // cmd.vx/vy 在 publish 时已经是 odom 系 (由 VelocityPostProcessor 转换)
      vx_world = cmd.vx;
      vy_world = cmd.vy;
      double vn = std::hypot(vx_world, vy_world);
      if (vn > 1e-6) { vx_world /= vn; vy_world /= vn; }

      double arrow_len = spd * 1.5;
      visualization_msgs::msg::Marker m;
      m.header.frame_id = frame_id; m.header.stamp = now;
      m.ns = "mppi"; m.id = 5; m.type = m.ARROW; m.action = m.ADD;
      m.points.resize(2);
      m.points[0].x = rx;                                m.points[0].y = ry;                                m.points[0].z = 0.09;
      m.points[1].x = rx + vx_world * arrow_len;         m.points[1].y = ry + vy_world * arrow_len;         m.points[1].z = 0.09;
      m.scale.x = 0.04; m.scale.y = 0.08;
      m.color.r = 1.0f; m.color.g = 0.0f; m.color.b = 1.0f; m.color.a = 0.9f;
      m.lifetime = lifetime;
      ma.markers.push_back(m);
    }
  }

  // ── ⑦ 指令数值文本 (白色; heading 模式加前缀) ──
  {
    visualization_msgs::msg::Marker m;
    m.header.frame_id = frame_id; m.header.stamp = now;
    m.ns = "mppi"; m.id = 6; m.type = m.TEXT_VIEW_FACING; m.action = m.ADD;
    m.pose.position.x = rx + 0.3f; m.pose.position.y = ry + 0.3f; m.pose.position.z = 0.20;
    m.scale.z = 0.07;
    if (heading_mode) {
      m.color.r = 1.0f; m.color.g = 0.3f; m.color.b = 0.0f; m.color.a = 0.95f;  // 红色文字
    } else {
      m.color.r = 1.0f; m.color.g = 1.0f; m.color.b = 1.0f; m.color.a = 0.9f;   // 白色文字
    }
    m.lifetime = lifetime;
    char buf[64];
    if (heading_mode) {
      snprintf(buf, sizeof(buf), "[HEADING] ang=%.2f", cmd.omega);
    } else {
      snprintf(buf, sizeof(buf), "vx=%.2f vy=%.2f ang=%.2f",
               cmd.vx, cmd.vy, cmd.omega);
    }
    m.text = buf;
    ma.markers.push_back(m);
  }

  pub_->publish(ma);
}

}  // namespace nav2_custom_plugins_v2
