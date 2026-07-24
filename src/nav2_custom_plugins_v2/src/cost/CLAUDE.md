# Cost System (src/cost/)

## 设计

GPU 上不能用虚函数 → 函数指针注册表实现运行时多态。
三层调用: `CriticManager` → `XxxCategory` → `XxxCritic::compute()`

## 类层级

```
CriticBase                              (critic_common.cuh)
├── ObstacleCritic : CriticBase         (obstacle_critic.cuh)
│   └── FootprintCritic                 — 足迹采样碰撞检测
├── HeadingCritic : CriticBase          (heading_critic.cuh)
│   ├── PathAlignCritic                — 点到路径最近距离
│   ├── PathAngleCritic                — 朝向对齐
│   └── PathDeviationCritic            — 走廊偏离软墙
└── SpeedCritic : CriticBase            (speed_critic.cuh)
    └── SpeedRewardCritic              — 1:1 THEMIS 速度方向对齐
```

## 容器

```
CriticManager                           (critic_manager.cuh)
├── ObstacleCategory  — subs_[0] = {footprintFn}
├── HeadingCategory   — subs_[0..2] = {pathAlignFn, pathAngleFn, pathDeviationFn}
└── SpeedCategory     — subs_[0] = {speedRewardFn}
```

大类权重: OBSTACLE=0.60, HEADING=0.30, SPEED=0.10
(CriticManager::init() 硬编码, YAML 参数 obstacle_ratio/tracking_ratio/speed_ratio 待接入)

## 两层归一化

层 1 — 大类内加权和: 各 Category 容器内部 Σ(w × fn), 不平均
层 2 — 大类间加权: cat_weights[] 乘大类结果

total = Σ cat_weights[c] × category.evaluate(...)

## Kernel 每步调用

```
CriticManager::evaluate(x, y, θ, vx, vy, ω, cmap, fp, path, goal)
  ├── cat_w[OBSTACLE] × obstacle_.evaluate()
  │     → FootprintCritic: 足迹网格 → costmap 双线性插值 → n⁴ 碰撞惩罚
  ├── cat_w[HEADING]  × heading_.evaluate()
  │     → PathAlign:      min_sq (点到路径段最近距离平方, m²)
  │     → PathAngle:      4×err² (不归一化, 原始平方, rad²)
  │     → PathDeviation:  excess², excess=max(0, dist−0.5m), m²
  └── cat_w[SPEED]   × speed_.evaluate()
        → SpeedReward: THEMIS 公式 (对齐奖励 + 侧向抑制×2)
```

## SpeedRewardCritic — THEMIS 公式

```cpp
speed = hypot(vx, vy)
if speed < 0.02: return 0                                    // 停止中性
alignment = (vx×target_vx_r + vy×target_vy_r) / speed;       // cos(err)
lateral   = |vx×target_vy_r - vy×target_vx_r| / speed;        // |sin(err)|
if (alignment < 0)  return speed × 5.0;                       // 反向重罚
return -speed × alignment + 2.0 × speed × lateral;            // 对齐奖励 + 侧向抑制
// 0° → -speed, 26.6° → 0 (中性), 45° → +0.71×speed
```

## 新增代价子类

给 HEADING 加新子类:
1. 写子类: `class NewCritic : public HeadingCritic { compute() }`
2. 写 wrapper: `static float newCriticFn(...) { N c; return c.compute(...); }`
3. 注册: `HeadingCategory::init()` 加 `subs_[N] = {newCriticFn, true, weight}`
→ CriticManager 不动

## GPU 数据结构

| 结构 | 用途 | 字段 |
|------|------|------|
| CostmapInfo | OBSTACLE | data ptr, w, h, res, origin_x/y |
| Footprint | OBSTACLE | front/back/left/right, sample_spacing, rear_obstacle_cost |
| PathInfo | HEADING | x/y ptr, num_pts, path_tangent, goal_yaw, goal_x/y |
| GoalInfo | SPEED | target_vx_r, target_vy_r, goal_x/y, lookahead_x/y, overshoot_weight |
