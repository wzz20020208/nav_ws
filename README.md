# nav_ws — THEMIS 导航工作空间

基于 ROS 2 Humble + Nav2 的全向双足机器人自主导航系统，核心控制器为 GPU 加速的 MPPI 局部规划器。

## 系统架构

```
传感器数据                         导航栈
─────────                        ────────
/joint_states ──→ TF 发布        Nav2 Planner (NavfnPlanner)
    │                                 │
/depth_image  ──→ 降采样→点云    全局代价地图 (3m×3m)
    │                                 │
/odom         ──→ 里程计           MPPI GPU 控制器 ◄── 本仓库核心
                                       │
                                   /cmd_vel → 机器人底盘
```

## 快速启动

```bash
# 1. 构建
cd ~/THEMIS/THEMIS/nav_ws
colcon build --symlink-install
source install/setup.bash

# 2. 启动 TF + 传感器管线
ros2 launch nav_launch nav_launch.py

# 3. 启动导航 (含 MPPI 控制器)
ros2 launch nav_launch themis_navigation.launch.py

# 4. 启动建图 (可选, 首次运行或更新地图时使用)
ros2 launch nav_launch themis_rtabmap_rgbd_mapping.launch.py
```

## 包列表

### 控制器与规划

| 包 | 说明 |
|----|------|
| `nav2_custom_plugins` | **核心** — GPU 加速 MPPI 控制器 (`MPPIGPUController`)，含五模式状态机、虚拟机器人窄道检测、三组件代价架构 |
| `nav_launch` | 启动入口 + YAML 参数配置 |
| `mppi_benchmark` | GPU vs CPU 代价计算延迟 benchmark（独立于 ROS） |

### 传感器与 TF

| 包 | 说明 |
|----|------|
| `joint_tf_publisher` | 订阅 `/joint_states`，发布 28 个关节 TF |
| `depth_to_pcl` | 深度图降采样 (40×) → 点云 (`/chest_camera/points_sparse`) |
| `base_state_to_odom` | Gazebo link_states → `/odom` TF |

### 控制与状态

| 包 | 说明 |
|----|------|
| `velocity_controller` | 导航指令 → `/cmd_vel` |
| `state_manager` | 机器人状态聚合 |
| `keyboard_vel_controller` | 键盘遥操作 |
| `th02_a7_description` | 机器人 URDF 模型 + 网格文件 |

### 外部依赖

| 包 | 说明 |
|----|------|
| `rtabmap_ros` | RGB-D SLAM (建图/定位) |
| `themis_state_msgs` | 自定义 ROS 2 消息定义 |

## MPPI GPU 控制器

本仓库的核心组件。主要特性：

- **三组件代价**：$J = w_{\text{obs}} J_{\text{obstacle}} + w_{\text{track}} J_{\text{tracking}} + w_{\text{prog}} J_{\text{progress}}$，每组件单一标量权重
- **角度匹配速度奖励**：匹配速度矢量方向与前瞻点方向的角度差，$>$45° 偏差时直行受罚，消除全向底盘"推头"行为
- **GPU 加速**：$N=3000$ 条轨迹并行，单帧 0.33 ms（CPU: 27.4 ms，加速 83×）
- **五模式状态机**：TERMINAL_ALIGN / LATERAL_SHIFT / HEADING_MISALIGN / NARROW_PASSAGE / NORMAL
- **虚拟机器人窄道探测**：前方 0.2 m + 0.4 m 处放置轻量 MPPI 实例预判通行性

详细文档见：

- [README (插件机制)](src/nav2_custom_plugins/README.md)
- [GPU 加速原理](src/nav2_custom_plugins/docs/MPPI_GPU_ACCELERATION.md)
- [方法论文档](src/nav2_custom_plugins/docs/MPPI_METHOD.md)

## 参数调优

编辑 `src/nav_launch/config/themis_navigation.yaml` 中 `FollowPath` 段：

```yaml
# 三组件权重 (优先调整)
obstacle_weight: 10.0    # ↑ 更保守, ↓ 更激进
tracking_weight: 30.0    # ↑ 更贴路径
progress_weight: 1.5     # ↑ 更快, ↓ 更稳 (防推头)

# 速度限制 (硬件约束, 勿动)
max_v: 0.40
max_vy: 0.20
max_w: 0.36
```

修改后重启导航 launch 即可生效。

## 依赖

- ROS 2 Humble
- Nav2
- CUDA ≥ 12.0 (GPU 控制器)
- OpenMP (CPU benchmark)
- Gazebo (仿真)
- RTAB-Map (建图/定位)

## TF 树

```
map → odom → BASE_LINK
                ├── 左右腿各 5 个关节
                ├── 左右臂各 7 个关节
                ├── NECK → HEAD
                └── chest_depth_camera_optical_link
```

## 已知问题

1. `depth_to_pcl`: 降采样后 CameraInfo K 矩阵未同步缩放，点云 3D 投影偏移
2. `base_state_to_odom`: C++ / Python 双版本共存
3. 相机参数在 URDF 和 SDF 中定义不一致
4. 全局规划器近终点时高频重规划可能干扰状态机（已通过 `setPlan` 门控缓解）
