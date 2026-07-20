---
name: clangd-pitfalls
description: clangd 配置常见坑及修复
metadata:
  type: reference
---

## 5 个坑

1. **`-isystem` 缺空格**: `-isystem/path` → clangd 不识别，`std::string` 变 `int`。正确: `-isystem /path`
2. **GCC 版本错**: `--gcc-install-dir` 必须指向系统实际安装版本 (`gcc -v` 确认)
3. **`.cu`/`.cuh` 当 C++ 解析**: 需 `If: PathMatch: .*\.c(uh?|u)$` 切 `-x cuda` + `--cuda-gpu-arch=sm_80` (匹配 CUDA 13.0)
4. **`compile_commands.json` 优先级高于 `.clangd`**: 过期版本需删除/rename
5. **ROS2 嵌套头文件**: `pkg/header.hpp` 形式需手动加 `-I/opt/ros/humble/include/<pkg>`

**Why:** 每次重建 clangd 配置都会踩这些坑。
**How to apply:** 修改 `.clangd` 后 `Ctrl+Shift+P → clangd: Restart language server`。
