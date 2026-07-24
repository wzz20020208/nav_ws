# MPPI Steering Controller (THEMIS)

GPU-accelerated MPPI (Model Predictive Path Integral) controller for Nav2, with decoupled `[vx, vy, omega]` control space.

## Overview

This plugin replaces Nav2's default controller with a sampling-based MPPI optimizer. At each 10Hz control cycle, it generates **N** trajectories over a horizon of **H** steps, evaluates them against multi-category cost functions on GPU, and outputs the exponentially-weighted control sequence.

### Key Features

- **Decoupled control space** — `[vx, vy, omega]` in body frame. vx/vy control linear velocity (forward/left), omega controls angular velocity independently.
- **GPU acceleration** — CUDA kernels evaluate all N×H trajectory steps in parallel: costmap footprint collision detection, path alignment, heading angle, speed reward, and a weighted-sum kernel for the MPPI update.
- **Warm-start base sequence** — Previous frame's optimal control sequence is shifted and reused as the sampling baseline, improving temporal consistency.
- **Heading state machine** — Pre-MPPI check: if heading error exceeds threshold, skip MPPI and output a pure rotation command.
- **Terminal alignment** — Hysteresis-based final orientation alignment near the goal.
- **Lookahead deceleration** — Speed scales smoothly as the robot approaches the lookahead point.
- **Dual output mode** — `global` mode outputs target yaw angle in odom frame; `base_link` mode outputs angular velocity.
- **RViz visualization** — MarkerArray on `/mppi_visualization` showing robot pose, lookahead point, sampled trajectories, best trajectory, and velocity command.
- **Cost breakdown logging** — Optional per-frame cost component dump to file for tuning.

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                  Nav2 Controller API                 │
│         computeVelocityCommands(pose, vel)           │
└──────────────────────┬──────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│              MPPISteeringController                  │
│  ┌─────────────┐  ┌──────────┐  ┌────────────────┐  │
│  │ PathManager │  │StateMach.│  │ NoiseGenerator │  │
│  │ (closest,   │  │ (heading │  │ (CPU, NLN)     │  │
│  │  lookahead, │  │  check)  │  │                │  │
│  │  yaw)       │  │          │  │                │  │
│  └─────────────┘  └──────────┘  └────────────────┘  │
│                                                      │
│  ┌──────────────────────────────────────────────┐    │
│  │              MPPIPipeline (CPU↔GPU bridge)    │    │
│  │  ┌───────────┐  ┌───────────┐  ┌──────────┐  │    │
│  │  │ GPUEngine │  │GPUUploader│  │CriticMgr │  │    │
│  │  │ (buffers, │  │ (memcpy,  │  │ (cost    │  │    │
│  │  │  kernels) │  │  register)│  │  eval)   │  │    │
│  │  └───────────┘  └───────────┘  └──────────┘  │    │
│  └──────────────────────────────────────────────┘    │
│                                                      │
│  ┌─────────────────┐  ┌────────────────────────┐     │
│  │ VelocityPostProc│  │ VisualizationPublisher │     │
│  │ (extract,clamp, │  │ (MarkerArray → RViz)   │     │
│  │  δ→ω, transform)│  │                        │     │
│  └─────────────────┘  └────────────────────────┘     │
└──────────────────────────────────────────────────────┘
```

## File Structure

```
├── include/nav2_custom_plugins_v2/
│   ├── mppi_steering_controller.hpp   # Nav2 plugin entry point
│   ├── core/mppi_core.hpp             # Types, params, kinematics, noise, rollout
│   ├── gpu/gpu_engine.hpp             # GPU buffer management + kernel launch
│   ├── gpu/gpu_uploader.hpp           # GPU async upload (base, rollout, path)
│   ├── pipeline/mppi_pipeline.hpp     # CPU↔GPU bridge layer
│   └── modules/
│       ├── param_loader.hpp           # YAML → MPPIParams
│       ├── path_manager.hpp           # Closest point, lookahead, yaw, path info
│       ├── velocity_postprocessor.hpp # Extract, clamp, δ→ω, frame transform
│       ├── state_machine.hpp          # Heading misalignment → rotate-in-place
│       └── visualization.hpp          # RViz MarkerArray publisher
├── src/
│   ├── mppi_steering_controller.cpp   # Main control loop (computeVelocityCommands)
│   ├── core/mppi_core.cpp             # Rollout, kinematics, control sequence
│   ├── gpu/
│   │   ├── gpu_engine.cpp             # GPU buffer allocation, kernel launchers
│   │   ├── gpu_uploader.cpp           # Buffer registry, cudaMemcpyAsync wrappers
│   │   └── mppi_gpu_kernels.cu        # CUDA kernels: cost_eval, weighted_sum
│   ├── cost/                          # Cost functions (see "Cost System Design" below)
│   ├── modules/                       # Module implementations
│   └── pipeline/mppi_pipeline.cpp     # Pipeline glue
├── test/test_critics.cu               # Unit test for cost functions
├── msg/VelocitySteering.msg           # Custom message: vx, vy, steering_angle
├── CMakeLists.txt                     # Build with CUDA + ROS 2
├── plugins.xml                        # Plugin registration for Nav2
└── package.xml                        # ROS 2 package manifest
```

## Control Flow (10Hz)

```
① Read pose → RobotState {x, y, yaw}
② PathManager: findClosestIndex → computeLookahead → getYaw (planner or tangent)
②b StateMachine: evaluateHeading → if |err| > threshold:
      → output pure rotation, skip MPPI
③ NoiseGenerator: generate N×H NLN noise (biased toward lookahead direction)
④ batch_rollout: u = base + noise, kinematic integrate → N trajectories
⑤ uploadBase → GPU (warm-start sequence, H×4 floats)
⑥ uploadRollout → GPU (trajectories, N×H×6 floats)
⑦ uploadPath → GPU (path waypoints)
⑧ Assemble CostmapInfo + Footprint + PathInfo + GoalInfo
⑨ launchCost → GPU cost_eval_kernel → d_costs[N]
⑩ CPU scan: min_cost = min(costs), best_idx = argmin
⑪ launchWeightedSum → GPU kernel → result[H×4] (weighted average control)
⑫ VelocityPostProcessor: extract step0 → clamp → δ→ω (base_link) or rotate (global)
⑬ Lookahead deceleration: scale = kp + (1-kp) × min(1, dist/decel_dist)
⑭ Terminal alignment: if dist < threshold, output goal_yaw directly
⑮ base_seq_.shiftAndDecay(0.5) + fill with new control
⑯ Publish TwistStamped + RViz markers
```

## Cost Functions

Costs are evaluated on GPU via a three-category hierarchy:

| Category | Weight | Sub-Critics | Description |
|----------|--------|-------------|-------------|
| **OBSTACLE** | 0.60 | FootprintCritic | Footprint grid sampling → costmap bilinear interpolation → n⁴ collision penalty |
| **HEADING** | 0.30 | PathAlignCritic | Squared distance to nearest path segment |
| | | PathAngleCritic | 4 × (heading_error²), with cosine annealing near goal |
| | | PathDeviationCritic | Excess beyond corridor width (soft wall) |
| **SPEED** | 0.10 | SpeedRewardCritic | THEMIS formula: alignment reward + lateral suppression (6×) |
| | | BaseSimilarityCritic | Temporal consistency: (u − base_u[t])² regularization |

### Cost Formula

```
total = cat_w[OBSTACLE] × Σ(w_sub × footprint_cost)
      + cat_w[HEADING]  × Σ(w_sub × (align + angle + deviation))
      + cat_w[SPEED]    × Σ(w_sub × (speed_reward + base_similarity))

d_costs[s] = cost_scale × (total / H) + overshoot_penalty + terminal_dist
```

### SpeedRewardCritic (THEMIS)

```
speed = hypot(vx, vy)
if speed < 0.02 → neutral (0)
alignment = (vx·target_vx_r + vy·target_vy_r) / speed   // cos(err)
lateral   = |vx·target_vy_r - vy·target_vx_r| / speed    // |sin(err)|
if alignment < 0 → speed × 5.0                            // reverse penalty
return -min(speed, max_feasible_v) × alignment + 6.0 × speed × lateral
```

### Cost System Design

GPU 上禁用了虚函数（no vtable），采用**函数指针注册表**实现运行时多态。三层调用链：

```
CriticManager → XxxCategory → XxxCritic::compute()
```

#### Class Hierarchy

```
CriticBase                              (critic_common.cuh)
├── ObstacleCritic : CriticBase         (obstacle_critic.cuh)
│   └── FootprintCritic                 — footprint grid → costmap bilinear → n⁴ collision
├── HeadingCritic : CriticBase          (heading_critic.cuh)
│   ├── PathAlignCritic                — min squared distance to path segment
│   ├── PathAngleCritic                — 4×err², cosine annealing near goal
│   └── PathDeviationCritic            — excess beyond corridor width (soft wall)
└── SpeedCritic : CriticBase            (speed_critic.cuh)
    ├── SpeedRewardCritic              — THEMIS alignment reward + lateral suppression
    └── BaseSimilarityCritic           — temporal consistency: (u − base_u[t])²
```

#### Container Structure

```
CriticManager                           (critic_manager.cuh)
├── ObstacleCategory  — subs_[0] = {footprintFn}
├── HeadingCategory   — subs_[0..2] = {pathAlignFn, pathAngleFn, pathDeviationFn}
└── SpeedCategory     — subs_[0..1] = {speedRewardFn, baseSimilarityFn}
```

Each category's weight (`obstacle_ratio` / `tracking_ratio` / `speed_ratio`) is multiplied by the category's internal weighted sum.

#### Two-Layer Normalization

- **Layer 1** — intra-category: weighted sum `Σ(w_sub × fn_output)`, no averaging
- **Layer 2** — inter-category: `cat_weights[c] × category.evaluate(...)`

```
total = Σ cat_weights[c] × category.evaluate(...)
```

#### Per-Step Kernel Call

```
CriticManager::evaluate(x, y, θ, vx, vy, ω, cmap, fp, path, goal)
  ├── cat_w[OBSTACLE] × obstacle_.evaluate()
  │     → FootprintCritic: footprint grid → costmap bilinear → n⁴ collision
  ├── cat_w[HEADING]  × heading_.evaluate()
  │     → PathAlign:      min squared distance to path segment (m²)
  │     → PathAngle:      4×err² (raw squared radians)
  │     → PathDeviation:  excess², where excess = max(0, dist − 0.5m)
  └── cat_w[SPEED]   × speed_.evaluate()
        → SpeedReward:      THEMIS formula (alignment reward + lateral ×6)
        → BaseSimilarity:   (vx−base_vx)² + (vy−base_vy)² + (ω−base_ω)²
```

#### GPU Data Structures

| Struct | Category | Fields |
|--------|----------|--------|
| `CostmapInfo` | OBSTACLE | `data` ptr, `w`, `h`, `res`, `origin_x`, `origin_y` |
| `Footprint` | OBSTACLE | `front`, `back`, `left`, `right`, `sample_spacing`, `rear_obstacle_cost` |
| `PathInfo` | HEADING | `x`/`y` ptr (GPU memory), `num_pts`, `path_tangent`, `goal_yaw`, `goal_x`, `goal_y` |
| `GoalInfo` | SPEED | `target_vx_r`, `target_vy_r`, `max_feasible_v`, `goal_x`, `goal_y`, `lookahead_x`, `lookahead_y`, `lookahead_overshoot_weight` |
| `CriticParams` | ALL | 3 category weights + 6 sub-weights (see Parameters table above) |

#### Adding a New Cost Sub-Class

To add a new critic to the HEADING category:

1. Write subclass: `class NewCritic : public HeadingCritic { float compute(...) { ... } }`
2. Write wrapper: `static float newCriticFn(...) { NewCritic c; return c.compute(...); }`
3. Register in `HeadingCategory::init()`: `subs_[N] = {newCriticFn, true, weight}`
4. `CriticManager` stays unchanged — the new critic is picked up automatically.

## MPPI Update

Exponentially-weighted average over trajectories:

```
w[s] = exp(-(cost[s] - min_cost) / λ)

result[t].vx     = Σ w[s] × batch.vx[s,t] / Σ w[s]
result[t].vy     = Σ w[s] × batch.vy[s,t] / Σ w[s]
result[t].omega  = Σ w[s] × batch.omega[s,t] / Σ w[s]
```

The first step (`t=0`) of the result is extracted as the control command.

## Parameters

All parameters are read from the Nav2 controller YAML config under the plugin namespace.

### Control Space

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `num_samples` | int | 8000 | Number of trajectory samples (N) |
| `prediction_horizon` | int | 5 | Trajectory length in steps (H) |
| `dt` | double | 0.05 | Integration time step (s) |
| `max_v` | double | 0.4 | Max forward velocity (m/s) |
| `min_v` | double | -0.4 | Max reverse velocity (m/s) |
| `max_vy` | double | 0.2 | Max lateral velocity (m/s) |
| `max_w` | double | 0.6 | Max angular velocity (rad/s) |

### Noise

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `action_std_v` | double | 0.2 | Std dev of vx noise |
| `action_std_vy` | double | 0.1 | Std dev of vy noise |
| `action_std_w` | double | 0.4 | Std dev of omega noise |

### Cost Weights

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `lambda` | double | 0.05 | MPPI temperature |
| `cost_scale` | double | 50.0 | Overall cost scaling factor |
| `obstacle_ratio` | double | 0.70 | OBSTACLE category weight |
| `tracking_ratio` | double | 0.20 | HEADING category weight |
| `speed_ratio` | double | 0.05 | SPEED category weight |
| `footprint_weight` | double | 1.0 | OBSTACLE sub-weight |
| `path_align_weight` | double | 1.0 | PathAlign sub-weight |
| `path_angle_weight` | double | 2.0 | PathAngle sub-weight |
| `path_deviation_weight` | double | 0.3 | PathDeviation sub-weight |
| `speed_reward_weight` | double | 1.0 | SpeedReward sub-weight |
| `base_similarity_weight` | double | 0.5 | BaseSimilarity sub-weight |

### Footprint

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `footprint_front` | double | 0.17 | Front overhang (m) |
| `footprint_back` | double | 0.17 | Rear overhang (m) |
| `footprint_left` | double | 0.28 | Left overhang (m) |
| `footprint_right` | double | 0.28 | Right overhang (m) |

### Lookahead & Terminal

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `min_lookahead_dist` | double | 0.8 | Min lookahead distance (m) |
| `lookahead_kp` | double | 0.3 | Min speed ratio at lookahead (0=stop, 1=no decel) |
| `lookahead_decel_dist` | double | 0.5 | Distance to begin deceleration (m) |
| `lookahead_overshoot_weight` | double | 5.0 | Penalty for passing lookahead point |
| `terminal_angle_dist` | double | 0.3 | Distance to trigger terminal alignment (m) |
| `terminal_angle_tolerance` | double | 0.17 | Alignment tolerance (rad, ≈10°) |

### State Machine

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `enable_heading_speed_limit` | bool | true | Enable pre-MPPI heading check |
| `heading_misalign_threshold` | double | 1.047 | Heading error threshold (rad, ≈60°) |

### Output Mode

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `use_global_mode` | bool | true | true=odom frame (target angle); false=base_link (angular velocity) |
| `use_planner_yaw` | bool | true | true=planner orientation; false=self-computed tangent |

### Logging

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `enable_file_log` | bool | true | Enable cost breakdown logging |
| `log_file_path` | string | /tmp/mppi_steering_controller.log | Log file path |

## Dependencies

- **ROS 2** (Humble or later)
- **Nav2** (nav2_core, nav2_costmap_2d, nav2_util)
- **CUDA Toolkit** (≥11.0)
- **GPU** with compute capability ≥8.0 (SM80: A100; SM86: RTX 30xx; SM89: RTX 40xx; SM90: H100; SM120: Blackwell)
  > **注意**: 请根据实际设备调整 `CMakeLists.txt` 中的 `CUDA_ARCHITECTURES`：
  > - 部分设备的 SM 版本未在上方列出，可通过 `nvidia-smi --query-gpu=compute_cap --format=csv` 查询
  > - CUDA 版本过旧时（如 CUDA 11.x），`sm120` 会报 `Unsupported gpu architecture` 错误，需删除该值
  > - 例如：笔记本 RTX 4060 为 SM89，Jetson Orin 为 SM87，RTX 5090 为 SM120
- **pluginlib**, **tf2_ros**, **tf2_geometry_msgs**

## Build

```bash
cd ~/your_ws
colcon build --packages-select nav2_custom_plugins_v2 --cmake-args -DCMAKE_BUILD_TYPE=Release
```

The build compiles:
- `libnav2_custom_plugins_v2_gpu.so` — the plugin shared library
- `test_critics` — GPU cost function unit test

## Usage

In your Nav2 controller server config:

```yaml
controller_server:
  ros__parameters:
    controller_plugin_ids: ["MPPISteeringController"]
    MPPISteeringController:
      plugin: "nav2_custom_plugins_v2/MPPISteeringController"
      num_samples: 8000
      prediction_horizon: 5
      dt: 0.05
      max_v: 0.4
      max_vy: 0.2
      max_w: 0.6
      # ... (all parameters above)
```

## Published Topics

| Topic | Type | Description |
|-------|------|-------------|
| `/cmd_vel_mppi` | `geometry_msgs/Twist` | Raw MPPI output (before Nav2 smoothing) |
| `/mppi_visualization` | `visualization_msgs/MarkerArray` | Debug visualization for RViz |

## Output Modes

### Global Mode (`use_global_mode: true`)

- `frame_id`: `"odom"`
- `linear.x`, `linear.y`: velocity in odom frame (postprocessor rotates body-frame vx/vy)
- `angular.z`: **target yaw angle** in `[0, 2π)`
- Useful when a downstream node handles the yaw tracking (e.g., a separate steering controller)

### Base-Link Mode (`use_global_mode: false`)

- `frame_id`: `"BASE_LINK"`
- `linear.x`, `linear.y`: velocity in body frame
- `angular.z`: **angular velocity** in rad/s (clamped to `[-max_w, max_w]`)
- Standard Nav2 TwistStamped interface

## Tuning Guide

1. **Obstacle avoidance too aggressive**: reduce `obstacle_ratio` or `cost_scale`
2. **Robot cuts corners**: increase `path_align_weight` or `path_angle_weight`
3. **Robot drives too slowly**: increase `speed_ratio` or `speed_reward_weight`
4. **Lateral oscillation (vy jitter)**: increase `base_similarity_weight`, reduce `action_std_vy`
5. **Too much exploration noise**: reduce `action_std_v`, `action_std_vy`, `action_std_w`
6. **Overshoots lookahead point**: increase `lookahead_overshoot_weight` or `min_lookahead_dist`
7. **Stops too early before goal**: reduce `lookahead_kp` or `lookahead_decel_dist`
8. **Heading oscillation near goal**: adjust `terminal_angle_tolerance` and `terminal_angle_dist`
9. **MPPI temperature**: lower `lambda` → greedier output (sharp softmax), higher → more uniform (flat softmax)

Enable `enable_file_log: true` and inspect `/tmp/mppi_steering_controller.log` to see per-frame cost component breakdown:

```
cost: total=X.XX | angle=X.XX(w=X.XX) align=X.XX(w=X.XX) speed=X.XX(w=X.XX) obst=X.XX(w=X.XX) | ...
output: ctrl_omg=X.XX out_omg=X.XX ctrl_vx=X.XX ctrl_vy=X.XX global=1
```

## Performance

Benchmark results on **NVIDIA RTX 5060 Laptop** (SM 120, 26 SMs, 7.5 GB, CUDA 12.0).  
Run with: `./build/nav2_custom_plugins_v2/benchmark_mppi`

### GPU Kernel Throughput (default config: N=8000, H=5)

| Kernel | Time | Throughput |
|--------|------|------------|
| `cost_eval_kernel` (cost evaluation) | **0.076 ms** | 524M trajectory-step evaluations/s |
| `weighted_sum_kernel` (softmax weighting) | **0.152 ms** | 263M weight computations/s |
| **Total GPU time per frame** | **~0.23 ms** | — |

### GPU vs CPU Comparison (H=5, 20-thread OpenMP)

| N | GPU (ms) | CPU 1-thread (ms) | CPU 20-thread (ms) | GPU Speedup vs 20t |
|---|----------|-------------------|---------------------|---------------------|
| 1000 | 0.064 | 2.78 | 0.27 | **4×** |
| 4000 | 0.066 | 10.37 | 6.98 | **106×** |
| 8000 | 0.077 | 20.77 | 7.48 | **97×** |
| 16000 | 0.114 | 42.22 | 7.02 | **62×** |

### Key Takeaways

- **GPU is barely loaded at current settings.** Cost evaluation time stays nearly flat as N scales from 1000 to 16000 (0.064→0.114 ms, only 1.8× increase for 16× more work). The 26 SMs are far from saturation.
- **Per-frame GPU budget is only 0.23 ms** — just **0.23%** of the 100 ms control cycle at 10 Hz.
- **Massive headroom for future computation:**
  - Higher sample counts (N=32000+, deeper exploration)
  - Longer horizons (H=20+, further lookahead)
  - Dense semantic costmaps (lane markings, traversability scores)
  - Learned cost functions (CNN-encoded features, vision-based critics)
  - Multi-sensor fusion (LiDAR point clouds, depth images as GPU textures)
  - Real-time trajectory optimization with richer dynamics models
- At N=8000, GPU is **~100× faster** than a 20-thread CPU implementation, freeing CPU cores for perception, localization, and other Nav2 components.

### GPU Memory Footprint (default: N=8000, H=5)

| Component | Size |
|-----------|------|
| Trajectory buffers | 1.56 MB |
| Costmap (200×200) | 0.04 MB |
| Path (50 pts) | <0.01 MB |
| **Total** | **1.60 MB** |

Memory usage scales linearly with N×H — 6.2 MB at N=16000, H=10. Even entry-level GPUs with 4 GB VRAM have abundant space.