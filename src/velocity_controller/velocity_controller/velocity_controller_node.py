import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist


class VelocityControllerNode(Node):
    def __init__(self):
        super().__init__('velocity_controller')

        # ── 参数 ──
        # cmd_vel 超时阈值 [s]，超过该时间未收到指令则强制归零 (默认 3 个控制周期)
        self.declare_parameter('cmd_vel_timeout', 0.3)
        self.cmd_vel_timeout = self.get_parameter('cmd_vel_timeout').value

        self.subscription = self.create_subscription(
            Twist,
            '/cmd_vel',
            self.cmd_vel_callback,
            10
        )

        from Play.Others import locomotion as lm_api
        self.lm_api = lm_api
        lm_api.set_locomotion_mode(1)  # set walking mode

        # ── 看门狗 ──
        # 记录最近一次收到 cmd_vel 的时间戳；is_stopped 用于避免重复发送 0 速度
        self.last_cmd_time = self.get_clock().now()
        self.is_stopped = True  # 启动时未收到任何指令，视为已停止
        # 以超时阈值的一半为周期检查，保证及时触发
        self.watchdog_timer = self.create_timer(
            self.cmd_vel_timeout / 2.0,
            self.watchdog_callback
        )

        self.get_logger().info(
            f'Velocity controller started, listening to /cmd_vel '
            f'(watchdog timeout={self.cmd_vel_timeout}s)'
        )

    def cmd_vel_callback(self, msg):
        v = (msg.linear.x, msg.linear.y)
        w = msg.angular.z
        self.lm_api.set_walking_velocity(v, w)
        self.last_cmd_time = self.get_clock().now()
        self.is_stopped = False
        self.get_logger().debug(f'Received: v={v}, w={w}')

    def watchdog_callback(self):
        """cmd_vel 超时保护: 超过 timeout 未收到指令则强制发送 0 速度，防止机器人保持上一次速度暴走。"""
        if self.is_stopped:
            return
        elapsed = (self.get_clock().now() - self.last_cmd_time).nanoseconds / 1e9
        if elapsed >= self.cmd_vel_timeout:
            self.lm_api.set_walking_velocity((0.0, 0.0), 0.0)
            self.is_stopped = True
            self.get_logger().warn(
                f'cmd_vel timeout ({elapsed:.2f}s >= {self.cmd_vel_timeout}s), '
                f'stopping robot (zero velocity)'
            )

    def stop_robot(self):
        """归零并切回站立模式，用于节点退出时的安全处理。"""
        try:
            self.lm_api.set_walking_velocity((0.0, 0.0), 0.0)
            self.lm_api.set_locomotion_mode(0)  # standing mode
            self.get_logger().info('Velocity controller stopping: zeroed velocity, standing mode')
        except Exception as e:  # noqa: BLE001 - 退出路径尽力而为，不阻断关闭
            self.get_logger().error(f'Failed to stop robot on shutdown: {e}')


def main(args=None):
    rclpy.init(args=args)
    controller = VelocityControllerNode()
    try:
        rclpy.spin(controller)
    except KeyboardInterrupt:
        pass
    finally:
        controller.stop_robot()
        controller.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
