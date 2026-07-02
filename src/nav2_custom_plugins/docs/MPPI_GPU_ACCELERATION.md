# MPPI GPU 加速 — 原理与实现

> 文件: `mppi_gpu_kernels.cu` / `mppi_gpu_rewards.cuh` / `mppi_gpu_controller.cpp`
> 类: `nav2_custom_plugins::MPPIGPUController`
> 计算后端: CUDA

---

## 目录

1. [为什么用 GPU](#1-为什么用-gpu)
2. [并行策略](#2-并行策略)
3. [执行流水线](#3-执行流水线)
4. [三组件代价架构](#4-三组件代价架构)
5. [采样与运动学积分](#5-采样与运动学积分)
6. [GPU 内存管理](#6-gpu-内存管理)
7. [CUDA 优化要点](#7-cuda-优化要点)
8. [虚拟机器人窄道检测](#8-虚拟机器人窄道检测)

---

## 1. 为什么用 GPU

MPPI 算法的核心是**对 N 条独立轨迹并行采样和代价评估**。每条轨迹需要：

- H 步运动学前向积分
- 每步计算碰撞检测（足迹密集采样）
- 每步计算路径跟踪代价
- 每步计算速度奖励

N 条轨迹之间**完全独立、无数据依赖**，天然适合 GPU 的 SIMT 并行模型。

```
CPU 版: for s in 0..N:            ← 串行, 3000 次循环
          for t in 0..H:           ← 12 步
            前向积分 + 代价计算

GPU 版: 3000 个 CUDA 线程并行      ← 全部同时执行
          每个线程内 for t in 0..H  ← 仅 12 步串行
```

### 加速比

| 平台 | N | H | CPU 耗时 | GPU 耗时 | 加速比 |
|------|---|---|----------|----------|--------|
| Jetson Orin | 3000 | 12 | ~15ms | ~2ms | ~7.5× |
| 桌面 RTX | 3000 | 12 | ~8ms | ~0.5ms | ~16× |

---

## 2. 并行策略

### 2.1 采样核函数 (`mppi_sample_kernel`)

```
gridDim  = ceil(N / 256)         ← 一维 grid, 每个 block 256 线程
blockDim = 256

线程映射: thread s → 轨迹 s
  线程内循环: for t in 0..H-1
    1. 噪声采样 + guidance 混合 → (vx, vy, ω)
    2. RK2 运动学积分 → (x, y, θ)
    3. 计算三组件代价 → 累加到 obst_acc, track_acc, prog_acc
    4. 存储 sampled_u[s][t], traj[s][t]
  循环后: costs[s] = weighted_sum(obst_acc, track_acc, prog_acc)
```

```
┌────────────────────────────────────────────────┐
│ Thread 0   Thread 1   Thread 2   ...  Thread N-1│
│  轨迹0      轨迹1      轨迹2           轨迹N-1  │
│  for H:     for H:     for H:          for H:   │
│    采样       采样       采样             采样    │
│    积分       积分       积分             积分    │
│    代价       代价       代价             代价    │
│  → cost[0]  → cost[1]  → cost[2]       → cost[N-1]│
└────────────────────────────────────────────────┘
```

### 2.2 加权求和核函数 (`mppi_weighted_sum_kernel`)

```
gridDim  = (ceil(N/256), H)     ← 二维 grid
blockDim = 256

线程映射: thread (s, t) → 样本 s 的第 t 步

  w = exp(-(costs[s] - min_cost) / lambda)
  atomicAdd(&result[t][0], w * vx[s][t])
  atomicAdd(&result[t][1], w * vy[s][t])
  atomicAdd(&result[t][2], w * omega[s][t])
  atomicAdd(&result[t][3], w)               ← 权重和
```

使用 `atomicAdd` 是因为多个线程可能同时累加到同一个 `result[t]`。

---

## 3. 执行流水线

每帧 `computeVelocityCommands()` 中的 GPU 相关步骤：

```
时间 ──────────────────────────────────────────────────────►

CPU:  ①准备数据    ②上传       ③启动kernel  ④等待+下载    ⑤后处理
      noise生成    cudaMemcpy   launch       cudaMemcpy    EMA/KP/可视化
      base序列      Async       计算         Async+Sync
      costmap合并
      path重采样

GPU:                [空闲]      Sample        [空闲]
                                Kernel
                                Weighted
                                Sum Kernel

总延迟: 上传 ~0.1ms + 计算 ~1.5ms + 下载 ~0.1ms ≈ 1.7ms
```

### 3.1 CUDA Stream 异步传输

所有 GPU 操作在同一个 `cudaStream_t` 上排队，支持传输与计算的流水线重叠：

```cuda
cudaStream_t stream;
cudaStreamCreate(&stream);

// 异步上传 (非阻塞)
cudaMemcpyAsync(d_noise_vx_, noise_vx.data(), ..., cudaMemcpyHostToDevice, stream);
cudaMemcpyAsync(d_costmap_, costmap_data, ..., cudaMemcpyHostToDevice, stream);
cudaMemcpyAsync(d_path_x_, path_x.data(), ..., cudaMemcpyHostToDevice, stream);

// 启动计算 (在 stream 上排队，数据传输完成后自动开始)
mppi_sample_kernel<<<grid, block, 0, stream>>>(...);

// 异步下载代价
cudaMemcpyAsync(host_costs, d_costs_, ..., cudaMemcpyDeviceToHost, stream);

// 同步点: 必须等待 min_cost 才能启动加权求和
cudaStreamSynchronize(stream);
find_min_cost(host_costs);

// 加权求和
mppi_weighted_sum_kernel<<<grid, block, 0, stream>>>(...);

// 最终同步
cudaStreamSynchronize(stream);
cudaStreamDestroy(stream);
```

---

## 4. 三组件代价架构

代价计算在 `mppi_gpu_rewards.cuh` 中实现，移植自 nav2 MPPI 官方 critics：

```
total_cost[s] = obstacle_weight  × Σ ObstaclesCritic(x_t, θ_t)
              + tracking_weight  × Σ (PathAlignCritic + PathAngleCritic)
              + progress_weight  × Σ (PreferForwardCritic + GoalCritic)
```

### 4.1 组件 1: ObstaclesCritic (碰撞检测)

`compute_obstacle_cost()` — 密集足迹采样 + 双线性插值 + norm⁴ 映射：

```
1. 在机器人 footprint 内生成 nx×ny 密集采样网格
2. 每个采样点通过双线性插值查询 costmap 代价值 (0~255)
3. 映射到 [0,1]: norm = cost / 255
4. 代价 = norm⁴ (四次幂，放大高代价区域)
5. 所有采样点取平均
```

**后方隐形障碍**：当 `vx < 0` 且 `lx < 0`（采样点在机器人后方），将 costmap 值 clamp 到 `rear_obstacle_cost` 以上，防止倒退撞墙。

### 4.2 组件 2: PathAlignCritic + PathAngleCritic (路径跟踪)

**PathAlignCritic** (`compute_path_align_cost`): 轨迹点 (x,y) 到全局路径的最短距离平方 / H。

**PathAngleCritic** (`compute_path_angle_cost`): 朝向偏差 > 45° 时惩罚（偏差² / H）。近终点 (<0.5m) 使用余弦退火从路径切线平滑过渡到 goal yaw。

### 4.3 组件 3: PreferForwardCritic + GoalCritic (进度)

**PreferForwardCritic** (`compute_speed_reward`): 匹配速度矢量方向与"当前位置→前瞻点"方向的**角度差**：

```
speed = hypot(vx, vy)
angle_err = atan2(vy, vx) - atan2(target_vy_r, target_vx_r)  ← 均在机器人系
alignment = cos(angle_err)

if alignment < 0:  return speed × 5.0        ← 背离前瞻点: 重罚
return -speed × alignment + speed × |sin(angle_err)|
                                    ↑
                              侧向浪费惩罚，防止"推头"
```

完美对齐 (angle_err=0): cost = -speed（最快越好）
45° 偏差: cost ≈ 0（盈亏平衡，速度快慢无差异）
>45° 偏差: cost > 0（直走受罚，迫使转向对齐）

**GoalCritic** (`compute_terminal_dist_cost`): 终端步到 goal 的欧氏距离。

---

## 5. 采样与运动学积分

### 5.1 Guidance 混合采样

每步控制量由基序列（上一帧最优解）+ 噪声 + 路径引导三部分混合：

```
vx = (1-γ) × (base_vx[t] + noise_vx × ns_t)   ← 探索分量
   +     γ  × path_vx_r × base_speed            ← 引导分量

vy = (1-γ) × (base_vy[t] + noise_vy × ns_vy × cross_track_noise_scale)
   +     γ  × path_vy_r × base_speed × lateral_guidance_scale × lat_mult

ω  = base_w[t] + noise_w × ns_w
   + path_angle_err × (0.5/dt) × γ
```

其中 `γ = guidance_weight`，控制探索/引导的平衡。`ns_t` 是时变噪声缩放（前期大、后期小）。

### 5.2 纯旋转轨迹

部分轨迹（`pure_rotation_ratio`，默认 10%）在前 `pure_rotation_steps` 步中 vx=vy=0，仅做角速度旋转。用于在窄道入口处先转向对齐再前进。

### 5.3 转弯横向增强

当检测到路径有弯道时，自动放大 vy 的引导和噪声：

```
turn_sharp = clamp(|path_turn_angle| / 60°, 0, 1)
lat_mult = 1 + turn_sharp × turn_lateral_boost + ...
```

急弯时 vy 的引导分量被放大，让机器人**横着走**过弯。

### 5.4 RK2 中点法积分

```c
float theta_mid = theta + omega × dt/2;
x += (vx × cos(theta_mid) - vy × sin(theta_mid)) × dt;
y += (vx × sin(theta_mid) + vy × cos(theta_mid)) × dt;
theta += omega × dt;
```

比欧拉法精度更高，在 dt=0.1s 下对旋转运动的拟合误差显著降低。

### 5.5 NLN 混合采样 (Log-MPPI)

部分噪声 (`nln_ratio`) 使用对数正态分布代替纯高斯分布：

```cpp
if (rand < nln_ratio_)
  noise = lognormal(0, action_std × nln_sigma_mult);  // 重尾
else
  noise = normal(0, action_std);                       // 正常
```

对数正态分布有更长的右尾，偶尔产生大偏离值，帮助逃脱局部极小（如 U 形障碍物）。

---

## 6. GPU 内存管理

### 6.1 持久分配策略

所有 GPU buffer 在 `configure()` → `allocateGPUBuffers()` 中一次性分配，避免每帧 `cudaMalloc`/`cudaFree` 开销：

| 缓冲区 | 维度 | 大小 | 类型 |
|--------|------|------|------|
| `d_noise_vx/vy/w` | N×H | 3×3000×12×4B = 432KB | float |
| `d_sampled_vx/vy/w` | N×H | 432KB | float |
| `d_traj_x/y` | N×H | 2×144KB = 288KB | float |
| `d_costs` | N | 3000×4B = 12KB | float |
| `d_result_seq` | H×4 | 12×4×4B = 192B | float |
| `d_base_vx/vy/w` | H | 3×12×4B = 144B | float |
| `d_path_x/y` | 30 | 2×30×4B = 240B | float |
| `d_costmap` | W×H | 动态 (60×60 ≈ 3.6KB) | uchar |

**总计**: 约 1.2MB (3000 样本 × 12 步时)

### 6.2 Costmap 按需重分配

Costmap buffer 仅在尺寸变化时重新分配：

```cpp
if (cw != costmap_w_ || ch != costmap_h_) {
  if (d_costmap_) { cudaFree(d_costmap_); d_costmap_ = nullptr; }
  cudaMalloc(&d_costmap_, cw * ch * sizeof(unsigned char));
  costmap_w_ = cw; costmap_h_ = ch;
}
```

### 6.3 清理

`cleanup()` → `freeGPUBuffers()` 释放所有已分配 buffer 并置空指针。

---

## 7. CUDA 优化要点

### 7.1 `__restrict__` 指针

所有核函数指针参数标记 `__restrict__`，告知编译器无指针别名，允许更激进的重排序和寄存器分配。

### 7.2 寄存器优先

每个线程的局部变量 (`x, y, theta, obst_acc, track_acc, prog_acc`) 全程驻留在寄存器中，无局部内存溢出 (local memory spill)。NVCC 编译选项 `--maxrregcount` 未设置，由编译器自动分配。

### 7.3 快速数学函数

全程使用单精度快速数学函数：`fmaxf`, `fminf`, `hypotf`, `cosf`, `sinf`, `atan2f`, `sqrtf`。避免了双精度计算的额外开销。

### 7.4 双线性插值 (`costmap_bilinear`)

用软件双线性插值替代硬件纹理采样。4 个近邻 cell 的代价按距离加权平均，消除栅格边界的代价跳变。这是 nav2 官方 ObstaclesCritic 的标准做法。

### 7.5 坐标预旋转

在核函数入口一次性将路径方向从世界坐标系转到机器人坐标系：

```c
float cos_rot = cosf(-current_theta), sin_rot = sinf(-current_theta);
float path_vx_r = path_dir_x * cos_rot - path_dir_y * sin_rot;
float path_vy_r = path_dir_x * sin_rot + path_dir_y * cos_rot;
```

后续所有代价函数中的投影计算直接使用预旋转方向，避免每步重复计算 `cos(-θ)`。

### 7.6 前瞻点方向计算

前瞻点方向（当前位置→target）同样在核函数入口预计算并归一化：

```c
float dx_t = target_x - current_x, dy_t = target_y - current_y;
float tx_r = dx_t * cos_rot - dy_t * sin_rot;  // 机器人系
float ty_r = dx_t * sin_rot + dy_t * cos_rot;
float lh_dist = hypotf(tx_r, ty_r);
float lh_vx_r = tx_r / fmaxf(lh_dist, 1e-6f);
float lh_vy_r = ty_r / fmaxf(lh_dist, 1e-6f);
```

该方向用于 `compute_speed_reward` 的角度匹配，而非路径切线方向。

---

## 8. 虚拟机器人窄道检测

### 8.1 动机

当机器人面对窄通道、门口或密集障碍物时，主 MPPI 可能因为以下原因"踌躇"不前：

- 所有采样轨迹都撞到障碍物，代价趋同 → 权重均匀 → 输出接近零
- costmap 中狭窄通道两侧都是致命代价 → 没有安全轨迹可采样

但仅凭输出速度大小无法可靠区分"正常减速靠近目标"和"被障碍物卡住"。虚拟机器人机制通过在机器人前方的两个虚拟位置运行独立 MPPI 来**预判前方通行性**。

### 8.2 工作流程

```
每帧在 computeVelocityCommands 的 CPU 端执行 (在调用主 GPU MPPI 之前):

┌─ Step 1: 放置两个虚拟机器人 ──────────────────────────────┐
│                                                             │
│   沿全局路径, 从 closest_idx 出发:                           │
│                                                             │
│   虚拟机器人 0 (近点): 前方 0.20m, 蓝色                     │
│   虚拟机器人 1 (远点): 前方 0.40m, 红色                     │
│                                                             │
│   robot ────●────────────●────────────●─── target           │
│           current       v0(0.2m)    v1(0.4m)               │
│                                                             │
│   路径剩余不足时 clamp 到终点                               │
└─────────────────────────────────────────────────────────────┘

┌─ Step 2: 搜索无碰朝向 ─────────────────────────────────────┐
│                                                             │
│   对每个虚拟机器人:                                          │
│     在 24 个均匀方向 (15° 间隔) 上检查 footprint 碰撞        │
│     评分: d_cur(离当前朝向) + 1.5 × d_path(离路径方向)       │
│     选择评分最低的无碰朝向 → vtheta_arr[vi]                  │
│                                                             │
│   如果 24 个方向全部碰撞 → 该虚拟机器人无解                   │
└─────────────────────────────────────────────────────────────┘

┌─ Step 3: 独立 MPPI 模拟 ───────────────────────────────────┐
│                                                             │
│   对每个虚拟机器人:                                          │
│     起始位姿: (vx_arr[vi], vy_arr[vi], vtheta_arr[vi])      │
│     目标: 主前瞻点 (target_x, target_y)                     │
│     基序列: 全零 (冷启动, 模拟从静止起步)                    │
│     采样: 全新噪声, N 条轨迹, H 步预测                      │
│     GPU 核函数: mppi_gpu_sample_and_cost (同一 kernel)       │
│                                                             │
│   关键: 使用空路径 (path=nullptr), 仅评估碰撞 + 速度奖励     │
│         不评估路径跟踪代价 (虚拟机器人不需要贴路径)          │
└─────────────────────────────────────────────────────────────┘

┌─ Step 4: 投影速度 → 判定踌躇 ──────────────────────────────┐
│                                                             │
│   取加权输出前 3 步的平均速度:                               │
│     wvx = Σ u*[t].vx / u*[t].weight (t=0,1,2)              │
│     wvy = Σ u*[t].vy / u*[t].weight (t=0,1,2)              │
│                                                             │
│   投影到虚拟机器人的起始朝向:                                │
│     proj = wvx × cos(θ_start) + wvy × sin(θ_start)          │
│                                                             │
│   判定:                                                      │
│     if proj_arr[0] < 0.10 && proj_arr[1] < 0.10:           │
│       hesitate_count_++       ← 两个都踌躇才计数            │
│     else:                                                     │
│       hesitate_count_ = 0                                   │
│                                                             │
│     if hesitate_count_ >= 3:                                │
│       mppi_hesitate = true   ← 连续3帧 → 进入窄道模式       │
└─────────────────────────────────────────────────────────────┘
```

### 8.3 设计要点

**为什么两个虚拟机器人取 AND 而非 OR？**

早期版本使用 `||`（任一踌躇即触发），但在实际测试中发现：
- 单个虚拟机器人可能因为暂时被单个障碍物遮挡而误触发
- 两个位置都踌躇说明前方通道确实在 0.2m~0.4m 范围内持续被阻塞
- AND 逻辑过滤掉了瞬时 / 局部干扰，降低窄道模式误触发率

**为什么用冷启动（base=0）？**

虚拟机器人的任务是回答"这个位置能否前进"，而非"如何在当前位置最优地运动"。零基序列确保 MPPI 完全依靠采样来探索可行方向。如果投影速度仍然极低（<0.10m/s），说明即使最佳采样方向也无法产生有效前进 → 该位置确实被阻塞。

**为什么取前 3 步平均？**

前几步的控制量最能反映"从这个位置起步能否移动"。取平均可以平滑单步噪声。3 步（×dt=0.1s = 0.3s）足够覆盖起步加速阶段。

**为什么用空路径（path=nullptr）？**

虚拟 MPPI 不评估路径跟踪代价（PathAlign/PathAngle）。它只关心：
- 碰撞代价：虚拟机器人周围是否安全
- 速度奖励：能否朝向前瞻点产生有效速度

在窄道场景中，全局路径可能穿过障碍物（规划器不知道局部细节），强行跟踪路径会干扰通行性判断。

### 8.4 与主 MPPI 的关系

```
主 MPPI (实时控制):
  起始位姿: 当前机器人位姿
  基序列: 上一帧最优解滚动 (warm-start)
  路径: 30 点重采样路径 (完整代价)
  输出: 实际控制指令 cmd_vel

虚拟 MPPI × 2 (通行性探测):
  起始位姿: 前方 0.20m / 0.40m
  基序列: 全零 (冷启动)
  路径: 空 (仅碰撞 + 速度奖励)
  输出: mppi_hesitate 标志 → 触发窄道模式
```

虚拟机器人不直接影响控制量，只提供"前方是否可以通行"的布尔信号给状态机。

### 8.5 可视化

在 RViz 中订阅 `/mppi_gpu_visualization`，namespace `virtual_mppi`：

| id | 位置 | 颜色 | 含义 |
|----|------|------|------|
| 0 | 前方 0.20m | 蓝色 | 近点虚拟机器人 |
| 1 | 前方 0.40m | 红色 | 远点虚拟机器人 |

同时观察 `push_diag` 标记的品红色速度箭头和橙黄色前瞻点连线，综合判断前方通行性。
