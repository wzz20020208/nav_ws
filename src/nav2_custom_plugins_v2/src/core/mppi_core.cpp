/**
 * @file mppi_core.cpp
 * @brief MPPI 核心 — 噪声生成、采样、运动学积分、轨迹 rollout
 */

#include "nav2_custom_plugins_v2/core/mppi_core.hpp"

#include <random>
#include <cmath>

namespace nav2_custom_plugins_v2
{

// ═══════════════════════════════════════════════════════════════════════════
// NoiseGenerator
// ═══════════════════════════════════════════════════════════════════════════

NoiseGenerator::NoiseGenerator(const MPPIParams &params)
  : p_(params)
{}

void NoiseGenerator::generate(int N, int H, double path_yaw, double current_yaw)
{
  const int total = N * H;
  noise_vx_.resize(total);
  noise_vy_.resize(total);
  noise_delta_.resize(total);

  std::mt19937 rng(std::random_device{}());
  std::normal_distribution<float> gauss(0.0f, 1.0f);

  const float bias_weight = 0.15f;
  float lookahead_in_robot = static_cast<float>(path_yaw - current_yaw);
  const float bias_vx = bias_weight * cosf(lookahead_in_robot);
  const float bias_vy = bias_weight * sinf(lookahead_in_robot);

  const float delta_bias_weight = 0.5f;
  float heading_err = static_cast<float>(path_yaw - current_yaw);
  while (heading_err > M_PI)  heading_err -= 2.0f * M_PI;
  while (heading_err < -M_PI) heading_err += 2.0f * M_PI;
  const float bias_delta = delta_bias_weight * heading_err;

  for (int i = 0; i < total; ++i) {
    noise_vx_[i]    = gauss(rng) * p_.action_std_v    + bias_vx;
    noise_vy_[i]    = gauss(rng) * p_.action_std_vy   + bias_vy;
    noise_delta_[i] = gauss(rng) * p_.action_std_delta + bias_delta;
  }
}

// ═══════════════════════════════════════════════════════════════════════════
// 批量 rollout
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
  batch.delta.resize(total);

  const auto &nvx = noise_gen.noise_vx();
  const auto &nvy = noise_gen.noise_vy();
  const auto &nd  = noise_gen.noise_delta();

  for (int s = 0; s < N; ++s) {
    RobotState state = start;
    const int row = s * H;

    for (int t = 0; t < H; ++t) {
      const int idx = row + t;

      // 噪声衰减: scale=1.0, t=0→1.0, t=H-1→0.5
      double ratio = static_cast<double>(t) / std::max(1, H - 1);
      double ns = std::max(0.1, (1.0 - 0.5 * ratio) * 1.0);

      Control base = base_seq.step(t);
      Control u;
      u.vx    = std::clamp(base.vx    + nvx[idx] * ns, params.min_v,  params.max_v);
      u.vy    = std::clamp(base.vy    + nvy[idx] * ns, -params.max_vy, params.max_vy);
      u.delta = std::clamp(base.delta + nd[idx]  * ns, -params.max_steering_angle, params.max_steering_angle);

      state = kinematic_integrate(state, u, params.dt, params.max_w);

      batch.x[idx]     = static_cast<float>(state.x);
      batch.y[idx]     = static_cast<float>(state.y);
      batch.theta[idx] = static_cast<float>(state.theta);
      batch.vx[idx]    = static_cast<float>(u.vx);
      batch.vy[idx]    = static_cast<float>(u.vy);
      batch.delta[idx] = static_cast<float>(u.delta);
    }
  }

  return batch;
}

// ═══════════════════════════════════════════════════════════════════════════
// KinematicModel
// ═══════════════════════════════════════════════════════════════════════════

KinematicModel::KinematicModel(const MPPIParams &params)
  : max_w_(params.max_w)
  , max_steering_angle_(params.max_steering_angle)
{}

RobotState KinematicModel::integrate(const RobotState &state, const Control &u, double dt) const
{
  return kinematic_integrate(state, u, dt, max_w_);
}

double KinematicModel::deltaToOmega(double delta, double current_theta, double dt) const
{
  double dtheta = delta - current_theta;
  while (dtheta > M_PI)  dtheta -= 2.0 * M_PI;
  while (dtheta < -M_PI) dtheta += 2.0 * M_PI;
  return std::clamp(dtheta / dt, -max_w_, max_w_);
}

// ═══════════════════════════════════════════════════════════════════════════
// ControlSequence
// ═══════════════════════════════════════════════════════════════════════════

void ControlSequence::resize(int H)
{
  vx.resize(H, 0.0);
  vy.resize(H, 0.0);
  delta.resize(H, 0.0);
}

void ControlSequence::shiftAndDecay(double decay)
{
  if (vx.empty()) return;
  for (size_t i = 0; i < vx.size() - 1; ++i) {
    vx[i]    = vx[i + 1];
    vy[i]    = vy[i + 1];
    delta[i] = delta[i + 1];
  }
  size_t last = vx.size() - 1;
  vx[last]    *= decay;
  vy[last]    *= decay;
  delta[last] *= decay;
}

Control ControlSequence::step(int t) const
{
  return {vx[static_cast<size_t>(t)], vy[static_cast<size_t>(t)], delta[static_cast<size_t>(t)]};
}

}  // namespace nav2_custom_plugins_v2
