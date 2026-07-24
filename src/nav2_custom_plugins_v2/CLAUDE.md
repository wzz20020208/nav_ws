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

## 修改记录 (2026-07-24)

### BaseSimilarityCritic — 时序一致性正则

新增 SPEED 大类子代价，对每条采样轨迹每步控制量 `(vx, vy, omega)` 与 warm-start base 序列求欧氏距离平方：
```
cost = (vx - base_vx[t])² + (vy - base_vy[t])² + (ω - base_ω[t])²
```
约束相邻帧最优序列不跳变，解决 vy 帧间横跳。

### SpeedRewardCritic 改进

1. **lateral 系数 2.0 → 6.0**: 中性点从 26.6° 缩到 9.5°，偏离前瞻点惩罚大幅增强
2. **奖励速度 cap**: `reward_speed = min(speed, max_feasible_v)`, max_feasible_v 由 `max_v/max_vy` 矩形包络在该方向的可达速度自动算出。超速无额外甜头，速度自然稳在可行范围
3. **GoalInfo 加 max_feasible_v**: PathManager::buildGoalInfo 自动计算

### Bug 修复: uploadBase double→float

`MPPIPipeline::uploadBase` 原来将 `std::vector<double>` 直接以 `const double*` 上传到 GPU 的 `float*` buffer (H×4 字节 vs H×8 字节)，GPU 读到随机位模式 → NaN/Inf → 代价链污染 → total=nan → weighted sum 输出垃圾 → base_seq 被腐蚀 → H 帧后恢复正常。现已修复为 float 转换后再上传。

### 参数调整

| 参数 | 旧 | 新 | 原因 |
|------|-----|-----|------|
| lateral 系数 | 2.0 | 6.0 | 压窄偏离容忍 |
| speed_ratio | 0.10 | 0.15 | 速度代价声音更大 |
| base_similarity_weight | - | 0.5 | 帧间一致性 |
| action_std_v | 0.5 | 0.2 | 降低纵向探索噪声 |
| action_std_vy | 0.5 | 0.1 | 降低侧向探索噪声 |
