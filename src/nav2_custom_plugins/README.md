# THEMIS Nav2 自定义插件

> 包名: `nav2_custom_plugins`
> 框架: Nav2 (ROS 2) Controller Plugin

---

## 目录

1. [Nav2 控制器插件机制](#1-nav2-控制器插件机制)
2. [插件列表](#2-插件列表)
3. [参数配置](#3-参数配置)
4. [使用方式](#4-使用方式)

---

## 1. Nav2 控制器插件机制

### 1.1 插件加载流程

```
┌──────────────────────────────────────────────────────────────────┐
│  launch 文件                                                      │
│  ├─ params_file: themis_navigation.yaml                          │
│  └─ controller_server 节点启动                                    │
│       │                                                           │
│       ▼                                                           │
│  Controller Server (nav2_controller)                              │
│  ├─ 读取 controller_plugins: ["FollowPath"]                      │
│  ├─ 查找 FollowPath.plugin: "nav2_custom_plugins/MPPIGPUController"│
│  ├─ pluginlib 加载 → plugins.xml 中的 class 注册                  │
│  ├─ 调用 controller->configure(node, name, tf, costmap_ros)      │
│  ├─ 调用 controller->activate()                                   │
│  └─ 进入控制循环 (controller_frequency: 10Hz)                     │
│       │                                                           │
│       ├─ 收到新全局路径 → controller->setPlan(path)              │
│       ├─ 每周期: controller->computeVelocityCommands(pose, vel)   │
│       └─ 发布 cmd_vel → 机器人底盘                                │
└──────────────────────────────────────────────────────────────────┘
```

### 1.2 生命周期

Nav2 控制器遵循 ROS 2 LifecycleNode 管理模式，通过 controller server 间接调用：

```
configure()                        ← 节点启动时
  ├─ declare_parameter + get_parameter  加载所有参数
  ├─ 订阅传感器/代价地图 topic
  ├─ 分配 GPU 缓冲区 (cudaMalloc)
  └─ 初始化发布者 (可视化/诊断)

activate()                         ← 配置完成后
  └─ 重置状态标志位

computeVelocityCommands()          ← 每周期调用 (10Hz)
  ├─ 接收参数: PoseStamped (当前位姿), Twist (当前速度)
  ├─ 返回: TwistStamped (速度指令)
  └─ setPlan() 随时可能被调用来更新全局路径

deactivate()                       ← 节点停用时
  └─ 清理运行时状态

cleanup()                          ← 节点销毁时
  ├─ 写入统计数据 CSV
  └─ 释放 GPU 缓冲区 (cudaFree)
```

### 1.3 参数传递机制

Nav2 使用 ROS 2 参数服务器，参数声明和读取在 `configure()` 中完成：

```cpp
// 节点获取参数的标准模式
auto node_ptr = node_.lock();

// 1. 声明参数（带默认值）
node_ptr->declare_parameter(plugin_name_ + ".max_v", max_v_);

// 2. 读取参数（覆盖默认值）
node_ptr->get_parameter(plugin_name_ + ".max_v", max_v_);
```

参数通过 YAML 文件传入，命名空间遵循 Nav2 约定：

```yaml
controller_server:
  ros__parameters:
    FollowPath:                    # ← plugin_name_
      plugin: "nav2_custom_plugins/MPPIGPUController"
      max_v: 0.40                 # ← 对应 plugin_name_ + ".max_v"
      prediction_horizon: 12
      # ... 所有参数在此命名空间下
```

### 1.4 setPlan 调用时机

`setPlan()` 在以下情况被 controller server 调用：

| 触发条件 | 频率 |
|----------|------|
| 新导航目标被设置 | 偶尔 |
| 全局规划器计算出新路径 | 最高 10Hz (planner_frequency) |
| costmap 变化导致重规划 | 随环境变化 |

**关键影响**：规划器高频重规划 → `setPlan` 高频调用 → 控制器内部状态可能被打断。当前实现中仅在 goal 移动 >0.1m 时才重置状态锁。

### 1.5 computeVelocityCommands 调用

```
Controller Server 主循环 (controller_frequency: 10Hz):
  1. 从 tf 获取当前位姿 (odom→base_link)
  2. 获取当前速度 (odom twist)
  3. 调用 controller->computeVelocityCommands(pose, velocity, goal_checker)
  4. 发布返回的 cmd_vel 到底盘
  5. 检查 goal_checker 是否到达目标
```

---

## 2. 插件列表

| 插件名 | 类型 | 说明 |
|--------|------|------|
| `MPPIGPUController` | Controller | **主力** GPU 加速 MPPI 控制器 |
| `MPPIController` | Controller | CPU 版 MPPI (调试/对比用) |
| `MPCController` | Controller | 传统 MPC (已弃用) |
| `AdaptiveProgressChecker` | ProgressChecker | 振荡感知进度检测器 |

---

## 3. 参数配置

完整参数见 `nav_launch/config/themis_navigation.yaml`，核心参数速查：

### MPPI 核心

| 参数 | 默认 | 说明 |
|------|------|------|
| `num_samples` | 3000 | 每帧轨迹采样数 (GPU 线程数) |
| `prediction_horizon` | 12 | 预测步数 |
| `dt` | 0.1 | 步长时间 (s) |
| `max_v / min_v` | 0.40 / -0.10 | 前向速度限制 |
| `max_vy` | 0.20 | 横向速度限制 |
| `max_w` | 0.36 | 角速度限制 |
| `lambda` | 1.0 | MPPI 温度参数 |

### 三组件代价权重

| 参数 | 默认 | 对应 critic |
|------|------|-------------|
| `obstacle_weight` | 10.0 | ObstaclesCritic — 碰撞检测 |
| `tracking_weight` | 30.0 | PathAlign + PathAngle + GoalAngle |
| `progress_weight` | 1.5 | PreferForward + GoalCritic |

### 采样噪声

| 参数 | 默认 | 说明 |
|------|------|------|
| `action_std_v` | 0.5 | vx 噪声标准差 |
| `action_std_vy` | 0.3 | vy 噪声标准差 |
| `action_std_w` | 0.4 | ω 噪声标准差 |
| `nln_ratio` | 0.35 | 对数正态混合比例 |
| `noise_decay_rate` | 0.55 | 噪声时序衰减率 |
| `guidance_weight` | 0.2 | 路径方向引导强度 |

---

## 4. 使用方式

### 4.1 启动

```bash
ros2 launch nav_launch themis_navigation.launch.py
```

### 4.2 切换控制器

在 `themis_navigation.yaml` 中修改：

```yaml
FollowPath:
  plugin: "nav2_custom_plugins/MPPIGPUController"  # GPU 版
  # plugin: "nav2_custom_plugins/MPPIController"   # CPU 版
```

### 4.3 调参流程

1. 修改 `themis_navigation.yaml` 中的参数
2. 重启导航 launch 文件
3. 观察 RViz 中的 `/mppi_gpu_visualization` 和诊断标记 `push_diag`
4. 检查日志 `/tmp/mppi_gpu_controller.log` 和 `/tmp/narrow_diag.csv`

**调参优先级**: 三组件权重 > 噪声参数 > 前瞻点/终端参数
