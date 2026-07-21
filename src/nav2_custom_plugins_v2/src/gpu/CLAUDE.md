# GPU Infrastructure (src/gpu/)

## 文件

| 文件 | 角色 |
|------|------|
| `mppi_gpu_common.cuh` | 工具函数: costmap bilinear, normalize_angle, M_PI_F |
| `mppi_gpu_kernels.cu` | cost_eval_kernel, weighted_sum_kernel |
| `mppi_gpu_critics.cuh` | [兼容转发] → cost/critic_manager.cuh |
| `gpu_engine.cpp/.hpp` | GPU 缓冲区管理, kernel 启动 |
| `gpu_uploader.cpp/.hpp` | 缓冲区名→指针映射, cudaMemcpyAsync, uploadPath |

## GPUEngine

管理 GPU 缓冲区 (by name)，提供 `getDevicePtr(buf_name)`。
Kernel 启动:
- `launchCostKernel(cmap, fp, path, goal, N, H, stream)` → d_costs[N]
- `launchWeightedSumKernel(min_cost, lambda, N, H, stream)` → d_result[H×4]

## GPUUploader

- `registerAll(N, H)`: 注册所有缓冲区 (base_seq, rollout x/y/θ/vx/vy/ω, costs, result)
- `uploadBase(base_seq, H, stream)`: 上传 warm-start 序列
- `uploadRollout(batch, N, H, stream)`: 上传 N×H 轨迹
- `uploadPath(xs, ys, n, stream)`: 上传路径点

## Kernel (mppi_gpu_kernels.cu)

### cost_eval_kernel
每线程一条轨迹 (N threads):
```
for t in 0..H:
    CriticManager.evaluate(x, y, θ, vx, vy, cmap, fp, path, goal)
    // → OBSTACLE + HEADING + SPEED 三层代价
    // 前瞻越界惩罚: if (along > 0) total += overshoot_weight × along²
d_costs[s] = total / H
```

### weighted_sum_kernel
单 block (H threads):
```
for s in 0..N:
    w = exp(-(cost[s] - min_cost) / lambda)
    for t in 0..H:
        result[t] += w × rollout[s,t]
result /= Σw
```

## MPPIPipeline (pipeline/)

CPU→GPU 桥接层:
```cpp
uploadBase(base_seq, H, stream)
uploadRollout(batch, N, H, stream)
launchCost(cmap, fp, path, goal, N, H, stream) → costs
launchWeightedSum(min_cost, lambda, N, H, stream) → result
```
