# nav_ws 项目结构

## 数据流

```
/joint_states                    /chest_camera/.../depth/image_raw
     │                                      │
     ▼                                      ▼
joint_tf_publisher              depth_to_pcl (降采样 → 点云)
     │                                      │
     ├── 关节 TF (动态)                      │
     ├── 相机 TF (静态)                      │
     │                                      │
     ▼                                      ▼
/odom ←── base_state_to_odom           /chest_camera/points_sparse
     │                                      │
     └──────────┬───────────────────────────┘
                ▼
           TF Tree (RViz / 导航)
                │
                ▼
       velocity_controller ──→ /cmd_vel
                │
                ▼
       memory_manager / memory_pub (THEMIS Play 层)
```

---

## 包列表

### nav_launch
- **类型**: 启动入口
- **关键文件**: `launch/nav_launch.py`
- **功能**: 串联启动全部节点，顺序为:
  memory_manager → memory_pub → velocity_controller → depth_to_pcl → tf_launch → joint_tf_publisher → base_state_to_odom

### themis_state_msgs
- **类型**: 自定义 ROS 2 消息
- **关键文件**: `msg/*.msg` (16 个消息定义)
- **主要消息**: BaseState, BatteryState, BearArray, DetectedObjectArray, FootArray, JointArray, THJointState, ThreadState 等

### th02_a7_description
- **类型**: 机器人模型描述
- **关键文件**: `launch/rsp.launch.py`, `urdf/TH02-A7.urdf`, `meshes/*.STL` (33 个)
- **功能**: Robot State Publisher 启动 + 完整机器人 URDF/SDF 模型 + 相机 xacro 宏

### joint_tf_publisher
- **类型**: C++ TF 发布节点
- **关键文件**: `src/joint_tf_publisher_node.cpp`
- **功能**: 订阅 `/joint_states`，发布关节动态 TF；启动时发布 `BASE_LINK → chest_depth_camera_optical_link` 静态 TF

### depth_to_pcl
- **类型**: 深度图处理管线
- **关键文件**: `launch/depth_to_pcl_launch.py`, `launch/tf_launch.py`, `src/depth_to_pcl_node.cpp`
- **管线**: `CropDecimateNode` (降采样 40x) → `PointCloudXyzNode` (投影点云)
- **已知问题**: 降采样后 CameraInfo 的 K 矩阵未同步缩放

### base_state_to_odom
- **类型**: 里程计发布
- **关键文件**: `src/base_state_to_odom_node.cpp`, `src/base_state_to_odom_node.py`
- **功能**: 从 Gazebo link_states 提取 BASE_LINK 位姿，发布 `/odom` TF
- **问题**: C++ 和 Python 两版并存，仅使用 C++ 版

### velocity_controller
- **类型**: Python 速度控制节点
- **关键文件**: `velocity_controller/velocity_controller_node.py`
- **功能**: 接收导航指令，发布 `/cmd_vel`

### state_manager
- **类型**: Python 状态管理节点
- **关键文件**: `state_manager/state_manager_node.py`
- **功能**: 机器人状态聚合管理

---

## TF 树

```
BASE_LINK
├── HIP_R → HIP_ABAD_R → FEMUR_R → TIBIA_R → ANKLE_R → FOOT_R   (右腿)
├── HIP_L → HIP_ABAD_L → FEMUR_L → TIBIA_L → ANKLE_L → FOOT_L   (左腿)
├── UPPERSHOULDER_R → LOWERSHOULDER_R → UPPERARM_R → ELBOW_R → FOREARM_R → UPPERWRIST_R → LOWERWRIST_R  (右臂)
├── UPPERSHOULDER_L → LOWERSHOULDER_L → UPPERARM_L → ELBOW_L → FOREARM_L → UPPERWRIST_L → LOWERWRIST_L  (左臂)
├── NECK → HEAD                                                  (头部)
└── chest_depth_camera_optical_link                              (相机, 静态)
```

---

## 已知问题

1. **depth_to_pcl**: 降采样 `decimation_x/y=40` 后，输出的 CameraInfo K 矩阵和宽高未同步缩放 (仍为 960×640)，导致点云 3D 投影偏移
2. **base_state_to_odom**: C++ 和 Python 双版本，应清理只保留一个
3. **相机定义不一致**: `th02_a7_description/urdf/` 和 `Simulation/models/themis/model.sdf` 中的相机参数 (FOV, 分辨率) 存在差异
4. **nav_launch**: 使用 `ExecuteProcess` 裸启命令而非 ROS 2 标准的 `IncludeLaunchDescription`
