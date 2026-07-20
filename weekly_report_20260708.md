# 周报 2026.06.30 – 2026.07.07

## 1. 三代价归一化：obstacle / tracking / speed 比值约束为 1.0

### 问题

原三组件代价使用独立的 `_weight` 参数各自自由调节。不同代价分量的数值范围差异大（如障碍物 cost ∈ [0,1]，路径距离可能是几十米），直接加权导致某一项在数值上压倒其他项，调参时互相拉扯难以收敛。

### 方案

将三组件重构为**比值归一化 + 统一 per-step 均值**：

```cpp
costs[s] = cost_scale * (
    obstacle_ratio * (obst_acc / horizon)   // ObstaclesCritic
  + tracking_ratio * (track_acc / horizon)  // PathAlign + PathAngle + GoalAngle
  + speed_ratio    * (prog_acc / horizon)   // PreferForward + GoalCritic
);
```

**约束**: `obstacle_ratio + tracking_ratio + speed_ratio = 1.0`

各代价函数内部不再除以 horizon，统一在组合处做 `* inv_h`，保证三个分量在相同尺度上可比，避免某一项因数值量级过大而压倒其他项。

**最终参数**:

| 参数 | 值 | 含义 |
|------|-----|------|
| `cost_scale` | 10.0 | 全局缩放 |
| `obstacle_ratio` | 0.4 | 障碍物占比 40% |
| `tracking_ratio` | 0.4 | 路径跟踪占比 40% |
| `speed_ratio` | 0.2 | 速度/进度占比 20% |

### 效果

三个惩罚项的影响力受比值约束，不再互相压倒，任意场景下各分量的贡献比例可控。

---

## 2. Heading 旋转方向安全检查

### 问题

HEADING_MISALIGN 状态下机器人需要原地旋转修正朝向，但旋转方向固定取最短路径，不考虑旋转过程中碰撞箱扫过的区域是否有致命障碍物——可能导致旋转时碰撞箱角撞墙。

### 方案

在 HEADING_MISALIGN 执行旋转前，对旋转方向做**碰撞预检测**：

1. 模拟 `dt=0.15s` 以 `max_w` 旋转后的 footprint 四角 + 中心坐标
2. 对旋转路径做 **sweep 采样**（3 个中间角度），检测是否命中致命障碍物（cost ≥ 254）
3. 默认方向不安全 → 尝试反向；双向都不安全 → 停止旋转，等状态机重新决策

```cpp
auto checkRotSafe = [&](double sign) -> bool {
  double dt_check = 0.15;
  for (int si = 0; si <= 2; ++si) {           // sweep 采样
    double ta = current_theta + sign * max_w_ * dt_check * (si / 3.0);
    for (int ci = 0; ci < 5; ++ci) {           // 四角 + 中心
      int mx, my;
      // world → costmap 坐标转换
      if (costmap[mx, my] >= 254) return false;
    }
  }
  return true;
};

bool fwd_safe = checkRotSafe( rot_sign);   // 默认方向
bool rev_safe = checkRotSafe(-rot_sign);   // 反方向
if (!fwd_safe && rev_safe)  rot_sign = -rot_sign;   // 换向
if (!fwd_safe && !rev_safe) rot_sign = 0.0;          // 两侧都堵, 停止
```

### 效果

旋转方向有碰撞预检测，避免"旋转撞墙"；双向堵死时静默等待而非强行旋转。
