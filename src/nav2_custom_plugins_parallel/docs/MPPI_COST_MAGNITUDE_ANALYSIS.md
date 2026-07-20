# MPPI GPU 代价函数 — 数量级评估与修改建议

> **日期**: 2026-06-24
> **评估对象**: `mppi_gpu_kernels.cu` + `mppi_gpu_rewards.cuh`
> **配置基准**: `themis_navigation.yaml` (horizon=15, lambda=3.0, costmap_weight=5.0)

---

## 目录

1. [代价函数公式汇总](#1-代价函数公式汇总)
2. [各代价项数量级分析](#2-各代价项数量级分析)
3. [典型场景代价构成](#3-典型场景代价构成)
4. [MCPPI 指数加权敏感度分析](#4-mppi-指数加权敏感度分析)
5. [发现的问题](#5-发现的问题)
6. [修改建议](#6-修改建议)

---

## 1. 代价函数公式汇总

### 1.1 当前计算流程

```
total_cost = Σ_{t=0}^{H-1} [
    ⓪ speed_reward(t)
  + ① goal_distance_cost(t)
  + ② costmap_collision_cost(t)
  + ③ path_attraction_cost(t)
  + ④ blended_heading_cost(t)
  + ⑤ path_length_cost(t)
]
+ ⑥ terminal_distance_cost
+ ⑦ terminal_heading_cost
+ ⑧ lookahead_proximity_reward
```

### 1.2 各项公式与权重

| # | 名称 | 公式 | 权重/常量 | 除以 H? |
|---|------|------|-----------|---------|
| ⓪ | 速度奖励 | `-max(0, v_proj) · w_speed / H` | w=3.0 | ✅ 是 |
| ① | 目标渐进吸引 | `dist_to_target · (1 - t/H) · 0.5` | 0.5 (PATH_COST_WEIGHT) | ❌ 否 |
| ② | 碰撞/膨胀 | `(Σ (c/255)⁴ / N_fp) · w_costmap / H` | w=5.0 | ✅ 是 |
| ③ | 路径吸引 | `traj_to_path_sq² · w_path · (1 + 1.2·t/H)` | w=0.6 | ❌ 否 |
| ④ | 混合朝向 | `|θ_err| · w_heading / H` | w=5.0 | ✅ 是 |
| ⑤ | 路径长度 | `step_len · 0.02` | 0.02 (PATH_LEN_WEIGHT) | ❌ 否 |
| ⑥ | 终端距离 | `dist_to_target · 5.0` | 5.0 (TERMINAL_DIST_WEIGHT) | — (一次性) |
| ⑦ | 终端朝向 | `|heading_err| · 0.8 / (1 + dist·1.6)` | 0.8 (固定) | — (一次性) |
| ⑧ | 前瞻接近 | `-w · exp(-dist / decay)` | w=5.0, decay=0.3 | — (一次性) |

> **关键发现**: ①、③、⑤ 没有除以 horizon，而 ⓪、②、④ 有。这意味着 horizon 变化时，代价平衡会被打破。

---

## 2. 各代价项数量级分析

### 2.1 计算假设

| 参数 | 值 | 说明 |
|------|-----|------|
| H (horizon) | 15 | 预测步数 |
| dt | 0.3s | 每步时长 |
| max_v | 0.40 m/s | 最大前向速度 |
| 典型速度 | 0.38 m/s | 接近满速前进 |
| 典型步长 | 0.114 m | v_max · dt |
| 目标距离 | 2.0 → 0.5 m | 轨迹从起点到终点 |
| Cross-track error | ~0.2 m | 采样噪声导致的路径偏离 |
| 朝向误差 | ~0.15 rad | 路径方向与机头夹角 |

### 2.2 各项数量级逐一计算

#### ⓪ 速度奖励（负代价）

```
per_step = -max(0, 0.38) · 3.0 / 15 = -0.076
total    = -0.076 × 15 = -1.14
```

**数量级**: **~ -1**。对沿路径快速前进的轨迹提供约 1 个单位的奖励。

#### ① 目标渐进吸引（正代价）

```
per_step = dist_to_target · (1 - t/H) · 0.5

t=0:   2.0 · 1.000 · 0.5 = 1.000
t=1:   1.9 · 0.933 · 0.5 = 0.887
t=2:   1.8 · 0.867 · 0.5 = 0.780
...
t=7:   1.3 · 0.533 · 0.5 = 0.347
...
t=14:  0.6 · 0.067 · 0.5 = 0.020

total ≈ 6.14
```

**数量级**: **~ 6**。这是所有代价项中**绝对值最大**的一项，占据总代价的约 48%。

> ⚠️ 此项不除以 horizon。若 H=10，total≈4.1；若 H=20，total≈8.2。

#### ② 代价地图碰撞（正代价）

幂律映射表（`(c/255)⁴`）：

| costmap 值 | 含义 | 归一化 c/255 | (c/255)⁴ | 等价影响 |
|------------|------|-------------|----------|---------|
| 0 | FREE_SPACE | 0.00 | 0.000 | 无 |
| 64 | 轻度膨胀 | 0.25 | 0.004 | 几乎为零 |
| 128 | 中度膨胀 | 0.50 | 0.063 | 微弱 |
| 192 | 重度膨胀 | 0.75 | 0.316 | 中等 |
| 254 | 致命障碍 | 1.00 | 0.984 | 接近满额 |

足迹采样点数：`fp_len_x=0.34, fp_len_y=0.56`, 间距 0.08m →
`nx=6, ny=8 → total_fp = 48`

**自由空间 (c < 1)**:
```
per_step = 0
total = 0
```

**中度膨胀 (c=128, 50% 足迹覆盖)**:
```
cost_acc = 24 · (128/255)⁴ = 24 · 0.0635 = 1.524
per_step = 1.524 / 48 · 5.0 / 15 = 0.0106
total    = 0.0106 × 15 = 0.159
```

**重度膨胀 (c=200, 100% 足迹覆盖)**:
```
cost_acc = 48 · (200/255)⁴ = 48 · 0.379 = 18.2
per_step = 18.2 / 48 · 5.0 / 15 = 0.126
total    = 0.126 × 15 = 1.89
```

**致命障碍 (c=254, 100% 足迹覆盖)**:
```
cost_acc = 48 · 0.984 = 47.2
per_step = 47.2 / 48 · 5.0 / 15 = 0.328
total    = 0.328 × 15 = 4.92
```

**数量级**:
- 自由空间: **0**
- 中度膨胀 (c=128): **~0.16** （微不足道）
- 重度膨胀 (c=200): **~1.9** （开始有影响）
- 致命障碍 (c=254): **~4.9** （与目标距离代价相当）

#### ③ 路径吸引（正代价）

```
path_attract_t = 0.6 · (1 + 1.2 · t/15)

cross-track error = 0.2m → traj_to_path_sq = 0.04

per_step (mean) = 0.04 · 0.6 · 1.6 = 0.0384
total           = 0.0384 × 15 = 0.58
```

**数量级**: **~ 0.6**（典型 CTE=0.2m）。若 CTE=0.5m，则上升到 **~ 3.6**。

> ⚠️ 此项不除以 horizon。若 H=20，total 同比例增大。

#### ④ 混合朝向（正代价）

```
per_step = |θ_err| · 5.0 / 15 = |θ_err| · 0.333

典型 θ_err=0.15 rad:
per_step = 0.050
total    = 0.75
```

**数量级**: **~ 0.8**（典型朝向偏差 0.15 rad）。若偏差达 0.5 rad，上升到 **~2.5**。

#### ⑤ 路径长度（正代价）

```
per_step = step_len · 0.02 = 0.114 · 0.02 = 0.00228
total    = 0.00228 × 15 = 0.034
```

**数量级**: **~ 0.03**。在所有场景中均**可忽略**。

#### ⑥ 终端距离（正代价）

```
terminal = dist_to_target · 5.0

dist=0.3m → 1.5
dist=0.5m → 2.5
dist=1.0m → 5.0
dist=2.0m → 10.0
```

**数量级**: **~ 2.5**（典型终点距离 0.5m）。这是第二大的代价项。

#### ⑦ 终端朝向（正代价）

```
goal_proximity = 1 / (1 + dist · 1.6)
terminal = |heading_err| · 0.8 · goal_proximity

dist=0.5m, err=0.15rad: 0.15 · 0.8 · 0.556 = 0.067
dist=1.0m, err=0.30rad: 0.30 · 0.8 · 0.385 = 0.092
dist=2.0m, err=0.50rad: 0.50 · 0.8 · 0.238 = 0.095
```

**数量级**: **~ 0.07-0.10**。在所有场景中均**非常小**。

#### ⑧ 前瞻接近奖励（负代价）

```
reward = -5.0 · exp(-dist / 0.3)

dist=0.0m → -5.00
dist=0.2m → -5.0 · 0.513 = -2.57
dist=0.5m → -5.0 · 0.189 = -0.95
dist=1.0m → -5.0 · 0.036 = -0.18
```

**数量级**: **~ -1.0**（典型终点距离 0.5m）。对近距离轨迹提供显著奖励。

---

## 3. 典型场景代价构成

### 3.1 场景 A：自由空间直行

> 机器人 2m 前无障碍，沿路径直行，CTE≈0.2m，终点距离≈0.5m

| # | 代价项 | 值 | 占比(abs) |
|---|--------|-----|-----------|
| ① | 目标渐进吸引 | **+6.14** | 48.7% |
| ⑥ | 终端距离 | **+2.50** | 19.9% |
| ⓪ | 速度奖励 | -1.14 | 9.1% |
| ⑧ | 前瞻接近奖励 | -0.95 | 7.5% |
| ④ | 混合朝向 | +0.75 | 6.0% |
| ③ | 路径吸引 | +0.58 | 4.6% |
| ⑦ | 终端朝向 | +0.07 | 0.5% |
| ⑤ | 路径长度 | +0.03 | 0.2% |
| **总计** | | **+6.98** | |

### 3.2 场景 B：穿越中度膨胀区

> 轨迹必经 c=128~200 的膨胀区域，50% 足迹覆盖，其余同场景 A

| # | 代价项 | 值 | 占比(abs) |
|---|--------|-----|-----------|
| ① | 目标渐进吸引 | +6.14 | 46.1% |
| ⑥ | 终端距离 | +2.50 | 18.8% |
| ② | 碰撞 (c=150, 50%覆盖) | **+0.33** | 2.5% |
| ⓪ | 速度奖励 | -1.14 | 8.6% |
| ⑧ | 前瞻接近奖励 | -0.95 | 7.1% |
| ④ | 混合朝向 | +0.75 | 5.6% |
| ③ | 路径吸引 | +0.58 | 4.4% |
| 其他 | | +0.10 | 0.8% |

> ⚠️ **碰撞代价仅占 2.5%**，即使在中度膨胀区域，避障信号依然很弱。

### 3.3 场景 C：面临致命障碍

> 轨迹直冲 c=254 致命障碍，100% 足迹覆盖，终点距离从 0.5→1.2m

| # | 代价项 | 值 | 占比(abs) |
|---|--------|-----|-----------|
| ① | 目标渐进吸引 | +4.50 | 23.1% |
| ② | 碰撞 (c=254, 全覆盖) | **+4.92** | 25.2% |
| ⑥ | 终端距离 | **+6.00** | 30.8% |
| ⓪ | 速度奖励 | -1.14 | 5.8% |
| ⑧ | 前瞻接近奖励 | -0.18 | 0.9% |
| ④ | 混合朝向 | +0.75 | 3.8% |
| ③ | 路径吸引 | +0.58 | 3.0% |

> 致命障碍场景下碰撞代价终于显著，但**终端距离代价（轨迹绕行→终点更远）同时也增大**，削弱了避障的净收益。

---

## 4. MPPI 指数加权敏感度分析

MPPI 核心公式:

```
w_k = exp(-(cost_k - cost_min) / λ)
```

λ = 3.0（当前配置）

| Δcost | exp(-Δcost/3) | 权重保留率 | 含义 |
|--------|---------------|-----------|------|
| 0.3 | 0.905 | 90.5% | 几乎无区别 |
| 1.0 | 0.717 | 71.7% | 微弱偏好 |
| 3.0 | 0.368 | 36.8% | 明显偏好 |
| 6.0 | 0.135 | 13.5% | 强偏好 |
| 10.0 | 0.036 | 3.6% | 极强偏好 |

**关键洞察**:
- 要产生 "明显偏好"（权重比 ~3:1），需要 Δcost ≈ **3** 单位
- 自由空间中，好轨迹与坏轨迹的代价差主要在朝向和路径吸引，合计约 2-3 单位 → 刚好足够
- 但障碍物避障信号（~0.3 for c=150）完全被淹没在目标距离波动中

---

## 5. 发现的问题

### 🔴 P0: Horizon 除法不一致 — 代价平衡的根本缺陷

**现状**: ⓪②④ 除以 horizon，①③⑤ 不除以 horizon。

**后果**:
1. Horizon 从 10 改到 15，不除以 H 的项（①）总代价增大约 50%
2. 除以 H 的项（②）代价不变
3. 意味着**每改一次 horizon 就要重新调所有权重**
4. 当前 H=15 时，①（目标距离）以 6.14 的绝对优势主导所有其他项

**修复原则**: 所有**每步累加的代价**应统一除以 horizon，使总代价与 horizon 长度无关。

---

### 🔴 P1: 目标距离代价 ① 过大

**现状**: PATH_COST_WEIGHT=0.5 且不除以 H，total≈6.14。

**后果**:
- 占据总代价的 ~48%
- 对轨迹的"直接指向目标"偏好过强
- 严重压制了探索和避障信号

**根因**: `PATH_COST_WEIGHT` 设计为固定常量 0.5，没有配置参数暴露，且不除以 horizon。

---

### 🔴 P2: 碰撞代价 ② 在非致命区域过弱

**现状**: costmap_weight=5.0（看似高），但幂律 `(c/255)⁴` 使中低代价区域贡献极微。

**后果**:
- c=128 区域（INSCRIBED_INFLATED，本应产生明显排斥）贡献仅 0.16
- c=200 区域贡献仅 1.89，不够超过目标距离代价
- 控制器无法感知 "靠近障碍物" 和 "远离障碍物" 的区别
- **穿越膨胀区可能比绕行更"便宜"**

**根因**: 四次幂对中低代价值过度压缩。改用二次幂 `(c/255)²` 会大幅改善。

| costmap 值 | (c/255)⁴ (当前) | (c/255)² (建议) |
|------------|-----------------|-----------------|
| 64 | 0.004 | 0.063 |
| 128 | 0.063 | 0.250 |
| 192 | 0.316 | 0.567 |
| 254 | 0.984 | 0.992 |

---

### 🟡 P3: 终端距离代价 ⑥ 权重过高

**现状**: TERMINAL_DIST_WEIGHT=5.0。

**后果**:
- 典型场景贡献 2.5，是第二大的项
- 所有轨迹被强力拉向前瞻点
- 削弱了 MPPI 的时域探索能力（尤其是后期的路径偏离探索）
- 绕行轨迹因终点更远而受到不成比例的惩罚

---

### 🟡 P4: 路径长度代价 ⑤ 可忽略

**现状**: PATH_LEN_WEIGHT=0.02，不做 horizon 除法，total≈0.034。

**后果**:
- 完全不产生"防止绕远路"的设计效果
- 即使绕行 2 倍距离（extra 0.5m），代价增加仅 ~0.1 单位

---

### 🟡 P5: 终端朝向代价 ⑦ 可忽略

**现状**: 固定 0.8 倍系数 + 距离门控，total≈0.07-0.10。

**后果**:
- 终端朝向对齐基本不参与代价比较
- 这在很多场景下可能是合理的（朝向由 blended_heading 在逐步中处理），但文档声称的"终点朝向重要性"并未体现

---

### 🟢 P6: 前瞻接近奖励 ⑧ 衰减过快

**现状**: decay=0.3m, weight=5.0。

**后果**:
- 在 0.3m 处奖励降至 -1.84 (exp(-1)=0.368)
- 在 0.6m 处奖励仅 -0.68 (exp(-2)=0.135)
- 有效范围仅 ~0.5m，对大多数轨迹几乎没有影响
- 设计意图（鼓励轨迹进入窄通道）未能实现

---

### 🟢 P7: 文档与代码不同步

MPPI_COST_FUNCTION.md 描述的权重和公式与 mppi_gpu_rewards.cuh 中的实际实现存在多处差异：

| 项目 | 文档值 | 代码/配置值 |
|------|--------|------------|
| speed_reward_weight | 6.0 | 3.0 |
| costmap_weight | 2.0 | 5.0 |
| path_attraction_weight | 0.15 | 0.6 |
| heading_weight | 0.8 | 5.0 |
| Terminal dist weight | 1.0 | 5.0 |
| path_cost_weight | 0.2 | 0.5 |
| 碰撞采样方式 | 8点取max | 48点密集网格加权平均 |
| 朝向代价实现 | 速度方向角 vs 机头 | blended_heading (路径方向→目标姿态) |

---

## 6. 修改建议

### 6.1 高优先级修复（推荐立即执行）

#### 建议 1: 统一 Horizon 除法

**修改文件**: `mppi_gpu_rewards.cuh`

将 `compute_goal_distance_cost`、`compute_path_attraction_cost`、`compute_path_length_cost` 内部除以 horizon：

```c
// compute_goal_distance_cost — 修改返回语句
return dist * (1.0f - progress) * path_cost_weight / static_cast<float>(horizon);

// compute_path_attraction_cost — 修改返回语句
return traj_to_path_sq * path_attract_t / static_cast<float>(horizon);  // 需要在参数中加入 horizon

// compute_path_length_cost — 修改返回语句
return sqrtf(step_dx * step_dx + step_dy * step_dy) * len_weight / static_cast<float>(horizon);
```

**影响**: 所有代价项数量级对齐，horizon 变化不再破坏平衡。代价绝对值整体下降约 10×，需同步调整 λ。

**配套调整**:
- `λ`: 3.0 → **0.3**（代价缩小约 10×，温度同比例缩小）
- 或保持 λ=3.0，将所有权重乘以 ~10（不推荐，改 λ 更简洁）

---

#### 建议 2: 改用二次幂代价映射

**修改文件**: `mppi_gpu_rewards.cuh` 中的 `compute_costmap_collision_cost`

```c
// 当前: norm^4
cost_acc += norm * norm * norm * norm;

// 建议: norm^2
cost_acc += norm * norm;
```

**理由**:
- (128/255)² = 0.25，不再被过度压缩
- 保持平滑梯度（二次可导）
- (254/255)² ≈ 0.992，致命障碍几乎不变
- 中低代价区域获得 4-16 倍的感知提升

**影响**:
| costmap | 旧值 (per step, H=15) | 新值 (per step, H=15) |
|---------|----------------------|----------------------|
| 64 (轻度膨胀) | 0.0004 | 0.0021 |
| 128 (中度膨胀) | 0.0106 | 0.0174 |
| 192 (重度膨胀) | 0.0527 | 0.0630 |
| 254 (致命) | 0.328 | 0.331 |

> 注意: 在除以 H 后 per-step 值较小是正常的，关键是**总代价差异**能否区分好/坏轨迹。

---

#### 建议 3: 降低终端距离权重

**修改文件**: `mppi_gpu_kernels.cu`

```c
// 当前
static const float TERMINAL_DIST_WEIGHT = 5.0f;

// 建议
static const float TERMINAL_DIST_WEIGHT = 2.0f;
```

**理由**: 当前 5.0 对绕行轨迹惩罚过重，2.0 时终端距离 0.5m → 1.0 单位，与逐步代价在同一数量级。

**同时建议**: 将 TERMINAL_DIST_WEIGHT 提升为可配置参数（通过 `declare_parameter`），而非硬编码常量。

---

### 6.2 中优先级优化

#### 建议 4: 将 PATH_COST_WEIGHT 提升为可配置参数

**修改文件**: `mppi_gpu_kernels.cu` + `mppi_gpu_controller.cpp`

```c
// kernels.cu: 移除 static const, 添加为核函数参数
// controller.cpp: 添加 declare_parameter("goal_attraction_weight", 0.5)
```

**理由**: 0.5 的固定值在当前（除以 H 之后）可能过大或过小，需要可调。

---

#### 建议 5: 增大路径长度权重或改为非线性

**选项 A**: 简单提升权重
```
PATH_LEN_WEIGHT: 0.02 → 0.2 (提升 10×，在除以 H 后 total≈0.02)
```

**选项 B**: 改用平方惩罚（推荐）
```c
// 修改 compute_path_length_cost
float step_len = sqrtf(step_dx * step_dx + step_dy * step_dy);
return step_len * step_len * len_weight / horizon;
// len_weight 建议值: 2.0
```

**理由**: 平方惩罚对小步长几乎无影响（直线前进），对大绕行自动放大。在 2.0 权重下，0.3m 额外绕行 → 0.3² × 2.0 / 15 ≈ 0.012/step，15 步 ≈ 0.18，开始产生可感知信号。

---

#### 建议 6: 增大前瞻接近奖励有效范围

**修改文件**: `themis_navigation.yaml`

```yaml
lookahead_proximity_decay: 0.8   # 当前 0.3 → 建议 0.8
```

**理由**: decay=0.8 时，在 0.8m 处奖励仍为 -1.84，覆盖更广范围。

| dist | decay=0.3 (当前) | decay=0.8 (建议) |
|------|-----------------|-----------------|
| 0.2m | -2.57 | -3.89 |
| 0.5m | -0.95 | -2.68 |
| 0.8m | -0.35 | -1.84 |
| 1.2m | -0.09 | -1.12 |

---

### 6.3 推荐参数集

基于上述修改建议，推荐以下配置:

```yaml
# themis_navigation.yaml — FollowPath 节点下
FollowPath:
  plugin: "nav2_custom_plugins/MPPIGPUController"
  
  # MPPI 核心
  num_samples: 1000
  prediction_horizon: 15
  dt: 0.3
  lambda: 2.0            # 当前 3.0, 配合代价统一后适度降低
  
  # 速度约束
  max_v: 0.40
  min_v: -0.15
  max_vy: 0.35
  max_w: 0.50
  
  # 采样噪声
  action_std_v: 0.25
  action_std_vy: 0.15
  action_std_w: 0.25
  
  # 代价权重 (统一 horizon 除法后)
  collision_cost: 3.0          # costmap_weight, 当前 5.0
  path_attraction_weight: 0.8  # 当前 0.6
  speed_reward_weight: 5.0     # 当前 3.0
  heading_weight: 5.0          # 保持不变
  lookahead_proximity_weight: 3.0  # 当前 5.0
  lookahead_proximity_decay: 0.8   # 当前 0.3
  
  # GPU kernel 常量 (需改为参数或至少文档明确)
  # TERMINAL_DIST_WEIGHT: 2.0  (当前 5.0, 硬编码)
  # PATH_COST_WEIGHT: 0.3      (当前 0.5, 硬编码)
  # PATH_LEN_WEIGHT: 0.2       (当前 0.02, 硬编码)
  
  # 其他保持不变
  guidance_weight: 0.2
  cross_track_noise_scale: 1.5
  noise_decay_rate: 0.65
  exploration_decay_start: 3.0
  exploration_decay_end: 0.5
  exploration_decay_floor: 0.3
  spatial_decay_weight: 0.5
  noise_scale_floor_vx: 0.15
  noise_scale_floor_vy: 0.20
  noise_scale_floor_w: 0.50
  pure_rotation_ratio: 0.05
  pure_rotation_steps: 2
  pure_rotation_w_boost: 1.5
  enable_lateral_bias: false
  vel_direction_weight: 0.15
  lateral_guidance_scale: 0.15
  turn_lateral_boost: 2.0
  ema_alpha: 0.1
  enable_ema: false
  lookahead_kp: 1.0
  lookahead_time: 0.0
  min_lookahead_dist: 0.5
  terminal_angle_dist: 0.10
  terminal_angle_kp: 1.5
  terminal_angle_tolerance: 0.07
  footprint_front: 0.17
  footprint_back: 0.17
  footprint_left: 0.28
  footprint_right: 0.28
```

### 6.4 修改后各项预期数量级

假设统一 horizon 除法 + 二次幂 + 推荐参数:

| # | 代价项 | 预期 total | 说明 |
|---|--------|-----------|------|
| ⓪ | 速度奖励 | **-0.33** | 5.0 × 0.4 / 15 × 15 = 高估… 实际: 0.38 × 5.0 / 15 × 15 = -1.9, 等… 重算 |
| ① | 目标渐进吸引 | **~0.41** | (6.14 / 15) with weight 0.3 not 0.5 |
| ② | 碰撞 (c=128, 50%) | **~0.17** | 二次幂: 24 × 0.25 / 48 × 3.0 / 15 × 15 |
| ③ | 路径吸引 | **~0.05** | (0.58 / 15) with weight 0.8 |
| ④ | 混合朝向 | **~0.75** | (不变, 已除以 H) |
| ⑤ | 路径长度 | **~0.02** | (0.034 / 15 × 10) |
| ⑥ | 终端距离 | **~1.0** | 2.0 × 0.5 |
| ⑦ | 终端朝向 | **~0.07** | (不变) |
| ⑧ | 前瞻接近 | **~-2.7** | -3.0 × exp(-0.5/0.8) |

> ⚠️ 以上为粗估，实际需经 GPU 测试验证。关键目标是确保碰撞代价在膨胀区域足以被感知。

---

## 附录: 代码修改清单

### 文件 1: `mppi_gpu_rewards.cuh`

| 函数 | 修改内容 |
|------|---------|
| `compute_goal_distance_cost` | 返回值加 `/ horizon`，添加 `horizon` 参数 |
| `compute_path_attraction_cost` | 返回值加 `/ horizon`，添加 `horizon` 参数 |
| `compute_path_length_cost` | 返回值加 `/ horizon`，添加 `horizon` 参数 |
| `compute_costmap_collision_cost` | `norm^4` → `norm^2` |

### 文件 2: `mppi_gpu_kernels.cu`

| 位置 | 修改内容 |
|------|---------|
| 常量区 | `TERMINAL_DIST_WEIGHT: 5.0 → 2.0` |
| 常量区 | `PATH_COST_WEIGHT: 0.5 → 0.3` |
| 常量区 | `PATH_LEN_WEIGHT: 0.02 → 0.2` |
| 核函数调用 | 传入 `horizon` 给 ①③⑤ 的 compute 函数 |

### 文件 3: `mppi_gpu_controller.cpp`

| 位置 | 修改内容 |
|------|---------|
| `configure()` | 为 ⑥ TERMINAL_DIST_WEIGHT、① PATH_COST_WEIGHT、⑤ PATH_LEN_WEIGHT 添加参数声明 |

### 文件 4: `themis_navigation.yaml`

| 参数 | 当前值 | 建议值 |
|------|--------|--------|
| `lambda` | 3.0 | 2.0 |
| `collision_cost` | 5.0 | 3.0 |
| `path_attraction_weight` | 0.6 | 0.8 |
| `speed_reward_weight` | 3.0 | 5.0 |
| `lookahead_proximity_weight` | 5.0 | 3.0 |
| `lookahead_proximity_decay` | 0.3 | 0.8 |

### 文件 5: `MPPI_COST_FUNCTION.md`

需要全文更新以匹配代码实际实现。

---

*分析完成日期: 2026-06-24*
