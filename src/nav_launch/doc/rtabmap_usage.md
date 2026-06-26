# RTAB-Map 使用指南 — THEMIS (TH02-A7)

## 文件结构

```
nav_launch/
├── config/
│   ├── themis_rtabmap.yaml                # 建图参数 (54 项)
│   └── themis_rtabmap_localization.yaml   # 定位参数
├── launch/
│   ├── themis_rtabmap.launch.py           # 建图启动
│   └── themis_rtabmap_localization.launch.py  # 定位启动
└── doc/
    └── rtabmap_usage.md                   # 本文件
```

## 快速开始

### 建图

```bash
# 终端 1: 启动仿真 (已有的 nav_launch)
ros2 launch nav_launch nav_launch.py

# 终端 2: 启动 RTAB-Map 建图
ros2 launch nav_launch themis_rtabmap.launch.py

# 增量建图 (不清空已有地图)
ros2 launch nav_launch themis_rtabmap.launch.py extra_args:=""
```

**建图时 rtabmap_viz 会自动打开**，可实时查看点云配准、位姿图和栅格地图。

### 建图完成 → 保存地图

建图过程中地图自动保存到 `~/.ros/themis_map.db`，无需手动操作。

### 定位

```bash
# 终端 1: 启动仿真
ros2 launch nav_launch nav_launch.py

# 终端 2: 启动 RTAB-Map 定位模式 (自动加载已有地图)
ros2 launch nav_launch themis_rtabmap_localization.launch.py
```

## 常用参数

### 建图

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `database_path` | `~/.ros/themis_map.db` | 地图保存路径 |
| `extra_args` | `--delete_db_on_start` | 启动时清空旧地图 |
| `localization` | `false` | 建图模式 |

### 定位

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `database_path` | `~/.ros/themis_map.db` | 已有地图路径 |
| `extra_args` | `""` | 额外 CLI 参数 |
| `initial_pose` | `""` | 初始位姿 `"x y z roll pitch yaw"` |

## 数据流

```
仿真/Gazebo
  └─ /chest_camera/chest_camera/depth/image_raw  (深度图)
       └─ depth_to_pcl 节点 (降采样 + 转点云)
            └─ /chest_camera/points_sparse  (稀疏点云)
                 └─ RTAB-Map SLAM ── 位姿图 + 栅格地图
                                       │
                       base_state_to_odom ── /odom (里程计)
```

## 建图 vs 定位 参数差异

| 参数 | 建图 | 定位 | 说明 |
|------|------|------|------|
| `Icp/Iterations` | 30 | 50 | 定位更精确 |
| `Icp/MaxCorrespondenceDistance` | 1.0 m | 0.5 m | 定位拒绝误匹配 |
| `Icp/MaxTranslation` | 2.0 m | 0.5 m | 定位拒绝跳变 |
| `Icp/VoxelSize` | 0.1 m | 0.05 m | 定位降采样更细 |
| `Mem/STMSize` | 30 | 0 | 定位关闭回环 |
| `RGBD/ProximityBySpace` | true | false | 定位关闭空间回环 |
| `RGBD/LinearUpdate` | 0.1 m | 0.0 | 定位不创建新节点 |
| 数据库 | 默认清空 | 保留地图 | — |

## 修改参数

直接编辑对应的 YAML 文件即可，中文注释标注了每个参数的含义。常用调整：

```yaml
# 地面过滤 - 根据实际地面高度调整
Grid/MaxGroundHeight: 0.1       # 高于此值为障碍物 (米)

# ICP 配准 - 点云稀疏时降低
Icp/MaxCorrespondenceDistance: 1.0  # 对应点搜索半径

# 栅格分辨率 - 精度 vs 性能
Grid/CellSize: 0.05             # 越小精度越高但计算量越大

# 回环 - 环境重复时关闭
RGBD/ProximityBySpace: "true"   # 空间回环检测
```

## 故障排查

```bash
# 查看 RTAB-Map 节点日志
ros2 run rqt_console rqt_console

# 手动启动 rviz 查看地图
ros2 run rviz2 rviz2

# 检查话题是否正常
ros2 topic list | grep -E "rtabmap|chest_camera|odom"

# 检查 TF 树
ros2 run tf2_tools view_frames
```
