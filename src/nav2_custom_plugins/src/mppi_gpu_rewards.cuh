#ifndef MPPI_GPU_REWARDS_CUH_
#define MPPI_GPU_REWARDS_CUH_

/**
 * @file mppi_gpu_rewards.cuh
 * @brief MPPI 代价函数 — 各奖励/惩罚项独立封装
 *
 * 本文件将 MPPI 轨迹代价函数的每一个奖励或惩罚项封装为独立的
 * __device__ 函数，每个函数包含：
 *   - 数学公式的详细推导
 *   - 参数含义说明
 *   - 返回值含义（正值为惩罚/代价，负值为奖励）
 *
 * 主核函数 mppi_sample_kernel 在每个预测步依次调用这些函数，
 * 累加得到该轨迹的总代价:
 *
 *   total_cost = Σᵗ ( speed_reward(t)           // ⓪ 负代价=奖励
 *                    + goal_distance_cost(t)      // ①
 *                    + costmap_collision_cost(t)  // ②
 *                    + path_attraction_cost(t)    // ③
 *                    + heading_alignment_cost(t)  // ④
 *                    + path_length_cost(t) )      // ⑤
 *              + terminal_distance_cost           // ⑤ 终端
 *              + terminal_heading_cost            // ⑥ 终端
 *
 * 设计原则:
 *   - 每项代价都除以 horizon (在核函数中统一处理)，确保不同
 *     horizon 长度的代价可比
 *   - 奖励项返回负值，惩罚项返回正值，总和越小越好
 *   - 每个函数尽量无副作用，参数显式传递
 */

#include "mppi_gpu_common.cuh"

// 兼容旧版 CUDA：CUDART_PI_F 在 CUDA 10.2+ 的 math_constants.h 中定义
#ifndef CUDART_PI_F
#define CUDART_PI_F 3.14159265358979323846f
#endif

// ═══════════════════════════════════════════════════════════════════════════
// ⓪ 沿路径方向速度奖励 (Speed Reward)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算沿路径方向的速度奖励（负代价 → 鼓励快速前进）
 *
 * 只奖励沿路径方向的速度分量，不奖励横向或后退运动。
 *
 * 公式:
 *   speed_along_path = vx · path_vx + vy · path_vy    // 点积, 投影到路径方向
 *   reward = -max(0, speed_along_path) · w_speed / H
 *
 * 性质:
 *   - speed_along_path > 0: 机器人沿路径方向前进 → 负代价(奖励)
 *   - speed_along_path ≤ 0: 后退或静止 → 零代价(不奖励)
 *   - 横向速度 vy 若与路径方向一致，也会获得部分奖励
 *
 * 为什么用 max(0, ·):
 *   不惩罚后退，只鼓励前进。后退在某些避障场景中可能是合理的。
 *
 * @param  vx                   机器人坐标系下的前向速度 (m/s)
 * @param  vy                   机器人坐标系下的横向速度 (m/s)
 * @param  path_vx_robot        路径方向在机器人坐标系下的 x 分量 (归一化向量)
 * @param  path_vy_robot        路径方向在机器人坐标系下的 y 分量 (归一化向量)
 * @param  speed_reward_weight  速度奖励权重 w_speed (≥0)
 * @param  horizon              预测时域步数 H
 * @return                      负值 = 奖励 (cost 减少), 零或正 = 无奖励
 */
__device__ inline float compute_speed_reward(
    float vx, float vy,
    float path_vx_robot, float path_vy_robot,
    float speed_reward_weight, int horizon)
{
  // 速度向量在路径方向上的投影（点积）
  float speed_along_path = vx * path_vx_robot + vy * path_vy_robot;

  // 只奖励正向分量: 奖励 = -max(0, speed_proj) · w / H
  return -fmaxf(0.0f, speed_along_path) * speed_reward_weight
         / static_cast<float>(horizon);
}

// ═══════════════════════════════════════════════════════════════════════════
// ① 目标点渐进吸引代价 (Goal Distance Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算目标点渐进吸引代价 — 越靠近终点、越后期，代价越高
 *
 * 设计思路:
 *   前期 (t≈0): 允许轨迹偏离目标去绕障碍物
 *   后期 (t≈H): 强制轨迹逼近目标，确保最终到达
 *
 * 公式:
 *   progress = t / H                 // 归一化时间 ∈ [0, 1)
 *   cost = dist_to_goal · (1 - progress) · w_path
 *
 * 性质:
 *   - t=0:      cost = dist · 1.0 · w_path  (全额成本)
 *   - t=H-1:    cost = dist · (1/H) · w_path (几乎为零)
 *   - 使得早期步有探索自由，后期步被强约束
 *
 * @param  x, y            当前轨迹点的世界坐标 (m)
 * @param  target_x, target_y  前瞻目标点的世界坐标 (m)
 * @param  t                当前预测步索引 (0 ~ H-1)
 * @param  horizon          预测时域步数 H
 * @param  path_cost_weight 路径代价权重 w_path
 * @return                  正值 = 代价 (cost 增加)
 */
__device__ inline float compute_goal_distance_cost(
    float x, float y,
    float target_x, float target_y,
    int t, int horizon, float path_cost_weight)
{
  float dx = target_x - x;
  float dy = target_y - y;
  float dist = sqrtf(dx * dx + dy * dy);

  // 归一化进度: progress ∈ [0, 1)
  float progress = static_cast<float>(t) / static_cast<float>(horizon);

  // 代价 = dist · (1 - progress) · weight / H
  // 后期 progress→1, (1-progress)→0, 代价趋零 → 强制靠近目标
  // 除以 horizon 确保不同预测长度的代价可比
  return dist * (1.0f - progress) * path_cost_weight
         / static_cast<float>(horizon);
}

// ═══════════════════════════════════════════════════════════════════════════
// ② 代价地图碰撞/膨胀惩罚 (Costmap Collision Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算代价地图碰撞与膨胀代价 — 统一的加权求和
 *
 * 对碰撞箱密集网格的每个足迹点采样 costmap，加权求和后归一化。
 * 不再区分已知/未知 — costmap 本身就是统一的代价表达。
 *
 * 后方隐形障碍 (运动方向感知):
 *   机器人后方是传感器盲区 (未知区域)。
 *   前进 (vx >= 0): 后方正在被离开 → 不施加地板 (无惩罚)
 *   后退 (vx <  0): 后方正在被进入 → 施加代价地板 (有惩罚)
 *   这样 MPPI 采样出后退轨迹时会承受额外代价，自然偏好前进。
 *
 *   对后方足迹点 (fp_lx < 0) 且机器人正在后退 (vx < 0):
 *     effective_val = max(actual_costmap_val, rear_obstacle_cost)
 *   即使 costmap 显示后方为自由空间，也会被地板抬升到 rear_obstacle_cost，
 *   形成"隐形障碍"效应。
 *   rear_obstacle_cost=0 时禁用此功能。
 *
 *   代价映射 (norm^4):
 *     rear_obstacle_cost=  0 → 无效果
 *     rear_obstacle_cost=128 → (0.50)^4 = 0.063 (轻度障碍)
 *     rear_obstacle_cost=160 → (0.63)^4 = 0.155 (轻~中度)
 *     rear_obstacle_cost=192 → (0.75)^4 = 0.316 (中度障碍)
 *     rear_obstacle_cost=254 → (1.00)^4 = 0.984 (致命障碍)
 *
 * 公式:
 *   cost = Σ (max(cell_valᵢ, floorᵢ) / 255)^4 / total_fp · w_costmap / H
 *   其中 floorᵢ = rear_obstacle_cost (fp_lx < 0 且 vx < 0), 0 (其他情况)
 *
 * @param  x, y                  机器人中心的当前世界坐标 (m)
 * @param  cos_theta, sin_theta  机器人朝向的余弦/正弦
 * @param  costmap               代价地图数据指针
 * @param  costmap_w, costmap_h  代价地图尺寸
 * @param  costmap_res           代价地图分辨率 (m/cell)
 * @param  costmap_origin_x, costmap_origin_y  代价地图原点 (m)
 * @param  costmap_weight        代价地图总权重 w_costmap
 * @param  horizon               预测时域步数 H
 * @param  fp_front, fp_back, fp_left, fp_right  碰撞箱半尺寸 (m)
 * @param  fp_sample_spacing     足迹采样间距 (m)
 * @param  rear_obstacle_cost    后方隐形障碍代价地板 (0~255), 0=禁用
 * @param  vx                    当前步的机器人前向速度 (m/s), 用于判断运动方向
 * @return                       总代价地图代价 (正值 = 惩罚)
 */
__device__ float compute_costmap_collision_cost(
    float x, float y,
    float cos_theta, float sin_theta,
    const unsigned char* __restrict__ costmap,
    int costmap_w, int costmap_h,
    float costmap_res, float costmap_origin_x, float costmap_origin_y,
    float costmap_weight, int horizon,
    float fp_front, float fp_back, float fp_left, float fp_right,
    float fp_sample_spacing, float rear_obstacle_cost, float vx)
{
  if (costmap == nullptr || costmap_w <= 0 || costmap_h <= 0) {
    return 0.0f;
  }

  // ══ 密集网格采样 ══
  float fp_len_x = fp_front + fp_back;
  float fp_len_y = fp_left + fp_right;

  int nx = static_cast<int>(ceilf(fp_len_x / fp_sample_spacing)) + 1;
  int ny = static_cast<int>(ceilf(fp_len_y / fp_sample_spacing)) + 1;
  nx = (nx < 2) ? 2 : ((nx > 10) ? 10 : nx);
  ny = (ny < 2) ? 2 : ((ny > 10) ? 10 : ny);

  int total_fp = nx * ny;

  // ══ 统一幂律代价: 每采样点 norm^4 ══
  // 后方足迹点施加代价地板 rear_obstacle_cost
  float cost_acc = 0.0f;
  float step_x = (nx > 1) ? fp_len_x / static_cast<float>(nx - 1) : 0.0f;
  float step_y = (ny > 1) ? fp_len_y / static_cast<float>(ny - 1) : 0.0f;

  for (int iy = 0; iy < ny; ++iy) {
    float fp_ly = -fp_right + static_cast<float>(iy) * step_y;
    for (int ix = 0; ix < nx; ++ix) {
      float fp_lx = -fp_back + static_cast<float>(ix) * step_x;

      // 机器人坐标系 → 世界坐标系
      float fp_wx = x + fp_lx * cos_theta - fp_ly * sin_theta;
      float fp_wy = y + fp_lx * sin_theta + fp_ly * cos_theta;

      // 双线性插值采样
      float cell_val = costmap_bilinear(fp_wx, fp_wy, costmap,
                                        costmap_w, costmap_h,
                                        costmap_res, costmap_origin_x, costmap_origin_y);

      // 后方隐形障碍: 仅在后退时 (vx < 0) 抬升后方足迹点代价地板
      // 前进时后方正在被离开, 不需要地板
      if (fp_lx < 0.0f && vx < 0.0f && rear_obstacle_cost > 0.0f) {
        cell_val = fmaxf(cell_val, rear_obstacle_cost);
      }

      // 统一幂律代价: 自由空间跳过, 障碍物按 (c/255)^4 累加
      if (cell_val >= 1.0f) {
        float norm = cell_val / 255.0f;
        float norm2 = norm * norm;
        cost_acc += norm2 * norm2;  // norm^4
      }
    }
  }

  // 归一化 (每采样点平均) → 加权 → 时域归一化
  return (cost_acc / static_cast<float>(total_fp))
         * costmap_weight / static_cast<float>(horizon);
}

// ═══════════════════════════════════════════════════════════════════════════
// ③ 全局路径吸引代价 (Path Attraction / Cross-Track Error)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算全局路径吸引代价 — 惩罚偏离全局路径的轨迹
 *
 * 使用轨迹点到最近路径段的距离平方作为惩罚（已在核函数中预计算
 * traj_to_path_sq，此处仅做加权）。
 *
 * 公式:
 *   cost = traj_to_path_sq · path_attract_weight_t
 *
 * 其中 path_attract_weight_t 在核函数中随时间递增:
 *   path_attract_t = w_path_attract · (1 + progress · 1.2)
 *
 * 时间递增的设计意图:
 *   前期 (t≈0): 允许轨迹偏离路径绕开障碍物
 *   后期 (t≈H): 强约束轨迹回到路径上
 *
 * @param  traj_to_path_sq    轨迹点到最近路径段的最短距离平方 (m²)
 * @param  path_attract_t     当前时间步的路径吸引权重 (已含时间递增)
 * @return                    正值 = 代价 (cost 增加)
 */
__device__ inline float compute_path_attraction_cost(
    float traj_to_path_sq, float path_attract_t, int horizon)
{
  // 代价 = 偏离距离² · 时间递增权重 / H
  // 除以 horizon 确保不同预测长度的代价可比
  return traj_to_path_sq * path_attract_t
         / static_cast<float>(horizon);
}

// ═══════════════════════════════════════════════════════════════════════════
// ④ 路径长度代价 (Path Length Penalty)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算路径长度代价 — 防止轨迹绕远路
 *
 * 每一步累加实际走过的欧氏距离，惩罚绕路行为。
 *
 * 公式:
 *   step_len = √((x - x_prev)² + (y - y_prev)²)
 *   cost = step_len · w_len
 *
 * 作用:
 *   在多个候选轨迹代价相当时，选择路径更短（更直接）的那条，
 *   避免不必要的迂回。
 *
 * @param  x, y           当前轨迹点的世界坐标 (m)
 * @param  prev_x, prev_y 上一轨迹点的世界坐标 (m)
 * @param  len_weight     路径长度权重 w_len
 * @return                正值 = 代价 (cost 增加)
 */
__device__ inline float compute_path_length_cost(
    float x, float y, float prev_x, float prev_y,
    float len_weight, int horizon)
{
  float step_dx = x - prev_x;
  float step_dy = y - prev_y;
  // 代价 = 步长 · 权重 / H
  // 除以 horizon 确保不同预测长度的代价可比
  return sqrtf(step_dx * step_dx + step_dy * step_dy) * len_weight
         / static_cast<float>(horizon);
}

// ═══════════════════════════════════════════════════════════════════════════
// ⑥ 终端距离代价 (Terminal Distance Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算终端距离代价 — 轨迹终点到目标点的距离惩罚
 *
 * 在预测时域结束后，对轨迹终点与目标点的剩余距离施加一次性惩罚。
 * 这是 MPPI 标准做法: 终端代价确保优化不会选择"在时域内看起来便宜
 * 但最终离目标很远"的短视轨迹。
 *
 * 公式:
 *   dist_to_goal = √((target_x - x_final)² + (target_y - y_final)²)
 *   cost = dist_to_goal · w_terminal_dist
 *
 * @param  x_final, y_final      轨迹终点的世界坐标 (m)
 * @param  target_x, target_y    前瞻目标点的世界坐标 (m)
 * @param  terminal_dist_weight  终端距离权重 w_terminal_dist
 * @return                       正值 = 代价 (cost 增加)
 */
__device__ inline float compute_terminal_distance_cost(
    float x_final, float y_final,
    float target_x, float target_y,
    float terminal_dist_weight)
{
  float dx = target_x - x_final;
  float dy = target_y - y_final;
  float dist_to_goal = sqrtf(dx * dx + dy * dy);
  return dist_to_goal * terminal_dist_weight;
}

// ═══════════════════════════════════════════════════════════════════════════
// ⑦ 终端朝向对齐代价 (Terminal Heading Alignment Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算终端朝向对齐代价 — 鼓励轨迹终点朝向目标
 *
 * 距离目标越近，朝向对齐越重要。使用距离门控 (goal_proximity)
 * 在远处削弱此项，避免过早强制朝向影响避障。
 *
 * 公式:
 *   goal_proximity = 1 / (1 + dist_to_goal · 1.6)      // 距离门控
 *   heading_to_goal = atan2(y_goal - y, x_goal - x)     // 目标方向
 *   heading_err = heading_to_goal - theta_final          // 朝向偏差
 *   cost = |heading_err| · 0.8 · goal_proximity
 *
 * 距离门控曲线 (goal_proximity):
 *   d = 0.0m   → prox = 1.00   (最近, 朝向对齐最重要)
 *   d = 0.5m   → prox = 0.56
 *   d = 1.0m   → prox = 0.38
 *   d = 1.5m   → prox = 0.29
 *   d = 2.0m   → prox = 0.24   (较远, 朝向对齐不重要)
 *
 * 设计意图:
 *   平缓的衰减使 MPPI 在 1~2m 就开始考虑朝向对齐，
 *   而非等到终点附近才突然要求对准。
 *
 * @param  x_final, y_final  轨迹终点的世界坐标 (m)
 * @param  theta_final       轨迹终点的朝向 (rad)
 * @param  target_x, target_y  前瞻目标点的世界坐标 (m)
 * @return                   正值 = 代价 (cost 增加)
 */
__device__ inline float compute_terminal_heading_cost(
    float x_final, float y_final, float theta_final,
    float target_x, float target_y)
{
  float dx = target_x - x_final;
  float dy = target_y - y_final;
  float dist_to_goal = sqrtf(dx * dx + dy * dy);

  // 距离门控: 越近 → goal_proximity 越大 → 朝向对齐越重要
  float goal_proximity = 1.0f / (1.0f + dist_to_goal * 1.6f);

  // 目标方向
  float heading_to_goal = atan2f(dy, dx);

  // 朝向偏差
  float heading_err = heading_to_goal - theta_final;
  heading_err = normalize_angle(heading_err);

  // 终端朝向代价 = 偏差² · 0.8 · 门控系数
  // 二次方: 小偏差容忍, 大偏差重罚 (0.5rad→0.25, 1.0rad→1.0, πrad→9.87)
  return heading_err * heading_err * 0.8f * goal_proximity;
}

// ═══════════════════════════════════════════════════════════════════════════
// ⑧ 余弦退火混合朝向代价 (Blended Heading Cost)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 余弦退火混合朝向代价 — 随距离从路径朝向过渡到目标姿态朝向
 *
 * 在每一步根据轨迹点到最终目标的距离，用余弦退火在"路径方向"
 * 和"目标姿态朝向"之间平滑过渡：
 *
 *   alpha = (1 + cos(π·t)) / 2,  t = clamp((d - d_end) / (d_start - d_end), 0, 1)
 *
 *   blended_heading = lerp(path_angle, goal_yaw, alpha)
 *   cost = (theta - blended_heading)² · weight / H    (二次方: 小偏差容忍, 大偏差重罚)
 *
 * 余弦退火曲线:
 *   d ≥ d_start (远): alpha ≈ 0 → 完全跟随路径方向
 *   d ∈ (d_end, d_start): alpha 平滑过渡
 *   d ≤ d_end  (近): alpha ≈ 1 → 完全跟随目标姿态朝向
 *
 *   离最终目标越近，路径朝向倾向下降，目标姿态朝向倾向上升。
 *
 * @param  theta               当前轨迹点的朝向 (rad)
 * @param  path_angle          路径在世界坐标系下的方向角 (rad)
 * @param  goal_yaw            最终目标姿态的朝向 (rad)
 * @param  dist_to_goal        当前点到最终目标的欧氏距离 (m)
 * @param  anneal_start        退火起始距离 (m)，>此距离完全跟随路径
 * @param  anneal_end          退火结束距离 (m)，<此距离完全跟随目标姿态
 * @param  weight              代价权重
 * @param  horizon             预测步数 (用于归一化)
 * @return                     正值 = 代价 (cost 增加)
 */
__device__ inline float compute_blended_heading_cost(
    float theta, float path_angle, float goal_yaw,
    float dist_to_goal, float anneal_start, float anneal_end,
    float weight, int horizon)
{
  // 归一化距离: t=0 在 anneal_end (近), t=1 在 anneal_start (远)
  float t = (dist_to_goal - anneal_end) / (anneal_start - anneal_end);
  t = fmaxf(0.0f, fminf(1.0f, t));

  // 余弦退火: alpha=1 近目标 (目标姿态主导), alpha=0 远离 (路径主导)
  float alpha = (1.0f + cosf(CUDART_PI_F * t)) * 0.5f;

  // 角度线性插值 (处理环绕)
  float diff = normalize_angle(goal_yaw - path_angle);
  float blended_heading = path_angle + alpha * diff;

  // 朝向偏差
  float heading_err = normalize_angle(theta - blended_heading);

  // 二次方: 0.1rad→0.01, 0.5rad→0.25, 1.0rad→1.0, πrad→9.87
  return heading_err * heading_err * weight / static_cast<float>(horizon);
}

// ═══════════════════════════════════════════════════════════════════════════
// ⑧ 前瞻点位置接近奖励 (Lookahead Proximity Reward)
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算前瞻点位置接近奖励 — 鼓励轨迹终点靠近前瞻点
 *
 * 纯位置奖励。朝向匹配已统一由 compute_blended_heading_cost 处理:
 *   远离前瞻点时跟随路径方向，靠近前瞻点时过渡到 lookahead_theta
 *   (CPU 端搜索的最优摆放朝向，在终点处等于 goal_yaw)。
 *
 * 公式:
 *   dist = √((lx - x_final)² + (ly - y_final)²)
 *   reward = -weight · exp(-dist / decay)
 *
 * @param  x_final, y_final    轨迹终点的世界坐标 (m)
 * @param  lookahead_x, lookahead_y   前瞻目标点的世界坐标 (m)
 * @param  weight              位置奖励权重 (≥0)
 * @param  decay               衰减距离 (m)
 * @return                     负值 = 奖励 (cost 减少)
 */
__device__ inline float compute_lookahead_proximity_reward(
    float x_final, float y_final,
    float lookahead_x, float lookahead_y,
    float weight, float decay)
{
  float dx = lookahead_x - x_final;
  float dy = lookahead_y - y_final;
  float dist = sqrtf(dx * dx + dy * dy);
  return -weight * expf(-dist / decay);
}

// ═══════════════════════════════════════════════════════════════════════════
// 辅助: 计算轨迹点到全局路径的最短距离平方
// ═══════════════════════════════════════════════════════════════════════════

/**
 * @brief 计算轨迹点到全局路径所有线段的最短距离平方
 *
 * 遍历路径段，对每个段调用 point_to_segment_dist_sq，取最小值。
 * 此值被 path_attraction_cost 复用。
 *
 * @param  x, y             轨迹点世界坐标 (m)
 * @param  path_x, path_y   全局路径点坐标数组 (世界坐标, m)
 * @param  num_path_pts     路径点数量
 * @return                  到路径的最短距离平方 (m²)，无路径时返回 FLT_MAX
 */
__device__ inline float compute_min_path_distance_sq(
    float x, float y,
    const float* __restrict__ path_x,
    const float* __restrict__ path_y,
    int num_path_pts)
{
  float min_dist_sq = FLT_MAX;

  if (num_path_pts < 2 || path_x == nullptr || path_y == nullptr) {
    return min_dist_sq;
  }

  for (int p = 0; p < num_path_pts - 1; ++p) {
    float d_sq = point_to_segment_dist_sq(
        x, y, path_x[p], path_y[p], path_x[p + 1], path_y[p + 1]);
    if (d_sq < min_dist_sq) min_dist_sq = d_sq;
  }

  return min_dist_sq;
}

#endif  // MPPI_GPU_REWARDS_CUH_
