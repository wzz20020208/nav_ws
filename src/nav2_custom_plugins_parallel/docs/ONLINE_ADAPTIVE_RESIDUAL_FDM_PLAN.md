# 在线自适应残差动力学 + 不确定性感知 MPPI

> **论文方向**: Online Adaptive Neural Residual Dynamics with Uncertainty-Aware MPPI for Smooth Legged Robot Navigation  
> **目标期刊**: IEEE RA-L (Q2, IF≈5) 或 Robotics and Autonomous Systems (Q2)  
> **创建日期**: 2026-06-24  
> **文献调研日期**: 2026-06-24  
> **状态**: 文献调研已完成 ✅ - 确认创新性，可推进实施

---

## 目录

1. [核心 Idea](#1-核心-idea)
2. [方法详述](#2-方法详述)
3. [对比实验设计](#3-对比实验设计)
4. [预期结果](#4-预期结果)
5. [实施路线图](#5-实施路线图)
6. [论文提纲](#6-论文提纲)
7. [文献调研与创新性分析](#7-文献调研与创新性分析)
8. [待办事项清单](#8-待办事项清单)

---

## 1. 核心 Idea

### 1.1 问题

足式机器人的速度响应存在严重的非均匀性——步态相位切换、足地接触力、机身摆动导致瞬时速度与 MPPI 指令之间存在**时变偏差**。当前使用简单运动学模型 `x_{t+1} = RK2(x_t, u_t)` 假设指令被完美执行，这在足式机器人上不成立。

### 1.2 三大贡献

```
C1 — 微型时序残差动力学网络
     GRU(44→12) + Output(12→6), 仅 1,700 参数
     18K 帧离线训练成形, 样本/参数比 10:1
     GPU kernel 内嵌, 每步 ~2K MAC, 20Hz 实时
C2 — 两阶段训练: 离线成形 + 在线适应
     Phase 2: 全部 1,700 参数在 18K 真机帧上训练 → GRU 学到时序模式
     Phase 4: 仅微调输出层 78 参数 → 持续适应地面变化
     解决 ETH FDM 对大量仿真数据的依赖

C3 — 不确定性注入 MPPI 代价函数
     网络同时输出残差 (Δv, Δω) 和不确定性 (σ_v, σ_ω)
     首次将 learned dynamics uncertainty 注入采样 MPC 代价函数
```

### 1.3 与现有工作的核心区别

| | 离线学习动力学模型 | Ours |
|---|---|---|
| 训练方式 | 离线 (大量仿真/真机数据) | **在线 (零真机预训练)** |
| 网络输出 | 状态预测 | **残差 + 不确定性** |
| 不确定性 | 无 或 用于约束 | **注入 MPPI 代价函数** |
| 推理平台 | 桌面 GPU 或 通用框架 | **Jetson Orin + 自定义 CUDA kernel** |
| 泛化 | 固定权重 | **持续适应新地形** |

**注**：文献调研确认，将 learned epistemic uncertainty 显式注入采样 MPC 代价函数的工作极为罕见。

---

## 2. 方法详述

### 2.0 为什么不做仿真预训练

仿真预训练的前提是**仿真和真机使用相同的运动控制器**，这样 FDM 在仿真中学到的误差模式（步态振荡频率、加减速滞后的时序结构）才能迁移到真机。但当前条件不满足：

```
B2 真机:  MPPI → cmd_vel → SportClient (宇树闭源固件) → B2 硬件 → odom
B2 仿真:  MPPI → cmd_vel → ???                                  → odom

unitree_mujoco 不支持 SportClient 仿真
如果用自训 RL 策略替代 → 仿真误差结构 ≠ 真机误差结构 → 预训练反而有害
```

**结论**: 在两个控制器不同的前提下，仿真预训练没有理论保证。放弃它。

### 2.1 路线选择

**当前实施: 路线 B — 纯真机在线学习**

```
Phase 1: 真机预热采集 (15 分钟)
  → Phase 2: 离线训练 FDM 输出层 (198 参数, ~2 分钟)
  → Phase 3: GPU Kernel 集成
  → Phase 4: 在线微调 (运行时持续)
```

**备选: 路线 A — 统一 RL 控制器**（未来条件具备时实施，见 §2.6）

### 2.2 路线 B 训练管线

```
┌──────────────────────────────────────────────────────────────┐
│  Phase 1: 真机预热采集 (15 分钟)                               │
│                                                               │
│  B2 + SportClient 在蘑菇工厂跑 3 条路径各 1 次                │
│  MPPI 正常输出 cmd_vel → SportClient.Move() → B2 执行         │
│  记录每帧: (cmd, odom, imu, gait_phase)                      │
│  数据量: ~18,000 帧                                           │
├──────────────────────────────────────────────────────────────┤
│  Phase 2: 离线训练 FDM (~2 分钟)                               │
│                                                               │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │ 网络: GRU(44→12) + Output(12→6), 共 ~1,700 参数          │ │
│  │                                                           │ │
│  │ 全部参数参与训练 (与旧方案不同, GRU 不是随机的)            │ │
│  │   损失: Gaussian NLL(Δ, σ²)                              │ │
│  │   正则: Dropout(0.1) + WeightDecay(1e-5)                 │ │
│  │   优化: Adam lr=3e-4, batch=128, 100 epochs              │ │
│  │   早停: val_loss 10 epoch 不降                            │ │
│  │                                                           │ │
│  │ 样本/参数比: 18,000/1,700 ≈ 10:1  ← 充足                 │ │
│  │   10:1 对于时序回归任务是安全的 (参考: 时序预测通常5:1+)    │ │
│  └─────────────────────────────────────────────────────────┘ │
│                                                               │
│  验证: 离线 R² > 0.5 (18K样本训练的1.7K参数网络应达到)        │
│  输出: fdm_init.bin                                           │
├──────────────────────────────────────────────────────────────┤
│  Phase 3: GPU Kernel 集成                                     │
│                                                               │
│  加载 fdm_init.bin → __constant__ memory                      │
│  实现 gru_cell() (12维隐层), linear(), fdm_infer()           │
│  不确定性代价注入                                              │
├──────────────────────────────────────────────────────────────┤
│  Phase 4: 在线微调 (运行时, 持续)                              │
│                                                               │
│  冻结: GRU(44→12) (~1,600 参数, Phase 2 训练成形)            │
│  微调: Output(12→6) (~80 参数)                                │
│  锚点: fdm_init.bin 的 Output 权重 (λ_forget=0.01)           │
│  数据: 滑动窗口 buffer 2000 对 (~100s)                        │
│  更新: Adam lr=1e-4, batch=64, 5 epochs, 背景线程             │
│  lr 衰减: ×0.95 / 1000步, 下限 1e-5                           │
│                                                               │
│  GRU 冻结的理由: 18K 样本已训练成形, 步态振荡频率/滞后时间等   │
│  时序模式已编码在 GRU 权重中; 在线阶段只需调整输出量级          │
└──────────────────────────────────────────────────────────────┘
```

### 2.5 网络架构

```
输入层 (每步, 44D):
├── 当前状态 (6D): [vx, vy, ω, roll, pitch, vz_body]
├── 控制指令 (3D): [cmd_vx, cmd_vy, cmd_ω]
├── 步态上下文 (3D): [gait_phase, stance_ratio, contact_state]
└── 隐状态 (12D): h_{t-1} (GRU 传递, 每线程维护)

时序编码 (GRU, 44→12):
  h_t = GRU(input_44D, h_{t-1})
  ← Phase 2 全部训练, Phase 4 冻结
  参数量: 3 × (44×12 + 12×12 + 12×2) ≈ 1,600
  设计理由: 12维隐层足够编码步态频率(1维)、滞后时间(1维)、
           转弯侧滑趋势(2维)等核心时序模式, 且不过参数化

输出层 (Linear, 12→6):
  [Δvx, Δvy, Δω, log_σ²vx, log_σ²vy, log_σ²ω] = W_out · h_t + b_out
  ← Phase 2 全部训练, Phase 4 在线微调
  参数量: 12×6 + 6 = 78

总参数: ~1,700
Phase 2 训练全部 1,700 参数 (18K 样本, 10:1 样本/参数比)
Phase 4 微调 78 参数 (GRU 冻结, 仅更新输出层)
```

**设计权衡**:

| | 大随机池 (废弃) | 小训练 GRU (当前) |
|---|---|---|
| GRU 隐层 | 32 (随机, 冻结) | **12 (训练, 后冻结)** |
| MLP 中间层 | 2 层 (随机, 冻结) | **去掉 (GRU 直连输出)** |
| 总参数 | 9,200 | **1,700** |
| 离线可训练 | 198 (2%) | **1,700 (100%)** |
| 在线可训练 | 198 | **78** |
| GRU 特征来源 | 随机初始化 | 数据驱动 |
| 理论依据 | Reservoir computing | 标准监督学习 |
| 离线 R² 预期 | 0.3-0.4 (不确定) | **0.5-0.6** |
| 样本需求 | 18,000+ | **18,000 ✓ (10:1)** |
| GPU 每步推理 | ~7K MAC | **~2K MAC** |
```
### 2.3 备选方案: 路线 A — 统一 RL 控制器

> **触发条件**: 当满足以下条件时可切换到此路线
> 1. B2 真机可部署自训 RL 策略（通过 `unitree_rl_lab` + `LowCmd` 接口）
> 2. 仿真和真机跑**同一个** RL 策略
> 3. 仿真中已训练好速度跟踪 RL 策略（Phase 1 RL 训练）

```
路线 A 的完整管线:

Phase A1: RL 速度跟踪策略训练 (仿真, ~2天)
  在 unitree_mujoco / Isaac Lab 中用 PPO 训练 B2 速度跟踪
  输入: cmd_vel → 输出: 12 关节位置
  训练到速度跟踪 RMSE < 5 cm/s
  → policy.onnx

Phase A2: 仿真 FDM 数据收集 (自动化, ~7小时)
  加载 policy.onnx 到 unitree_mujoco
  MPPI + RL策略 在随机场景中导航
  记录 (cmd, odom, imu, gait) 50万帧
  → sim_dataset.h5

Phase A3: FDM 全精度预训练 (离线)
  用 sim_dataset 训练 GRU+MLP 全部 8000 参数
  损失: Gaussian NLL
  → fdm_pretrained.bin

Phase A4: 真机校准 + 在线微调
  同路线 B 的 Phase 4, 但 GRU 权重是预训练的而非随机
  锚点: fdm_pretrained 的输出层权重

路线 A vs B 的核心区别:
  ┌──────────┬─────────────────────┬─────────────────────┐
  │          │ 路线 A              │ 路线 B              │
  ├──────────┼─────────────────────┼─────────────────────┤
  │ 前置条件 │ 自训 RL 策略可部署   │ 无                  │
  │ GRU 特征 │ 预训练 (有结构)      │ 18K数据训练          │
  │ 真机数据 │ 10 分钟 (校准)       │ 15 分钟 (预热+训练) │
  │ 在线收敛 │ 快 (GRU 已成形)      │ 中 (需先采集18K)     │
  │ 学术卖点 │ 仿真→真机迁移        │ 零仿真、纯在线       │
  └──────────┴─────────────────────┴─────────────────────┘
```

### 2.4 不确定性注入

```
标准 MPPI 代价:
  total_cost = Σ cost_terms  (碰撞/路径/朝向/...)

Ours 新增:
  cost_σ = w_σ · (σ_vx + σ_vy + σ_ω) / horizon
  total_cost += cost_σ

  σ_raw = exp(log_σ²) / 2  → σ = softplus(σ_raw) + ε

效果:
  ┌──────────────┬──────────────┬─────────────────────┐
  │ 场景         │ σ 水平       │ MPPI 行为            │
  ├──────────────┼──────────────┼─────────────────────┤
  │ 匀速巡航     │ 低 (0.01)    │ 正常，不增加代价     │
  │ 步态切换     │ 中 (0.05)    │ 轻微代价，稍保守     │
  │ 转弯/U型调头 │ 高 (0.15)    │ 显著代价，降速谨慎   │
  │ 地面湿滑     │ 很高 (0.30+) │ 强代价，极度保守     │
  └──────────────┴──────────────┴─────────────────────┘

  关键: 不需要手写"什么时候该保守"的规则
       网络自己学会 σ 和实际误差的相关性
```

### 2.6 GPU Kernel 集成

```cuda
// 每步推理 (替换原始 RK2 中的 vx,vy,ω 直接使用)
__device__ void fdm_infer(
    float vx, float vy, float omega,
    float cmd_vx, float cmd_vy, float cmd_omega,
    float gait_phase, float roll, float pitch, float vz,
    float* h_prev,          // [FDM_HIDDEN] 隐状态 (寄存器)
    const float* weights,   // __constant__ 权重
    float* real_v, float* sigma)
{
    // 1. 拼输入
    float in[FDM_INPUT] = {vx, vy, omega, cmd_vx, cmd_vy, cmd_omega,
                           gait_phase, roll, pitch, vz,
                           h_prev[0], ..., h_prev[FDM_HIDDEN-1]};

    // 2. GRU 步 (12维隐层, Phase 2 训练, Phase 4 冻结)
    float h_new[FDM_HIDDEN];  // FDM_HIDDEN = 12
    gru_cell_12(in, h_prev, h_new, weights);

    // 3. 输出层 (78 参数, 在线可微调)
    float out[6];
    linear_12_6(h_new, out, weights + GRU_WEIGHT_SIZE);

    // 4. 解包输出
    real_v[0] = cmd_vx + out[0];
    real_v[1] = cmd_vy + out[1];
    real_v[2] = cmd_omega + out[2];
    sigma[0] = softplus(out[3]) + 1e-6f;
    sigma[1] = softplus(out[4]) + 1e-6f;
    sigma[2] = softplus(out[5]) + 1e-6f;

    // 6. 更新隐状态
    for (int i = 0; i < FDM_HIDDEN; i++) h_prev[i] = h_new[i];
}
```

### 2.7 在线学习: 背景线程

```
主线程 (20Hz MPPI):                    背景线程:
┌──────────────────────┐              ┌─────────────────────────┐
│ fdm_infer()          │              │ while running:           │
│ → real_v, σ          │              │   sleep(50ms)            │
│ cost += σ · w_σ      │    push      │   if buffer.size() < 64:│
│                      │ ────────────→│     continue             │
│ buffer.push(          │              │                          │
│   input, Δ_true)     │              │   for epoch in 1..5:     │
│                      │              │     batch = sample(64)   │
└──────────────────────┘              │     loss = NLL + EMA     │
                                       │     Adam(θ_output, lr)  │
Ring Buffer: 2000 对 (~100s)          │                          │
自动淘汰最旧数据                        │   cudaMemcpyToSymbol(    │
                                       │     OUTPUT_OFFSET)      │
EMA 正则: λ_forget = 0.01             └─────────────────────────┘
锚点 = Stage 2 校准权重
```



---

## 3. 对比实验设计

### 3.1 基线方法

| # | 方法 | 动力学 | 在线 | 不确定性 | 说明 |
|---|------|--------|------|---------|------|
| B1 | Vanilla MPPI | RK2 | ✗ | ✗ | 当前系统, 性能下界 |
| B2 | MPPI + 离线 FDM | 预训练 GRU-MLP | ✗ | ✗ | 对标离线学习范式 |
| B3 | MPPI + 在线残差 | MLP 在线微调 | ✓ | ✗ | 消融: 去掉不确定性 |
| **B4** | **纯在线学习** | **随机初始化在线学习** | **✓** | **✓** | **消融: 验证仿真预训练价值** |
| Ours | 完整方案 | 仿真预训练+在线微调 | ✓ | ✓ | B3 + 不确定性注入 |

### 3.2 实验场景 — 蘑菇工厂

```
┌─────────────────────────────────────────────────┐
│              蘑菇工厂俯视示意图                    │
│                                                   │
│  ═══════════  ═══════════  ═══════════           │
│  ‖ 菌架1  ‖  ‖ 菌架2  ‖  ‖ 菌架3  ‖            │
│  ‖        ‖  ‖        ‖  ‖        ‖            │
│  ═══════════  ═══════════  ═══════════           │
│       ← 通道A →    ← 通道B →    ← 通道C →        │
│       0.8m 宽      1.0m 宽      1.2m 宽          │
│  ═══════════  ═══════════  ═══════════           │
│  ‖ 菌架4  ‖  ‖ 菌架5  ‖  ‖ 菌架6  ‖            │
│  ═══════════  ═══════════  ═══════════           │
│                                                   │
│  地面: 环氧树脂/瓷砖, 可能有冷凝水 (低摩擦)       │
│  任务: 沿通道巡检, 通道尽头 U 型调头               │
└─────────────────────────────────────────────────┘

S1 — 直行窄通道 (0.8m 宽, 长 8m)
      仅需直行, 验证基本速度均匀性
      通道宽度 = 机器人宽 0.6m + 两侧各 0.1m 余量
      → 碰撞代价持续存在, MPPI 容易犹豫/推头

S2 — 窄通道 + 直角转弯 (1.0m 宽, 通道尽头右转)
      接近转弯时步态切换 + 机头大幅度转动
      速度不均匀性最显著

S3 — 窄通道 U 型调头 (1.2m 宽走廊尽头 180° 掉头)
      需要完全停下 → 原地旋转 → 重新加速
      步态从行走→静止→旋转→行走, 速度断裂最严重
```

### 3.3 速度档位

| 档位 | 指令速度 | 场景特征 |
|------|---------|---------|
| 低速 | 0.15 m/s | 谨慎通过超窄通道 (S1 0.8m), 步态频繁切换 |
| 中速 | 0.30 m/s | 正常巡检速度, S2/S3 主要工作区间 |
| 高速 | 0.40 m/s | S3 宽通道 (1.2m), 考验极限跟踪 |

### 3.4 实验矩阵

```
3 场景 × 3 速度 × 5 方法 = 45 组
每组 5 次重复 = 225 次实验
```

### 3.5 量化指标

**L1 — 速度跟踪精度 (核心)**:
```
RMSE_vx  = sqrt(mean((cmd_vx - odom_vx)²))
RMSE_vy  = sqrt(mean((cmd_vy - odom_vy)²))
RMSE_ω   = sqrt(mean((cmd_ω  - odom_ω)²))
采样: 全帧 @ 20Hz
```

**L2 — 速度平滑度 (核心)**:
```
Jerk_v   = mean(|acc_{t+1} - acc_t|)     ← 加速度逐帧变化
SPARC     = -∫ ω̂(ω) log ω̂(ω) dω           ← 频域谱弧长
  其中 ω̂(ω) = 归一化傅里叶幅值
```

**L3 — 轨迹精度**:
```
CTE_RMSE  = sqrt(mean(cross_track_error²))
Terminal_Pose_Error = hypot(dx, dy) + |dθ|
```

**L4 — 在线学习效率**:
```
Convergence_Time  = RMSE 降至稳态 90% 的时间
Adaptation_Speed  = 地形切换后恢复时间
```

**L5 — 导航成功率**:
```
Success_Rate       = 无碰撞到达终点比例
Avg_Nav_Time       = 成功实验的平均耗时
```

**L6 — 不确定性校准 (仅 Ours)**:
```
ECE = Σ |B_m|/n · |acc(B_m) - conf(B_m)|
可靠性图: 预测 σ vs 实际 RMSE 的散点图 + 线性拟合
```

### 3.6 关键对比分析

```
┌──────────────────────────────────────────────────────────┐
│ 对比 1: B1 vs B2 vs Ours — 整体有效性                    │
│   预期: Ours > B2 > B1                                    │
│   关键: Ours vs B2 的差距验证在线学习的价值               │
│   最显著场景: S2(直角转弯) — 离线训练无法覆盖所有转弯工况 │
├──────────────────────────────────────────────────────────┤
│ 对比 2: B3 vs Ours — 不确定性的贡献 (消融)               │
│   预期: Ours > B3, 差距 S3(U型调头) > S2 > S1            │
│   解释: 步态断裂越严重, σ 的保守引导越有价值              │
├──────────────────────────────────────────────────────────┤
│ 对比 3: B4 vs Ours — 仿真预训练的价值 (消融)             │
│   预期: Ours > B4 (收敛速度更快, 稳定性更好)              │
│   解释: 验证"仿真学结构, 真机学量级"的设计哲学            │
│   关键指标: L4 在线学习效率 (Convergence_Time)           │
├──────────────────────────────────────────────────────────┤
│ 对比 4: S1 vs S3 — 场景复杂度梯度                         │
│   预计难度: S1(直行) < S2(转弯) < S3(U型调头)            │
│   预期: B1/B2 在 S3 严重退化, Ours 维持                   │
├──────────────────────────────────────────────────────────┤
│ 对比 5: 低速 vs 高速 — 速度依赖性                        │
│   核心矛盾: 窄通道需要低速(安全) vs 低速步态波动大(不平滑)│
│   预期: Ours 在低速窄通道找到最佳平衡点                   │
├──────────────────────────────────────────────────────────┤
│ 对比 6: 不确定性校准质量                                  │
│   S3 U型调头时 σ 应有明显的 启动→高峰→衰减 时序模式       │
│   可视化: σ 时序曲线 vs 实际 RMSE 逐帧对比               │
└──────────────────────────────────────────────────────────┘
```

### 3.7 预期结果

```
表 1: 速度跟踪 RMSE_vx (cm/s)

方法          S1-低速    S2-中速    S3-中速    S3-低速
             (直行0.8m) (转弯1.0m) (U型1.2m) (U型0.15)
────────────────────────────────────────────────────
B1 (Vanilla)  11.8±1.9   13.6±2.3   15.2±2.8   18.4±3.1
B2 (离线FDM)   7.5±1.3   10.2±1.8   12.1±2.2   14.6±2.5
B3 (在线-σ)    5.8±1.0    7.8±1.4    9.3±1.6   11.2±1.9
Ours           5.1±0.8    6.2±1.1    7.4±1.3    8.1±1.5
────────────────────────────────────────────────────
提升 vs B1:   57%↓       54%↓       51%↓       56%↓
Δ (σ贡献):    12%↓       21%↓       20%↓       28%↓
```

```
表 2: 速度平滑度 Jerk_vx (m/s³)

方法          S1-中速    S2-中速    S3-中速    S3-低速
────────────────────────────────────────────────────
B1 (Vanilla)   2.6±0.4    4.3±0.7    5.8±0.9    6.5±1.0
B2 (离线FDM)   1.9±0.3    3.5±0.5    4.7±0.7    5.5±0.8
B3 (在线-σ)    1.5±0.2    2.7±0.4    3.5±0.5    4.2±0.6
Ours           1.2±0.2    2.0±0.3    2.6±0.4    3.0±0.5
────────────────────────────────────────────────────
提升 vs B1:   54%↓       53%↓       55%↓       54%↓
```

---

## 4. 实施路线图 (路线 B)

```
Phase 1 ──── Phase 2 ──── Phase 3 ──── Phase 4 ──── Phase 5
 真机预热     离线训练      GPU集成      在线微调      实验+论文
 (0.5周)     (0.5周)       (2周)        (1周)         (3周)

总计: ~7 周
```

| Phase | 输入 | 产出 | 验证标准 |
|-------|------|------|---------|
| P1 真机预热 | B2 + 蘑菇工厂 | ~18,000 帧 | 数据覆盖 S1-S3 三个场景 |
| P2 离线训练 | 18,000 帧 | fdm_init.bin | 离线 R² > 0.4; 随机特征有效性验证 |
| P3 GPU 集成 | fdm_init.bin | kernel 推理通过 | S1 上跑通, 无 NaN, σ∈[0.01,0.3] |
| P4 在线微调 | 运行时数据 | 收敛模型 | loss 稳定下降, R² 逐帧改善 |
| P5 全量实验 | 最终模型 | 180 次数据 | 全指标 p<0.05 |

### 4.1 Phase 1: 真机预热采集

```
任务:
├─ B2 + SportClient 在蘑菇工厂跑 3 条路径各 1 次
│   S1: 直行窄通道 0.8m × 8m, 中速 0.3 m/s
│   S2: 窄通道 1.0m + 尽头直角转弯, 中速 0.3 m/s
│   S3: 宽通道 1.2m + U型调头, 低速 0.15 m/s
│   每条 ~5 分钟 → 共 ~15 分钟 → ~18,000 帧
│
├─ 记录每帧: (cmd_vx, cmd_vy, cmd_ω, odom_vx, odom_vy, odom_ω,
│              roll, pitch, vz_body, gait_phase, stance, contact)
│
└─ 计算残差: Δv = odom_v - cmd_v (训练目标)
```

### 4.2 Phase 2: 离线训练 FDM 输出层

```
任务:
├─ Xavier 初始化 GRU + MLP L1 + MLP L2 → 冻结
├─ 随机初始化输出层 198 参数
├─ 用 18,000 帧训练输出层
│   损失: Gaussian NLL(Δ, σ²)
│   优化: Adam lr=3e-4, batch=128, 50 epochs
├─ 验证: held-out R² > 0.4 (确认随机特征 + 输出层有表达能力)
└─ 输出: fdm_init.bin
```

### 4.3 Phase 3: GPU Kernel 集成

```
任务:
├─ 加载 fdm_init.bin → GPU __constant__ memory
├─ 实现 CUDA kernel: gru_cell(), mlp_forward(), fdm_infer()
├─ 每线程 GRU 隐状态管理 (d_thread_hidden_states)
├─ 不确定性代价注入 (uncertainty_weight 参数)
└─ 测试: S1 场景跑通, 输出无 NaN
```

### 4.4 Phase 4: 在线微调

```
任务:
├─ Ring Buffer: 2000 对, 滑动窗口
├─ 后台线程: Adam lr=1e-4, batch=64, 5 epochs
├─ EMA 锚点: fdm_init.bin 的输出层权重 (λ_forget=0.01)
├─ lr 衰减: ×0.95 / 1000 步, 下限 1e-5
└─ 监控: loss 移动平均, σ 均值, 权重变化量
```

### 4.5 Phase 5: 全量实验

```
180 次实验 (3 场景 × 3 速度 × 4 方法 × 5 重复)
分析: RMSE / Jerk / SPARC / CTE / Success / ECE
统计: Wilcoxon signed-rank, p<0.05, Cohen's d
```

---

## 5. 论文提纲

```
1. Introduction
   - 足式机器人导航的核心挑战: 速度不均匀
   - MPPI 的优势和运动学模型的局限
   - 本文贡献 (3 点)

2. Related Work
   - 2.1 MPPI for Legged Locomotion
   - 2.2 Learned Dynamics Models (ETH FDM, Neural ODE)
   - 2.3 Online Learning for Robotics
   - 2.4 Uncertainty in Model-Based Control

3. Method
   - 3.1 Problem Formulation
   - 3.2 Neural Residual Dynamics Architecture
   - 3.3 Online Adaptation with Forgetting
   - 3.4 Uncertainty-Aware MPPI Cost Function
   - 3.5 GPU-Accelerated Real-Time Implementation

4. Experimental Setup
   - 4.1 Robot Platform (THEMIS)
   - 4.2 Terrain Scenarios (S1-S4)
   - 4.3 Baselines (B1-B3)
   - 4.4 Metrics

5. Results
   - 5.1 Velocity Tracking Accuracy
   - 5.2 Velocity Smoothness
   - 5.3 Ablation Study (σ contribution)
   - 5.4 Online Learning Efficiency
   - 5.5 Uncertainty Calibration
   - 5.6 Trajectory-Level Performance

6. Discussion
   - 6.1 When Does Online Learning Help Most?
   - 6.2 Limitations and Failure Cases
   - 6.3 Future Work (Multi-Terrain, Multi-Gait)

7. Conclusion
```

---

## 6. 新增 ROS 参数速查

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `fdm_enabled` | false | 启用 FDM 动力学模型 |
| `fdm_model_path` | "" | 预训练权重路径 (空=随机初始化) |
| `fdm_online_learning` | true | 启用在线学习 |
| `fdm_learning_rate` | 1e-4 | 在线学习率 |
| `fdm_buffer_size` | 2000 | Replay buffer 大小 |
| `fdm_batch_size` | 64 | 在线学习批大小 |
| `fdm_forget_lambda` | 0.01 | 遗忘强度 |
| `uncertainty_weight` | 0.5 | σ 注入 MPPI 代价的权重 |
| `fdm_hidden_dim` | 32 | GRU 隐层维度 (仅训练时) |

---

*方案创建: 2026-06-24 | 状态: 待实施*
