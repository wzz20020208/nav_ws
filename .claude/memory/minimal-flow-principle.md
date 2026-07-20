---
name: minimal-flow-principle
description: 逐步搭建 — 每个新模块先用已有类跑通最小端到端，再加功能
metadata:
  type: project
---

## 最小流程原则

不着急写完整的 controller / kernel / 全功能。
每搭一个新模块，先用**已有的、已验证的类**拼出最小端到端流程，编译 + 运行通过后，再往上加。

## 已验证的模块

- GPUEngine — registerBuffer / upload / download (测试通过)
- GPUUploader — registerAll / uploadNoise / uploadBase / uploadCostmap / uploadPath (测试通过)
- mppi_gpu_common.cuh — normalize_angle / costmap_bilinear (编译通过)
- mppi_gpu_critics.cuh — ObstacleCritic + CriticManager + 三类归一化 + 可配置大类权重 (编译 + kernel 内 evaluate 通过)
- mppi_gpu_kernels.cu — launch_mppi_sample_kernel / launch_mppi_weighted_sum_kernel (编译通过)
- mppi_steering_controller.cpp — 最小 Nav2 插件骨架 (configure 建 GPU 引擎 + 注册 buffer, compute 返回零速度)

## 当前文件结构

```
nav2_custom_plugins_v2/
├── include/nav2_custom_plugins_v2/
│   ├── gpu/
│   │   ├── gpu_engine.hpp          GPU 引擎 (map 管理 buffer)
│   │   └── gpu_uploader.hpp        GPU 上传管理
│   ├── mppi_core.hpp               ControlSequence 等基础类型
│   ├── mppi_steering_controller.hpp Nav2 插件头文件
│   └── ... (其他业务模块, 未实现)
├── src/
│   └── gpu/
│       ├── gpu_engine.cpp          引擎实现
│       ├── gpu_uploader.cpp        上传实现 (表驱动注册)
│       ├── test_gpu_upload.cpp     上传测试 (已通过)
│       ├── test_minimal_flow.cu    最小流程测试 (待编译)
│       ├── test_critics.cu         CriticManager 测试 (已通过)
│       ├── mppi_gpu_common.cuh     GPU 工具函数
│       ├── mppi_gpu_critics.cuh    代价函数族
│       └── mppi_gpu_kernels.cu     CUDA kernels + extern "C"
└── src/mppi_steering_controller.cpp  Nav2 插件最小实现
```

## IDE / clangd 配置

- 使用 clangd (禁用 Microsoft C/C++ 扩展)
- `compile_commands.json` 在 workspace 根目录 (从 build/ 拷贝)
- `.vscode/settings.json`: `C_Cpp.intelliSenseEngine: "disabled"`, clangd 指向 workspace 根
- `.clangd`: 含项目 + CUDA + ROS2 + 系统 include 路径
- 不再手动维护 c_cpp_properties.json

## 原则

- 每次只加一块
- 先写独立测试 (.cu / .cpp)，验证通过后，再考虑放进 controller
- 不在 GPUEngine 里放业务逻辑 (如 registerSampling)
- 上传/下载/内核各司其职，不交叉
