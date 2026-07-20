---
name: no-copy-old-code
description: 严禁从旧包 nav2_custom_plugins 拷贝代码
metadata:
  type: feedback
---

# 严禁拷贝旧代码

`src/nav2_custom_plugins/` 是旧 MPPI 实现（[vx,vy,omega]），仅作算法参考。

**严禁**从旧包拷贝以下内容到 v2：
- `.cuh` GPU 头文件（`mppi_gpu_common.cuh`, `mppi_gpu_rewards.cuh` 等）
- `.cu` kernel 文件（`mppi_gpu_kernels.cu`）
- 任何包含旧控制空间逻辑的代码

**Why:** v2 是完全重写的新架构（[vx,vy,delta]），模块化设计（mppi_core / path_manager / cost_evaluator / kinematic_model 等）。拷贝旧代码会污染新架构，破坏重构意义。

**How to apply:** GPU 侧代码应从头设计，调用 v2 已有的 `__host__ __device__` 模块方法，不做内联裸写。函数命名与结构与 v2 host 端模块对应。
