---
name: gpu-buffer-registration-pattern
description: GPU buffer 注册采用表驱动模式，新增 buffer 只需加一行配置
metadata:
  type: project
---

## GPU buffer 注册模式

`GPUUploader::registerAll()` 不硬编码每个 buffer 的注册调用，而是遍历一张静态配置表 `kSamplingBuffers[]`。

表结构:
- `name`: buffer 名称 (字符串 key)
- `size`: 尺寸模式枚举 (`BufSize::NxH | H | N | Hx4`)

`toBytes(size, N, H)` 将枚举转为实际字节数。

新增 buffer 时:
1. 在 `kSamplingBuffers[]` 表中加一行 `{"name", BufSize::XX}`
2. 如果尺寸模式不匹配现有枚举，在 `BufSize` 和 `toBytes()` 中添加新的 case
3. `registerAll()` 函数体不动

**文件:** [[src/nav2_custom_plugins_v2/src/gpu/gpu_uploader.cpp]]