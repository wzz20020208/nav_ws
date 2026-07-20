# 周报 2026.07.08 – 2026.07.13

本周围绕 MPPI GPU 控制器的路径跟踪、全向运动和代价体系做了几项改进,并对"横向速度缺失"问题做了一轮系统排查。

## 1. PathAngle 朝向代价重构

**问题**

原 PathAngle 代价把两件事混在一起:远离终点时对齐路径切线、接近终点时通过余弦退火过渡到 goal_yaw,同时还做 180° 对称(θ 与 θ+π 等价)。逻辑分支多、退火过渡不好调,而且切线方向每帧可能抖动。

**方案**

改为直接使用 waypoint 的 SE2 推荐朝向(rec_yaw)作为唯一朝向目标,惩罚项简化为:

```cpp
float err = normalize_angle(theta - rec_yaw);
return 4.0f * err * err;
```

去掉 GoalAngle 退火和 180° 对称,旋转方向交给上层 heading 状态机决定,代价函数只负责"对齐推荐朝向"。rec_yaw 取自前瞻点(lh_idx),比 closest_idx+1 更稳定、不易抖。

**效果**

朝向代价逻辑单一、参数可控,不再有远近两套分支和退火过渡。

## 2. 新增独立的"偏离路径代价"(软墙)

**问题**

原有 PathAlign 只是"轨迹点到路径的最短距离平方",脱离路径后拉回力度随距离平方增长,但相对其他项偏弱,机器人会离路径较远。

**方案**

新增 compute_path_deviation_cost,构造一堵软墙:

```cpp
float excess = dist - corridor;      // 回廊外的超出量
if (excess <= 0) return 0;           // 回廊内不罚
return weight * excess * excess;     // 超出量的平方，陡增
```

- 在回廊半宽 path_corridor 内完全不惩罚,允许正常微调
- 超出后按超出量的平方快速增长,把机器人挡回路径
- 独立于 cost_scale,单独叠加,有专属旋钮 path_deviation_weight
- 复用 PathAlign 已算好的最近距离,无额外遍历开销

参数:path_deviation_weight = 150.0,path_corridor = 0.15 m。

**效果**

脱离路径的拉回明显变强;仍在配合避障做权重平衡(软墙不能盖过绕障需求)。

## 3. 代价权重体系修复与重新标定

**问题**

排查中发现 YAML 里旧的 obstacle_weight / tracking_weight / progress_weight(10/30/30)已被代码弃用、完全不生效。归一化重构后代码改读 cost_scale + obstacle_ratio / tracking_ratio / speed_ratio,而这些新参数 YAML 里没写,运行时一直用硬编码默认值。调参"改了没反应"的根因就在这里。

**方案**

- 把 YAML 对齐到现行归一化体系,用 cost_scale × ratio 复现期望的实效权重
- 重新标定 cost_scale、lambda,让 exp(-(cost-min)/λ) 的加权区分度恢复(代价绝对值缩小后 λ=1.0 会把 softmax 压平)

关键参数:cost_scale = 100,obstacle / tracking / speed_ratio = 0.1 / 0.3 / 0.8,lambda = 0.5。

## 4. 全向机器人"横向速度缺失"排查(进行中)

**现象**

全向机器人,期望"朝向归朝向、平移方向指向前瞻点"。但实测前瞻点在正侧方(he ≈ 84°)时,机器人靠原地转向去对齐,几乎不产生横向速度 vy,vx / omega 主导。

**排查手段**

加了 VYDBG 诊断,每 10 帧打印:最低代价样本的 (vx, vy)、全体样本 vy 的分布区间、前瞻点在机器人系下的方向、base 序列偏置。

**目前结论**

- vy 采样正常:样本 vy 铺满 [-0.2, 0.2]、|vy| 均值约 0.17,排除"没采样到 vy"的猜测
- 代价地形偏平:最优样本在前向 (0, 0) 与侧向 (-0.1, 0.2) 之间跳变,代价挤在 1.2–1.6,argmin 近乎随机,加权平均退化成样本均值,vx 保留 base 前向、vy 左右对称被抵消
- 下一步:定位是哪一代价分量压平了侧向区分度(疑似进度项里"到终点距离"数值大、近似常数,淹没了 speed_reward 的侧向偏好),已在准备代价分量拆解诊断

## 5. 其它

- velocity_controller 看门狗:原节点只在收到 /cmd_vel 时写速度,是保持型指令;上游断连时机器人会保持上一次速度。补上超时(0.3s)看门狗强制归零 + 退出时归零并切回站立模式。
- 可视化 frame 统一:MPPI 轨迹与推头诊断 marker 之前部分标 map、部分标 odom,坐标其实都是 odom 系,导致标 map 的 marker 被 map→odom 变换二次偏移。全部统一到 odom。
