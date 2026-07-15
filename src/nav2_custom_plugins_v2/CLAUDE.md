# THEMIS Navigation Workspace

## 工作区结构

```
nav_ws/
├── src/
│   ├── nav2_custom_plugins/        # 旧 MPPI 插件 ([vx,vy,omega], 仅供参考)
│   ├── nav2_custom_plugins_v2/     # 新 MPPI 插件 ([vx,vy,delta], 开发中)
│   ├── nav_launch/                 # 启动配置 & YAML 参数
│   ├── velocity_controller/        # 底层速度控制器 (Python)
│   ├── navigation/                 # 导航配置
│   └── ...
```

## nav2_custom_plugins_v2 — 架构

### 控制空间

`[vx, vy, delta]` — 解耦模型:
- **vx, vy**: 全局初始朝向系线速度, 不随机器人朝向旋转
- **delta**: 目标身体朝向角, 以 max_w 限速跟踪, 不影响线速度方向

### 模块 (9 个, 组合优于继承)

```
MPPISteeringController (Nav2 接口 + 流程编排)
├── mppi_core              参数 + 类型 + 运动学
├── path_manager            路径解析、前瞻点、最近点
├── cost_evaluator          代价后处理
├── velocity_postprocessor  EMA→减速→钳位→δ→ω
├── gpu_engine              GPU 封装 (噪声/缓冲区/内核)
├── state_machine           状态机 (heading/narrow)
├── visualization           RViz 可视化
└── mppi_logger             日志+统计
```

### 运动学

```
位置 (全局系, 无需旋转):
  x += vx * dt
  y += vy * dt

朝向 (max_w 限速向 delta 靠拢):
  dtheta = clamp(normalize(delta - theta), ±max_w*dt)
  theta += dtheta

δ → ω (TwistStamped 兼容):
  omega = clamp(normalize(delta - theta) / dt, ±max_w)
```

### 消息

- 输出: `VelocitySteering.msg` (vx, vy, steering_angle) → `/cmd_vel_steering`
- 兼容: `TwistStamped` 返回给 Nav2 controller_server

---

## 旧包 GPU 加载流程分析 (参考)

旧包 `nav2_custom_plugins` 的 `MPPIGPUController` 是重构参考。以下是其 GPU 加载流程的详细分析。

### 概念: CPU 和 GPU 各有一片内存

```
┌──────────────┐     ┌──────────────┐     ┌──────────────┐
│   CPU 主机内存 │ ←──→│   PCIe 总线   │ ←──→│   GPU 显存    │
│  host_noise  │ ────── cudaMemcpy ────→ │ d_noise_     │
│  host_costs  │ ←──── cudaMemcpy ──── │ d_costs_     │
└──────────────┘     └──────────────┘     └──────────────┘
```

### 阶段 0: 启动 (configure, 调一次)

```
configure()
  ├─ 读 YAML → 成员变量 (num_samples=8000, max_v=0.4, ...)
  └─ allocateGPUBuffers() → cudaMalloc 在显存圈 15 块地
```

分配的 GPU 缓冲区:

| 名称 | 大小 | 用途 |
|------|------|------|
| d_noise_vx/vy/w_ | 8000×5×4B = 160KB ×3 | 噪声 (CPU预生成, 每帧上传) |
| d_base_vx/vy/w_ | 5×4B = 20B ×3 | warm-start 基序列 |
| d_sampled_vx/vy/w_ | 8000×5×4B = 160KB ×3 | 内核输出: 实际采用的控制量 |
| d_costs_ | 8000×4B = 32KB | 每条轨迹总代价 |
| d_result_seq_ | 5×4×4B = 80B | 加权平均最优序列 |
| d_traj_x/y_ | 8000×5×4B = 160KB ×2 | 轨迹世界坐标 |
| d_path_x/y_ | 30×4B = 120B ×2 | 全局路径点 |

d_costmap_ 不在这里分配, 每帧按需 cudaMalloc/cudaFree。

### 阶段 1: 每帧 CPU 准备 (computeVelocityCommands, 10Hz 调用)

**1.1 warm-start 基序列**

```
首帧: 用机器人到目标的朝向差初始化
  optimal_vx_seq_  = [cos(angle)*speed ×5]
  optimal_vy_seq_  = [sin(angle)*speed ×5]
  optimal_omega_seq_ = [angle/dt ×5]

后续帧: 左移一位, 尾部×0.5
  [t0, t1, t2, t3, t4] → [t1, t2, t3, t4, t4×0.5]
```

**1.2 噪声生成 (CPU, NLN 混合)**

```cpp
for (40000 次 = 8000轨迹 × 5步):
  35% → 对数正态 (重尾大跳, 探索)
  65% → 正态分布 (小幅稳定)

noise_vx[i] = 采样值;  noise_vy[i] = 采样值;  noise_w[i] = 采样值;
```

输出三个 `vector<float>`, 各 40000 个元素 (480KB), 存在主机内存。

**1.3 路径提取 + costmap 准备**

路径重采样 → host_path_x/y (最多30点)。costmap 合并 local+global。

### 阶段 2: GPU 上传+计算 (host wrapper 内)

**2.1 上传 (cudaMemcpyAsync, 全部异步)**

```
noise_vx → d_noise_vx_     40000×4B = 160KB
noise_vy → d_noise_vy_     160KB
noise_w  → d_noise_w_      160KB
base_vx  → d_base_vx_      20B
base_vy  → d_base_vy_      20B
base_w   → d_base_w_       20B
costmap  → d_costmap_      按地图尺寸
path_x/y → d_path_x/y      120B
```

**2.2 内核: mppi_sample_kernel**

```
<<<(8000+255)/256=33 blocks, 256 threads>>>
= 8000 个 CUDA 线程完全并行

每线程 (一条轨迹):
  for t = 0..4:
    1. 采样: vx=clamp(bvx[t]+noise_vx[t]*scale, min_v, max_v)
    2. RK2积分: x+=dx*dt; y+=dy*dt; theta+=omega*dt
    3. 代价: obstacle(碰撞检测) + tracking(路径偏离) + speed(速度方向)
  写回 d_costs_[线程号], d_sampled_* [线程号×5..]
```

**2.3 内核是异步的** — host wrapper return 时 GPU 可能还在跑。stream 保证顺序 (上传→内核→下载)。

### 阶段 3: 下载结果 (两次同步等待)

```
① cudaMemcpyAsync(d_costs_ → host_costs, 32KB)
   cudaStreamSynchronize(stream)        ← 阻塞! 等 GPU

② CPU 扫 8000 个代价, 找 min_cost

③ mppi_weighted_sum_kernel<<<...>>>     ← 又一次内核
   对 5 个时间步, 各用 exp(-(cost-min)/lambda) 加权平均 8000 条轨迹的控制量

④ cudaMemcpyAsync(d_result_seq_ → host_result_seq, 80B)
   cudaStreamSynchronize(stream)        ← 又阻塞!

⑤ cudaStreamDestroy(stream)
```

### 阶段 4: CPU 后处理

```
提取 step0: best_vx/vy/omega = host_result_seq[0..2]
钳位 → EMA平滑 → 前瞻减速 → 死区保护
→ 写入 TwistStamped (返回给 Nav2)
→ 存 optimal_*_seq_ 供下帧 warm-start
```

### 阶段 5: 关闭 (cleanup, 调一次)

```
freeGPUBuffers() → 15 个 cudaFree
```

### 一帧时间线

```
CPU: [噪声] [基序列] [costmap] ─上传─ 空闲...... ─下载─ [等] [找min] [加权] [等] [后处理] [发布]
GPU:                        空闲. ─上传─ [8000线程5步rollout] ─下载─ [8000线程加权] ─下载─ 空闲.
                              ←── CPU/GPU 交替忙, 无法真正并行 ──→
```

### 问题

1. **host wrapper 参数 40+ 个**: 加一个参数要改声明+调用+内核签名, 三个地方
2. **CPU 生成噪声 → PCIe 上传**: 480KB 噪声每帧走 PCIe, GPU 可以自己生成
3. **两次 Synchronize**: CPU 干等 GPU 两次
4. **costmap 手动管理**: 合并 local+global 做了 ~100 行, 尺寸变化时手动 free+malloc
5. **CUDA API 散落**: cudaMalloc/Memcpy/Stream 直接写在 controller.cpp 业务逻辑里
6. **无封装**: 没有 GPU 资源管理类, 所有细节暴露在 controller 中

---

## 开发原则

- 旧包 `nav2_custom_plugins` 仅作算法参考, 不拷贝代码
- 全部分模块从头编写
- GPU 部分完全重构, 不沿用旧架构
- 每次修改后 git commit
