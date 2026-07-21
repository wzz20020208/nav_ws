/**
 * @file mppi_core.cpp
 * @brief MPPI 核心 — 噪声生成、采样、运动学积分、轨迹 rollout
 *
 * 控制空间: [vx, vy, omega]  (1:1 THEMIS)
 */

#include "nav2_custom_plugins_v2/core/mppi_core.hpp"

#include <random>
#include <cmath>

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGenerator — 零均值高斯 + 路径方向偏置 + omega 朝向偏置
// ═══════════════════════════════════════════════════════════════════════════

NoiseGenerator::NoiseGenerator(const MPPIParams &params)
  : p_(params)
{}

void NoiseGenerator::generate(int N, int H, double path_yaw, double current_yaw)
{
  const int total = N * H;
  noise_vx_.resize(total);
  noise_vy_.resize(total);
  noise_w_.resize(total);

  std::mt19937 rng(std::random_device{}());
  std::normal_distribution<float> gauss(0.0f, 1.0f);

  // vx/vy 偏置: 向 lookahead 方向偏移
  const float bias_weight = 0.0f;
  float lookahead_in_robot = static_cast<float>(path_yaw - current_yaw);
  const float bias_vx = bias_weight * cosf(lookahead_in_robot);
  const float bias_vy = bias_weight * sinf(lookahead_in_robot);

  // omega 偏置: 朝向偏差越大, omega 越偏向纠正方向
  float heading_err = static_cast<float>(path_yaw - current_yaw);
  while (heading_err > M_PI)  heading_err -= 2.0f * M_PI;
  while (heading_err < -M_PI) heading_err += 2.0f * M_PI;
  const float omega_bias = 0.5f * heading_err;  // 偏差→纠正偏置

  for (int i = 0; i < total; ++i) {
    noise_vx_[i] = gauss(rng) * p_.action_std_v + bias_vx;
    noise_vy_[i] = gauss(rng) * p_.action_std_vy + bias_vy;
    noise_w_[i]  = gauss(rng) * p_.action_std_w + omega_bias;
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// batch_rollout — u = base + noise * ns, omega 加路径朝向偏置
// ═══════════════════════════════════════════════════════════════════════════

BatchTrajectories batch_rollout(
    const RobotState &start,
    const ControlSequence &base_seq,
    const NoiseGenerator &noise_gen,
    const MPPIParams &params,
    int N, int H)
{
  const int total = N * H;

  BatchTrajectories batch;
  batch.x.resize(total);
  batch.y.resize(total);
  batch.theta.resize(total);
  batch.vx.resize(total);
  batch.vy.resize(total);
  batch.omega.resize(total);

  const auto &nvx = noise_gen.noise_vx();
  const auto &nvy = noise_gen.noise_vy();
  const auto &nw  = noise_gen.noise_w();

  const float max_v_f  = static_cast<float>(params.max_v);
  const float min_v_f  = static_cast<float>(params.min_v);
  const float max_vy_f = static_cast<float>(params.max_vy);
  const float max_w_f  = static_cast<float>(params.max_w);

  for (int s = 0; s < N; ++s) {
    RobotState state = start;
    const int row = s * H;

    for (int t = 0; t < H; ++t) {
      const int idx = row + t;

      double ratio = static_cast<double>(t) / std::max(1, H - 1);
      double ns = std::max(0.1, (1.0 - 0.5 * ratio) * 1.0);

      Control base = base_seq.step(t);
      float bvx    = static_cast<float>(base.vx);
      float bvy    = static_cast<float>(base.vy);
      float bw     = static_cast<float>(base.omega);

      float vx    = bvx + nvx[idx] * ns;
      float vy    = bvy + nvy[idx] * ns;
      float omega = bw  + nw[idx]  * ns;

      vx    = fminf(max_v_f, fmaxf(min_v_f, vx));
      vy    = fminf(max_vy_f, fmaxf(-max_vy_f, vy));
      omega = fminf(max_w_f, fmaxf(-max_w_f, omega));

      Control u;
      u.vx = vx; u.vy = vy; u.omega = omega;

      state = kinematic_integrate(state, u, params.dt, params.max_w);

      batch.x[idx]     = static_cast<float>(state.x);
      batch.y[idx]     = static_cast<float>(state.y);
      batch.theta[idx] = static_cast<float>(state.theta);
      batch.vx[idx]    = vx;
      batch.vy[idx]    = vy;
      batch.omega[idx] = omega;
    }
  }

  return batch;
}

// ═══════════════════════════════════════════════════════════════════════════
// KinematicModel
// ═══════════════════════════════════════════════════════════════════════════

KinematicModel::KinematicModel(const MPPIParams &params)
  : max_w_(params.max_w)
{}

RobotState KinematicModel::integrate(const RobotState &state, const Control &u, double dt) const
{
  return kinematic_integrate(state, u, dt, max_w_);
}

// ═══════════════════════════════════════════════════════════════════════════
// ControlSequence
// ═══════════════════════════════════════════════════════════════════════════

void ControlSequence::resize(int H)
{
  vx.resize(H, 0.0);
  vy.resize(H, 0.0);
  omega.resize(H, 0.0);
}

void ControlSequence::shiftAndDecay(double decay)
{
  if (vx.empty()) return;
  for (size_t i = 0; i < vx.size() - 1; ++i) {
    vx[i]    = vx[i + 1];
    vy[i]    = vy[i + 1];
    omega[i] = omega[i + 1];
  }
  size_t last = vx.size() - 1;
  vx[last]    *= decay;
  vy[last]    *= decay;
  omega[last] *= decay;
}

Control ControlSequence::step(int t) const
{
  return {vx[static_cast<size_t>(t)], vy[static_cast<size_t>(t)], omega[static_cast<size_t>(t)]};
}

}  // namespace nav2_custom_plugins_v2
