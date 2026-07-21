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

大类权重: OBSTACLE=0.50, HEADING=0.40, SPEED=0.10 (YAML 配置)

## Kernel 每步调用

```
CriticManager::evaluate(x, y, θ, vx, vy, cmap, fp, path, goal)
  ├── cat_w[OBSTACLE] × obstacle_.evaluate()
  │     → FootprintCritic: 足迹网格 → costmap 双线性插值 → n⁴ 碰撞惩罚
  ├── cat_w[HEADING]  × heading_.evaluate()
  │     → PathAlign:      1-exp(-dist²)
  │     → PathAngle:      4×err² (不归一化, 原始平方)
  │     → PathDeviation:  1-exp(-excess²), corridor=0.5m
  └── cat_w[SPEED]   × speed_.evaluate()
        → SpeedReward: THEMIS 公式 (alignment 奖励 + lateral×2 侧向惩罚)
```

## SpeedRewardCritic — THEMIS 公式

```cpp
alignment = (vx×target_vx_r + vy×target_vy_r) / speed;   // cos(err)
lateral   = |vx×target_vy_r - vy×target_vx_r| / speed;    // |sin(err)|
if (alignment < 0)  return speed × 5.0;                    // 反向重罚
return -speed × alignment + 2.0 × speed × lateral;         // 对齐奖励 + 侧向抑制
// 0° → -speed, 26.6° → 0 (中性), 45° → +0.71×speed
```

## 新增代价子类

给 OBSTACLE 加 DistanceFieldCritic:
1. 写子类: `class DistanceFieldCritic : public ObstacleCritic { compute() }`
2. 写 wrapper: `static float distanceFn(...) { D c; return c.compute(...); }`
3. 注册: `ObstacleCategory::init()` 加 `subs_[N] = {distanceFn, true, weight}`
→ CriticManager 不动

## GPU 数据结构

| 结构 | 用途 | 字段 |
|------|------|------|
| CostmapInfo | OBSTACLE | data ptr, w, h, res, origin_x/y |
| Footprint | OBSTACLE | front/back/left/right, samples, rear_penalty |
| PathInfo | HEADING | x/y ptr, num_pts, path_tangent, goal_yaw |
| GoalInfo | SPEED | target_vx_r, target_vy_r, goal_x/y, lookahead_x/y, overshoot_weight |
