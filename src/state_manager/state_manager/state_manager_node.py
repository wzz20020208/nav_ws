import sys
import termios
import tty

from rclpy.node import Node


class StateManager(Node):
    def __init__(self):
        super().__init__('state_manager')
        self.get_logger().info('State manager started - WASD to move, Q/E mode, ESC to quit')

        self.keys = {}
        self.mode = 1

    def set_mode(self, mode):
        self.mode = max(0, mode)

    def process_key(self, key):
        if key == '\x1b':  # ESC
            return False
        elif key in 'wasdWASD':
            self.keys[key.lower()] = True
        elif key == 'q':
            self.set_mode(self.mode - 1)
        elif key == 'e':
            self.set_mode(self.mode + 1)
        elif key == 's':
            self.keys.clear()
        return True

    def get_velocity(self):
        lin_x = 0.1 if self.keys.get('w') else (-0.1 if self.keys.get('s') else 0.0)
        lin_y = 0.1 if self.keys.get('a') else (-0.1 if self.keys.get('d') else 0.0)
        return (lin_x, lin_y, 0.0)


def main():
    from Play.Others import locomotion as lm_api
    from Play.Others import wbc as wbc_api

    rclpy.init(args=None)
    node = StateManager()

    old_settings = termios.tcgetattr(sys.stdin)
    tty.setcbreak(sys.stdin.fileno())

    lm_api.set_locomotion_mode(node.mode)

    try:
        while rclpy.ok():
            if sys.stdin.readable():
                try:
                    key = sys.stdin.read(1)
                except:
                    key = ''

                if not node.process_key(key):
                    break

            v = node.get_velocity()[:2]
            w = 0.0
            lm_api.set_walking_velocity(v, w)

            q, dq, u = wbc_api.get_joint_states(0)
            status = f'\rM={node.mode} V=({v[0]:.2f},{v[1]:.2f},0.00) Q={q} DQ={dq} U={u}   '
            print(status, end='', flush=True)

    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
        node.destroy_node()
        rclpy.shutdown()
        print('\nShutting down...')


if __name__ == '__main__':
    main()