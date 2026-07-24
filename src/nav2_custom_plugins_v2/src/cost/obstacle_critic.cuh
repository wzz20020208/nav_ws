/**
 * @file obstacle_critic.cuh
 * @brief OBSTACLE 分类 — 障碍物代价: 基类 + 足迹子类 + 容器(注册表函数指针)
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 三层角色
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   ObstacleCritic : CriticBase       ← 分类基类: 声明此代价属于 OBSTACLE 大类
 *     ↑ 继承                             子类通过继承自动获得 kCategory=OBSTACLE
 *   FootprintCritic : ObstacleCritic  ← 具体子类: 只负责 compute(), 完全无状态
 *
 *   ObstacleCategory                  ← 容器: 持有注册表 subs_[], 管理子类调度
 *     subs_[i] = { fn, enabled, weight }  每个槽位绑定一个代价函数 + 元数据
 *     evaluate()                         遍历 subs_ → 直接调 fn → 加权和
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 完整调用链 (从 GPU kernel 到实际计算)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   CriticManager::evaluate(x,y,θ,vx,vy,cmap,fp)
 *     → cat_weights_[OBSTACLE] × obstacle_.evaluate(...)
 *         → for i in 0..count_:
 *             subs_[i].fn(x,y,cos_t,sin_t,vx,cmap,fp)    ← 函数指针直调
 *               → FootprintCritic::compute(x,y,cos_t,sin_t,vx,cmap,fp)
 *                   → 足迹网格采样 → costmap 双线性插值 → 四次方碰撞惩罚
 *             total += subs_[i].weight × result
 *           return total                                    ← 加权和
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 新增子类的步骤 (以 DistanceFieldCritic 为例)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   1. 写子类: class DistanceFieldCritic : public ObstacleCritic { compute() }
 *   2. 写 wrapper: __device__ static float distanceFieldFn(...) { D c; return c.compute(...); }
 *   3. 注册: init() 内加 subs_[1] = { distanceFieldFn, true, 0.4f }
 *   → CriticManager 完全不需要改
 */

#ifndef MPPI_OBSTACLE_CRITIC_CUH_
#define MPPI_OBSTACLE_CRITIC_CUH_

#include "mppi_gpu_common.cuh"                        // costmap_bilinear()
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"  // CostmapInfo, Footprint
#include "critic_common.cuh"                          // CriticBase, CriticCategory

// ═══════════════════════════════════════════════════════════════════════════════
// ObstacleCritic — OBSTACLE 分类基类
// ═══════════════════════════════════════════════════════════════════════════════
//
// 作用: 声明"我是障碍物类代价", 提供共享工具 (未来扩展)。
// 所有障碍物相关的代价子类继承自此, 自动获得 kCategory=OBSTACLE。
//
// 为什么存在但几乎是空的:
//   - kCategory 让 CriticManager 知道此代价所属大类, 用于大类间加权
//   - 未来多个子类 (Footprint + DistanceField 等) 共享的工具函数可提升至此
//   - GPU 上不可用虚函数, 实际分发由 ObstacleCategory 容器通过函数指针完成

class ObstacleCritic : public CriticBase
{
public:
  /// 归属大类 — CriticManager 用它做第二大层加权:
  ///   total += cat_weights_[OBSTACLE] × obstacle_.evaluate()
  static constexpr CriticCategory kCategory = CriticCategory::OBSTACLE;
};

// ═══════════════════════════════════════════════════════════════════════════════
// FootprintCritic — 足迹采样碰撞检测 (ObstacleCritic 子类)
// ═══════════════════════════════════════════════════════════════════════════════
//
// 完全无状态 — 所有数据从参数传入, 实例仅作为 compute() 的载体。
// GPU 上每个线程临时构造即可, 零内存开销 (编译器优化掉)。
//
// 算法:
//   1. 将机器人足迹 (front/back × left/right) 按 sample_spacing 离散为 nx×ny 网格
//   2. 每个采样点经旋转矩阵 [cos -sin; sin cos] 变换到世界坐标
//   3. 在 costmap 上双线性插值读取栅格值 val ∈ [0,255]
//   4. val ≥ 1.0 的采样点: n = val/255, 累加 n⁴  (碰撞点四次方惩罚)
//   5. 返回 总累加 / (nx*ny)  (全足迹平均, 保证不同采样密度下代价可比)
//
// 四次方惩罚的设计理由:
//   - n ∈ [0,1], n⁴ 对低成本 ≈ 0, 对高成本急剧放大
//   - val=10  → n≈0.04 → n⁴=0.000002  (轻微膨胀 → 几乎无惩罚)
//   - val=254 → n≈0.996→ n⁴=0.984      (严重膨胀 → 几乎满分惩罚)
//   - 线性惩罚无法区分"擦边"和"撞上", n⁴ 可以

class FootprintCritic : public ObstacleCritic
{
public:
  /// @brief 计算单轨迹步的障碍物碰撞代价
  /// @param x, y      机器人世界坐标 (轨迹积分后的位置)
  /// @param cos_t, sin_t  cos(θ), sin(θ) — 调用者预计算, 避免重复三角函数
  /// @param vx         当前步 x 方向速度, 用于倒车检测 (>0 前进, <0 倒车)
  /// @param cmap       代价地图: 数据指针 + 尺寸 + 分辨率 + 原点偏移
  ///                   注: CostmapInfo::data 指向 GPU 显存中的 unsigned char 数组
  /// @param fp         机器人足迹参数: 前后/左右尺寸 + 采样间距 + 倒车垫高值
  /// @return           归一化代价 ∈ [0,1], 0=完全安全, 1=每个采样点都 lethal(255)
  __device__ float compute(
      float x, float y, float cos_t, float sin_t, float vx,
      const CostmapInfo &cmap, const Footprint &fp) const
  {
    // 空地图保护: costmap 未就绪时返回 0, 不惩罚任何轨迹
    if (cmap.data == nullptr || cmap.w <= 0 || cmap.h <= 0) return 0.0f;

    // ── 步骤 1: 计算采样网格密度 ──
    // flx = 足迹总长 (m), fly = 足迹总宽 (m)
    // nx/ny = 采样点数 = floor(尺寸 / 采样间距) + 1, 钳位 [2, 10]
    //   下限 2: 至少覆盖足迹两端 (前后或左右)
    //   上限 10: 防止 10×10=100 次双线性插值拖慢 GPU kernel
    float flx = fp.front + fp.back;    // 足迹总长度 (前后方向, m)
    float fly = fp.left + fp.right;    // 足迹总宽度 (左右方向, m)

    // __float2int_rd: CUDA 内建函数, float→int 向负无穷方向取整 = floor()
    int nx = __float2int_rd(flx / fp.sample_spacing) + 1;  // 长度方向采样点数
    int ny = __float2int_rd(fly / fp.sample_spacing) + 1;  // 宽度方向采样点数
    nx = (nx < 2) ? 2 : ((nx > 10) ? 10 : nx);             // 钳位到 [2, 10]
    ny = (ny < 2) ? 2 : ((ny > 10) ? 10 : ny);

    // ── 步骤 2: 计算采样步长 ──
    // sx/sy = 相邻采样点间距 (m)
    // 例: flx=0.6m, nx=8 → sx=0.6/(8-1)=0.0857m
    float sx = (nx > 1) ? flx / (nx - 1) : 0.0f;  // 长度方向步长
    float sy = (ny > 1) ? fly / (ny - 1) : 0.0f;  // 宽度方向步长
    float acc = 0.0f;  // n⁴ 碰撞代价累加器

    // ── 步骤 3 & 4: 双层循环遍历所有足迹采样点 ──
    // 局部坐标系: 原点 = base_link (机器人中心)
    //   lx ∈ [-back, +front]  向前为正
    //   ly ∈ [-right, +left]  向左为正
    // 旋转矩阵 [cosθ -sinθ; sinθ cosθ] 将局部坐标转到世界系:
    //   wx = x + lx*cosθ - ly*sinθ
    //   wy = y + lx*sinθ + ly*cosθ
    for (int iy = 0; iy < ny; ++iy) {
      float ly = -fp.right + iy * sy;          // 局部 y, 从最右侧扫描到最左侧
      for (int ix = 0; ix < nx; ++ix) {
        float lx = -fp.back + ix * sx;         // 局部 x, 从最后端扫描到最前端

        // 2D 旋转变换: 局部坐标 → 世界坐标
        float wx = x + lx * cos_t - ly * sin_t;
        float wy = y + lx * sin_t + ly * cos_t;

        // costmap 双线性插值: 世界坐标 → 栅格坐标 → 插值 → [0, 255]
        // 越界点自动钳位到 costmap 边界栅格
        float val = costmap_bilinear(wx, wy, cmap.data,
            cmap.w, cmap.h, cmap.res, cmap.origin_x, cmap.origin_y);

        // ── 倒车特殊处理 ──
        // 当采样点在车身后半部 (lx<0) 且正在倒车 (vx<0) 时,
        // 用 rear_obstacle_cost (默认 160) 垫高该点的代价。
        // 模拟倒车雷达: 倒车时后方传感器盲区大, 需要更保守的检测。
        if (lx < 0.0f && vx < 0.0f && fp.rear_obstacle_cost > 0.0f)
          val = fmaxf(val, fp.rear_obstacle_cost);

        // ── 碰撞检测 + 四次方惩罚 ──
        // costmap 值含义: 0=free, 1-254=inscribed/inflated, 255=lethal
        // val ≥ 1 表示有障碍物膨胀, 做四次方惩罚
        // n = val/255 ∈ [0,1], n⁴ 对低成本接近 0
        if (val >= 1.0f) {
          float n = val / 255.0f;        // 归一化到 [0, 1]
          acc += n * n * n * n;          // 四次方放大
        }
      }
    }

    // ── 步骤 5: 取平均 → 返回 ──
    // 除以采样点总数, 保证不同 footprint 尺寸或采样密度下代价可比
    return acc / static_cast<float>(nx * ny);
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// ObstacleCategory — OBSTACLE 分类容器
// ═══════════════════════════════════════════════════════════════════════════════
//
// 核心职责: 管理 OBSTACLE 大类下的所有子代价函数, 提供统一的 evaluate() 接口。
// CriticManager 只持有此类实例, 不需要知道内部有哪些子类。
//
// 设计关键 — 函数指针注册表:
//   - 每个 SubEntry 自带 fn 函数指针, evaluate() 遍历表直接调用
//   - 无需 switch-case 或虚函数, 编译器知道函数指针目标 → 仍可内联优化
//   - 子类是 stateless 的, wrapper 函数内临时构造实例调用 compute()
//
// 为什么用 int 索引而非枚举做 setWeight/setEnabled:
//   - 枚举值在注册表槽位数量可控 (≤4) 时过度设计
//   - 直接用数组索引更简洁: setWeight(0, 0.8f) 设第一个子类权重

class ObstacleCategory
{
public:
  // ═════════════════════════════════════════════════════════════════════════
  // SubFn — 子代价函数签名
  // ═════════════════════════════════════════════════════════════════════════
  //
  // 所有 OBSTACLE 子类的 compute() 必须与此签名兼容。
  // 参数说明:
  //   x, y      — 世界坐标
  //   cos_t, sin_t — 朝向预计算值, 避免重复三角函数
  //   vx        — x 方向速度, FootprintCritic 用它判断倒车
  //   cmap      — 代价地图 (数据在 GPU 显存)
  //   fp        — 足迹参数 (前后左右尺寸 + 采样间距)
  // 返回:       — 归一化代价

  typedef float (*SubFn)(float x, float y, float cos_t, float sin_t,
                         float vx, const CostmapInfo &cmap, const Footprint &fp);

  // ═════════════════════════════════════════════════════════════════════════
  // SubEntry — 注册表条目
  // ═════════════════════════════════════════════════════════════════════════
  //
  // 每个条目绑定一个子代价函数的三要素:
  //   fn      — 函数指针, evaluate() 循环中直接调用, 无需 switch 转发
  //   enabled — 运行时开关, false 时 evaluate() 跳过此条目
  //   weight  — 子类在大类内的权重, 用于区分不同子类的重要程度
  //
  // 示例: subs_[0] = { footprintFn, true, 1.0f }
  //        subs_[1] = { distanceFn,  true, 0.4f }
  //        → OBSTACLE 总分 = 1.0*fn0 + 0.4*fn1  (加权和)

  struct SubEntry
  {
    SubFn fn;         ///< 指向 wrapper 函数的指针 (__device__ 静态方法)
    bool  enabled;    ///< 运行时开关, false=跳过, 默认 true
    float weight;     ///< 大类内权重系数, 默认 1.0
  };

  // ═════════════════════════════════════════════════════════════════════════
  // 静态包装函数 — 每个子类一个
  // ═════════════════════════════════════════════════════════════════════════
  //
  // 为什么需要 wrapper:
  //   子类的 compute() 是成员函数, 无法直接取地址赋给普通函数指针。
  //   wrapper 是静态函数, 内部临时构造子类实例 → 调用 compute()。
  //   子类完全无状态, 临时构造零开销 (nvcc 会优化掉)。
  //
  // 新增子类时在此区域加对应的 wrapper 函数。

  /// FootprintCritic 的 wrapper
  __device__ static float footprintFn(
      float x, float y, float cos_t, float sin_t, float vx,
      const CostmapInfo &cmap, const Footprint &fp)
  {
    FootprintCritic c;  // stateless, 临时构造 → 编译器优化为零开销
    return c.compute(x, y, cos_t, sin_t, vx, cmap, fp);
  }
  // TODO: __device__ static float distanceFieldFn(...) { DistanceFieldCritic c; return c.compute(...); }

  // ═════════════════════════════════════════════════════════════════════════
  // 注册表操作
  // ═════════════════════════════════════════════════════════════════════════

  /// 初始化注册表: 填入所有启用的子代价函数
  /// __host__ __device__: 可在 CPU 或 GPU 端调用 (kernel 内由 CriticManager::init() 调用)
  __host__ __device__ void init()
  {
    // 槽位 0: 足迹碰撞检测 (当前唯一子类, 权重 1.0)
    subs_[0] = { footprintFn, true, 1.0f };
    // 槽位 1: 距离场代价 (未来)
    // subs_[1] = { distanceFieldFn, true, 0.4f };
    count_ = 1;  // 当前已注册的子类数量
  }

  /// 按槽位索引设置权重
  /// @param idx  注册表槽位 (0-based, < count_)
  /// @param w    新权重值 (无范围校验, 调用者保证合理)
  __host__ __device__ void setWeight(int idx, float w)
  {
    if (idx >= 0 && idx < count_) subs_[idx].weight = w;
  }

  /// 按槽位索引开关子类
  /// @param idx  注册表槽位
  /// @param on   true=启用, false=禁用 (evaluate 跳过)
  __host__ __device__ void setEnabled(int idx, bool on)
  {
    if (idx >= 0 && idx < count_) subs_[idx].enabled = on;
  }

  // ═════════════════════════════════════════════════════════════════════════
  // evaluate — 大类求值入口
  // ═════════════════════════════════════════════════════════════════════════
  //
  /// 遍历注册表中所有启用的子类 → 调函数指针 → 加权和
  ///
  /// 公式: result = Σ_i (weight[i] × subs_[i].fn(...))
  ///
  /// 调用频率极高 (N×H×10Hz), 设计上追求:
  ///   - 循环体简洁 (一次函数指针调用 + 一次乘加)
  ///   - 无分支预测压力 (count_≤4, 循环展开友好)
  ///   - 函数指针目标在编译期已知 → nvcc 可能内联
  ///
  /// @param x, y      世界坐标
  /// @param cos_t, sin_t  cos(θ), sin(θ) 预计算值
  /// @param vx         当前步 x 速度 (传入子类用于倒车检测, heading/speed 子类忽略)
  /// @param vy         当前步 y 速度 (预留, obstacle 子类不使用)
  /// @param cmap       代价地图
  /// @param fp         足迹参数
  /// @return           OBSTACLE 大类归一化代价
  __device__ float evaluate(
      float x, float y, float cos_t, float sin_t,
      float vx, float vy,
      const CostmapInfo &cmap, const Footprint &fp) const
  {
    float total = 0.0f;   // 加权代价累加: Σ (weight × compute_result)
    int active = 0;       // 启用的子类计数 (当前仅用于跳过 disabled 条目)

    for (int i = 0; i < count_; ++i) {
      const SubEntry &e = subs_[i];
      if (!e.enabled) continue;  // 跳过被禁用的子类

      // 函数指针直调 → 无 switch, 无虚表, 编译器知道目标
      total += e.weight * e.fn(x, y, cos_t, sin_t, vx, cmap, fp);
      active++;
    }

    // 大类内加权和 (不平均), active=0 时返回 0
    return total;  // THEMIS: weighted sum
  }

private:
  // ═════════════════════════════════════════════════════════════════════════
  // 成员变量
  // ═════════════════════════════════════════════════════════════════════════

  /// 注册表最大容量 (编译期常量, 栈上分配, 零堆内存)
  static constexpr int kMaxSubs = 4;

  /// 子代价函数注册表
  ///   每个槽位 = { 函数指针, 开关, 权重 }
  ///   由 init() 填充, setWeight/setEnabled 运行时调整
  ///   新增子类: 在 init() 加一行, 无需改 evaluate()
  SubEntry subs_[kMaxSubs];

  /// 当前已注册的子类数量 (≤ kMaxSubs)
  /// evaluate() 只遍历 subs_[0..count_-1]
  int count_ = 0;
};

#endif  // MPPI_OBSTACLE_CRITIC_CUH_
