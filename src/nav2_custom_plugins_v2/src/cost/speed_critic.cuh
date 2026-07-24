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
 *   SpeedRewardCritic (1:1 THEMIS):
 *     speed = hypot(vx, vy)
 *     if speed < 0.02: return 0                          // 停止中性
 *     alignment = (vx×target_vx_r + vy×target_vy_r) / speed  // cos(err)
 *     lateral   = |vx×target_vy_r - vy×target_vx_r| / speed   // |sin(err)|
 *     if alignment < 0: return speed × 5.0                // 反向重罚
 *     return -speed × alignment + 6.0 × speed × lateral   // 对齐奖励 + 侧向抑制
 *
 *   效果:
 *     - 0°: 返回 -speed (最大奖励)
 *     - ~9.5°: 返回 ≈0 (中性, atan(1/6))
 *     - 45°: 返回 +3.53×speed (惩罚)
 *     - >90°: 返回 +5.0×speed (重度惩罚)
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
 *           return Σ(w × fn)  // 加权和
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
  ///   return -reward_speed × alignment + 6.0 × speed × lateral
  ///   0° → -speed (奖励),  26.6° → 0 (中性),  45° → +0.71×speed (惩罚)
  __device__ float compute(float vx, float vy, const GoalInfo &goal) const
  {
    float speed = hypotf(vx, vy);
    if (speed < 0.02f) return 0.0f;  // 停止中性
    float alignment = (vx * goal.target_vx_r + vy * goal.target_vy_r) / speed;  // cos(err)
    float lateral = fabsf(vx * goal.target_vy_r - vy * goal.target_vx_r) / speed; // |sin(err)|
    if (alignment < 0.0f) return speed * 5.0f;  // 反向 (>90°): 重罚
    float reward_speed = fminf(speed, goal.max_feasible_v);  // cap 奖励上限
    return -reward_speed * alignment + 6.0f * speed * lateral;
  }
};

// ═══════════════════════════════════════════════════════════════════════════════
// BaseSimilarityCritic — warm-start 一致性约束
// ═══════════════════════════════════════════════════════════════════════════════
//
// 比较当前采样控制量 (vx, vy, omega) 与 warm-start base 序列对应步的控制量,
// 欧氏距离平方作为代价 — 偏差越大代价越高, 约束相邻帧最优序列不跳变。
//
//   代价 = (vx - base_vx[t])² + (vy - base_vy[t])² + (ω - base_ω[t])²

class BaseSimilarityCritic : public SpeedCritic
{
public:
  __device__ float compute(float vx, float vy, float omega,
                           int t, const float *base_vx,
                           const float *base_vy, const float *base_omega) const
  {
    float dvx = vx - base_vx[t];
    float dvy = vy - base_vy[t];
    float dw  = omega - base_omega[t];
    return dvx * dvx + dvy * dvy + dw * dw;
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
  // SPEED 子类需要速度分量和目标方向, 不需要位置/朝向/代价地图。
  // 签名与 obstacle/heading 不同: 每个大类有独立的 SubFn。
  //
  // 新增 base_vx/vy/omega + t: 供 BaseSimilarityCritic 与 warm-start 比较;
  // SpeedRewardCritic 忽略这些参数。

  typedef float (*SubFn)(float vx, float vy, float omega,
                         const GoalInfo &goal,
                         int t, const float *base_vx,
                         const float *base_vy, const float *base_omega);

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
      float vx, float vy, float omega,
      const GoalInfo &goal,
      int /*t*/, const float * /*base_vx*/,
      const float * /*base_vy*/, const float * /*base_omega*/)
  {
    SpeedRewardCritic c;
    (void)omega;  // SpeedRewardCritic 不使用 omega/base
    return c.compute(vx, vy, goal);
  }

  __device__ static float baseSimilarityFn(
      float vx, float vy, float omega,
      const GoalInfo &goal,
      int t, const float *base_vx,
      const float *base_vy, const float *base_omega)
  {
    (void)goal;  // BaseSimilarityCritic 不使用 goal
    BaseSimilarityCritic c;
    return c.compute(vx, vy, omega, t, base_vx, base_vy, base_omega);
  }

  // ═════════════════════════════════════════════════════════════════════════
  // 注册表操作
  // ═════════════════════════════════════════════════════════════════════════

  __host__ __device__ void init()
  {
    subs_[0] = { speedRewardFn,       true, 1.0f };
    subs_[1] = { baseSimilarityFn,    true, 0.0f };
    count_ = 2;
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
  /// 遍历注册表 → 函数指针直调 → 加权和
  /// @param t 当前时间步, base 指针: 供 BaseSimilarityCritic 与 warm-start 比较
  /// @return SPEED 大类代价 (可能为负 = 奖励, 加权和)
  __device__ float evaluate(float vx, float vy, float omega,
                             const GoalInfo &goal,
                             int t, const float *base_vx,
                             const float *base_vy, const float *base_omega) const
  {
    float total = 0.0f;
    int active = 0;

    for (int i = 0; i < count_; ++i) {
      const SubEntry &e = subs_[i];
      if (!e.enabled) continue;
      total += e.weight * e.fn(vx, vy, omega, goal, t, base_vx, base_vy, base_omega);
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
