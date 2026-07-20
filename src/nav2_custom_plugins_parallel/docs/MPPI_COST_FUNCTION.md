# MPPI GPU Controller — 代价函数全览

> 文件: `src/mppi_gpu_kernels.cu` → `mppi_sample_kernel()` + `src/mppi_gpu_rewards.cuh`
> 所有**逐步代价项**在 GPU 内核中每步计算，**统一除以 `horizon`** 归一化后累加。
> 终端项在循环后一次性加入，不除以 horizon。

---

## 一、控制量采样 (Control Sampling)

### Guidance 混合机制

```
vx = (1−γ) · (base_vx + noise_vx·scale) + γ · path_dir_x_robot · base_speed_clamped
vy = (1−γ) · (base_vy + noise_vy·cross_track_scale) + γ · path_dir_y_robot · base_speed_clamped · lateral_scale
ω  = base_w + noise_w·scale + blended_omega_error · (0.5/dt) · γ
```

| 参数 | 默认 | 作用 |
|------|------|------|
| `guidance_weight` (γ) | 0.2 | 0=纯base+噪声, 1=纯路径锚定 |
| `cross_track_noise_scale` | 1.5 | 横向噪声缩放 |
| `lateral_guidance_scale` | 0.15 | 横向 guidance 缩放，越小越抑制 vy |
| `noise_decay_rate` | 0.65 | 噪声随时间衰减 |

### 控制量 Clamp

```
vx ∈ [min_v, max_v],  vy ∈ [−max_vy, max_vy],  ω ∈ [−max_w, max_w]
```

### 平滑到达阻尼

| 阻尼 | 条件 | 效果 |
|------|------|------|
| **平滑到达阻尼** | dist_to_target < 0.4m | `vx *= √(dist/0.4)` — ease-out 减速 |

---

## 二、逐步代价（每 timestep 累加，统一除以 horizon 归一化）

### ⓪ 速度奖励 (Speed Reward) ◆ 奖励

```
cost += −max(0, speed_along_path) × speed_reward_weight / H
```

- `speed_along_path = vx · path_vx_robot + vy · path_vy_robot`  (速度在路径方向上的投影)
- 只奖励沿路径方向的前进分量，横向/后退不奖励
- **默认**: `speed_reward_weight` = 5.0

### ① 目标渐进吸引 (Goal Distance Cost) ◆ 惩罚

```
cost += ‖(x,y) − (target_x,target_y)‖ × (1 − t/H) × PATH_COST_WEIGHT / H
```

- 越靠近终点、越后期（`t/H → 1`），权重越小 → "先别急，后期再收束"
- **默认**: `PATH_COST_WEIGHT` = 0.3 (硬编码)

### ② 代价地图碰撞/膨胀 (Costmap Collision) ◆ 惩罚

**48 点密集足迹网格采样**，双线性插值查询 costmap。碰撞箱半尺寸决定网格覆盖范围：

```
fp_len_x = front + back  (0.34m, nx=6 points)
fp_len_y = left + right  (0.56m, ny=8 points)
total_fp = 48 采样点
```

**幂律代价映射** (二次幂 `norm²`):

| costmap 值 | 含义 | (c/255)² | 影响 |
|------------|------|----------|------|
| < 1 | FREE_SPACE | 0.000 | 无 |
| 64 | 轻度膨胀 | 0.063 | 微弱 |
| 128 | 中度膨胀 | 0.250 | 可感知 |
| 192 | 重度膨胀 | 0.567 | 显著 |
| 254 | 致命障碍 | 0.992 | 接近满额 |

```
per_step = Σ(norm²) / total_fp × costmap_weight / H
```

- **默认**: `costmap_weight` = 3.0

> **设计意图**: 二次幂在保持平滑梯度的同时，让中低代价区域（膨胀层）产生足够的排斥信号，避免控制器"偏好"穿越膨胀区而非绕行。

### ③ 全局路径吸引 (Path Attraction) ◆ 惩罚

```
cost += traj_to_path² × path_attract_t / H
path_attract_t = path_attraction_weight × (1 + 1.2 × t/H)
```

- 轨迹点到最近路径段的距离平方
- 渐进增强：远端 (`t→H`) 惩罚更严
- **默认**: `path_attraction_weight` = 0.8
- `PATH_FOLLOW_SCALE_INCREMENT` = 1.2 (硬编码，控制递增速度)

### ④ 混合朝向代价 (Blended Heading) ◆ 惩罚

```
cost += |θ − blended_heading| × heading_weight / H
blended_heading = lerp(path_angle, lookahead_theta, alpha)
alpha = cosine_anneal(dist_to_lookahead, 0, anneal_scale)
```

- 余弦退火在路径方向与目标姿态之间平滑过渡
- 远目标 → 跟随路径方向；近目标 → 匹配 `lookahead_theta`
- `lookahead_theta` 由 CPU 端搜索得到（窄通道=最优通过姿态, 终点=goal_yaw）
- **默认**: `heading_weight` = 5.0

### ⑤ 路径长度代价 (Path Length) ◆ 惩罚

```
cost += step_len × PATH_LEN_WEIGHT / H
```

- 每步位移距离的线性惩罚，防止绕远路
- **默认**: `PATH_LEN_WEIGHT` = 0.2 (硬编码)

---

## 三、终端代价（循环后一次性加入，不除以 H）

### ⑥ 终端距离 (Terminal Distance) ◆ 惩罚

```
cost += ‖(x_T,y_T) − (target_x,target_y)‖ × TERMINAL_DIST_WEIGHT
```

- 轨迹终点到前瞻点的直线距离
- **默认**: `TERMINAL_DIST_WEIGHT` = 2.0 (硬编码)

### ⑦ 终端朝向对齐 (Terminal Heading) ◆ 惩罚

```
cost += |heading_err| × 0.8 × goal_proximity
goal_proximity = 1 / (1 + dist × 1.6)
```

- 距离门控：0m→prox=1.0, 0.5m→prox=0.56, 1.0m→prox=0.38
- 权重固定 0.8

### ⑧ 前瞻点接近奖励 (Lookahead Proximity) ◆ 奖励

```
cost += −weight × exp(−dist / decay)
```

- 鼓励轨迹终点靠近前瞻点位置
- **默认**: `lookahead_proximity_weight` = 3.0, `lookahead_proximity_decay` = 0.8

---

## 四、代价函数总结表

| # | 名称 | 类型 | 权重参数 | 权重默认 | /H? | 作用 |
|---|------|------|----------|----------|-----|------|
| ⓪ | 速度奖励 | **奖励**(−) | `speed_reward_weight` | 5.0 | ✅ | 鼓励沿路径方向快速前进 |
| ① | 目标渐进吸引 | 惩罚(+) | `PATH_COST_WEIGHT` | 0.3 (固定) | ✅ | 越后期越靠近前瞻点 |
| ② | 碰撞/膨胀 | 惩罚(+) | `costmap_weight` | 3.0 | ✅ | 48点密集足迹采样, (c/255)² 幂律 |
| ③ | 全局路径吸引 | 惩罚(+) | `path_attraction_weight` | 0.8 | ✅ | Cross-track error², 渐进增强 |
| ④ | 混合朝向 | 惩罚(+) | `heading_weight` | 5.0 | ✅ | 路径方向→目标姿态余弦退火过渡 |
| ⑤ | 路径长度 | 惩罚(+) | `PATH_LEN_WEIGHT` | 0.2 (固定) | ✅ | 反绕路 |
| ⑥ | 终端距离 | 惩罚(+) | `TERMINAL_DIST_WEIGHT` | 2.0 (固定) | — | 轨迹终点到前瞻点 |
| ⑦ | 终端朝向 | 惩罚(+) | (固定) | 0.8×门控 | — | 终点朝向对齐, 距离门控 |
| ⑧ | 前瞻接近 | **奖励**(−) | `lookahead_proximity_weight` | 3.0 | — | 鼓励终点靠近前瞻点, exp衰减 |

---

## 五、MPPI 指数加权

```
w_k = exp(−(cost_k − min_cost) / λ)
u*_t = Σ_k w_k · u_k,t / Σ_k w_k
```

- **λ (温度系数)**: 默认 2.0。越小越贪婪（更偏向最优轨迹），越大越均匀。
- λ=2.0 时，Δcost=2 的轨迹权重比为 1/e≈0.368；Δcost=6 时比为 0.05。

---

## 六、典型代价数量级（H=15, 自由空间直行）

| 代价项 | 预期 total | 说明 |
|--------|-----------|------|
| ⓪ 速度奖励 | −1.27 | 沿路径全速前进 |
| ① 目标渐进吸引 | +0.41 | 除以 H 后大幅缩小 |
| ② 碰撞 (自由空间) | 0 | — |
| ③ 路径吸引 | +0.05 | 除以 H 后缩小 |
| ④ 混合朝向 | +0.75 | 典型朝向偏差 |
| ⑤ 路径长度 | +0.02 | 除以 H 后微量 |
| ⑥ 终端距离 | +1.00 | 终点 0.5m, 权重 2.0 |
| ⑦ 终端朝向 | +0.07 | 距离门控后 |
| ⑧ 前瞻接近 | −1.61 | 终点 0.5m, decay 0.8 |
| **总计** | **−0.58** | |

> **关键变化**: 统一 /H 后，逐步代价总量从 ~8 降至 ~0.5-1.0，与终端代价（~1-2）在同一数量级。
> 这使得 MPPI 能够更均衡地权衡"跟随路径"和"到达目标"。

---

## 七、CPU 端附加行为 (computeVelocityCommands)

### 前瞻点最优朝向搜索
- 在前瞻点处采样 24 个候选朝向
- 对每个朝向评估碰撞代价 (`eval_theta`)
- 选择碰撞代价最小的朝向作为 `lookahead_theta`
- 窄通道 → 自动选择最优通过姿态；终点 → 等于 goal_yaw

### 横向偏好分析
- 在前瞻点处沿路径法向扫描 costmap (0.3m~1.8m)
- 比较左右两侧空闲度，带迟滞选择绕行方向
- 将 path_direction 微旋 ±6° 偏向空闲侧
- 可通过 `enable_lateral_bias` 关闭

### 终端角度对准
- dist_to_final_goal < `terminal_angle_dist` (0.10m)
- 退化 MPPI → 纯角度 P 控制器
- vx=vy=0, ω = terminal_angle_kp × yaw_err

### EMA 输出平滑
- `cmd = α × raw + (1−α) × prev_cmd`
- `ema_alpha` = 0.1 (约 0.25s 时间常数@20Hz)
- 当前默认关闭 (`enable_ema: false`)

### 前瞻点 KP 减速
- 读取 costmap 在前瞻点处的代价值
- `scale = 1 − cost/254 × (1 − kp)`
- 代价越高速度越低
- 当前 kp=1.0，即关闭此功能

---

## 八、参数速查表

### 代价权重

| 参数 | 默认值 | 类型 | 说明 |
|------|--------|------|------|
| `collision_cost` | 3.0 | yaml | 代价地图碰撞权重 (costmap_weight) |
| `path_attraction_weight` | 0.8 | yaml | Cross-track error 权重 |
| `speed_reward_weight` | 5.0 | yaml | 速度奖励权重 |
| `heading_weight` | 5.0 | yaml | 混合朝向权重 |
| `lookahead_proximity_weight` | 3.0 | yaml | 前瞻接近奖励权重 |
| `lookahead_proximity_decay` | 0.8 | yaml | 前瞻接近奖励衰减距离 (m) |
| `TERMINAL_DIST_WEIGHT` | 2.0 | 硬编码 | 终端距离权重 |
| `PATH_COST_WEIGHT` | 0.3 | 硬编码 | 目标渐进吸引基础权重 |
| `PATH_LEN_WEIGHT` | 0.2 | 硬编码 | 路径长度权重 |
| `PATH_FOLLOW_SCALE_INCREMENT` | 1.2 | 硬编码 | 路径吸引时间递增因子 |

### MPPI 核心

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `num_samples` | 1000 | 每帧采样轨迹数 |
| `prediction_horizon` | 15 | 预测步数 H |
| `dt` | 0.3 | 每步时间 (s) |
| `lambda` | 2.0 | MPPI 温度系数 |
| `max_v` / `min_v` | 0.40 / −0.15 | 前向速度范围 |
| `max_vy` | 0.35 | 横向速度上限 |
| `max_w` | 0.50 | 角速度上限 |

### 碰撞箱

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `footprint_front` | 0.17 | 前向半尺寸 (m) |
| `footprint_back` | 0.17 | 后向半尺寸 (m) |
| `footprint_left` | 0.28 | 左向半尺寸 (m) |
| `footprint_right` | 0.28 | 右向半尺寸 (m) |
| `FP_SAMPLE_SPACING` | 0.08 | 足迹采样间距 (m), 硬编码 |

---

*最后更新: 2026-06-24 — 统一 horizon 除法 + norm² 幂律*
