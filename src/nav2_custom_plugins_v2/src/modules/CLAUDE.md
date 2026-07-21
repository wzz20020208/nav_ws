# Modules (src/modules/)

## PathManager

### findClosestIndex
增量搜索: 从 `prev_closest_idx_` 向后扫，找最近路径点。"只进不退"。

### computeLookahead
1:1 THEMIS: Euclidean 距离 ≥ `min_lookahead_dist` 的首个点，默认终点兜底。

```cpp
for (int i = closest_idx; i <= last_idx; ++i) {
    if (hypot(plan[i] - robot) >= min_dist) { lh_idx = i; break; }
}
```

### getPlannerYaw / getTangentYaw
- `use_planner_yaw=true`: `atan2(ori.z, ori.w)` × 2
- `use_planner_yaw=false`: `atan2(dy, dx)` 相邻点切线

### buildGoalInfo
`target_vx_r, target_vy_r` = cos/sin(dir_to_lh - yaw) — body 系，与 rollout vx/vy 同系。

---

## StateMachine

### evaluateHeading
朝向偏差判定，在 MPPI 之前调用。

```
err = lookahead_yaw - current_yaw → normalize to [-π,π]
进入: abs(err) > heading_misalign_threshold (≈60°)
退出: abs(err) ≤ threshold × 0.5 (迟滞)
目标: dec.omega = current_yaw + err → if/else 归一到 [-π,π]
```

`rotate_in_place=true` 时: global 模式发 dec.omega 作目标角度，base_link 发 sign(err)×max_w。

---

## VelocityPostProcessor

```cpp
process(result, yaw, global_mode)
  → 提取 step0 vx/vy/omega
  → clamp vx→[min_v,max_v], vy→[-max_vy,max_vy], omega→[-max_w,max_w]
  → global 模式: vx/vy 旋转到 odom 系 (vx·cos - vy·sin, vx·sin + vy·cos)
```

---

## Visualization

Topic: `/mppi_visualization` (MarkerArray), frame_id="odom"

| id | 类型 | 内容 |
|----|------|------|
| 0 | ARROW (青) | 机器人朝向 |
| 1 | SPHERE (黄) | 前瞻点 |
| 2 | LINE_STRIP (橙) | 机器人→前瞻点连线 |
| 3 | LINE_STRIP (绿) | 最优轨迹 |
| 4 | LINE_LIST (浅蓝) | 采样轨迹散布 |
| 5 | ARROW (品红) | 速度指令 |
| 6 | TEXT_VIEW_FACING (白) | vx/vy/ang 数值 |
