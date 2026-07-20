# MPPI GPU Controller — 算法与架构文档

> **包名**: `nav2_custom_plugins`  
> **类名**: `nav2_custom_plugins::MPPIGPUController`  
> **框架**: Nav2 (ROS 2) Controller Plugin  
> **计算后端**: CUDA GPU 加速  

---

## 目录

1. [整体架构](#1-整体架构)
2. [完整控制流程](#2-完整控制流程)
3. [MPPI 算法核心](#3-mppi-算法核心)
4. [GPU 硬件加速方法](#4-gpu-硬件加速方法)
5. [代价函数详解](#5-代价函数详解)
6. [附加安全层](#6-附加安全层)
7. [数据流与内存管理](#7-数据流与内存管理)
8. [参数总览](#8-参数总览)

---

## 1. 整体架构

```
┌─────────────────────────────────────────────────────┐
│                  Nav2 Controller Server              │
│  ┌───────────────────────────────────────────────┐  │
│  │         MPPIGPUController (本插件)             │  │
│  │                                                │  │
│  │  configure()                                   │  │
│  │  ├─ 参数加载 (MPPI + GPU + 安全层)              │  │
│  │  ├─ 订阅全局代价地图 /global_costmap/costmap    │  │
│  │  ├─ 分配 GPU 持久缓冲区 (16 个 buffer)          │  │
│  │  └─ 初始化 可视化发布者                         │  │
│  │                                                │  │
│  │  computeVelocityCommands()  ← 20Hz 调用        │  │
│  │  ┌──────────────────────────────────────────┐  │  │
│  │  │ ① 前置处理 (CPU)                         │  │  │
│  │  │ ② 终端角度对准检测 (CPU, 提前返回)        │  │  │
│  │  │ ③ 前瞻点 + 全局路径重采样 (CPU)          │  │  │
│  │  │ ④ 横向偏好分析 (CPU, costmap 扫描)       │  │  │
│  │  │ ⑤ 全局代价地图合并 (CPU, tf2 变换)       │  │  │
│  │  │ ⑥ 噪声预生成 (CPU, 高斯分布)             │  │  │
│  │  │ ⑦ GPU 采样 + 代价计算 (CUDA)             │  │  │
│  │  │ ⑧ GPU 加权求和 (CUDA, 全时域)            │  │  │
│  │  │ ⑨ 后处理 (CPU)                           │  │  │
│  │  │    · EMA 低通滤波                         │  │  │
│  │  │    · 前瞻点 KP 减速                       │  │  │
│  │  │ ⑩ 可视化 + 统计数据记录                   │  │  │
│  │  └──────────────────────────────────────────┘  │  │
│  │                                                │  │
│  │  cleanup()                                     │  │
│  │  ├─ 写入统计数据 CSV 文件                       │  │
│  │  └─ 释放 GPU 缓冲区                            │  │
│  └───────────────────────────────────────────────┘  │
└─────────────────────────────────────────────────────┘

外部输入:
  ├─ 局部代价地图 (Costmap2DROS, odom 帧, 3m×3m 滚动窗口)
  ├─ 全局代价地图 (OccupancyGrid, map 帧, rtapmap/map)
  ├─ 全局路径   (Path, 来自 planner_server)
  ├─ 机器人位姿  (PoseStamped, odom 帧)
  └─ tf2 变换    (map→odom)
```

---

## 2. 完整控制流程

### 2.1 `configure()` — 初始化阶段

```
1. 存储 node、tf、costmap_ros 指针
2. 声明并读取 50+ 个 Ros 参数
3. 订阅全局代价地图 (默认 /global_costmap/costmap)
4. 创建可视化发布者 (/mppi_gpu_visualization)
5. 初始化高斯噪声生成器 (std::mt19937 + normal_distribution)
6. 分配 GPU 持久缓冲区 (cudaMalloc, 约 16 个 buffer)
```

### 2.2 `computeVelocityCommands()` — 每帧执行 (20Hz)

```
┌─ Step 0: 提取当前位姿 ──────────────────────────────┐
│ current_x, current_y, current_theta                   │
│ target_x, target_y = 当前位姿 (初始值)                 │
└──────────────────────────────────────────────────────┘

┌─ Step 1: 轨迹观测窗口维护 ───────────────────────────┐
│ 将当前位姿 + 朝向推入 trail_poses_ (deque)             │
│ 清理超出 observation_window_duration (5s) 的旧点       │
│ 后续用于 GPU 核函数中的 FOV 视锥置信度计算             │
└──────────────────────────────────────────────────────┘

┌─ Step 2: 终端角度对准检测 ◄ 提前返回 ───────────────┐
│ if dist_to_final_goal < terminal_angle_dist (0.10m): │
│   ├─ vx=0, vy=0                                     │
│   ├─ ω = terminal_angle_kp × yaw_err (P 控制器)      │
│   └─ return (跳过 GPU)                               │
└──────────────────────────────────────────────────────┘

┌─ Step 3: 前瞻点选取 ─────────────────────────────────┐
│ 沿全局路径从 closest_idx 出发累计距离                  │
│ 在 min_lookahead_dist 处线性插值 → 精确前瞻点          │
│ 帧间指数平滑 (α=0.35) 抑制抖动                        │
└──────────────────────────────────────────────────────┘

┌─ Step 4: 全局路径均匀重采样 ─────────────────────────┐
│ 从 closest_idx 到路径终点，按均匀弧长采样              │
│ MAX_PATH_POINTS=30 个点 → host_path_x/y               │
│ 上传到 GPU 用于 cross-track error 计算                 │
└──────────────────────────────────────────────────────┘

┌─ Step 5: 横向偏好分析 ───────────────────────────────┐
│ 在前瞻点处沿路径法向扫描 costmap (0.3~1.8m, 步0.3m)   │
│ 比较左 vs 右障碍物密度，带迟滞切换偏好方向              │
│ 将 path_direction 微旋 ±6° 偏向空闲侧                  │
│ 可通过 enable_lateral_bias 参数关闭                    │
└──────────────────────────────────────────────────────┘

┌─ Step 6: 全局代价地图合并 ───────────────────────────┐
│ if use_global_costmap && 有最新数据:                  │
│   ├─ 复制局部 costmap → merged_costmap                │
│   ├─ 查找 tf2: map→odom 变换                          │
│   ├─ 遍历局部 costmap 每个 cell                        │
│   │    odom_world → map_world → global_cell            │
│   │    merged[cell] = max(local, global)               │
│   └─ 用 merged_costmap.data() 替换 costmap_data        │
└──────────────────────────────────────────────────────┘

┌─ Step 7: 最优控制序列滚动 + 噪声生成 ────────────────┐
│ 滚动 shift: optimal[i] = optimal[i+1], 尾部 ×0.5 衰减  │
│ 高斯噪声: noise ~ N(0, action_std)                     │
│ N×H = 1000×10 = 10,000 个噪声样本/维                   │
└──────────────────────────────────────────────────────┘

┌─ Step 8: GPU 采样 + 代价计算 ────────────────────────┐
│ cudaMemcpy: noise, base_seq, costmap, path → GPU      │
│ cudaMemcpy: trail_poses → GPU                          │
│ mppi_sample_kernel<<<(N+255)/256, 256>>>               │
│   每个线程 = 1 条轨迹 × H 步前向预测                    │
│   总计 1000 条轨迹并行                                  │
│ 返回: d_costs[N], d_sampled_vx/vy/w[N×H], d_traj[N×H] │
└──────────────────────────────────────────────────────┘

┌─ Step 9: GPU 加权求和 ───────────────────────────────┐
│ 拷贝 d_costs → host, 查找 min_cost                     │
│ mppi_weighted_sum_kernel<<<(N/256+1, H), 256>>>        │
│   2D grid: x=样本, y=时域步                            │
│   每个线程: w_k = exp(-(cost[k]-min_cost)/λ)            │
│            u*_t = Σ w_k · u[k,t] / Σ w_k               │
│ atomicAdd 到 d_result_seq[H×4]                         │
└──────────────────────────────────────────────────────┘

┌─ Step 10: 后处理 ────────────────────────────────────┐
│ ① EMA 低通滤波 (α=0.1, ~0.25s 时间常数@20Hz)         │
│    cmd = α × raw + (1-α) × prev_cmd                  │
│                                                       │
│ ② 前瞻点 KP 减速                                      │
│    读取 costmap 在前瞻点 (min_lookahead_dist) 处的代价  │
│    scale = 1 - cost/254 × (1 - kp)                    │
│    代价越高速度越低，kp=0.5 时最低降至 50%             │
│    vx *= scale, vy *= scale                           │
│                                                       │
│ ③ 存储当前 u* 序列到 optimal_seq (下次滚动用)          │
└──────────────────────────────────────────────────────┘

┌─ Step 11: 可视化 + 统计 ─────────────────────────────┐
│ 从 GPU 拷贝代价最低的 10 条轨迹                        │
│ 发布 MarkerArray (/mppi_gpu_visualization)             │
│ recordStatsFrame() — 记录到 stats_frames_              │
│ cleanup() 时写入 CSV 文件                              │
└──────────────────────────────────────────────────────┘
```

---

## 3. MPPI 算法核心

### 3.1 数学原理

Model Predictive Path Integral (MPPI) 是一种基于采样的随机最优控制方法。

**状态**: `(x, y, θ)` — 2D 平面位姿  
**控制**: `(vx, vy, ω)` — 全向速度指令  
**模型**: 中点法 (RK2) 运动学积分

```
给定:
  - 基控制序列 u_base = {u_0, u_1, ..., u_{H-1}} (上一帧的最优序列 shift 而来)
  - H = 预测步数 (10)
  - N = 采样数量 (1000)

对每条轨迹 k ∈ [0, N):
  对每步 t ∈ [0, H):
    ε_k,t ~ N(0, σ²)      ← 高斯噪声
    u_k,t = u_base[t] + ε_k,t   ← 控制 = 基序列 + 噪声
    (可选: 路径 guidance 混合)

    前向仿真 H 步:  x_{t+1} = RK2(x_t, u_k,t, dt)
    计算累积代价 C_k = Σ_t cost(x_t, u_k,t) + terminal_cost(x_H)

  采样权重: w_k = exp(-(C_k - min_C) / λ)   ← λ = 温度参数 (5.0)

  最优控制序列: u*_t = Σ_k w_k · u_k,t / Σ_k w_k
  输出第一步: cmd = u*_0
```

### 3.2 关键设计点

| 设计 | 说明 |
|------|------|
| **滚动窗口** | 每帧 shift optimal_seq 一位，尾部衰减，形成 warm-start |
| **Guidance 混合** | 基序列 (时序平滑) 与路径方向 (空间准确) 的软平衡 |
| **噪声衰减** | 前期大噪声探索 (100%) → 末端收紧 (15%)，轨迹先发散后收束 |
| **全时域加权** | 对每个 timestep 独立加权求和 (而非仅取第一步)，保持时序一致性 |

---

## 4. GPU 硬件加速方法

### 4.1 并行策略

```
数据并行: N 条独立轨迹 → N 个 CUDA 线程

Sample Kernel:
  gridDim = ceil(N / 256)
  blockDim = 256
  线程 s 处理轨迹 s 的全部 H 步

Weighted Sum Kernel:
  gridDim = (ceil(N/256), H)  ← 2D grid
  blockDim = 256
  线程 (s,t) 处理样本 s 的第 t 步
```

### 4.2 GPU 内存布局

| 缓冲区 | 维度 | 大小 (float) | 说明 |
|--------|------|-------------|------|
| `d_noise_vx/vy/w` | N × H | 10,000 | 预生成的控制噪声 |
| `d_base_vx/vy/w` | H | 10 | 基控制序列 |
| `d_costmap` | W × H | 3,600 | 代价地图 (char) |
| `d_costs` | N | 1,000 | 每条轨迹的总代价 |
| `d_sampled_vx/vy/w` | N × H | 10,000 | 每步实际控制量 |
| `d_result_seq` | H × 4 | 40 | 时域加权结果 [vx,vy,w,sum_w] |
| `d_traj_x/y` | N × H | 10,000 | 每条轨迹的预测位置 |
| `d_path_x/y` | 30 | 30 | 重采样后的全局路径 |
| `d_trail_x/y/cos/sin` | 200 | 200 | 近期轨迹点 (观测窗口) |

**持久分配**: 所有 GPU buffer 在 `configure()` 时一次性分配，避免每帧 `cudaMalloc`/`cudaFree` 的开销。  
**按需重分配**: costmap buffer 仅在尺寸变化时重新分配。

### 4.3 CUDA Stream 异步流水线

```
cudaStream_t stream;

① cudaMemcpyAsync: noise → GPU (6 次, 非阻塞)
② cudaMemcpyAsync: costmap → GPU
③ cudaMemcpyAsync: path → GPU
④ cudaMemcpyAsync: trail → GPU (4 次)
⑤ mppi_sample_kernel<<<..., stream>>>     ← 计算与数据传输重叠
⑥ cudaMemcpyAsync: d_costs → host
⑦ cudaStreamSynchronize → 获取 min_cost
⑧ mppi_weighted_sum_kernel<<<..., stream>>>
⑨ cudaMemcpyAsync: d_result_seq → host
⑩ cudaMemcpyAsync: d_traj_x/y → host (可视化 TOP-10)
⑪ cudaStreamSynchronize → 全部完成
```

### 4.4 关键 GPU 优化

| 优化 | 位置 | 效果 |
|------|------|------|
| `__restrict__` 指针 | 核函数参数 | 告知编译器无别名，允许激进优化 |
| 双线性插值 (`costmap_bilinear`) | `__device__` 函数 | 纹理采样替代硬件插值，消除栅格边界代价跳变 |
| `fmaxf/fminf/hypotf` | 全核函数 | 单精度快速数学函数 |
| 寄存器变量 | 局部变量 | `x,y,theta,cost` 全程寄存器驻留，无全局内存溢出 |
| 坐标预旋转 | 核函数开头 | 路径方向一次性从世界坐标系转机器人坐标系 |

---

## 5. 代价函数详解

详见 [`MPPI_COST_FUNCTION.md`](MPPI_COST_FUNCTION.md)

**8 项逐步代价 (每步 H 次，/H 归一化)**:

| # | 名称 | 类型 | 权重 | 合并方式 |
|---|------|------|------|---------|
| ⓪ | 沿路径速度 | 奖励(−) | speed_reward_weight | — |
| ① | 目标渐进吸引 | 惩罚(+) | 0.2×proximity | — |
| 2a | 已知障碍物 | 惩罚(+) | costmap_weight | 8点取 **max** |
| 2b | 各向异性势能 | 惩罚(+) | 0.3×costmap_weight | 8点取 **加权max** |
| 2c | 分方向未知区域 | 惩罚(+) | rear/side/front | 按方向取 **加权max** |
| ③ | Cross-track error | 惩罚(+) | path_attraction_weight | — |
| ④ | 朝向一致性 | 惩罚(+) | heading_weight | — |
| ⑤ | 路径长度 | 惩罚(+) | 0.02 | — |

> **关键设计**: 碰撞箱碰撞代价 (2a/2b/2c) 在 8 个足迹点上取**加权最大值**而非求和，避免多障碍物场景下的代价叠加效应。距离中心越近的足迹点权重越高。

**2 项终端代价 (循环后一次性)**:

| # | 名称 | 类型 | 权重 |
|---|------|------|------|
| ⑥ | 终点距离 | 惩罚(+) | 1.0 |
| ⑦ | 终点朝向 | 惩罚(+) | 0.6×goal_proximity |

---

## 6. 两层控制架构

控制器按"距目标远近"将控制分为两层，由远及近：

```
┌────────────────────────────────────────────────────────────────┐
│ 层级 1: MPPI 全程                                                │
│   覆盖整个路径。GPU 并行采样 N 条轨迹，全时域加权求和，        │
│   输出平滑连续的控制序列。转弯时自动增强横向运动。             │
│   同时由前瞻点 KP 减速提供前馈安全制动。                       │
├────────────────────────────────────────────────────────────────┤
│ 层级 2: 终端角度对准 (dist_to_final < terminal_angle_dist_)    │
│   距离目标极近 (<0.10m)，跳过 GPU，纯 P 控制器对准 goal yaw。  │
│   朝向到位后完全停止 (omega=0)。带迟滞退出 (0.20m)。           │
└────────────────────────────────────────────────────────────────┘
```

**关键设计原则**:
- **MPPI 是唯一主力**：全程负责运动控制，保留避障能力直到终端
- **不再退化到纯追踪回退**：保持 MPPI 的避障能力直到终端
- **终端对准是终点**：纯旋转对齐 goal pose 朝向

### 6.1 层级 1: 前瞻点 KP 减速

```
条件: 始终启用，在 MPPI 输出 + EMA 之后执行
行为: 读取 costmap 在前瞻点 (min_lookahead_dist) 处的代价
      scale = 1 - cost/254 × (1 - kp)
      代价越高速度越低，kp=0.5 时最低降至 50%
      vx *= scale, vy *= scale
目的: 前馈安全制动，在进入高代价区域前主动减速
```

### 6.2 层级 2: 终端角度对准

```
条件: dist_to_final_goal < terminal_angle_dist_ (0.10m)
行为: 跳过 GPU，纯角度 P 控制器
      vx=vy=0, ω = terminal_angle_kp_ × yaw_err (clamp ±max_w)
      朝向到位 (<terminal_angle_tolerance_, 0.07rad) → ω=0
迟滞: 进入 0.10m，退出 0.20m，防止边界抖动
```

### 6.3 EMA 输出平滑

```
cmd = ema_alpha_ × raw + (1-ema_alpha_) × prev_cmd  (20Hz ≈ 0.25s 时间常数)
抑制帧间控制量跳变
```

### 6.4 转弯横向增强 (GPU Kernel 内)

```
检测路径转弯角度 → 急弯时自动放大 vy 引导/噪声并放松朝向约束
由 turn_lateral_boost 参数控制强度 (当前 2.0)
```

---

## 7. 数据流与内存管理

### 7.1 帧间数据流向

```
Frame t:
  optimal_seq[t]  ──────────────► base_seq (shift + tail decay)
  noise (CPU 新生成)             ──┐
  base_seq                       ──┼──► GPU Sample Kernel ──► costs[N]
  costmap (局部+全局合并)        ──┤                           sampled_u[N×H]
  path (重采样30点)              ──┤                           traj[N×H]
  trail (观测窗口)               ──┘
                                      │
  costs[N] ──► find min_cost ────────► GPU Weighted Sum ──► u*[H]
                                                                │
  u*[0] ──► EMA ──► KP减速 ──► cmd_vel                           │
  u*[H] ──► store to optimal_seq (用于 Frame t+1)               │
```

### 7.2 生命周期

| 资源 | 创建 | 销毁 |
|------|------|------|
| GPU buffers | `configure()` → `allocateGPUBuffers()` | `cleanup()` → `freeGPUBuffers()` |
| 全局代价地图订阅 | `configure()` | 自动 (rclcpp) |
| 可视化发布者 | `configure()` | 自动 (rclcpp) |
| 统计帧缓存 | 每帧累积 | `cleanup()` → `writeStatsToFile()` |
| 最优序列 | `computeVelocityCommands()` 首帧初始化 | 自动 (成员变量) |

---

## 8. 参数总览

### 8.1 核心 MPPI 参数

| 参数 | 默认 | 说明 |
|------|------|------|
| `num_samples` | 1000 | 每帧采样轨迹数 (GPU 线程数) |
| `prediction_horizon` | 10 | 预测步数 H |
| `dt` | 0.3 | 每步时间 (s), 总预测长度 H×dt=3.0s |
| `max_v` | 1.00 | 前向速度上限 (m/s) |
| `min_v` | -0.10 | 前向速度下限 (禁止大幅后退) |
| `max_vy` | 0.10 | 横向速度上限 (m/s) |
| `max_w` | 0.50 | 角速度上限 (rad/s) |
| `action_std_v` | 0.10 | vx 噪声标准差 |
| `action_std_vy` | 0.05 | vy 噪声标准差 |
| `action_std_w` | 0.10 | ω 噪声标准差 |
| `lambda` | 5.0 | MPPI 温度系数 (越大越均匀) |
| `collision_cost` | 2.0 | 障碍物代价权重 |

### 8.2 Guidance 与路径追踪参数

| 参数 | 默认 | 说明 |
|------|------|------|
| `guidance_weight` | 0.35 | 路径方向 vs base 序列混合比 |
| `cross_track_noise_scale` | 0.5 | 横向噪声缩放 |
| `noise_decay_rate` | 0.85 | 噪声时变衰减率 |
| `path_attraction_weight` | 0.15 | Cross-track error 惩罚权重 |
| `lateral_guidance_scale` | 0.15 | 横向 guidance 抑制 |
| `vel_direction_weight` | 0.15 | 速度方向惩罚 (已废弃) |
| `speed_reward_weight` | 6.0 | 前向速度奖励权重 |
| `heading_weight` | 0.8 | 朝向一致性权重 |
| `heading_anneal_start` | 1.5 | 余弦退火起始距离 (m) |
| `heading_anneal_end` | 0.2 | 余弦退火结束距离 (m) |
| `turn_lateral_boost` | 2.0 | 转弯横向增强 (0=关闭) |

### 8.3 前瞻与减速参数

| 参数 | 默认 | 说明 |
|------|------|------|
| `min_lookahead_dist` | 1.0 | 前瞻距离 (m) |
| `lookahead_time` | 0.0 | 前瞻时间 (s), 0=固定距离 |
| `lookahead_kp` | 0.5 | KP 减速系数 (0=停, 1=不减速) |

### 8.4 代价地图参数

| 参数 | 默认 | 说明 |
|------|------|------|
| `footprint_front/back/left/right` | 0.30/0.30/0.20/0.20 | 碰撞箱半尺寸 (m) |
| `unknown_cost_rear` | 1.0 | 后方未知代价 (全额) |
| `unknown_cost_side` | 0.7 | 侧方未知代价 |
| `unknown_cost_front` | 0.0 | 前方未知代价 (传感器覆盖) |
| `observation_window_duration` | 5.0 | 观测窗口时长 (s) |
| `trust_radius` | 0.5 | 信任半径 (m) |
| `observed_area_cost_scale` | 0.1 | 观测区未知折扣 |
| `fov_half_angle` | 55.0 | 相机 FOV 半角 (度) |
| `fov_range` | 3.5 | 有效观测距离 (m) |

### 8.5 终端对准参数

| 参数 | 默认 | 说明 |
|------|------|------|
| `ema_alpha` | 0.1 | EMA 平滑系数 |
| `terminal_angle_dist` | 0.10 | 终端对准触发距离 (m) |
| `terminal_angle_kp` | 1.5 | 终端角度 P 增益 |
| `terminal_angle_tolerance` | 0.07 | 终端角度到位阈值 (rad, ≈4°) |

### 8.6 功能开关

| 参数 | 默认 | 说明 |
|------|------|------|
| `enable_lateral_bias` | false | 横向偏好分析 (当前关闭) |
