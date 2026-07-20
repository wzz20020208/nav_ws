/**
 * @file gpu_engine.cpp
 * @brief GPUEngine 实现 — 封装全部 CUDA 交互
 *
 * CUDA 编程基础概念:
 *   - cudaMalloc:  在 GPU 显存上分配空间 (类似 malloc, 但在显卡上)
 *   - cudaFree:    释放 GPU 显存
 *   - cudaMemcpyAsync:  CPU ↔ GPU 之间拷贝数据, Async 表示异步 (不阻塞 CPU)
 *   - cudaStream_t: CUDA 流, 同一流内操作保证顺序执行, 不同流之间可并行
 *   - 异步操作返回后 CPU 继续跑, 需要 cudaStreamSynchronize 等 GPU 完成
 *
 * 本类不管理 stream — 由调用者创建/同步/销毁
 */

#include "nav2_custom_plugins_v2/gpu/gpu_engine.hpp"

#include <cstring>
#include <stdexcept>

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════
// 构造 / 析构 / 移动
// ═══════════════════════════════════════════════════════════════════════════

GPUEngine::~GPUEngine()
{
  // 释放所有已注册的 GPU buffer
  freeAll();
}

// 移动构造: 直接接管对方已注册的所有 buffer
// std::unordered_map 的移动是 O(1), 只转移内部指针不拷贝数据
GPUEngine::GPUEngine(GPUEngine &&other) noexcept
  : buffers_(std::move(other.buffers_))
{}

// 移动赋值: 先释放自己持有的 buffer, 再接管对方的
GPUEngine &GPUEngine::operator=(GPUEngine &&other) noexcept
{
  if (this != &other) {
    freeAll();                                // 释放旧资源
    buffers_ = std::move(other.buffers_);     // 接管新资源
  }
  return *this;
}

// 遍历所有已注册 buffer, 逐个 cudaFree
void GPUEngine::freeAll()
{
  for (auto &kv : buffers_) {
    if (kv.second.ptr) {
      // cudaFree: 释放 GPU 显存, 参数是 cudaMalloc 返回的指针
      cudaFree(kv.second.ptr);
      kv.second.ptr = nullptr;
    }
  }
  buffers_.clear();
}

// ═══════════════════════════════════════════════════════════════════════════
// Buffer 注册
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 按名称注册一个 GPU buffer
 *
 * 调用 cudaMalloc 在 GPU 显存上分配指定大小的空间,
 * 并将设备指针存入内部 map, 后续通过名称读写。
 *
 * 已存在的名称不会重复分配 (避免显存泄漏)。
 *
 * @param name   唯一标识名, 如 "noise_vx", "costmap", "costs"
 * @param bytes  分配字节数, 如 N * H * sizeof(float)
 */
void GPUEngine::registerBuffer(const std::string &name, size_t bytes)
{
  // 防重复: 同名 buffer 已注册则跳过
  if (buffers_.find(name) != buffers_.end()) return;

  // cudaMalloc: 在 GPU 设备端分配显存
  //   参数1: 输出 — 设备指针 (指向 GPU 显存地址)
  //   参数2: 分配字节数
  //   返回值: cudaSuccess (0) 表示成功, 其他值表示错误
  void *ptr = nullptr;
  cudaError_t err = cudaMalloc(&ptr, bytes);
  if (err != cudaSuccess) {
    // cudaGetErrorString: 将错误码转为可读字符串 (如 "out of memory")
    throw std::runtime_error(
        std::string("cudaMalloc(") + name + "): " + cudaGetErrorString(err));
  }

  // 存入 map: key=名称, value={设备指针, 尺寸}
  buffers_[name] = {ptr, bytes};
}

/**
 * @brief 按名称注销 buffer, 释放对应 GPU 显存
 */
void GPUEngine::unregisterBuffer(const std::string &name)
{
  auto it = buffers_.find(name);
  if (it != buffers_.end()) {
    if (it->second.ptr) cudaFree(it->second.ptr);
    buffers_.erase(it);
  }
}

/**
 * @brief 获取已注册 buffer 的 GPU 设备指针
 * @return 设备指针, 未注册则返回 nullptr
 *
 * 设备指针指向 GPU 显存, CPU 不能直接解引用,
 * 只能通过 cudaMemcpy 读写或传给 kernel 使用。
 */
void *GPUEngine::getDevicePtr(const std::string &name) const
{
  auto it = buffers_.find(name);
  return (it != buffers_.end()) ? it->second.ptr : nullptr;
}

// ═══════════════════════════════════════════════════════════════════════════
// 📤 Upload — 主机内存 → GPU 显存
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 将数据从 CPU 上传到指定的 GPU buffer
 *
 * cudaMemcpyAsync 是异步的:
 *   - 调用后立刻返回, 不等待拷贝完成
 *   - 同一 stream 内, 后续操作 (如 kernel 启动) 会等这次拷贝完成再执行
 *   - 调用者需要在合适时机 cudaStreamSynchronize(stream) 确保数据到位
 *
 * @param name       目标 buffer 名称 (需已注册)
 * @param host_data  CPU 端数据指针
 * @param stream     CUDA 流, 控制异步操作的执行顺序
 */
void GPUEngine::upload(const std::string &name, const void *host_data,
                       cudaStream_t stream)
{
  auto it = buffers_.find(name);
  if (it == buffers_.end()) return;  // 未注册, 静默跳过

  // cudaMemcpyAsync: 异步拷贝
  //   参数1: 目标地址 (GPU 设备指针)
  //   参数2: 源地址   (CPU 主机指针)
  //   参数3: 拷贝字节数
  //   参数4: 方向 — HostToDevice (CPU→GPU) 或 DeviceToHost (GPU→CPU)
  //   参数5: CUDA 流 (同流内保序)
  cudaMemcpyAsync(it->second.ptr, host_data, it->second.bytes,
                  cudaMemcpyHostToDevice, stream);
}

// ═══════════════════════════════════════════════════════════════════════════
// 📥 Download — GPU 显存 → 主机内存
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 将数据从 GPU buffer 下载到 CPU 内存
 *
 * 同样是异步的, 调用者随后需要 cudaStreamSynchronize 等待完成。
 *
 * @param name       源 buffer 名称 (需已注册)
 * @param host_data  CPU 端接收缓冲区 (调用者预先分配)
 * @param stream     CUDA 流
 */
void GPUEngine::download(const std::string &name, void *host_data,
                         cudaStream_t stream)
{
  auto it = buffers_.find(name);
  if (it == buffers_.end()) return;

  // cudaMemcpyAsync: 方向改为 DeviceToHost (GPU→CPU)
  cudaMemcpyAsync(host_data, it->second.ptr, it->second.bytes,
                  cudaMemcpyDeviceToHost, stream);
}

}  // namespace nav2_custom_plugins_v2
