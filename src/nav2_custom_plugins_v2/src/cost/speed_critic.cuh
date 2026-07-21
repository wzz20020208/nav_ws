/**
 * @file speed_critic.cuh
 * @brief SPEED 分类 — 速度方向对齐 + 终点距离
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 三层角色 (同 obstacle/heading 模式)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   SpeedCritic : CriticBase           ← 分类基类: kCategory=SPEED
 *     ↑ 继承
 *   SpeedRewardCritic : SpeedCritic    ← 速度方向对齐前瞻点
 *
 *   SpeedCategory                      ← 容器: 注册表 + evaluate()
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 算法参考 (来自 THEMIS nav2_custom_plugins PreferForwardCritic)
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   SpeedRewardCritic:
 *     speed = hypot(vx, vy)
 *     if speed ≈ 0: return 0
 *     angle_err = atan2(vy, vx) - atan2(target_vy_r, target_vx_r)
 *     abs_err = |angle_err|
 *     if abs_err > π/2 (背离前瞻点):
 *       return speed × (5.0 + 3.0 × (abs_err - π/2))  // 重度惩罚, 随角度递增
 *     else:
 *       return speed × (3.0 × abs_err - 1.0)          // 线性惩罚, 0°=奖励, ~19°=中性
 *
 *   线性惩罚优于 cos 对齐:
 *     - 0°: 返回 -speed (奖励, 负代价)
 *     - ~19°: 返回 ≈0 (中性)
 *     - >19°: 递增正惩罚
 *     - 梯度 = 3×speed/rad, 远强于 cos 在 0° 处的零梯度
 *
 *   TerminalDistCritic (见 mppi_gpu_kernels.cu, 非注册表子类):
 *     dist = sqrt((goal_x - x)² + (goal_y - y)²)
 *     仅轨迹终点调用一次, 不在 per-step evaluate() 中
 *
 * ═══════════════════════════════════════════════════════════════════════════════
 * 调用链
 * ═══════════════════════════════════════════════════════════════════════════════
 *
 *   CriticManager::evaluate(..., goal)
 *     → cat_w[SPEED] × speed_.evaluate(vx, vy, goal)
 *         → for i in subs_:
 *             subs_[i].fn(vx, vy, goal)  ← 函数指针直调
 *           return Σ(w × fn) / active_count
 */

#ifndef MPPI_SPEED_CRITIC_CUH_
#define MPPI_SPEED_CRITIC_CUH_

#include "mppi_gpu_common.cuh"                        // M_PI_F, M_PI_2_F, normalize_angle
#include "nav2_custom_plugins_v2/core/mppi_core.hpp"  // GoalInfo
#include "critic_common.cuh"                          // CriticBase, CriticCategory

// ═══════════════════════════════════════════════════════════════════════════════
// SpeedCritic — SPEED 分类基类
// ═══════════════════════════════════════════════════════════════════════════════

class SpeedCritic : public CriticBase
{
public:
  /// 归属大类 — CriticManager 用它做第二大层加权
  static constexpr CriticCategory kCategory = CriticCategory::SPEED;
};

// ═══════════════════════════════════════════════════════════════════════════════
// SpeedRewardCritic — 速度方向矢量对准前瞻点
// ═══════════════════════════════════════════════════════════════════════════════
//
// target = 机器人→前瞻点方向 (从 buildGoalInfo 传入)
// err = normalize_angle(atan2(vy,vx) - target)
// raw = speed × (3×|err| - 1),  cost = sigmoid(raw)

class SpeedRewardCritic : public SpeedCritic
{
public:
  /// speed reward: 1:1 THEMIS — 对齐奖励, 侧向重罚, 反向重罚
  ///   alignment = cos(err), lateral = |sin(err)|
  ///   return -speed × alignment + 2.0 × speed × lateral
  ///   0° → -speed (奖励),  26.6° → 0 (中性),  45° → +0.71×speed (惩罚)
  __device__ float compute(float vx, float vy, const GoalInfo &goal) const
  {
    float speed = hypotf(vx, vy);
    if (speed < 0.02f) return 0.0f;  // 停止中性
    float alignment = (vx * goal.target_vx_r + vy * goal.target_vy_r) / speed;  // cos(err)
    float lateral = fabsf(vx * goal.target_vy_r - vy * goal.target_vx_r) / speed; // |sin(err)|
    if (alignment < 0.0f) return speed * 5.0f;  // 反向 (>90°): 重罚
    return -speed * alignment + 2.0f * speed * lateral;  // 对齐奖励 + 侧向抑制
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// SpeedCategory — SPEED 分类容器
// ═══════════════════════════════════════════════════════════════════════════════
//
// 管理 SPEED 大类下的子代价函数, 通过函数指针注册表统一调度。
// 注意: TerminalDistCritic 不在此注册表中 — 它是 per-trajectory 终点代价,
// 在 kernel 中单独计算。

class SpeedCategory
{
public:
  // ═════════════════════════════════════════════════════════════════════════
  // SubFn — 子代价函数签名
  // ═════════════════════════════════════════════════════════════════════════
  //
  // SPEED 子类只需要速度分量和目标方向, 不需要位置/朝向/代价地图。
  // 签名与 obstacle/heading 不同: 每个大类有独立的 SubFn。

  typedef float (*SubFn)(float vx, float vy, const GoalInfo &goal);

  /// 注册表条目: { 函数指针, 开关, 大类内权重 }
  struct SubEntry
  {
    SubFn fn;         ///< 函数指针 → 直调 compute()
    bool  enabled;    ///< false 时 evaluate() 跳过
    float weight;     ///< 大类内权重系数
  };

  // ═════════════════════════════════════════════════════════════════════════
  // 静态包装函数
  // ═════════════════════════════════════════════════════════════════════════

  __device__ static float speedRewardFn(
      float vx, float vy, const GoalInfo &goal)
  {
    SpeedRewardCritic c;
    return c.compute(vx, vy, goal);
  }

  // ═════════════════════════════════════════════════════════════════════════
  // 注册表操作
  // ═════════════════════════════════════════════════════════════════════════

  __host__ __device__ void init()
  {
    subs_[0] = { speedRewardFn, true, 1.0f };
    count_ = 1;
  }

  __host__ __device__ void setWeight(int idx, float w)
  {
    if (idx >= 0 && idx < count_) subs_[idx].weight = w;
  }

  __host__ __device__ void setEnabled(int idx, bool on)
  {
    if (idx >= 0 && idx < count_) subs_[idx].enabled = on;
  }

  // ═════════════════════════════════════════════════════════════════════════
  // evaluate — SPEED 大类求值
  // ═════════════════════════════════════════════════════════════════════════
  //
  /// 遍历注册表 → 函数指针直调 → 加权平均
  /// @return SPEED 大类归一化代价 (可能为负 = 奖励)
  __device__ float evaluate(float vx, float vy, const GoalInfo &goal) const
  {
    float total = 0.0f;
    int active = 0;

    for (int i = 0; i < count_; ++i) {
      const SubEntry &e = subs_[i];
      if (!e.enabled) continue;
      total += e.weight * e.fn(vx, vy, goal);
      active++;
    }

    return total;  // THEMIS: weighted sum
  }

private:
  static constexpr int kMaxSubs = 4;
  SubEntry subs_[kMaxSubs];
  int count_ = 0;
};

#endif  // MPPI_SPEED_CRITIC_CUH_
