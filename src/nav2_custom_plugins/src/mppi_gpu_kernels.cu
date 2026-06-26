/**
 * @file mppi_gpu_kernels.cu
 * @brief MPPI GPU 内核 — 采样、轨迹滚动、代价计算与全时域加权求和
 *
 * 本文件是 MPPI GPU 加速的入口，包含两个 CUDA 内核:
 *   1. mppi_sample_kernel      — 并行采样 N 条轨迹并计算每条代价
 *   2. mppi_weighted_sum_kernel — 按代价指数加权，对每个 timestep 独立求和
 *
 * 以及两个 host 端包装函数 (extern "C"):
 *   3. mppi_gpu_sample_and_cost — 上传数据到 GPU，启动采样内核
 *   4. mppi_gpu_weighted_sum    — 启动加权求和内核
 *
 * 探索范围两层衰减机制:
 *   宏观层 (CPU 每帧): exploration_range_scale ∈ [floor, 1.0]
 *     基于机器人→前瞻点距离线性插值，远距离=收缩跟随路径，近距离=充分探索精细定位。
 *   微观层 (GPU 每步): spatial_decay_weight ∈ [0, 1]
 *     混合时间进度 (t/H) 与空间进度 (累计行进距离/max_travel)，
 *     快速轨迹的空间进度领先 → 噪声更快衰减；慢速轨迹反之。
 *     二者独立相乘: noise *= temporal_spatial_blend * exploration_range_scale
 *
 * 代价函数设计:
 *   本内核将每个奖励/惩罚项的计算委托给 mppi_gpu_rewards.cuh 中的
 *   独立 __device__ 函数，主循环中仅负责调用与累加：
 *
 *     total_cost = Σᵗ (                                  // 逐步累加
 *       compute_speed_reward(...)           // ⓪ 速度奖励 (负代价)
 *     + compute_goal_distance_cost(...)     // ① 目标渐进吸引
 *     + compute_costmap_collision_cost(...) // ② 碰撞/膨胀代价
 *     + compute_path_attraction_cost(...)   // ③ Cross-track error
 *     + compute_heading_alignment_cost(...) // ④ 朝向对齐
 *     + compute_blended_heading_cost(...)   // ④½ 混合朝向 (路径→目标姿态)
 *     + compute_path_length_cost(...)       // ⑤ 路径长度
 *     )
 *     + compute_terminal_distance_cost(...) // ⑥ 终端距离
 *     + compute_terminal_heading_cost(...)  // ⑦ 终端朝向
 */

#include <cuda_runtime.h>
#include <cfloat>

#include "mppi_gpu_rewards.cuh"

// 兼容旧版 CUDA：CUDART_PI_F 在 CUDA 10.2+ 的 math_constants.h 中定义
#ifndef CUDART_PI_F
#define CUDART_PI_F 3.14159265358979323846f
#endif

// ═══════════════════════════════════════════════════════════════════════════
// 常量定义（所有可调参数已暴露为 ROS 参数，此处仅保留数学/物理常量）
// ═══════════════════════════════════════════════════════════════════════════

// ═══════════════════════════════════════════════════════════════════════════
// 内核 1: MPPI 轨迹采样与代价计算
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief MPPI 采样与代价计算内核 — 每个 CUDA 线程独立处理一条轨迹
 *
 * 网格映射: 一维 grid，每个 block 的每个 thread 处理一条完整轨迹
 *   global_thread_id = blockIdx.x * blockDim.x + threadIdx.x
 *   → 对应第 s 条采样轨迹 (s ∈ [0, num_samples))
 *
 * 每条轨迹的处理流程:
 *
 * ┌─────────────────────────────────────────────────────────────┐
 * │ 初始化: 计算路径方向在机器人坐标系下的投影                 │
 * ├─────────────────────────────────────────────────────────────┤
 * │ for t = 0 .. horizon-1:                                    │
 * │   ├─ 计算时变噪声缩放 (noise_decay)                        │
 * │   ├─ 计算时变路径吸引权重                                   │
 * │   ├─ 采样控制量 (base + 噪声 + guidance 混合, 含阻尼)      │
 * │   ├─ RK2 运动学积分 → 新位置 (x, y, θ)                    │
 * │   ├─ 存储实际控制量与轨迹位置                               │
 * │   ├─ 调用 compute_speed_reward(...)          → 累加 ⓪     │
 * │   ├─ 调用 compute_goal_distance_cost(...)     → 累加 ①     │
 * │   ├─ 计算 traj_to_path_sq                                   │
 * │   ├─ 调用 compute_costmap_collision_cost(...) → 累加 ②     │
 * │   ├─ 调用 compute_path_attraction_cost(...)   → 累加 ③     │
 * │   ├─ 调用 compute_heading_alignment_cost(...) → 累加 ④     │
 * │   ├─ 调用 compute_blended_heading_cost(...)   → 累加 ④½    │
 * │   └─ 调用 compute_path_length_cost(...)       → 累加 ⑤     │
 * ├─────────────────────────────────────────────────────────────┤
 * │ 终端代价:                                                   │
 * │   ├─ compute_terminal_distance_cost(...)      → 累加 ⑥     │
 * │   └─ compute_terminal_heading_cost(...)       → 累加 ⑦     │
 * └─────────────────────────────────────────────────────────────┘
 *
 * @note 控制量采样策略:
 *   以上一帧最优控制序列 (base) 为采样中心，在控制空间加高斯噪声。
 *   路径方向仅作为软引导 (guidance)，不再替代采样中心。
 *
 *   横向运动增强 (vy 放大) 由两个信号联合驱动:
 *     ① path_turn_angle:  路径自身弯曲度 (C++ 端预计算)
 *     ② target_side_ratio: 前瞻点偏离机头正前方的角度
 *   两者叠加 → 转弯 + 目标侧偏时自动横着走，同时放松朝向对齐约束。
 *
 *   omega guidance 同样采用余弦退火:
 *     omega += blended_omega_error * 0.5/dt * guidance_weight
 *     blended_omega_error = lerp(path_angle_error, goal_heading_error, alpha)
 *     远目标 → omega 修正朝向路径方向, 近目标 → omega 修正朝向 goal yaw
 *
 *   guidance_weight γ ∈ [0, 1]:
 *     γ → 0: 完全信任 base + 噪声 (时序最平滑，帧间一致)
 *     γ → 1: 完全锚定路径方向 (退化为旧行为，帧间离散跳变)
 *     建议值: 0.2 ~ 0.4
 */
__global__ void mppi_sample_kernel(
    // ── 预生成的噪声序列 (控制空间: m/s, rad/s) ──
    const float* __restrict__ noise_vx,
    const float* __restrict__ noise_vy,
    const float* __restrict__ noise_w,

    // ── 上一帧最优控制序列 (采样中心) ──
    const float* __restrict__ base_vx,
    const float* __restrict__ base_vy,
    const float* __restrict__ base_w,

    // ── 机器人当前状态 ──
    float current_x, float current_y, float current_theta,

    // ── 前瞻目标点 ──
    float target_x, float target_y,

    // ── 代价地图 ──
    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,

    // ── 控制参数 ──
    float dt, float min_v, float max_v, float max_vy, float max_w,

    // ── 代价权重 ──
    float costmap_weight,
    float speed_reward_weight,
    float heading_weight,
    float cost_vy_threshold,

    // ── 路径引导参数 ──
    float path_dir_x, float path_dir_y,
    float guidance_weight,
    float cross_track_noise_scale,
    float noise_decay_rate,
    float exploration_range_scale,
    float spatial_decay_weight,
    float noise_scale_floor_vx,
    float noise_scale_floor_vy,
    float noise_scale_floor_w,
    float pure_rotation_ratio,
    int pure_rotation_steps,
    float pure_rotation_w_boost,
    float lateral_guidance_scale,

    // ── 转弯横向增强参数 ──
    float path_turn_angle,
    float turn_lateral_boost,

    // ── 前瞻点位姿接近奖励 ──
    float lookahead_proximity_weight,
    float lookahead_proximity_decay,
    float lookahead_theta,

    // ── 碰撞箱尺寸 ──
    float fp_front, float fp_back, float fp_left, float fp_right,

    // ── 全局路径数据 ──
    const float* __restrict__ path_x,
    const float* __restrict__ path_y,
    int num_path_pts,
    float path_attraction_weight,

    // ── 代价函数内部参数（原硬编码常量，现为可配置参数） ──
    float terminal_dist_weight,
    float path_length_weight,
    float goal_attraction_weight,
    float path_follow_scale_increment,
    float goal_soft_radius,
    float base_speed_floor_ratio,
    float turn_lateral_max_boost,
    float footprint_sample_spacing,
    float rear_obstacle_cost,

    // ── 分层规划: 全局轨迹直接预测到终点 ──
    float final_goal_x, float final_goal_y, float final_goal_yaw,
    int global_horizon, int num_global_trajs,

    // ── 维度信息 ──
    int num_samples, int horizon,

    // ── 输出 ──
    float* __restrict__ costs,           // [num_samples]: 每条轨迹的总代价
    float* __restrict__ sampled_vx,      // [num_samples × horizon]: 每步实际 vx
    float* __restrict__ sampled_vy,      // [num_samples × horizon]: 每步实际 vy
    float* __restrict__ sampled_w,       // [num_samples × horizon]: 每步实际 omega
    float* __restrict__ traj_x,          // [num_samples × horizon]: 轨迹点 x
    float* __restrict__ traj_y)          // [num_samples × horizon]: 轨迹点 y
{
  // ── 线程 ID → 样本索引 ──
  int s = blockIdx.x * blockDim.x + threadIdx.x;
  if (s >= num_samples) return;

  // ════════════════════════════════════════════════════════════════
  // 阶段 0: 分层规划 — 全局轨迹 vs 局部轨迹
  // ════════════════════════════════════════════════════════════════
  // 前 num_global_trajs 条轨迹直接预测到最终目标点（全局视野）
  // 其余轨迹预测到前瞻点（局部避障）
  bool is_global = (s < num_global_trajs);
  float traj_target_x  = is_global ? final_goal_x  : target_x;
  float traj_target_y  = is_global ? final_goal_y  : target_y;
  int   traj_horizon   = is_global ? global_horizon : horizon;
  // 全局轨迹的 lookahead = 最终目标点（lookahead_proximity 自然指向终点）
  float traj_lookahead_x = traj_target_x;
  float traj_lookahead_y = traj_target_y;
  // 全局轨迹的朝向目标 = 最终 goal yaw; 局部轨迹 = CPU搜索的最优朝向
  float traj_lookahead_theta = is_global ? final_goal_yaw : lookahead_theta;

  // ════════════════════════════════════════════════════════════════
  // 阶段 1: 初始化 — 路径方向坐标变换
  // ════════════════════════════════════════════════════════════════

  // 将世界坐标系下的路径方向旋转到机器人坐标系
  // R(-θ) · [path_dir_x, path_dir_y]^T
  float cos_rot = cosf(-current_theta);
  float sin_rot = sinf(-current_theta);
  float path_vx_robot = path_dir_x * cos_rot - path_dir_y * sin_rot;
  float path_vy_robot = path_dir_x * sin_rot + path_dir_y * cos_rot;

  // 路径方向与机器人朝向的夹角 — 用于 omega guidance
  float path_angle_error = atan2f(path_vy_robot, path_vx_robot);

  // ── 转弯横向增强 ──
  // 两个信号共同决定横向运动放大倍数:
  //   ① path_turn_angle: C++ 端计算的路径转折角度 (rad), 捕捉前方路径弯曲
  //   ② target_angle:    机器人→前瞻点连线与机头方向的夹角, 目标在侧方=需横着走

  // ① 路径弯曲度 → turn_sharpness ∈ [0, 1]
  float turn_sharpness = fabsf(path_turn_angle) / (CUDART_PI_F / 3.0f);
  turn_sharpness = fminf(1.0f, turn_sharpness);

  // ② 前瞻点相对机器人的方位角 (机器人坐标系)
  //    0=正前方, ±π/2=正侧方, ±π=正后方
  float dx_target = target_x - current_x;
  float dy_target = target_y - current_y;
  float target_x_robot = dx_target * cos_rot - dy_target * sin_rot;
  float target_y_robot = dx_target * sin_rot + dy_target * cos_rot;
  float target_angle = atan2f(target_y_robot, target_x_robot);
  float target_angle_abs = fabsf(target_angle);

  // target_side_ratio ∈ [0, 1]: 0=目标正前方, 1=目标正侧方
  float target_side_ratio = target_angle_abs / (CUDART_PI_F / 2.0f);
  target_side_ratio = fminf(1.0f, target_side_ratio);

  // ③ 合并横向放大: 路径弯曲主导, 目标侧偏辅助
  float combined_lateral_boost = turn_sharpness * turn_lateral_boost
                               + target_side_ratio * turn_lateral_boost * 0.4f;
  float turn_lateral_mult = 1.0f + combined_lateral_boost;
  turn_lateral_mult = fminf(turn_lateral_max_boost, turn_lateral_mult);

  // ④ 合并噪声放大: 转弯时适度增加 vy 探索
  float combined_noise_boost = turn_sharpness * turn_lateral_boost * 0.4f
                             + target_side_ratio * turn_lateral_boost * 0.15f;
  float turn_noise_mult = 1.0f + combined_noise_boost;

  // ── 统一朝向目标: 预计算 ──
  // 路径方向角 (世界坐标系)
  float path_angle = atan2f(path_dir_y, path_dir_x);
  // 退火尺度: 在 lookahead_proximity_decay 距离内从路径方向过渡到 lookahead_theta
  float anneal_scale = fmaxf(0.5f, lookahead_proximity_decay);

  // ── omega guidance 余弦退火混合 ──
  // 统一朝向目标: 远 → 跟随路径方向, 近 → 匹配 lookahead_theta
  // lookahead_theta 由 CPU 端搜索得到，在终点处等于 goal_yaw
  float lookahead_heading_error = normalize_angle(traj_lookahead_theta - current_theta);
  float dist_to_lookahead_init = sqrtf((traj_target_x - current_x) * (traj_target_x - current_x)
                                      + (traj_target_y - current_y) * (traj_target_y - current_y));
  float t_omega = fminf(1.0f, dist_to_lookahead_init / anneal_scale);
  float alpha_omega = (1.0f + cosf(CUDART_PI_F * t_omega)) * 0.5f;
  float diff_omega = normalize_angle(lookahead_heading_error - path_angle_error);
  float blended_omega_error = path_angle_error + alpha_omega * diff_omega;

  // ── 轨迹初始状态 ──
  float x = current_x;
  float y = current_y;
  float theta = current_theta;

  // ── 代价累加器 ──
  float cost = 0.0f;

  // ── 上一位置 (用于路径长度代价) ──
  float prev_x = x, prev_y = y;

  // ── 空间衰减: 累计行进距离 (用于混合时间/空间进度) ──
  float cumulative_dist = 0.0f;
  float max_travel_dist = max_v * dt * horizon;

  // ════════════════════════════════════════════════════════════════
  // 阶段 2: 逐步前向预测
  // ════════════════════════════════════════════════════════════════
  for (int t = 0; t < horizon; ++t) {
    int idx = s * horizon + t;  // 全局索引: (样本, 时步)

    // ── 2.1 读取当前步的 base 控制量 ──
    float base_vx_t = base_vx[t];
    float base_vy_t = base_vy[t];
    float base_w_t = base_w[t];

    // 保底速度幅值: base 塌缩时拉升，防止采样退化导致死亡螺旋
    float base_speed = hypotf(base_vx_t, base_vy_t);
    float min_speed = base_speed_floor_ratio * max_v;
    if (base_speed < min_speed) {
      if (base_speed < 0.001f) {
        // 双零塌缩: 沿路径方向赋予最低速度
        base_vx_t = min_speed * path_vx_robot;
        base_vy_t = min_speed * path_vy_robot;
      } else {
        float scale = min_speed / base_speed;
        base_vx_t *= scale;
        base_vy_t *= scale;
      }
      base_speed = min_speed;
    }

    // ── 2.2 时变噪声缩放 (时间+空间混合衰减 × 宏观距离缩放) ──
    // 时间进度: t / (H-1)
    float progress_t = static_cast<float>(t) / fmaxf(1.0f, static_cast<float>(horizon - 1));

    // 空间进度: 累计行进距离 / 最大预期距离
    float spatial_progress = fminf(1.0f, cumulative_dist / fmaxf(0.01f, max_travel_dist));

    // 混合进度: spatial_decay_weight=0 → 纯时间; =1 → 纯空间
    float effective_progress = (1.0f - spatial_decay_weight) * progress_t
                             + spatial_decay_weight * spatial_progress;

    // 噪声缩放 = 时/空混合衰减 × 探索范围缩放 (宏观距离衰减)
    // 先乘距离衰减再钳位：确保任何距离下噪声不低于 floor，防止采样塌缩
    // 各维度独立 floor → vx/vy/omega 可分别设置最小采样范围
    float noise_scale_t = 1.0f - noise_decay_rate * effective_progress;
    noise_scale_t *= exploration_range_scale;
    float noise_scale_vx = fmaxf(noise_scale_floor_vx, noise_scale_t);
    float noise_scale_vy = fmaxf(noise_scale_floor_vy, noise_scale_t);
    float noise_scale_w  = fmaxf(noise_scale_floor_w,  noise_scale_t);

    // ── 2.3 时变路径吸引权重 ──
    float path_follow_scale = 1.0f + progress_t * path_follow_scale_increment;
    float path_attract_t = path_attraction_weight * path_follow_scale;

    // ════════════════════════════════════════════════════════════
    // 2.4 控制量采样: base + 噪声 (主体) ⊕ path_dir × speed (软引导)
    // ════════════════════════════════════════════════════════════

    float noise_vx_s = noise_vx[idx] * noise_scale_vx;
    float noise_vy_s = noise_vy[idx] * noise_scale_vy;
    float noise_w_s  = noise_w[idx] * noise_scale_w;

    // vx: (1-γ)·(base_vx + noise) + γ·(path_dir_vx · speed)
    float vx = (1.0f - guidance_weight) * (base_vx_t + noise_vx_s)
             + guidance_weight * path_vx_robot * base_speed;

    // vy: 转弯 + 目标侧偏 → 自动放大横向运动
    float vy = (1.0f - guidance_weight) *
                   (base_vy_t + noise_vy_s * cross_track_noise_scale * turn_noise_mult)
             + guidance_weight * path_vy_robot * base_speed
                   * lateral_guidance_scale * turn_lateral_mult;

    // omega: base + noise + 余弦退火朝向修正 (路径方向 → 目标姿态)
    float omega = base_w_t + noise_w_s
                + blended_omega_error * (0.5f / dt) * guidance_weight;

    // ── 纯角速度采样: 前 N 步原地旋转对准路径，再平移 ──
    if (static_cast<float>(s) < pure_rotation_ratio * static_cast<float>(num_samples)
        && t < pure_rotation_steps) {
      vx = 0.0f;
      vy = 0.0f;
      // 增强 omega 噪声 + 保留朝向引导，加速对准路径方向
      omega = base_w_t + noise_w_s * pure_rotation_w_boost
            + blended_omega_error * (0.5f / dt) * fmaxf(guidance_weight, 0.3f);
    }

    // ── 控制量钳制 ──
    vx    = fminf(max_v,  fmaxf(min_v,   vx));
    vy    = fminf(max_vy, fmaxf(-max_vy, vy));
    omega = fminf(max_w,  fmaxf(-max_w,  omega));

    // ── 2.5 平滑目标到达阻尼 ──
    // 替换硬截断 (GOAL_REACHED_DIST=0.25m break):
    // 在目标 0.4m 内平滑降低控制量→0，sqrt 曲线保证减速平缓 (ease-out)
    float dist_to_target = sqrtf(
        (traj_target_x - x) * (traj_target_x - x) + (traj_target_y - y) * (traj_target_y - y));
    if (dist_to_target < goal_soft_radius) {
      float arrival = dist_to_target / goal_soft_radius;
      arrival = sqrtf(arrival);
      vx    *= arrival;
      // vy/omega 不再阻尼，保留近目标时的横向调整能力
    }

    // ── 2.7 存储实际控制量 ──
    sampled_vx[idx] = vx;
    sampled_vy[idx] = vy;
    sampled_w[idx]  = omega;

    // ════════════════════════════════════════════════════════════
    // 2.8 中点法 (RK2) 运动学积分
    // ════════════════════════════════════════════════════════════
    // 欧拉法用步长起点的朝向计算整步位移，在大 ω 或 vy 下
    // 会产生抛物线式累积误差 ("甩尾"). 中点法先推进半步得到
    // 中间朝向 θ_mid，再用 θ_mid 计算整步的 x/y 位移，二阶精度.
    //
    // 半步:
    //   dx1 = vx·cos(θ) - vy·sin(θ)
    //   dy1 = vx·sin(θ) + vy·cos(θ)
    //   θ_mid = θ + ω · dt/2
    // 整步 (用 θ_mid):
    //   dx2 = vx·cos(θ_mid) - vy·sin(θ_mid)
    //   dy2 = vx·sin(θ_mid) + vy·cos(θ_mid)
    //   x_new = x + dx2·dt,  y_new = y + dy2·dt,  θ_new = θ + ω·dt

    float dtheta1 = omega;
    float half_dt = 0.5f * dt;
    float theta_mid = theta + dtheta1 * half_dt;

    float dx2 = vx * cosf(theta_mid) - vy * sinf(theta_mid);
    float dy2 = vx * sinf(theta_mid) + vy * cosf(theta_mid);

    float new_x = x + dx2 * dt;
    float new_y = y + dy2 * dt;
    float new_theta = theta + omega * dt;

    x = new_x;
    y = new_y;
    theta = new_theta;

    // 存储轨迹位置 (用于可视化和调试)
    if (traj_x != nullptr && traj_y != nullptr) {
      traj_x[idx] = x;
      traj_y[idx] = y;
    }

    // ════════════════════════════════════════════════════════════
    // 2.9 代价计算 — 调用各奖励/惩罚函数并累加
    // ════════════════════════════════════════════════════════════

    // ── vy 代价视角: 告知 MPPI 小 vy 会被 CPU 端抬升, 避免因小 vy 误判 ──
    float cost_vy = vy;
    if (cost_vy > 0.0f && cost_vy < cost_vy_threshold)       cost_vy = cost_vy_threshold;
    else if (cost_vy < 0.0f && cost_vy > -cost_vy_threshold) cost_vy = -cost_vy_threshold;

    // ⓪ 速度奖励: 鼓励沿路径方向快速前进 (负代价 = 奖励)
    cost += compute_speed_reward(
        vx, cost_vy, path_vx_robot, path_vy_robot,
        speed_reward_weight, horizon);

    // ① 目标渐进吸引: 越靠近终点、越后期，代价越高
    cost += compute_goal_distance_cost(
        x, y, traj_target_x, traj_target_y,
        t, horizon, goal_attraction_weight);

    // ── 计算轨迹点到全局路径的最短距离 (用于 path_attraction_cost) ──
    float traj_to_path_sq = compute_min_path_distance_sq(
        x, y, path_x, path_y, num_path_pts);

    // ② 代价地图碰撞/膨胀代价 (后方隐形障碍: 仅在后退时激活)
    cost += compute_costmap_collision_cost(
        x, y, cosf(theta), sinf(theta),
        costmap, costmap_w, costmap_h,
        costmap_res, costmap_origin_x, costmap_origin_y,
        costmap_weight, horizon,
        fp_front, fp_back, fp_left, fp_right,
        footprint_sample_spacing, rear_obstacle_cost, vx);

    // ③ 全局路径吸引: Cross-track error 惩罚 (渐进增强)
    if (num_path_pts >= 2) {
      cost += compute_path_attraction_cost(traj_to_path_sq, path_attract_t, horizon);
    }

    // ④ 统一混合朝向: 距离前瞻点越近, 朝向偏好从路径方向→lookahead_theta
    // lookahead_theta 在 CPU 端搜索得到 (窄通道=最优通过姿态, 终点=goal_yaw)
    {
      float dx_lh = traj_target_x - x;
      float dy_lh = traj_target_y - y;
      float dist_to_lookahead = sqrtf(dx_lh * dx_lh + dy_lh * dy_lh);
      cost += compute_blended_heading_cost(
          theta, path_angle, traj_lookahead_theta, dist_to_lookahead,
          anneal_scale, 0.0f,
          heading_weight, horizon);
    }

    // ⑤ 路径长度: 防止绕远路
    cost += compute_path_length_cost(x, y, prev_x, prev_y, path_length_weight, horizon);

    // ── 累计行进距离 (用于下一时步的空间进度计算) ──
    cumulative_dist += hypotf(x - prev_x, y - prev_y);

    prev_x = x;
    prev_y = y;
  }

  // ════════════════════════════════════════════════════════════════
  // 阶段 3: 终端代价
  // ════════════════════════════════════════════════════════════════

  // ⑥ 终端距离代价: 轨迹终点到目标点（局部=前瞻点, 全局=终点）的剩余距离
  cost += compute_terminal_distance_cost(
      x, y, traj_target_x, traj_target_y, terminal_dist_weight);

  // ⑦ 终端朝向对齐代价: 鼓励轨迹终点朝向目标
  cost += compute_terminal_heading_cost(
      x, y, theta, traj_target_x, traj_target_y);

  // ⑧ 位置接近奖励: 鼓励轨迹终点靠近目标
  // 全局轨迹 → 靠近终点; 局部轨迹 → 靠近前瞻点
  cost += compute_lookahead_proximity_reward(
      x, y, traj_lookahead_x, traj_lookahead_y,
      lookahead_proximity_weight, lookahead_proximity_decay);

  // ── 写入总代价 ──
  costs[s] = cost;
}

// ═══════════════════════════════════════════════════════════════════════════
// 内核 2: 全时域加权求和
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief MPPI 全时域加权求和内核 — 对每个 timestep 独立计算最优控制量
 *
 * 网格映射: 2D grid
 *   gridDim.x → 覆盖 num_samples 个样本
 *   gridDim.y → 覆盖 horizon 个时步
 *
 * 对每个 timestep t，计算:
 *   weight_k = exp(-(cost_k - min_cost) / lambda)     // 指数加权
 *   u*_t = Σ_k weight_k · sampled_u[k,t] / Σ_k weight_k
 *
 * 这是 MPPI 的核心更新公式: 代价越低的轨迹权重越大，
 * 最终控制量是全部采样在各 timestep 的加权平均。
 *
 * result_seq 布局 (每 timestep 4 个 float):
 *   [vx_0, vy_0, w_0, sum_w_0, vx_1, vy_1, w_1, sum_w_1, ...]
 *
 * 使用 atomicAdd 确保多线程安全累加。
 */
__global__ void mppi_weighted_sum_kernel(
    const float* __restrict__ costs,          // [num_samples]: 每条轨迹代价
    const float* __restrict__ sampled_vx,     // [num_samples × horizon]
    const float* __restrict__ sampled_vy,     // [num_samples × horizon]
    const float* __restrict__ sampled_w,      // [num_samples × horizon]
    float* __restrict__ result_seq,           // [horizon × 4]: 加权结果
    float min_cost,                            // 最小代价 (数值稳定性)
    float lambda,                              // 温度参数 (>0, 越小越贪婪)
    int num_samples,
    int horizon)
{
  int s = blockIdx.x * blockDim.x + threadIdx.x;  // 样本索引
  int t = blockIdx.y;                               // 时步索引

  if (s >= num_samples || t >= horizon) return;

  int idx = s * horizon + t;

  // 指数权重: w_k = exp(-(cost_k - min_cost) / lambda)
  // 减去 min_cost 防止 exp 下溢
  float weight = expf(-(costs[s] - min_cost) / lambda);

  // result_seq 布局: [vx_0, vy_0, w_0, sum_w_0, vx_1, ...]
  int base = t * 4;

  // atomicAdd: 多线程安全累加 (多个样本的同一时步可能并发写入)
  atomicAdd(&result_seq[base + 0], weight * sampled_vx[idx]);
  atomicAdd(&result_seq[base + 1], weight * sampled_vy[idx]);
  atomicAdd(&result_seq[base + 2], weight * sampled_w[idx]);
  atomicAdd(&result_seq[base + 3], weight);
}

// ═══════════════════════════════════════════════════════════════════════════
// Host 端包装函数 (extern "C" — 供 C++ 代码调用)
// ═══════════════════════════════════════════════════════════════════════════

extern "C" {

/**
 * @brief Host 端包装: 上传数据到 GPU 并启动 mppi_sample_kernel
 *
 * 执行步骤:
 *   1. 异步拷贝噪声、base 控制序列、代价地图到 GPU
 *   2. 异步拷贝近期轨迹点 (观测窗口) 到 GPU
 *   3. 异步拷贝全局路径点到 GPU
 *   4. 启动采样内核 (一维 grid)
 *
 * @return 0 = 成功, 非零 = cudaMemcpyAsync 或内核启动失败的错误码
 */
int mppi_gpu_sample_and_cost(
    // ── Host 端输入数据 ──
    const float* noise_vx, const float* noise_vy, const float* noise_w,
    const float* base_vx, const float* base_vy, const float* base_w,
    float current_x, float current_y, float current_theta,
    float target_x, float target_y,
    const unsigned char* costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float dt, float min_v, float max_v, float max_vy, float max_w,
    float costmap_weight,
    float path_dir_x, float path_dir_y,
    float guidance_weight,
    float cross_track_noise_scale,
    float noise_decay_rate,
    float exploration_range_scale,
    float spatial_decay_weight,
    float noise_scale_floor_vx,
    float noise_scale_floor_vy,
    float noise_scale_floor_w,
    float pure_rotation_ratio,
    int pure_rotation_steps,
    float pure_rotation_w_boost,
    float vel_direction_weight,
    float speed_reward_weight,
    float heading_weight,
    float cost_vy_threshold,
    float lateral_guidance_scale,
    float path_turn_angle,
    float turn_lateral_boost,
    float lookahead_proximity_weight,
    float lookahead_proximity_decay,
    float lookahead_theta,
    float fp_front, float fp_back, float fp_left, float fp_right,
    const float* path_x, const float* path_y,
    int num_path_pts,
    float path_attraction_weight,
    // ── 代价函数内部参数（原硬编码常量） ──
    float terminal_dist_weight,
    float path_length_weight,
    float goal_attraction_weight,
    float path_follow_scale_increment,
    float goal_soft_radius,
    float base_speed_floor_ratio,
    float turn_lateral_max_boost,
    float footprint_sample_spacing,
    float rear_obstacle_cost,

    // ── 分层规划: 全局轨迹参数 ──
    float final_goal_x, float final_goal_y, float final_goal_yaw,
    int global_horizon, int num_global_trajs,

    int num_samples, int horizon,

    // ── GPU 端缓冲区 (已预分配) ──
    float* d_noise_vx, float* d_noise_vy, float* d_noise_w,
    float* d_base_vx, float* d_base_vy, float* d_base_w,
    unsigned char* d_costmap,
    float* d_path_x, float* d_path_y,
    float* d_costs, float* d_sampled_vx, float* d_sampled_vy, float* d_sampled_w,
    float* d_traj_x, float* d_traj_y,
    cudaStream_t stream)
{
  size_t noise_bytes = num_samples * horizon * sizeof(float);
  size_t base_bytes = horizon * sizeof(float);
  size_t costmap_bytes = costmap_w * costmap_h * sizeof(unsigned char);

  cudaError_t err;

  // ── 上传噪声序列 ──
  err = cudaMemcpyAsync(d_noise_vx, noise_vx, noise_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 1;
  err = cudaMemcpyAsync(d_noise_vy, noise_vy, noise_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 2;
  err = cudaMemcpyAsync(d_noise_w, noise_w, noise_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 3;

  // ── 上传 base 控制序列 ──
  err = cudaMemcpyAsync(d_base_vx, base_vx, base_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 4;
  err = cudaMemcpyAsync(d_base_vy, base_vy, base_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 5;
  err = cudaMemcpyAsync(d_base_w, base_w, base_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 6;

  // ── 上传代价地图 ──
  err = cudaMemcpyAsync(d_costmap, costmap, costmap_bytes, cudaMemcpyHostToDevice, stream);
  if (err != cudaSuccess) return 7;

  // ── 上传全局路径点 ──
  if (num_path_pts > 0 && path_x != nullptr && path_y != nullptr) {
    size_t path_bytes = num_path_pts * sizeof(float);
    err = cudaMemcpyAsync(d_path_x, path_x, path_bytes, cudaMemcpyHostToDevice, stream);
    if (err != cudaSuccess) return 9;
    err = cudaMemcpyAsync(d_path_y, path_y, path_bytes, cudaMemcpyHostToDevice, stream);
    if (err != cudaSuccess) return 10;
  }

  // ── 启动采样内核 ──
  int threads_per_block = 256;
  int blocks = (num_samples + threads_per_block - 1) / threads_per_block;

  mppi_sample_kernel<<<blocks, threads_per_block, 0, stream>>>(
      d_noise_vx, d_noise_vy, d_noise_w,
      d_base_vx, d_base_vy, d_base_w,
      current_x, current_y, current_theta,
      target_x, target_y,
      d_costmap,
      costmap_w, costmap_h,
      costmap_res, costmap_origin_x, costmap_origin_y,
      dt, min_v, max_v, max_vy, max_w,
      costmap_weight,
      speed_reward_weight,
      heading_weight,
      cost_vy_threshold,
      path_dir_x, path_dir_y,
      guidance_weight,
      cross_track_noise_scale,
      noise_decay_rate,
      exploration_range_scale,
      spatial_decay_weight,
      noise_scale_floor_vx,
      noise_scale_floor_vy,
      noise_scale_floor_w,
      pure_rotation_ratio,
      pure_rotation_steps,
      pure_rotation_w_boost,
      lateral_guidance_scale,
      path_turn_angle,
      turn_lateral_boost,
      lookahead_proximity_weight,
      lookahead_proximity_decay,
      lookahead_theta,
      fp_front, fp_back, fp_left, fp_right,
      d_path_x, d_path_y,
      num_path_pts,
      path_attraction_weight,
      terminal_dist_weight,
      path_length_weight,
      goal_attraction_weight,
      path_follow_scale_increment,
      goal_soft_radius,
      base_speed_floor_ratio,
      turn_lateral_max_boost,
      footprint_sample_spacing,
      rear_obstacle_cost,
      final_goal_x, final_goal_y, final_goal_yaw,
      global_horizon, num_global_trajs,
      num_samples, horizon,
      d_costs, d_sampled_vx, d_sampled_vy, d_sampled_w,
      d_traj_x, d_traj_y);

  err = cudaGetLastError();
  if (err != cudaSuccess) return 8;

  return 0;
}

/**
 * @brief Host 端包装: 启动全时域加权求和内核
 *
 * 先清零 result_seq 缓冲区，然后启动 2D 内核。
 *
 * @return 0 = 成功, 非零 = cudaMemsetAsync 或内核启动失败的错误码
 */
int mppi_gpu_weighted_sum(
    const float* d_costs,
    const float* d_sampled_vx,
    const float* d_sampled_vy,
    const float* d_sampled_w,
    float* d_result_seq,
    float min_cost,
    float lambda,
    int num_samples,
    int horizon,
    cudaStream_t stream)
{
  // 清零结果缓冲区 (horizon × 4 个 float)
  cudaError_t err = cudaMemsetAsync(d_result_seq, 0,
                                     horizon * 4 * sizeof(float), stream);
  if (err != cudaSuccess) return 1;

  int threads_per_block = 256;
  int blocks_per_t = (num_samples + threads_per_block - 1) / threads_per_block;

  // 2D grid: dim3(blocks_per_t, horizon)
  dim3 grid(blocks_per_t, horizon);
  mppi_weighted_sum_kernel<<<grid, threads_per_block, 0, stream>>>(
      d_costs, d_sampled_vx, d_sampled_vy, d_sampled_w,
      d_result_seq,
      min_cost, lambda,
      num_samples, horizon);

  err = cudaGetLastError();
  if (err != cudaSuccess) return 2;

  return 0;
}

}  // extern "C"
