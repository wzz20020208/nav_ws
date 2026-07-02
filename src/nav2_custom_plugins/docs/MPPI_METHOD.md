# MPPI-GPU：面向全向移动机器人的 GPU 加速 MPPI 与状态机引导方法

## 1. 问题建模

考虑全向移动机器人在室内 cluttered 环境中的局部运动规划问题。机器人状态定义为 $\mathbf{x} = (x, y, \theta) \in SE(2)$，控制量为 $\mathbf{u} = (v_x, v_y, \omega) \in \mathbb{R}^3$，其中 $v_x, v_y$ 为机体坐标系下的纵向与横向速度，$\omega$ 为角速度。系统动力学采用带侧向滑移的单车模型：

$$\begin{bmatrix} \dot{x} \\ \dot{y} \\ \dot{\theta} \end{bmatrix} = \begin{bmatrix} \cos\theta & -\sin\theta & 0 \\ \sin\theta & \cos\theta & 0 \\ 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} v_x \\ v_y \\ \omega \end{bmatrix}$$

控制目标为：在每个控制周期内，求解最优控制序列 $\mathbf{u}_{0:H-1}^*$，使得有限预测时域 $H$ 内的代价函数 $J$ 最小化，同时满足运动学约束与避障要求。

## 2. MPPI 基础

模型预测路径积分控制（Model Predictive Path Integral, MPPI）[Williams et al., 2017] 是一种基于采样的随机最优控制方法。其核心思想是：每帧在标称控制序列上叠加高斯噪声，生成 $N$ 条采样轨迹：

$$\mathbf{u}_{0:H-1}^{(k)} = \mathbf{u}_{0:H-1}^{\text{base}} + \boldsymbol{\epsilon}^{(k)}, \quad \boldsymbol{\epsilon}_t^{(k)} \sim \mathcal{N}(\mathbf{0}, \boldsymbol{\Sigma})$$

每条轨迹经动力学模型前向仿真得到状态序列 $\mathbf{x}_{0:H}^{(k)}$，计算代价 $C^{(k)} = J(\mathbf{x}_{0:H}^{(k)}, \mathbf{u}_{0:H-1}^{(k)})$ 后，通过重要性加权平均得到最优控制：

$$\mathbf{u}_t^* = \frac{\sum_{k=1}^{N} w^{(k)} \mathbf{u}_t^{(k)}}{\sum_{k=1}^{N} w^{(k)}}, \quad w^{(k)} = \exp\left(-\frac{C^{(k)} - \min_j C^{(j)}}{\lambda}\right)$$

其中 $\lambda$ 为温度参数，控制加权的选择性。下一帧的标称序列由 $\mathbf{u}^*$ 向前平移一步得到（warm-start 滚动窗口）。

## 3. 三组件代价设计

相较于标准 MPPI 实践中多权重、多子项的手动调参方式，本文将代价函数整合为三个语义独立的组件，各由单一标量权重控制：

$$J = w_{\text{obs}} \cdot J_{\text{obstacle}} + w_{\text{track}} \cdot J_{\text{tracking}} + w_{\text{prog}} \cdot J_{\text{progress}}$$

这一设计消除了相关项之间的权重竞争问题，且具备直观的调参语义：$w_{\text{obs}}$ 控制安全裕量，$w_{\text{track}}$ 控制路径跟踪精度，$w_{\text{prog}}$ 控制行进积极性。

### 3.1 障碍物代价

每时间步的障碍物代价通过对机器人矩形足迹（footprint）进行间距为 $\delta$ 的密集采样，在占据栅格地图上进行双线性插值，并施加四次幂映射：

$$J_{\text{obstacle}} = \frac{1}{|\mathcal{F}|} \sum_{(x_i, y_i) \in \mathcal{F}} \left(\frac{c(x_i, y_i)}{255}\right)^4$$

其中 $\mathcal{F}$ 为足迹采样点集合，$c(x_i, y_i) \in [0, 255]$ 为采样点处的代价地图值。四次幂映射在致命障碍物附近（$c \geq 253$）产生极强的惩罚梯度，而在自由空间中代价趋近于零，有效抑制轨迹擦边行为。对于后退运动（$v_x < 0$），机器人后方的采样点代价值被钳位至下限 $c_{\text{rear}}$，防止倒车碰撞。

### 3.2 跟踪代价

跟踪代价由两个互补的子项构成，分别在空间域和角度域度量路径跟踪偏差：

**路径对齐（PathAlign）**度量轨迹点到全局参考路径（$P$ 个顶点构成的折线）的最短欧氏距离：

$$J_{\text{align}} = \frac{1}{H} \sum_{t=1}^{H} \min_{i \in [0, P-2]} d(\mathbf{x}_t, \overline{\mathbf{p}_i \mathbf{p}_{i+1}})$$

**路径角度（PathAngle）**在朝向偏差超过死区 $\theta_{\text{thresh}} = 45^\circ$ 时施加惩罚：

$$J_{\text{angle}} = \frac{1}{H} \sum_{t=1}^{H} \begin{cases} (\theta_t - \theta_{\text{path}})^2 & \text{if } |\theta_t - \theta_{\text{path}}| > \theta_{\text{thresh}} \\ 0 & \text{otherwise} \end{cases}$$

接近终点时（距终点距离小于 $d_{\text{anneal}}$），采用余弦退火策略将参考方向从路径切线平滑过渡到目标姿态朝向，无需单独设计终端控制器即可实现对接行为：

$$\alpha = \frac{1}{2}\left(1 + \cos\frac{\pi \cdot d_{\text{to\_goal}}}{d_{\text{anneal}}}\right), \quad \theta_{\text{ref}} = \theta_{\text{path}} + \alpha \cdot (\theta_{\text{goal}} - \theta_{\text{path}})$$

### 3.3 进度代价

进度代价在鼓励高效行进的同时，显式抑制全向底盘的一种典型退化行为——**推头**：即当机器人朝向未对准路径方向时，优化器倾向于通过增大前向速度来拉高投影奖励，而非通过转向来对齐方向。

本文不采用速度投影到路径切线的常规做法，而是直接匹配速度矢量方向与"当前位置→前瞻点"方向的角度差：

$$J_{\text{progress}} = \sum_{t=1}^{H} \left(-\|\mathbf{v}_t\| \cos \Delta \phi_t + \|\mathbf{v}_t\| \cdot |\sin \Delta \phi_t|\right) + \|\mathbf{x}_H - \mathbf{x}_{\text{goal}}\|$$

其中 $\Delta \phi_t = \angle(\mathbf{v}_t) - \angle(\mathbf{x}_{\text{lookahead}} - \mathbf{x}_t)$ 为速度方向与前瞻点方向的夹角（均在机器人坐标系下表达）。$\cos\Delta\phi$ 项奖励与前瞻点对齐的速度分量，$|\sin\Delta\phi|$ 项惩罚侧向速度浪费。当 $\Delta\phi = 45^\circ$ 时两项抵消（净激励为零），超过 $45^\circ$ 时直行产生净正代价——迫使优化器要么转向对齐，要么减速，而非盲目录加速。对于背离前瞻点的运动（$\cos\Delta\phi < 0$，即速度方向偏离超过 $90^\circ$），施加 5 倍惩罚。

## 4. GPU 并行化

MPPI 的核心计算瓶颈在于每控制周期需评估 $N \times H$ 次状态推演与代价计算。典型参数 $N = 3000, H = 12$ 意味着每秒需完成 36000 次动力学积分与代价评估（10 Hz 控制频率）。本文利用 MPPI 采样轨迹间天然的独立性，将每条轨迹映射到一个 CUDA 线程：

- **采样核函数（Sample Kernel）**：一维 grid $\lceil N/256 \rceil \times 256$ 线程，线程 $k$ 串行展开轨迹 $k$ 的全部 $H$ 步，代价累加器驻留在寄存器中。所有指针标注 `__restrict__` 以启用编译器激进优化。
- **加权求和核函数（Weighted Sum Kernel）**：二维 grid 尺寸 $(\lceil N/256 \rceil, H)$，线程 $(k, t)$ 通过 `atomicAdd` 将样本 $k$ 在时步 $t$ 的控制量累加到加权结果中。

所有 GPU 缓冲区（总计约 1.2 MB）在初始化时一次性持久分配，消除每帧 `cudaMalloc`/`cudaFree` 的开销。单一 CUDA stream 实现主机→设备异步传输、核函数执行、设备→主机结果回读的流水线重叠。在 NVIDIA Jetson Orin 平台上，GPU 路径的单帧延迟约 1.7 ms（同等 CPU 实现约 15 ms）。

## 5. 采样策略

### 5.1 引导-探索分解

每条采样控制量分解为探索项（warm-start 基序列周围的高斯噪声）和引导项（锚定路径方向）：

$$\mathbf{u}_t = (1 - \gamma) \cdot (\mathbf{u}_t^{\text{base}} + \boldsymbol{\epsilon}_t \odot \mathbf{n}_t) + \gamma \cdot \mathbf{u}_t^{\text{guide}}$$

其中 $\gamma \in [0, 1]$ 为引导权重，$\mathbf{n}_t$ 为沿时域衰减的噪声缩放因子（前期大噪声充分探索，末端收缩以保证精度），$\mathbf{u}_t^{\text{guide}}$ 将速度偏置向机体坐标系下的路径方向。噪声缩放 $\mathbf{n}_t$ 额外受空间进度（已行驶距离 / 最大行驶距离）调制，防止机器人静止时噪声坍缩。

### 5.2 对数正态混合采样

为逃脱局部极小（如 U 形障碍物），占比 $r_{\text{nln}}$ 的噪声样本从对数正态分布 $\log\mathcal{N}(0, (\sigma \cdot s)^2)$ 抽取，而非高斯分布 $\mathcal{N}(0, \sigma^2)$。对数正态分布具有更重的右尾，偶尔产生大幅扰动，帮助优化器发现纯高斯探索无法触及的逃逸轨迹。对数正态样本的符号均匀随机。混合比例 $r_{\text{nln}}$ 和尺度倍数 $s$ 控制探索-利用的平衡。

### 5.3 纯旋转子策略

部分轨迹（$r_{\text{rot}} = 10\%$）在前 $T_{\text{rot}}$ 步执行纯旋转（$v_x = v_y = 0$），并放大角速度噪声。该子策略确保优化器在窄道入口处显式评估"先原地转向再前进"的策略——在此类场景中，未经朝向对齐就直接前进将导致碰撞。

### 5.4 转弯横向增强

当参考路径存在弯曲时（以前瞻索引间隔的路径段角度差量化），横向速度的引导分量和噪声被放大：

$$\beta_{\text{lat}} = \min\left(\beta_{\max},\; 1 + \kappa \cdot \beta_{\text{turn}} + \delta \cdot 0.4 \cdot \beta_{\text{turn}}\right)$$

其中 $\kappa \in [0, 1]$ 为归一化路径曲率，$\delta \in [0, 1]$ 为前瞻点相对机器人前进轴的归一化横向偏移。该机制使全向底盘能够利用侧向运动能力流畅通过急弯。

## 6. 面向退化场景的状态机

纯 MPPI 优化在三种场景下可能表现出退化行为：(i) 朝向严重偏离路径，速度奖励驱动推头直冲；(ii) 窄通道中所有采样轨迹均碰撞，输出趋零；(iii) 终端接近时参考路径退化为单一线段，朝向计算不稳定。本文引入一个五模式轻量状态机，在上述场景中覆盖或增强 MPPI 输出。

**状态转移逻辑**按优先级降序排列：TERMINAL_ALIGN > LATERAL_SHIFT > HEADING_MISALIGN > NARROW_PASSAGE > NORMAL。转移保护禁止特殊模式间的直接跳转——系统必须经过 NORMAL，形成迟滞以抑制振荡。

### 6.1 虚拟机器人窄道探测

仅从优化器输出速度的大小无法可靠区分"被障碍物卡住"与"正常减速接近目标"。本文部署两个**虚拟机器人**——放置在沿参考路径前方 0.20 m 与 0.40 m 处的轻量 MPPI 实例——来预判前方通行性：

1. **放置**：虚拟机器人沿路径从最近点起算，按弧长距离 $d_0 = 0.20$ m 和 $d_1 = 0.40$ m 定位。若剩余路径不足，则钳位至终点。
2. **朝向搜索**：对每个虚拟机器人，在 24 个候选方向（15° 间隔）上评估足迹碰撞。最优朝向最小化加权评分 $d_{\text{current}} + 1.5 \cdot d_{\text{path}}$，兼顾与当前机器人朝向的接近程度和与路径方向的对齐程度。
3. **冷启动 MPPI**：每个虚拟机器人运行独立的 MPPI，基序列全零（冷启动），不评估路径跟踪代价（无路径输入），仅保留障碍物代价和速度奖励项。输出取前 3 个时步的加权平均速度。
4. **进度投影**：将输出速度转换至世界坐标系，投影到"虚拟机器人→前瞻点"方向。若**两个**虚拟机器人的投影速度连续 3 帧低于 0.10 m/s，触发 NARROW_PASSAGE 状态。

虚拟机器人机制回答的问题是：*"如果我已处于前方某位置，且以最优朝向摆好，能否朝目标产生有效前进？"*否定答案表明前方通道结构性阻塞，而非仅仅当前接近角度不佳。

### 6.2 横向移动

当朝向偏差超过 $\theta_{\text{misalign}} = 60^\circ$ 且前瞻点距目标在 1.0 m 以内时，LATERAL_SHIFT 状态沿路径向前搜索首个无碰足迹放置位（最远 0.50 m，步长 0.10 m）。机器人以全向平移方式直接运动至该安全点，同时将偏航角对准目标朝向。到达目标点（0.05 m 以内）后进入 1 秒冷却期，防止立即重新触发。12 秒超时提供安全逃逸。

### 6.3 朝向修正

当朝向偏差超过阈值但前瞻点距目标至少 0.5 m（即机器人尚未接近终点）时，HEADING_MISALIGN 状态指令以最大角速度沿短弧方向纯旋转。这防止了速度奖励在机器人严重偏航时激励推头直冲。

### 6.4 终端对准

当剩余路径长度降至 $d_{\text{term}} = 0.08$ m 以下时，系统进入 TERMINAL_ALIGN。代价地图膨胀代价被清零（机器人已在终点区域），但致命障碍物检测保持激活。第一阶段以 P 控制器将机器人旋转至目标偏航角，对齐后（误差 $< 0.07$ rad $\approx 4^\circ$）第二阶段沿直线平移逼近目标位姿。当前位置到目标的直线路径以 2 cm 间距密集采样，进行完整足迹碰撞检测；若检测到致命障碍物，平移被禁止。

### 6.5 重规划鲁棒性

实际部署中的一个工程挑战是全局规划器可能以高频（最高 10 Hz）重规划，发布目标位姿轻微扰动的新路径。每次收到新路径后无条件重置状态将导致所有特殊模式无法执行完毕。本文引入**目标变化门控**：状态锁（LATERAL_SHIFT、TERMINAL_ALIGN）在重规划间持续保持，仅当目标位姿变化超过 0.10 m 时才清零。此举将状态机的稳定性与规划器更新频率解耦。

## 7. 系统实现

控制器作为 `nav2_core::Controller` 插件实现于 Nav2 导航框架。GPU 核函数以 CUDA C++ 编写，由 `nvcc` 编译。所有参数通过 ROS 2 参数声明机制暴露，可在启动时通过 YAML 配置文件调整。控制器以 10 Hz 运行，典型参数 $N = 3000, H = 12, dt = 0.1$ s，对应 1.2 s 预测时域和最高速度下 0.48 m 的有效到达距离。完整系统包括局部与全局代价地图的 TF2 融合、足迹感知碰撞检测，以及采样轨迹与诊断标记的 RViz 可视化，运行于单一 ROS 2 节点内。

## 8. 计算性能实验

为定量评估 GPU 加速效果，将 MPPI 代价计算核心从控制器中抽离为独立 benchmark，分别在 GPU 和 CPU 上运行，对比单帧计算延迟。实验平台为 Intel Core i7-13700H（14 核 20 线程）+ NVIDIA GeForce RTX 5060 Laptop GPU（CUDA 13.0, Blackwell 架构），64 位 Ubuntu Linux。

### 8.1 实验设计

**测试数据**：随机生成 $N \times H$ 组速度指令序列（$v_x, v_y$ 服从 $\mathcal{N}(0, 0.5^2)$ m/s，$\omega$ 服从 $\mathcal{N}(0, 0.15^2)$ rad/s），60 × 60 cell 代价地图（3 m × 3 m，5% 致命障碍 + 10% 膨胀区），30 点弯曲参考路径。控制参数与实车部署一致（$w_{\text{obs}}=10.0, w_{\text{track}}=30.0, w_{\text{prog}}=1.5$，足迹 0.34 m × 0.56 m）。

**计时方法**：GPU 端使用 CUDA Event 在 stream 上分段记录上传（Host→Device）、核函数执行、下载（Device→Host）耗时。CPU 端使用 `std::chrono::high_resolution_clock` 测量全时域，采用 OpenMP `#pragma omp parallel for` 充分利用多核并行。每组参数预热 5 次后取 20–50 次运行的中位数。

**一致性验证**：GPU 与 CPU 输出代价序列逐元素对比，最大绝对偏差 $< 2 \times 10^{-5}$，确认两种实现逻辑等价。

### 8.2 结果

| 样本数 $N$ | GPU 上传 | GPU 核函数 | GPU 下载 | **GPU 总计** | CPU (OpenMP) | 加速比 |
|:---------:|--------:|---------:|--------:|:----------:|:-----------:|:-----:|
| 1000 | 0.04 ms | 0.24 ms | 0.01 ms | **0.29 ms** | 15.1 ms | 51.5× |
| 3000 | 0.06 ms | 0.26 ms | 0.01 ms | **0.33 ms** | 27.4 ms | 83.0× |
| 5000 | 0.06 ms | 0.28 ms | 0.01 ms | **0.34 ms** | 35.4 ms | 103.3× |
| 10000 | 0.08 ms | 0.43 ms | 0.02 ms | **0.53 ms** | 63.1 ms | 118.7× |

GPU 总延迟在 $N = 3000$（实车部署参数）时为 0.33 ms，其中核函数占 79%（0.26 ms），上传 448 KB 数据占 18%（0.06 ms），下载 12 KB 结果占 3%（0.01 ms）。GPU 延迟随 $N$ 增长呈次线性（1000→10000 仅增加 0.24 ms），而 CPU 延迟线性增长，加速比从 51.5× 提升至 118.7×。在 10 Hz 控制频率下，GPU 计算仅占用约 0.3% 的控制周期预算（3.3 ms / 100 ms），远低于 CPU 的 27.4 ms（27.4%）。

## 参考文献

- G. Williams et al., "Model Predictive Path Integral Control: From Theory to Parallel Computation," *Journal of Guidance, Control, and Dynamics*, 2017.
- G. Williams et al., "Information-Theoretic Model Predictive Control: Theory and Applications to Autonomous Driving," *IEEE Transactions on Robotics*, 2018.
- S. Macenski et al., "The Marathon 2: A Navigation System," *IROS*, 2023.
