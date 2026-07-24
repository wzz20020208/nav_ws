#ifndef NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_
#define NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_

/**
 * @file gpu_engine.hpp
 * @brief GPUEngine — 封装所有 GPU 交互, 上层只需按名称读写 buffer
 *
 * 使用方式:
 *   GPUEngine gpu;
 *   gpu.registerBuffer("noise_vx", N*H*sizeof(float));   // 注册 → 分配显存
 *   gpu.registerBuffer("costmap",  w*h);                 // 注册 → 分配显存
 *   gpu.upload("noise_vx", host_ptr, stream);            // CPU → GPU
 *   gpu.download("costs", host_buf, stream);             // GPU → CPU
 *
 * 关键概念:
 *   - 设备指针 (d_ptr):  指向 GPU 显存, CPU 不能直接解引用
 *   - cudaStream_t:      异步操作队列, 同流保序, 不同流可并行
 *   - upload / download: 都是异步的, 调用者负责 cudaStreamSynchronize
 */

#include <cuda_runtime.h>

#include <string>
#include <unordered_map>

// 前向声明 (完整定义在 core/mppi_core.hpp, 全局命名空间)
struct CostmapInfo;
struct Footprint;
struct PathInfo;
struct GoalInfo;
struct CriticParams;

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════════
// MPPI GPU buffer 名称常量 — uploader + kernel launcher 共用, 唯一真相源
// ═══════════════════════════════════════════════════════════════════════════════

namespace buf
{
  // 输入 — 噪声 (N×H)
  inline constexpr char noise_vx[]    = "noise_vx";
  inline constexpr char noise_vy[]    = "noise_vy";
  inline constexpr char noise_w[] = "noise_w";

  // 输入 — warm-start 基序列 (H)
  inline constexpr char base_vx[]     = "base_vx";
  inline constexpr char base_vy[]     = "base_vy";
  inline constexpr char base_w[]  = "base_w";

  // 输出 — 采样控制量 (N×H)
  inline constexpr char sampled_vx[]    = "sampled_vx";
  inline constexpr char sampled_vy[]    = "sampled_vy";
  inline constexpr char sampled_omega[] = "sampled_omega";

  // 输出 — 代价 (N)
  inline constexpr char costs[] = "costs";

  // 输出 — 加权结果序列 (H×4)
  inline constexpr char result_seq[] = "result_seq";

  // 输出 — 轨迹世界坐标 (N×H, 可为空)
  inline constexpr char traj_x[]     = "traj_x";
  inline constexpr char traj_y[]     = "traj_y";
  inline constexpr char traj_theta[] = "traj_theta";

  // 输入 — costmap + 路径
  inline constexpr char costmap[] = "costmap";
  inline constexpr char path_x[]  = "path_x";
  inline constexpr char path_y[]  = "path_y";
}

/// 注册在 GPUEngine 中的 buffer 元数据
struct GPUBuffer
{
  void *ptr = nullptr;   ///< GPU 设备指针 (cudaMalloc 返回)
  size_t bytes = 0;      ///< 分配字节数
};

class GPUEngine
{
public:
  GPUEngine() = default;
  ~GPUEngine();

  // 不可拷贝 (GPU 资源不能被两个对象共同拥有)
  GPUEngine(const GPUEngine &) = delete;
  GPUEngine &operator=(const GPUEngine &) = delete;

  // 支持移动 (unique_ptr 语义, 转移所有权)
  GPUEngine(GPUEngine &&other) noexcept;
  GPUEngine &operator=(GPUEngine &&other) noexcept;

  // ═══════════════════════════════════════════════════════════════
  // Buffer 注册 / 注销 / 查询
  // ═══════════════════════════════════════════════════════════════

  /// 注册一个命名的 GPU buffer, 调用 cudaMalloc 分配显存
  /// 若同名 buffer 已存在则跳过 (幂等)
  void registerBuffer(const std::string &name, size_t bytes);

  /// 注销并释放一个 GPU buffer
  void unregisterBuffer(const std::string &name);

  /// 查询已注册 buffer 的设备指针, 未注册返回 nullptr
  void *getDevicePtr(const std::string &name) const;

  // ═══════════════════════════════════════════════════════════════
  // 📤 Upload — CPU → GPU
  // ═══════════════════════════════════════════════════════════════

  /// 异步上传: 将 host_data 拷贝到名为 name 的 GPU buffer
  void upload(const std::string &name, const void *host_data,
              cudaStream_t stream);

  // ═══════════════════════════════════════════════════════════════
  // 📥 Download — GPU → CPU
  // ═══════════════════════════════════════════════════════════════

  /// 异步下载: 将名为 name 的 GPU buffer 拷贝到 host_data
  void download(const std::string &name, void *host_data,
                cudaStream_t stream);

  // ═══════════════════════════════════════════════════════════════
  // 📊 Compute — kernel 启动
  // ═══════════════════════════════════════════════════════════════

  /// 代价评估 kernel: N 线程并行, CriticManager 评估 H 步代价求和 → d_costs[N]
  void launchCostKernel(
      const struct CostmapInfo &cmap, const struct Footprint &fp,
      const struct PathInfo &path, const struct GoalInfo &goal,
      const struct CriticParams &critic_params,
      float cost_scale, int N, int H, cudaStream_t stream);

  /// 加权求和 kernel: softmax 加权 N 条轨迹控制量 → d_result_seq[H×4]
  void launchWeightedSumKernel(float min_cost, float lambda,
                               int N, int H, cudaStream_t stream);

private:
  /// name → { 设备指针, 字节数 }
  std::unordered_map<std::string, GPUBuffer> buffers_;

  /// 释放全部已注册 buffer
  void freeAll();
};

}  // namespace nav2_custom_plugins_v2

#endif  // NAV2_CUSTOM_PLUGINS_V2__GPU_ENGINE_HPP_
