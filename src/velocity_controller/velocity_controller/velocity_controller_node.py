import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist


class VelocityControllerNode(Node):
    def __init__(self):
        super().__init__('velocity_controller')
        self.subscription = self.create_subscription(
            Twist,
            '/cmd_vel',
            self.cmd_vel_callback,
            10
        )
        from Play.Others import locomotion as lm_api
        self.lm_api = lm_api
        lm_api.set_locomotion_mode(1)  # set walking mode
        self.get_logger().info('Velocity controller started, listening to /cmd_vel')

    def cmd_vel_callback(self, msg):
        v = (msg.linear.x, msg.linear.y)
        w = msg.angular.z
        self.lm_api.set_walking_velocity(v, w)
        self.get_logger().debug(f'Received: v={v}, w={w}')


def main(args=None):
    rclpy.init(args=args)
    controller = VelocityControllerNode()
    rclpy.spin(controller)
    controller.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()