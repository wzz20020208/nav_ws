#!/usr/bin/env python3
"""
速度跟随测试工具：记录 /cmd_vel 指令与 /odom 实际速度的误差。

用法:
  # 记录 60 秒，保存到默认文件
  python3 velocity_tracker.py

  # 指定时长和输出文件
  python3 velocity_tracker.py --duration 120 --output /tmp/vel_test.csv

  # 不打印实时输出，只保存
  python3 velocity_tracker.py --quiet

输出 CSV 列:
  timestamp, cmd_vx, cmd_vy, cmd_w, actual_vx, actual_vy, actual_w,
  error_vx, error_vy, error_w, error_vx_pct, error_vy_abs, error_w_abs
"""

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist
from nav_msgs.msg import Odometry
import csv
import argparse
import signal
import sys
import time
from collections import deque
import numpy as np


class VelocityTracker(Node):
    def __init__(self, output_file: str, quiet: bool = False):
        super().__init__('velocity_tracker')

        self.output_file = output_file
        self.quiet = quiet

        # 最新数据
        self.latest_cmd = None       # (stamp, vx, vy, w)
        self.latest_odom = None      # (stamp, vx, vy, w)

        # 订阅
        self.cmd_sub = self.create_subscription(
            Twist, '/cmd_vel', self.cmd_callback, 10)
        self.odom_sub = self.create_subscription(
            Odometry, '/odom', self.odom_callback, 10)

        # 记录频率 (Hz)
        self.record_hz = 50.0
        self.record_timer = self.create_timer(
            1.0 / self.record_hz, self.record_callback)

        # 统计数据
        self.records = []
        self.error_vx_acc = deque(maxlen=500)
        self.error_vy_acc = deque(maxlen=500)
        self.error_w_acc = deque(maxlen=500)

        # CSV 文件
        self.csvfile = open(output_file, 'w', newline='')
        self.writer = csv.writer(self.csvfile)
        self.writer.writerow([
            'timestamp', 'cmd_vx', 'cmd_vy', 'cmd_w',
            'actual_vx', 'actual_vy', 'actual_w',
            'error_vx', 'error_vy', 'error_w',
            'error_vx_pct', 'error_vy_abs', 'error_w_abs'
        ])

        # 状态打印
        self.stats_timer = self.create_timer(1.0, self.print_stats)
        self.start_time = self.get_clock().now()

        self.get_logger().info(
            f'Velocity tracker started. Recording to {output_file}')
        if not quiet:
            self.get_logger().info(
                'Format: cmd(vx,vy,w) → actual(vx,vy,w) | errors')

    def cmd_callback(self, msg: Twist):
        stamp = self.get_clock().now().nanoseconds / 1e9
        self.latest_cmd = (stamp, msg.linear.x, msg.linear.y, msg.angular.z)

    def odom_callback(self, msg: Odometry):
        stamp = self.get_clock().now().nanoseconds / 1e9
        self.latest_odom = (
            stamp,
            msg.twist.twist.linear.x,
            msg.twist.twist.linear.y,
            msg.twist.twist.angular.z
        )

    def record_callback(self):
        if self.latest_cmd is None or self.latest_odom is None:
            return

        cmd_t, cmd_vx, cmd_vy, cmd_w = self.latest_cmd
        odom_t, odom_vx, odom_vy, odom_w = self.latest_odom

        # 时间戳取较晚者
        ts = max(cmd_t, odom_t)
        rel_ts = ts - self.start_time.nanoseconds / 1e9

        # 误差计算
        err_vx = odom_vx - cmd_vx
        err_vy = odom_vy - cmd_vy
        err_w = odom_w - cmd_w

        # 百分比误差 (仅当指令非零时有意义)
        err_vx_pct = (err_vx / cmd_vx * 100.0) if abs(cmd_vx) > 0.001 else 0.0

        # 写入文件
        self.writer.writerow([
            f'{rel_ts:.4f}',
            f'{cmd_vx:.6f}', f'{cmd_vy:.6f}', f'{cmd_w:.6f}',
            f'{odom_vx:.6f}', f'{odom_vy:.6f}', f'{odom_w:.6f}',
            f'{err_vx:.6f}', f'{err_vy:.6f}', f'{err_w:.6f}',
            f'{err_vx_pct:.2f}', f'{abs(err_vy):.6f}', f'{abs(err_w):.6f}'
        ])

        # 累积统计
        self.records.append((odom_vx, odom_vy, odom_w, err_vx, err_vy, err_w))
        self.error_vx_acc.append(abs(err_vx))
        self.error_vy_acc.append(abs(err_vy))
        self.error_w_acc.append(abs(err_w))

    def print_stats(self):
        if not self.error_vx_acc:
            return
        if self.quiet:
            return

        elapsed = (self.get_clock().now() - self.start_time).nanoseconds / 1e9

        # 最近一段时间内的平均绝对误差
        mae_vx = np.mean(self.error_vx_acc)
        mae_vy = np.mean(self.error_vy_acc)
        mae_w = np.mean(self.error_w_acc)

        # 当前最新值
        if self.latest_cmd and self.latest_odom:
            _, cvx, cvy, cw = self.latest_cmd
            _, avx, avy, aw = self.latest_odom
        else:
            cvx = cvy = cw = 0.0
            avx = avy = aw = 0.0

        print(
            f'\r[{elapsed:6.0f}s] '
            f'cmd(vx={cvx:+6.3f}, vy={cvy:+6.3f}, w={cw:+6.3f}) → '
            f'act(vx={avx:+6.3f}, vy={avy:+6.3f}, w={aw:+6.3f}) | '
            f'MAE(vx={mae_vx:.4f}, vy={mae_vy:.4f}, w={mae_w:.4f})',
            end='', flush=True
        )

    def shutdown(self):
        """保存汇总统计并关闭文件"""
        self.csvfile.close()
        self.get_logger().info(f'\nData saved to {self.output_file}')

        if len(self.records) < 2:
            return

        arr = np.array(self.records)  # [odom_vx, odom_vy, odom_w, err_vx, err_vy, err_w]
        err_vx = arr[:, 3]
        err_vy = arr[:, 4]
        err_w = arr[:, 5]

        n = len(self.records)
        total_t = n / self.record_hz
        self.get_logger().info(f'{"="*60}')
        self.get_logger().info(f'  速度跟随误差汇总 (基于 {n} 条记录, ≈{total_t:.1f}s)')
        self.get_logger().info(f'{"="*60}')
        self.get_logger().info(f'  {"指标":<20} {"vx (m/s)":>12} {"vy (m/s)":>12} {"w (rad/s)":>12}')
        self.get_logger().info(f'  {"-"*56}')
        self.get_logger().info(f'  {"平均误差 (ME)":<20} {np.mean(err_vx):>12.4f} {np.mean(err_vy):>12.4f} {np.mean(err_w):>12.4f}')
        self.get_logger().info(f'  {"平均绝对误差 (MAE)":<20} {np.mean(np.abs(err_vx)):>12.4f} {np.mean(np.abs(err_vy)):>12.4f} {np.mean(np.abs(err_w)):>12.4f}')
        self.get_logger().info(f'  {"均方根误差 (RMSE)":<20} {np.sqrt(np.mean(err_vx**2)):>12.4f} {np.sqrt(np.mean(err_vy**2)):>12.4f} {np.sqrt(np.mean(err_w**2)):>12.4f}')
        self.get_logger().info(f'  {"最大误差":<20} {np.max(np.abs(err_vx)):>12.4f} {np.max(np.abs(err_vy)):>12.4f} {np.max(np.abs(err_w)):>12.4f}')
        self.get_logger().info(f'  {"误差标准差":<20} {np.std(err_vx):>12.4f} {np.std(err_vy):>12.4f} {np.std(err_w):>12.4f}')
        self.get_logger().info(f'{"="*60}')

        # 附加：指令范围统计
        cmd_mag = np.sqrt(arr[:, 0]**2 + arr[:, 1]**2)  # odom velocity magnitude
        self.get_logger().info(f'  实际速度范围: {np.min(cmd_mag):.4f} ~ {np.max(cmd_mag):.4f} m/s (均值 {np.mean(cmd_mag):.4f})')


def main():
    parser = argparse.ArgumentParser(description='速度跟随测试工具')
    parser.add_argument('--duration', '-d', type=float, default=60.0,
                        help='记录时长 (秒), 默认 60')
    parser.add_argument('--output', '-o', type=str,
                        default=f'/tmp/velocity_track_{time.strftime("%Y%m%d_%H%M%S")}.csv',
                        help='输出 CSV 文件路径')
    parser.add_argument('--quiet', '-q', action='store_true',
                        help='不打印实时输出')
    args = parser.parse_args()

    rclpy.init()

    tracker = VelocityTracker(output_file=args.output, quiet=args.quiet)

    # 定时关闭
    shutdown_requested = False

    def shutdown_timer():
        nonlocal shutdown_requested
        if not shutdown_requested:
            tracker.get_logger().info(
                f'\nDuration {args.duration}s reached, shutting down...')
            shutdown_requested = True
            rclpy.shutdown()

    # 手动 Ctrl+C 处理
    def sigint_handler(sig, frame):
        nonlocal shutdown_requested
        if not shutdown_requested:
            print('\nInterrupted, shutting down...')
            shutdown_requested = True
            rclpy.shutdown()

    signal.signal(signal.SIGINT, sigint_handler)

    # 使用 ROS timer 实现定时关闭
    timer = tracker.create_timer(args.duration, shutdown_timer)

    try:
        rclpy.spin(tracker)
    except (KeyboardInterrupt, SystemExit):
        pass
    except Exception as e:
        tracker.get_logger().error(f'Unexpected error: {e}')
    finally:
        tracker.shutdown()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
