# THEMIS MPPI Steering Controller

> **规则**: 任何代码修改必须先经用户确认同意，不得擅自改动。

## 控制空间

`[vx, vy, omega]` — body 系:
- **vx, vy**: body 系线速度 (前/左), 积分时按 theta 旋转到世界系
- **omega**: body 系角速度 (rad/s), 直接积入 theta

## 文件结构

```
include/nav2_custom_plugins_v2/
├── core/mppi_core.hpp               # 参数/类型/运动学
├── gpu/{gpu_engine, gpu_uploader}.hpp
├── pipeline/mppi_pipeline.hpp
├── mppi_steering_controller.hpp     # Nav2 插件入口
└── modules/{path_manager, velocity_postprocessor, state_machine, visualization}.hpp

src/
├── cost/                            # 代价函数 → 详见 src/cost/CLAUDE.md
├── gpu/                             # GPU 基础设施 → 详见 src/gpu/CLAUDE.md
├── modules/                         # 功能模块 → 详见 src/modules/CLAUDE.md
├── core/mppi_core.cpp
├── pipeline/mppi_pipeline.cpp
└── mppi_steering_controller.cpp
```

## 每帧数据流 (computeVelocityCommands, 10Hz)

```
setPlan(path) → path_mgr_.setPath, state_machine_.reset

① start = {x, y, yaw} from pose
② path_mgr_: closest → lookahead → yaw (planner/自算) → PathInfo/GoalInfo
②b state_machine_.evaluateHeading → rotate_in_place?
      ├── YES → 直接发旋转指令 (dec.omega = current_yaw + err → [-π,π]), return
      └── NO  ↓
③ noise_gen_.generate(N, H, lookahead_yaw, yaw)
④ batch_rollout(start, base_seq, noise, params) → BatchTrajectories
⑤ uploadBase + uploadRollout → GPU
⑥ uploadPath → GPU
⑦ CostmapInfo + Footprint
⑧ launchCost → d_costs[N]
⑨ min_cost = min(costs), best_idx
⑩ launchWeightedSum → result[H×4]
⑪ vel_postprocessor_.process(result, yaw, global_mode)
⑫ global 模式: omega_out = fmod(lookahead_yaw, [0,2π)) 直接发目标角度
⑬ 前瞻减速: scale = kp + (1-kp)×min(1, dist/decel_dist)
⑭ 终端对齐: dist<terminal_angle_dist 时直接发 goal_yaw
⑮ base_seq_.shiftAndDecay + fill
⑯ TwistStamped: frame_id, linear.x/y, angular.z (global=目标角度, base_link=角速度)
```

## 输出模式

- **global (true)**: frame_id="odom", angular.z=目标角度 [0,2π)
- **base_link (false)**: frame_id="BASE_LINK", angular.z=角速度 (rad/s)

## 开发原则

- 旧包 `nav2_custom_plugins` 仅作算法参考，不拷贝代码
- GPU 上禁用虚函数，用函数指针注册表实现多态
- 每个功能模块独立文件，controller 只做编排
- 每次修改后 git commit
