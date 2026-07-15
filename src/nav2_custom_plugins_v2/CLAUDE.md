# THEMIS Navigation Workspace

## Workspace Layout

```
nav_ws/
├── src/
│   ├── nav2_custom_plugins/        # 旧 MPPI 插件 ([vx,vy,omega] 控制空间)
│   ├── nav2_custom_plugins_v2/     # 新 MPPI 插件 ([vx,vy,delta] 控制空间, GPU 加速)
│   ├── nav_launch/                 # 启动配置 & 参数
│   ├── velocity_controller/        # 底层速度控制器 (Python)
│   ├── navigation/                 # 导航配置
│   └── ...
```

## nav2_custom_plugins_v2 — MPPI Steering Controller

### 控制空间

`[vx, vy, delta]` — 解耦模型:
- **vx, vy**: 全局初始朝向系线速度, 不随机器人朝向旋转
- **delta**: 目标身体朝向角, 以 max_w 限速跟踪, 不影响线速度方向

### 模块架构 (9 个模块)

```
MPPISteeringController (Nav2 接口适配 + 流程编排)
  ├── mppi_core          — 参数 + 类型 + 运动学
  ├── path_manager       — 路径解析 + 前瞻点 + 最近点
  ├── cost_evaluator     — 代价评估
  ├── velocity_postprocessor — EMA→减速→钳位→δ→ω
  ├── gpu_engine         — GPU 引擎 (噪声/缓冲区/内核)
  ├── state_machine      — 状态机
  ├── visualization      — RViz
  └── mppi_logger        — 日志统计
```

### 运动学模型

```
位置积分 (全局系, 无需旋转):
  x[t+1] = x[t] + vx[t] * dt
  y[t+1] = y[t] + vy[t] * dt

朝向跟踪 (以 max_w 限速向 delta 靠拢):
  Δθ = clamp(normalize(δ[t] - θ[t]), -max_w*dt, +max_w*dt)
  θ[t+1] = θ[t] + Δθ

δ → ω (TwistStamped 向后兼容):
  ω = clamp(normalize(δ - θ) / dt, ±max_w)
```

### 消息

- 输入: 标准 Nav2 (`Path`, `PoseStamped`, `OccupancyGrid`)
- 输出: `VelocitySteering.msg` (`vx, vy, steering_angle`) 发布到 `/cmd_vel_steering`
- 兼容: `TwistStamped` 返回给 Nav2 controller_server

### 构建

```bash
colcon build --packages-select nav2_custom_plugins_v2
```

### 参考

旧包 `nav2_custom_plugins` 保留在 `../nav2_custom_plugins/` 供算法参考,
不拷贝其代码。新包全部从零编写。
