# THEMIS Navigation Workspace

> **规则**: 任何代码修改 (Edit/Write) 必须先经用户确认同意，不得擅自改动。分析问题可以自由做，但改代码要问过。

## 工作区结构

```
nav_ws/
├── src/
│   ├── nav2_custom_plugins/        # 旧 MPPI 插件 ([vx,vy,omega], 仅供参考)
│   ├── nav2_custom_plugins_v2/     # 新 MPPI 插件 ([vx,vy,delta], 核心模块已完成)
│   ├── nav_launch/                 # 启动配置 & YAML 参数
│   ├── velocity_controller/        # 底层速度控制器 (Python)
│   └── ...
```

## nav2_custom_plugins_v2 — 架构

### 控制空间

`[vx, vy, omega]` — 1:1 THEMIS 模型:
- **vx, vy**: body 系线速度 (前/左), 运动学积分时按当前 theta 旋转到世界系
- **omega**: body 系角速度, 直接积入 theta

### 文件结构

```
nav2_custom_plugins_v2/
├── include/nav2_custom_plugins_v2/
│   ├── core/mppi_core.hpp               # 参数/类型/运动学
│   ├── gpu/gpu_engine.hpp               # GPU 缓冲区 + kernel 启动
│   ├── gpu/gpu_uploader.hpp             # GPU 缓冲区注册 + 上传
│   ├── pipeline/mppi_pipeline.hpp       # CPU→GPU 桥接层
│   ├── mppi_steering_controller.hpp     # Nav2 插件入口
│   └── modules/
│       ├── cost_evaluator.hpp           # 代价后处理 (CPU 端)
│       ├── path_manager.hpp
│       ├── velocity_postprocessor.hpp
│       ├── state_machine.hpp
│       ├── visualization.hpp
│       └── mppi_logger.hpp
│
├── src/
│   ├── cost/                            # 代价函数 — 领域逻辑, GPU/CPU 共用
│   │   ├── critic_common.cuh            # CriticCategory 枚举 + CriticBase 基类
│   │   ├── obstacle_critic.cuh          # OBSTACLE: FootprintCritic + ObstacleCategory
│   │   ├── heading_critic.cuh           # HEADING:  PathAlign + PathAngle + PathDeviation
│   │   ├── speed_critic.cuh             # SPEED:    SpeedRewardCritic + SpeedCategory
│   │   └── critic_manager.cuh           # 3 大分类容器 + 大类间加权求和
│   │
│   ├── gpu/                             # GPU 基础设施
│   │   ├── mppi_gpu_common.cuh          # costmap_bilinear, normalize_angle 等工具
│   │   ├── mppi_gpu_kernels.cu          # cost_eval_kernel, weighted_sum_kernel
│   │   ├── mppi_gpu_critics.cuh         # [兼容转发] → cost/critic_manager.cuh
│   │   ├── gpu_engine.cpp
│   │   └── gpu_uploader.cpp
│   │
│   ├── core/mppi_core.cpp
│   ├── pipeline/mppi_pipeline.cpp
│   ├── modules/path_manager.cpp         # 路径查询 (最近点/前瞻/yaw)
│   ├── modules/velocity_postprocessor.cpp  # 控制量后处理 (提取+clamp+δ→ω)
│   ├── modules/state_machine.cpp          # heading 原地旋转判定 (迟滞退出)
│   ├── modules/visualization.cpp          # RViz Marker 发布 (轨迹/前瞻/指令)
│   ├── mppi_steering_controller.cpp
│
├── test/test_critics.cu                 # 代价函数调用链测试
└── msg/VelocitySteering.msg
```

### 模块状态

| 模块 | 状态 | 说明 |
|------|------|------|
| mppi_core | ✅ | 参数/MppiParams, 类型 (BatchTrajectories, ControlSequence), 运动学, PathInfo/GoalInfo |
| gpu_engine | ✅ | GPU 缓冲区管理, kernel 启动 (launchCostKernel, launchWeightedSumKernel) |
| gpu_uploader | ✅ | 缓冲区名→指针映射, 异步上传 (cudaMemcpyAsync), uploadPath |
| pipeline | ✅ | CPU→GPU 桥接: uploadBase, uploadRollout, launchCost, launchWeightedSum |
| cost 系统 | ✅ | 代价函数架构 — 3 大类, 函数指针注册表, 两层归一化 |
| path_manager | ✅ | 最近点查询 (增量), 前瞻点计算, yaw 来源切换 (planner/自算), GPU 数据构建 |
| steering_controller | ✅ | configure/computeVelocityCommands, 含 steering publisher, use_planner_yaw 参数 |
| cost_evaluator | ⏸️ 搁置 | 当前功能只有 min_element+除法 (~10行), 不足以成模块, 等自适应 lambda/代价直方图 |
| velocity_postprocessor | ✅ | 提取+clamp+δ→ω, 解耦自 steering_controller 末尾 |
| state_machine | ✅ | heading 原地旋转判定 (迟滞退出: 进入=threshold, 退出=threshold×0.5) |
| visualization | ✅ | RViz MarkerArray: 机器人朝向/前瞻点/最优轨迹/采样散布/速度箭头 |
| mppi_logger | TODO | |

---

## 代价系统架构 (cost/)

### 设计原则

- GPU 上不能用虚函数 (全局内存两次访存 + 无法内联)
- 采用**函数指针注册表**实现运行时多态 — 兼顾灵活性和性能
- 三层调用链: `CriticManager` → `XxxCategory` → `XxxCritic::compute()`

### 类层级

```
CriticBase                              (critic_common.cuh — 顶级标签基类)
├── ObstacleCritic : CriticBase         (obstacle_critic.cuh — OBSTACLE 分类基类)
│   └── FootprintCritic : ObstacleCritic ← 已实现: 足迹采样碰撞检测
│       (DistanceFieldCritic)           ← 未来
│
├── HeadingCritic : CriticBase          (heading_critic.cuh)
│   ├── PathAlignCritic                ← 点到路径最近距离
│   ├── PathAngleCritic                ← 朝向对齐推荐朝向 (normalize_angle, 无退火)
│   └── PathDeviationCritic            ← 走廊偏离软墙
│
└── SpeedCritic : CriticBase            (speed_critic.cuh)
    ├── SpeedRewardCritic              ← 速度方向对齐前瞻点
    └── TerminalDistCritic             ← 终点距离 (kernel 内直接计算, 非注册表)
```

### 容器/管理器层级 (不在 CriticBase 继承链中)

```
CriticManager                           (critic_manager.cuh)
├── ObstacleCategory                    (obstacle_critic.cuh)
│     subs_[0] = { footprintFn, true, 1.0f }   ← FootprintCritic
│     evaluate() → 遍历注册表 → 函数指针直调 → 加权平均
│
├── HeadingCategory                     (heading_critic.cuh)
│     subs_[0..2] = { pathAlignFn, pathAngleFn, pathDeviationFn }
│
└── SpeedCategory                       (speed_critic.cuh)
      subs_[0] = { speedRewardFn, true, 1.0f }   ← SpeedRewardCritic
```

### 调用链 (kernel 每步)

```
cost_eval_kernel (mppi_gpu_kernels.cu)
  CriticManager mgr; mgr.init();
  for each timestep:
    mgr.evaluate(x, y, θ, vx, vy, cmap, fp, path, goal)
      │
      ├── cat_w[0] × obstacle_.evaluate(x, y, cos, sin, vx, vy, cmap, fp)
      │     → FootprintCritic: 足迹网格采样 → costmap 双线性插值 → n⁴ 碰撞惩罚
      │
      ├── cat_w[1] × heading_.evaluate(x, y, θ, path)
      │     → PathAlignCritic:     1-exp(-dist²)        [0,1)
      │     → PathAngleCritic:     1-exp(-err²)         [0,1)
      │     → PathDeviationCritic: 1-exp(-excess²)      [0,1), corridor=0.5m
      │
      └── cat_w[2] × speed_.evaluate(vx, vy, goal)
            → SpeedRewardCritic: sigmoid(speed×(3·|err|-1))  [0,1)

  (循环后) total += cat_w[SPEED] × sqrt((gx-x)² + (gy-y)²)  ← TerminalDistCritic
```

### 归一化 (三层)

1. **大类内平均** — 各 Category::evaluate(): `Σ(w × fn) / active_count`
2. **大类间加权** — CriticManager::evaluate(): `Σ cat_weights[c] × category.evaluate()`
3. **horizon 归一化** — kernel: `d_costs[s] = total / H` (保证不同 horizon 代价可比)

默认: cat_weights = {OBSTACLE:0.4, HEADING:0.3, SPEED:0.3}

所有子类返回值已指数归一化到 [0,1):
  PathAlign:        1-exp(-dist²)       (0m→0, 1m→0.63)
  PathAngle:        1-exp(-err²)        (0°→0, 45°→0.46)
  PathDeviation:    1-exp(-excess²)     (corridor=0.5m, 超出0.5m→0.22)
  SpeedReward:      sigmoid(raw)        (对齐→0.4, 中性→0.5, 背离→0.9)
  FootprintCritic:  n⁴ 平均            (原已 [0,1])

### 与虚函数多态的对比

| | 本系统 | C++ 虚函数 |
|---|---|---|
| 分发方式 | 注册表条目 → 函数指针 | 对象 vptr → 虚表 |
| 元数据 (weight/enabled) | 和 fn 指针打包在同一 SubEntry | 需额外结构 |
| GPU 开销 | 1 次栈上指针解引用 | 2 次 global memory 访存 |
| 内联 | 不可 (函数指针) | 不可 (虚调用) |
| 运行时灵活性 | ✅ setWeight/setEnabled | ✅ 替换对象 |
| 子类 stateless | ✅ 临时构造, 零开销 | 需要持久对象 |

### 新增代价子类步骤

给 OBSTACLE 加 DistanceFieldCritic:
1. 写子类: `class DistanceFieldCritic : public ObstacleCritic { compute() }`
2. 写 wrapper: `static float distanceFn(...) { D c; return c.compute(...); }`
3. 注册: `init()` 加 `subs_[1] = { distanceFn, true, 0.4f }`
→ CriticManager 不动

### FootprintCritic 算法概要

- 足迹离散化 nx×ny (钳位 [2,10]) → 旋转矩阵变换到世界系
- costmap 双线性插值 → val ≥ 1.0 做 n⁴ 惩罚 (n=val/255)
- 倒车时 (lx<0, vx<0) 垫高 rear_obstacle_cost
- 取全足迹平均, 代价 ∈ [0,1]

### 推荐朝向来源 (use_planner_yaw)

| 开关 | 来源 | 说明 |
|------|------|------|
| `true` | `pose.orientation` | Planner 给的 SE2 朝向 (navfn 含平滑+时间一致性优化) |
| `false` | `atan2(dy, dx)` | 自算相邻路径点切线方向 |

前瞻点 = 最近路径点沿路径前推 `min_lookahead_dist`。推荐朝向取前瞻点处的 yaw。

### GPU 数据结构

| 结构 | 使用者 | 内容 |
|------|--------|------|
| CostmapInfo | OBSTACLE | costmap 数据指针 + 尺寸 + 分辨率 + 原点 |
| Footprint | OBSTACLE | 前后左右尺寸 + 采样间距 + 倒车垫高值 |
| PathInfo | HEADING | 路径点 GPU 指针 + 点数 + 前瞻点切线 + 终点朝向 |
| GoalInfo | SPEED | 期望速度方向 + 终点坐标 |

### HEADING 状态机 (state_machine)

```
evaluateHeading(current_yaw, lookahead_yaw, dist_lh_to_goal, params)
  │
  ├── enable_heading_speed_limit=false → 直接放行 (NORMAL)
  │
  ├── 已在旋转中 (rotating_=true):  迟滞退出
  │     abs(err) ≤ threshold × 0.5 → 退出旋转
  │     else → 继续旋转
  │
  └── 首次判定:
        abs(err) > threshold → HEADING_MISALIGN
        else → NORMAL

HEADING_MISALIGN 时:
  - dec.omega = current_yaw + err → [-π,π] 最短路径目标角度
  - base_link 模式: angular.z = sign(err) × max_w
  - global 模式:   angular.z = dec.omega
  - 跳过整个 MPPI 管线 (noise gen / rollout / GPU)
```

### 输出模式 (use_global_mode)

| | base_link (false) | global (true) |
|---|---|---|
| frame_id | `BASE_LINK` | `odom` |
| linear.x/y | vx/vy 原样 | vx/vy 原样 (运动模型 x+=vx·dt, 已是全局量) |
| angular.z | ω = clamp(normalize(δ-θ), ±max_w·dt)/dt | delta (目标角度直出) |

### 全部可配参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| num_samples | int | 8000 | 轨迹采样数 |
| prediction_horizon | int | 5 | 预测步数 |
| dt | double | 0.05 | 积分步长 (s) |
| max_v / min_v | double | 0.4 / -0.4 | vx 限幅 |
| max_vy | double | 0.2 | vy 限幅 |
| max_steering_angle | double | 0.785 | delta 限幅 (rad) |
| max_w | double | 0.6 | 最大角速度 (rad/s) |
| action_std_v / vy / delta | double | 0.5 / 0.5 / 0.3 | 噪声标准差 |
| lambda | double | 4.0 | MPPI 温度 |
| footprint_* | double | 0.17~0.28 | 足迹尺寸 (m) |
| min_lookahead_dist | double | 0.3 | 前瞻距离 (m) |
| use_planner_yaw | bool | true | true=planner 朝向, false=自算切线 |
| use_global_mode | bool | false | true=odom 系输出, false=base_link 系 |
| enable_heading_speed_limit | bool | true | 启用 heading 状态机 |
| heading_misalign_threshold | double | 1.047 | 朝向偏差阈值 (rad, ≈60°) |

---

## 运动学

```
vx, vy 为 body 系 (前/左), 积分时旋转到世界系:
  x += (vx·cos θ - vy·sin θ) · dt
  y += (vx·sin θ + vy·cos θ) · dt
  theta += clamp(omega, ±max_w) · dt

坐标系一致性:
  - rollout vx, vy: body 系 (前/左)
  - GoalInfo target_vx_r, target_vy_r: body 系 (dir_to_lh - yaw)
  - NoiseGenerator 偏置: body 系 (path_yaw - current_yaw)
  - PostProcessor global 模式: body→odom 旋转 (vx_out = vx·c - vy·s)
  - PostProcessor base_link 模式: 原样输出 (body 系)
```

## 消息

- 输出: `VelocitySteering.msg` (vx, vy, steering_angle) → `/cmd_vel_steering`
- 兼容: `TwistStamped` 返回给 Nav2 controller_server
- 生成的 msg 命名空间: `nav2_custom_plugins_v2::msg` (与主代码共享外层 namespace)

---

## 每帧数据流 (computeVelocityCommands)

```
setPlan(path) → path_mgr_.setPath, state_machine_.reset

① start = {x, y, yaw} from pose
② path_mgr_: closest → lookahead → yaw (planner/自算) → PathInfo/GoalInfo
②b state_machine_.evaluateHeading → rotate_in_place?
      ├── YES → 直接输出旋转指令, return (跳过 MPPI)
      └── NO  ↓
③ noise_gen_.generate(N, H, lookahead_yaw)
④ batch_rollout(start, base_seq, noise, params) → BatchTrajectories
⑤ uploadBase + uploadRollout → GPU
⑥ uploadPath → GPU, fill PathInfo device pointers
⑦ CostmapInfo + Footprint
⑧ launchCostKernel(cmap, fp, path, goal) → d_costs[N] (/H 归一化)
⑨ min_cost = min(costs)
⑩ launchWeightedSumKernel → result[H×4]
⑪ vel_postprocessor_.process(result, yaw, global_mode)
      → extractStep0 → clamp → (δ→ω or passthrough)
⑫ base_seq_.shiftAndDecay + fill proc.control
⑬ TwistStamped: frame_id, linear.x/y, angular.z
```

## 开发原则

- 旧包 `nav2_custom_plugins` 仅作算法参考, 不拷贝代码
- 全部模块从头编写
- GPU 部分完全重构, 不沿用旧架构
- GPU 上禁用虚函数, 用函数指针注册表实现多态
- 每个功能模块独立文件 (`src/modules/`), controller 只做编排
- 每次修改后 git commit
