/**
 * @file mppi_gpu_critics.cuh
 * @brief [兼容转发] 代价函数已拆分至 src/cost/ 目录
 *
 * 拆分后的文件:
 *   src/cost/critic_common.cuh     — CriticCategory, CriticBase
 *   src/cost/obstacle_critic.cuh   — ObstacleCritic, FootprintCritic, ObstacleCategory
 *   src/cost/heading_critic.cuh    — HeadingCritic, PathAlignCritic, PathAngleCritic, ...
 *   src/cost/speed_critic.cuh      — SpeedCritic, SpeedRewardCritic, SpeedCategory
 *   src/cost/critic_manager.cuh    — CriticManager
 *
 * 新代码请直接 #include "critic_manager.cuh", 不要经过本文件。
 */

#ifndef MPPI_GPU_CRITICS_CUH_
#define MPPI_GPU_CRITICS_CUH_

#include "critic_manager.cuh"

#endif  // MPPI_GPU_CRITICS_CUH_
