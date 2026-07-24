# MPPI Steering Controller (THEMIS)

基于 GPU 加速的 MPPI（Model Predictive Path Integral）控制器，用于 Nav2 导航框架，采用解耦 `[vx, vy, omega]` 控制空间。

## 概述

本插件替代 Nav2 默认控制器，使用基于采样的 MPPI 优化器。在每个 10Hz 控制周期中，生成 **N** 条轨迹（每条 **H** 步），在 GPU 上并行评估多类代价函数，最终输出指数加权的控制序列。

### 核心特性

- **解耦控制空间** — `[vx, vy, omega]` 均为 body 系。vx/vy 控制线速度（前/左），omega 独立控制角速度。
- **GPU 加速** — CUDA kernel 并行评估全部 N×H 个轨迹步：代价地图足迹碰撞检测、路径对齐、朝向角度、速度奖励，以及 MPPI 加权求和。
- **Warm-start 基序列** — 复用上一帧最优控制序列作为本帧采样基线，提升时序一致性。
- **朝向状态机** — MPPI 前预判：朝向偏差超过阈值时跳过 MPPI，直接输出原地旋转指令。
- **终端朝向对齐** — 接近终点时带迟滞的朝向对齐逻辑。
- **前瞻点减速** — 接近前瞻点时平滑缩放速度。
- **双输出模式** — `global` 模式在 odom 系下发目标角度；`base_link` 模式下发角速度。
- **RViz 可视化** — 通过 `/mppi_visualization` 发布 MarkerArray，包含机器人位姿、前瞻点、采样轨迹、最优轨迹、速度指令。
- **代价分量日志** — 可选每帧代价分解写入文件，方便调参。

## 架构

```
┌─────────────────────────────────────────────────────┐
│                  Nav2 Controller API                 │
│         computeVelocityCommands(pose, vel)           │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│              MPPISteeringController                  │
│  ┌─────────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ PathManager │  │StateMach.│  │ NoiseGenerator │  │
│  │ (最近点,    │  │ (朝向    │  │ (CPU, NLN)     │  │
│  │  前瞻点,    │  │  预判)   │  │                │  │
│  │  推荐朝向)  │  │          │  │                │  │
│  └─────────────┘  └──────────┘  └────────────────┘  │
│                                                      │
│  ┌──────────────────────────────────────────────┐    │
│  │              MPPIPipeline (CPU↔GPU 桥接)      │    │
│  │  ┌───────────┐  ┌───────────┐  ┌──────────┐  │    │
│  │  │ GPUEngine │  │GPUUploader│  │CriticMgr │  │    │
│  │  │ (缓冲区,  │  │ (memcpy,  │  │ (代价    │  │    │
│  │  │  kernel)  │  │  注册)    │  │  评估)   │  │    │
│  │  └───────────┘  └───────────┘  └──────────┘  │    │
│  └──────────────────────────────────────────────┘    │
│                                                      │
│  ┌─────────────────┐  ┌────────────────────────┐     │
│  │ VelocityPostProc│  │ VisualizationPublisher │     │
│  │ (提取,钳位,    │  │ (MarkerArray → RViz)   │     │
│  │  δ→ω, 坐标系变换)│  │                        │     │
│  └─────────────────┘  └────────────────────────┘     │
└──────────────────────────────────────────────────────┘
```

## 文件结构

```
├── include/nav2_custom_plugins_v2/
│   ├── mppi_steering_controller.hpp   # Nav2 插件入口
│   ├── core/mppi_core.hpp             # 类型、参数、运动学、噪声、rollout
│   ├── gpu/gpu_engine.hpp             # GPU 缓冲区管理 + kernel 启动
│   ├── gpu/gpu_uploader.hpp           # GPU 异步上传 (base, rollout, path)
│   ├── pipeline/mppi_pipeline.hpp     # CPU↔GPU 桥接层
│   └── modules/
│       ├── param_loader.hpp           # YAML → MPPIParams
│       ├── path_manager.hpp           # 最近点、前瞻点、推荐朝向、路径信息
│       ├── velocity_postprocessor.hpp # 提取、钳位、δ→ω、坐标系变换
│       ├── state_machine.hpp          # 朝向偏差 → 原地旋转判定
│       └── visualization.hpp          # RViz MarkerArray 发布器
├── src/
│   ├── mppi_steering_controller.cpp   # 主控制循环 (computeVelocityCommands)
│   ├── core/mppi_core.cpp             # Rollout、运动学、控制序列
│   ├── gpu/
│   │   ├── gpu_engine.cpp             # GPU 缓冲区分配、kernel 启动器
│   │   ├── gpu_uploader.cpp           # 缓冲区注册、cudaMemcpyAsync 封装
│   │   └── mppi_gpu_kernels.cu        # CUDA kernel: cost_eval, weighted_sum
│   ├── cost/                          # 代价函数 (详见下方"代价系统设计"章节)
│   ├── modules/                       # 模块实现
│   └── pipeline/mppi_pipeline.cpp     # Pipeline 胶水代码
├── test/test_critics.cu               # 代价函数单元测试
├── msg/VelocitySteering.msg           # 自定义消息: vx, vy, steering_angle
├── CMakeLists.txt                     # 构建配置 (CUDA + ROS 2)
├── plugins.xml                        # Nav2 插件注册
└── package.xml                        # ROS 2 包清单
```

## 控制流 (10Hz)

```
① 读取位姿 → RobotState {x, y, yaw}
② PathManager: findClosestIndex → computeLookahead → getYaw (planner / 自算切线)
②b StateMachine: evaluateHeading → 若 |err| > 阈值:
      → 直接输出旋转指令, 跳过 MPPI
③ NoiseGenerator: 生成 N×H NLN 噪声 (向前瞻方向偏置)
④ batch_rollout: u = base + noise → 运动学积分 → N 条轨迹
⑤ uploadBase → GPU (warm-start 序列, H×4 float)
⑥ uploadRollout → GPU (轨迹, N×H×6 float)
⑦ uploadPath → GPU (路径点)
⑧ 组装 CostmapInfo + Footprint + PathInfo + GoalInfo
⑨ launchCost → GPU cost_eval_kernel → d_costs[N]
⑩ CPU 扫描: min_cost = min(costs), best_idx = argmin
⑪ launchWeightedSum → GPU kernel → result[H×4] (加权平均控制量)
⑫ VelocityPostProcessor: 提取 step0 → 钳位 → δ→ω (base_link) 或旋转 (global)
⑬ 前瞻减速: scale = kp + (1−kp) × min(1, dist/decel_dist)
⑭ 终端对齐: 若 dist < 阈值, 直接下发 goal_yaw
⑮ base_seq_.shiftAndDecay(0.5) + 填入新控制量
⑯ 发布 TwistStamped + RViz markers
```

## 代价函数

在 GPU 上通过三级层次结构评估代价：

| 大类 | 权重 | 子代价 | 描述 |
|------|------|--------|------|
| **OBSTACLE** (障碍物) | 0.60 | FootprintCritic | 足迹网格采样 → 代价地图双线性插值 → n⁴ 碰撞惩罚 |
| **HEADING** (路径跟踪) | 0.30 | PathAlignCritic | 到最近路径段的距离平方 |
| | | PathAngleCritic | 4 × (朝向误差²), 接近终点时余弦退火 |
| | | PathDeviationCritic | 超出走廊宽度的偏离量 (软墙) |
| **SPEED** (速度) | 0.10 | SpeedRewardCritic | THEMIS 公式: 对齐奖励 + 侧向抑制 (×6) |
| | | BaseSimilarityCritic | 时序一致性: (u − base_u[t])² 正则项 |

### 代价公式

```
total = cat_w[OBSTACLE] × Σ(w_sub × footprint_cost)
      + cat_w[HEADING]  × Σ(w_sub × (align + angle + deviation))
      + cat_w[SPEED]    × Σ(w_sub × (speed_reward + base_similarity))

d_costs[s] = cost_scale × (total / H) + overshoot_penalty + terminal_dist
```

### SpeedRewardCritic (THEMIS 公式)

```
speed = hypot(vx, vy)
若 speed < 0.02 → 中性 (0)
alignment = (vx·target_vx_r + vy·target_vy_r) / speed   // cos(误差)
lateral   = |vx·target_vy_r − vy·target_vx_r| / speed    // |sin(误差)|
若 alignment < 0 → speed × 5.0                            // 反向重罚
返回 −min(speed, max_feasible_v) × alignment + 6.0 × speed × lateral
```

### 代价系统设计

GPU 上禁用了虚函数（无 vtable），采用**函数指针注册表**实现运行时多态。三层调用链：

```
CriticManager → XxxCategory → XxxCritic::compute()
```

#### 类层级

```
CriticBase                              (critic_common.cuh)
├── ObstacleCritic : CriticBase         (obstacle_critic.cuh)
│   └── FootprintCritic                 — 足迹网格 → costmap 双线性插值 → n⁴ 碰撞
├── HeadingCritic : CriticBase          (heading_critic.cuh)
│   ├── PathAlignCritic                — 到路径段最近距离的平方
│   ├── PathAngleCritic                — 4×err², 接近终点时余弦退火
│   └── PathDeviationCritic            — 超出走廊宽度的偏离量 (软墙)
└── SpeedCritic : CriticBase            (speed_critic.cuh)
    ├── SpeedRewardCritic              — THEMIS 对齐奖励 + 侧向抑制
    └── BaseSimilarityCritic           — 时序一致性: (u − base_u[t])²
```

#### 容器结构

```
CriticManager                           (critic_manager.cuh)
├── ObstacleCategory  — subs_[0] = {footprintFn}
├── HeadingCategory   — subs_[0..2] = {pathAlignFn, pathAngleFn, pathDeviationFn}
└── SpeedCategory     — subs_[0..1] = {speedRewardFn, baseSimilarityFn}
```

每个大类权重 (`obstacle_ratio` / `tracking_ratio` / `speed_ratio`) 与大类内部的加权和相乘。

#### 两层归一化

- **第一层** — 大类内: 加权和 `Σ(w_sub × 函数输出)`, 不取平均
- **第二层** — 大类间: `cat_weights[c] × category.evaluate(...)`

```
total = Σ cat_weights[c] × category.evaluate(...)
```

#### Kernel 每步调用

```
CriticManager::evaluate(x, y, θ, vx, vy, ω, cmap, fp, path, goal)
  ├── cat_w[OBSTACLE] × obstacle_.evaluate()
  │     → FootprintCritic: 足迹网格 → costmap 双线性插值 → n⁴ 碰撞
  ├── cat_w[HEADING]  × heading_.evaluate()
  │     → PathAlign:      到路径段的最近距离平方 (m²)
  │     → PathAngle:      4×err² (原始平方, rad²)
  │     → PathDeviation:  excess², excess = max(0, dist − 0.5m)
  └── cat_w[SPEED]   × speed_.evaluate()
        → SpeedReward:      THEMIS 公式 (对齐奖励 + 侧向 ×6)
        → BaseSimilarity:   (vx−base_vx)² + (vy−base_vy)² + (ω−base_ω)²
```

#### GPU 数据结构

| 结构体 | 大类 | 字段 |
|--------|------|------|
| `CostmapInfo` | OBSTACLE | `data` 指针, `w`, `h`, `res`, `origin_x`, `origin_y` |
| `Footprint` | OBSTACLE | `front`, `back`, `left`, `right`, `sample_spacing`, `rear_obstacle_cost` |
| `PathInfo` | HEADING | `x`/`y` 指针 (GPU 显存), `num_pts`, `path_tangent`, `goal_yaw`, `goal_x`, `goal_y` |
| `GoalInfo` | SPEED | `target_vx_r`, `target_vy_r`, `max_feasible_v`, `goal_x`, `goal_y`, `lookahead_x`, `lookahead_y`, `lookahead_overshoot_weight` |
| `CriticParams` | 全部 | 3 个大类权重 + 6 个子权重 (见上方参数表) |

#### 新增代价子类

以 HEADING 大类为例：

1. 写子类: `class NewCritic : public HeadingCritic { float compute(...) { ... } }`
2. 写 wrapper: `static float newCriticFn(...) { NewCritic c; return c.compute(...); }`
3. 注册: 在 `HeadingCategory::init()` 中添加 `subs_[N] = {newCriticFn, true, weight}`
4. `CriticManager` 无需修改 — 新代价自动生效。

## MPPI 更新

对全部轨迹做指数加权平均：

```
w[s] = exp(−(cost[s] − min_cost) / λ)

result[t].vx     = Σ w[s] × batch.vx[s,t] / Σ w[s]
result[t].vy     = Σ w[s] × batch.vy[s,t] / Σ w[s]
result[t].omega  = Σ w[s] × batch.omega[s,t] / Σ w[s]
```

取第一步 (`t=0`) 的结果作为控制指令下发。

## 参数

所有参数从 Nav2 controller YAML 配置中读取，位于插件命名空间下。

### 控制空间

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `num_samples` | int | 8000 | 轨迹采样数 (N) |
| `prediction_horizon` | int | 5 | 轨迹步数 (H) |
| `dt` | double | 0.05 | 积分时间步长 (s) |
| `max_v` | double | 0.4 | 最大前进速度 (m/s) |
| `min_v` | double | -0.4 | 最大后退速度 (m/s) |
| `max_vy` | double | 0.2 | 最大侧向速度 (m/s) |
| `max_w` | double | 0.6 | 最大角速度 (rad/s) |

### 噪声

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `action_std_v` | double | 0.2 | vx 噪声标准差 |
| `action_std_vy` | double | 0.1 | vy 噪声标准差 |
| `action_std_w` | double | 0.4 | omega 噪声标准差 |

### 代价权重

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `lambda` | double | 0.05 | MPPI 温度系数 |
| `cost_scale` | double | 50.0 | 整体代价缩放 |
| `obstacle_ratio` | double | 0.70 | OBSTACLE 大类权重 |
| `tracking_ratio` | double | 0.20 | HEADING 大类权重 |
| `speed_ratio` | double | 0.05 | SPEED 大类权重 |
| `footprint_weight` | double | 1.0 | Footprint 子代价权重 |
| `path_align_weight` | double | 1.0 | PathAlign 子代价权重 |
| `path_angle_weight` | double | 2.0 | PathAngle 子代价权重 |
| `path_deviation_weight` | double | 0.3 | PathDeviation 子代价权重 |
| `speed_reward_weight` | double | 1.0 | SpeedReward 子代价权重 |
| `base_similarity_weight` | double | 0.5 | BaseSimilarity 子代价权重 |

### 足迹

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `footprint_front` | double | 0.17 | 前方超出 (m) |
| `footprint_back` | double | 0.17 | 后方超出 (m) |
| `footprint_left` | double | 0.28 | 左侧超出 (m) |
| `footprint_right` | double | 0.28 | 右侧超出 (m) |

### 前瞻 & 终点

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `min_lookahead_dist` | double | 0.8 | 最小前瞻距离 (m) |
| `lookahead_kp` | double | 0.3 | 前瞻点最低速度比例 (0=停止, 1=不减速) |
| `lookahead_decel_dist` | double | 0.5 | 开始减速的距离 (m) |
| `lookahead_overshoot_weight` | double | 5.0 | 超越前瞻点的惩罚系数 |
| `terminal_angle_dist` | double | 0.3 | 触发终端对齐的距离 (m) |
| `terminal_angle_tolerance` | double | 0.17 | 对齐容差 (rad, ≈10°) |

### 状态机

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `enable_heading_speed_limit` | bool | true | 启用 MPPI 前的朝向预判 |
| `heading_misalign_threshold` | double | 1.047 | 朝向误差阈值 (rad, ≈60°) |

### 输出模式

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `use_global_mode` | bool | true | true=odom 系 (目标角度); false=base_link 系 (角速度) |
| `use_planner_yaw` | bool | true | true=使用 planner 朝向; false=自算切线朝向 |

### 日志

| 参数 | 类型 | 默认值 | 描述 |
|-----------|------|---------|-------------|
| `enable_file_log` | bool | true | 启用代价分量日志 |
| `log_file_path` | string | /tmp/mppi_steering_controller.log | 日志文件路径 |

## 依赖

- **ROS 2** (Humble 或更高版本)
- **Nav2** (nav2_core, nav2_costmap_2d, nav2_util)
- **CUDA Toolkit** (≥11.0)
- **GPU** 计算能力 ≥8.0 (SM80: A100; SM86: RTX 30xx; SM89: RTX 40xx; SM90: H100; SM120: Blackwell)
  > **注意**: 请根据实际设备调整 `CMakeLists.txt` 中的 `CUDA_ARCHITECTURES`：
  > - 部分设备的 SM 版本未在上方列出，可通过 `nvidia-smi --query-gpu=compute_cap --format=csv` 查询
  > - CUDA 版本过旧时（如 CUDA 11.x），`sm120` 会报 `Unsupported gpu architecture` 错误，需删除该值
  > - 例如：笔记本 RTX 4060 为 SM89，Jetson Orin 为 SM87，RTX 5090 为 SM120
- **pluginlib**, **tf2_ros**, **tf2_geometry_msgs**

## 编译

```bash
cd ~/your_ws
colcon build --packages-select nav2_custom_plugins_v2 --cmake-args -DCMAKE_BUILD_TYPE=Release
```

构建产物：
- `libnav2_custom_plugins_v2_gpu.so` — 插件动态库
- `test_critics` — GPU 代价函数单元测试

## 使用方式

在 Nav2 controller server 配置中：

```yaml
controller_server:
  ros__parameters:
    controller_plugin_ids: ["MPPISteeringController"]
    MPPISteeringController:
      plugin: "nav2_custom_plugins_v2/MPPISteeringController"
      num_samples: 8000
      prediction_horizon: 5
      dt: 0.05
      max_v: 0.4
      max_vy: 0.2
      max_w: 0.6
      # ... (以上全部参数均可配置)
```

## 发布的 Topic

| Topic | 类型 | 描述 |
|-------|------|-------------|
| `/cmd_vel_mppi` | `geometry_msgs/Twist` | MPPI 原始输出 (未经 Nav2 平滑) |
| `/mppi_visualization` | `visualization_msgs/MarkerArray` | RViz 调试可视化 |

## 输出模式

### Global 模式 (`use_global_mode: true`)

- `frame_id`: `"odom"`
- `linear.x`, `linear.y`: odom 系速度 (后处理器将 body 系 vx/vy 旋转到 odom 系)
- `angular.z`: **目标朝向角度**，范围 `[0, 2π)`
- 适用于下游节点单独处理朝向跟踪的场景（如独立的 steering controller）

### Base-Link 模式 (`use_global_mode: false`)

- `frame_id`: `"BASE_LINK"`
- `linear.x`, `linear.y`: body 系速度
- `angular.z`: **角速度**，单位 rad/s，钳位在 `[-max_w, max_w]`
- 标准 Nav2 TwistStamped 接口

## 调参指南

1. **避障过于激进**: 降低 `obstacle_ratio` 或 `cost_scale`
2. **机器人切弯**: 增大 `path_align_weight` 或 `path_angle_weight`
3. **速度过慢**: 增大 `speed_ratio` 或 `speed_reward_weight`
4. **侧向抖动 (vy 横跳)**: 增大 `base_similarity_weight`，降低 `action_std_vy`
5. **探索噪声过大**: 降低 `action_std_v`、`action_std_vy`、`action_std_w`
6. **超越前瞻点**: 增大 `lookahead_overshoot_weight` 或 `min_lookahead_dist`
7. **终点前提早停止**: 降低 `lookahead_kp` 或 `lookahead_decel_dist`
8. **终点附近朝向振荡**: 调整 `terminal_angle_tolerance` 和 `terminal_angle_dist`
9. **MPPI 温度**: `lambda` 越小 → 输出越贪婪（softmax 尖锐）；越大 → 输出越均匀（softmax 平坦）

启用 `enable_file_log: true` 后，可查看 `/tmp/mppi_steering_controller.log` 获取每帧代价分量分解：

```
cost: total=X.XX | angle=X.XX(w=X.XX) align=X.XX(w=X.XX) speed=X.XX(w=X.XX) obst=X.XX(w=X.XX) | ...
output: ctrl_omg=X.XX out_omg=X.XX ctrl_vx=X.XX ctrl_vy=X.XX global=1
```

## 性能

以下数据在 **NVIDIA RTX 5060 Laptop** (SM 120, 26 SMs, 7.5 GB, CUDA 12.0) 上测得。  
运行方式: `./build/nav2_custom_plugins_v2/benchmark_mppi`

### GPU Kernel 吞吐（默认配置: N=8000, H=5）

| Kernel | 耗时 | 吞吐 |
|--------|------|------|
| `cost_eval_kernel`（代价评估） | **0.076 ms** | 5.24 亿轨迹步/秒 |
| `weighted_sum_kernel`（softmax 加权） | **0.152 ms** | 2.63 亿权重计算/秒 |
| **每帧 GPU 总耗时** | **~0.23 ms** | — |

### GPU vs CPU 对比（H=5, 20 线程 OpenMP）

| N | GPU (ms) | CPU 单线程 (ms) | CPU 20线程 (ms) | GPU 加速比 (vs 20t) |
|---|----------|-----------------|------------------|----------------------|
| 1000 | 0.064 | 2.78 | 0.27 | **4×** |
| 4000 | 0.066 | 10.37 | 6.98 | **106×** |
| 8000 | 0.077 | 20.77 | 7.48 | **97×** |
| 16000 | 0.114 | 42.22 | 7.02 | **62×** |

### 关键结论

- **当前配置下 GPU 远未达到瓶颈。** N 从 1000 增至 16000（16 倍工作量），代价评估耗时仅从 0.064ms 升至 0.114ms（1.8 倍），26 个 SM 远未饱和。
- **每帧 GPU 预算仅 0.23 ms** — 仅占 10Hz 控制周期（100ms）的 **0.23%**。
- **未来计算有巨大扩展空间:**
  - 更高采样数（N=32000+，更深的探索覆盖）
  - 更长预测视野（H=20+，更远的规划前瞻）
  - 稠密语义代价地图（车道线、可通行性评分）
  - 学习型代价函数（CNN 编码特征、视觉 critic）
  - 多传感器融合（LiDAR 点云、深度图作为 GPU 纹理）
  - 更丰富的动力学模型实时轨迹优化
- 默认配置 (N=8000) 下，GPU 比 20 线程 CPU 快 **~100 倍**，CPU 核心可完全释放给感知、定位等 Nav2 其他组件。

### GPU 显存占用（默认配置: N=8000, H=5）

| 组件 | 大小 |
|------|------|
| 轨迹缓冲区 | 1.56 MB |
| 代价地图 (200×200) | 0.04 MB |
| 路径 (50 点) | <0.01 MB |
| **合计** | **1.60 MB** |

显存占用随 N×H 线性增长 — N=16000, H=10 时为 6.2 MB。即使 4 GB 显存的入门级 GPU 也绰绰有余。