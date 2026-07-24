# THEMIS Hybrid-A* Global Planner

> **规则**: 任何代码修改必须先经用户确认同意，不得擅自改动。

## 算法概述

Hybrid-A*: 连续状态空间 [x, y, θ] 上的 A* 搜索, 使用运动学可行的运动原语 (恒定曲率弧线).

### 关键组件

1. **运动原语**: 差速底盘恒定曲率弧线, 控制空间 `[curvature, arc_length]`
2. **启发式** (两取最大):
   - 2D Dijkstra 障碍物距离 (从 goal 反向传播)
   - Reeds-Shepp 无碰撞路径长度 (非完整约束)
3. **分析扩张**: 每 N 次节点扩展, 尝试 RS 曲线直连 goal
4. **碰撞检测**: 机器人外包矩形采样点, 逐点查 costmap
5. **路径后处理**: 可见性简化 + 切线朝向赋值

## 文件结构

```
include/nav2_hybrid_a_star_planner/
├── hybrid_a_star_planner.hpp   # Nav2 插件入口 (nav2_core::GlobalPlanner)
├── hybrid_a_star.hpp           # Hybrid-A* 核心算法
└── planner_types.hpp           # 共享类型/参数/常量

src/
├── hybrid_a_star_planner.cpp   # 插件实现 (lifecycle + createPlan)
└── hybrid_a_star.cpp           # 算法实现 (搜索/碰撞/RS曲线/后处理)
```

## createPlan 数据流

```
① 参数校验: start/goal frame 一致性, 坐标在 costmap 内
② 转换: world → map 坐标
③ 锁 costmap, 复制 cost 数组到内部 buffer, 解锁
④ computeObstacleHeuristic(goal_map) → 2D Dijkstra 距离场
⑤ generateMotionPrimitives(turning_radius, arc_length)
⑥ A* 主循环:
     ├── 弹出 f_cost 最小节点
     ├── visited 去重检查
     ├── 分析扩张 (每 N 次): RS 曲线 → 碰撞检测 → 成功则返回
     └── 运动原语展开:
           ├── integrate(state, primitive) → next_state
           ├── 边界 + 碰撞检测
           └── g_cost + h_cost → push open_set
⑦ 路径重建: 从 goal node 回溯 parent_idx
⑧ 路径简化: 贪心可见性去冗余点
⑨ assignOrientations: 切线方向赋 yaw
⑩ 填充 nav_msgs::msg::Path, 返回
```

## 参数 (YAML)

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `max_iterations` | 50000 | A* 最大展开次数 |
| `goal_tolerance` | 0.3 | 目标到达容差 (m) |
| `turning_radius` | 0.5 | 最小转弯半径 (m) |
| `arc_length` | 0.4 | 运动原语弧长 (m) |
| `heading_bins` | 72 | 角度离散化 (72=5°) |
| `analytic_expansion_interval` | 10 | RS 分析扩张频率 |
| `allow_unknown` | false | 是否允许经过未知区域 |
| `num_angle_quantization` | 64 | Reeds-Shepp 角度离散化 |

## 开发原则

- 遵循 MPPI 插件 v2 的代码规范 (命名/注释/文件组织)
- 纯 C++ 实现, 无 CUDA 依赖
- 一个类一个文件, 插件类只做编排
- 每次修改后 git commit
