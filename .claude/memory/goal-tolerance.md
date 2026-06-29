---
name: goal-tolerance-5cm
description: 导航终点容差为 5cm
metadata:
  type: project
---

Goal checker 的 xy_goal_tolerance 为 **0.05m (5cm)**。

控制器所有硬编码阈值必须与此容差协调：
- `TERMINAL_POS_ARRIVED = 0.005m` (< 5cm, 停止距离在容差内)
- `DEAD_ZONE_POS_MIN = 0.02m` (< 5cm, 死区在容差内停用)
- `terminal_angle_dist_ = 0.10m` (> 5cm, 终端模式在容差外接管)
- `terminal_angle_tolerance_ = 0.07rad` (~4°, 严于 goal checker yaw 容差)

**Why:** 终点容差决定控制器的终止策略设计。阈值必须确保机器人能进入容差范围并被 goal checker 检测到。
**How to apply:** 修改任何与终点停止相关的阈值时，确保 TERMINAL_POS_ARRIVED < 容差 < terminal_angle_dist_。
