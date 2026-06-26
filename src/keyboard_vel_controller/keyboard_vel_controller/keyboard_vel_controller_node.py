#!/usr/bin/env python3
"""
Keyboard Velocity Controller — stdin toggle mode, publishes to /cmd_vel.

  w / s    : toggle forward / backward    (linear.x)
  a / d    : toggle strafe left / right   (linear.y)
  q / e    : toggle rotate left / right   (angular.z)
  Space    : stop all
  1 / 2    : LOW / HIGH speed
  z / x    : locomotion mode -1 / +1
  Esc      : quit

Speed: forward 0.3(LOW)/1.0(HIGH) m/s | backward & lateral max 0.5 m/s
"""
import math
import select
import sys
import termios
import threading
import tty

import rclpy
from rclpy.node import Node
from geometry_msgs.msg import Twist

DEG = math.pi / 180.0
BACK_MAX  = 0.25
LAT_MAX   = 0.5

MODES = {
    'LOW':  {'v': 0.3,  'w': 10.0 * DEG},
    'HIGH': {'v': 1.0,  'w': 20.0 * DEG},
}

RATE = 20.0


class KeyboardVelControllerNode(Node):
    def __init__(self):
        super().__init__('keyboard_vel_controller')
        self.pub = self.create_publisher(Twist, '/cmd_vel', 10)
        self._mode = 'LOW'
        self._locomotion_mode = 1
        self._active_vx = None
        self._active_vy = None
        self._active_wz = None
        self._vx = 0.0
        self._vy = 0.0
        self._wz = 0.0
        self._alive = True

        import os as _os
        themis_root = _os.path.expanduser('~/THEMIS/THEMIS')
        if themis_root not in sys.path:
            sys.path.insert(0, themis_root)
        from Play.Others import locomotion as lm_api
        self._lm_api = lm_api
        lm_api.set_locomotion_mode(self._locomotion_mode)

        self.create_timer(1.0 / RATE, self._publish)
        threading.Thread(target=self._input_loop, daemon=True).start()

        m = MODES[self._mode]
        self.get_logger().info(
            f'Ready [{self._mode}]: fwd={m["v"]:.1f}  back={BACK_MAX:.2f}  lat={LAT_MAX:.1f}  '
            f'ang={m["w"]/DEG:.0f} deg/s  loco={self._locomotion_mode}\n'
            f'  w/s=toggle fwd/back  a/d=toggle strafe  q/e=toggle rotate  Space=stop')

    @property
    def _v(self):
        return MODES[self._mode]['v']

    @property
    def _w(self):
        return MODES[self._mode]['w']

    def _update_vel(self):
        self._vx = self._v if self._active_vx == 'w' else \
                   -BACK_MAX if self._active_vx == 's' else 0.0
        self._vy = LAT_MAX if self._active_vy == 'a' else \
                   -LAT_MAX if self._active_vy == 'd' else 0.0
        self._wz = self._w if self._active_wz == 'q' else \
                   -self._w if self._active_wz == 'e' else 0.0

    def _stop_all(self):
        self._active_vx = self._active_vy = self._active_wz = None
        self._update_vel()

    def _input_loop(self):
        old = termios.tcgetattr(sys.stdin)
        try:
            tty.setcbreak(sys.stdin.fileno())
            while self._alive:
                if select.select([sys.stdin], [], [], 0.1)[0]:
                    k = sys.stdin.read(1)
                    if k == '\x1b':
                        self._stop_all()
                        self._publish()
                        self._alive = False
                        rclpy.shutdown()
                    elif k == ' ':
                        self._stop_all()
                    elif k == '1':
                        if self._mode != 'LOW':
                            self._mode = 'LOW'
                            self._update_vel()
                            self.get_logger().info(f'→ LOW (fwd={self._v:.1f} ang={self._w/DEG:.0f})')
                    elif k == '2':
                        if self._mode != 'HIGH':
                            self._mode = 'HIGH'
                            self._update_vel()
                            self.get_logger().info(f'→ HIGH (fwd={self._v:.1f} ang={self._w/DEG:.0f})')
                    elif k == 'z':
                        self._locomotion_mode = max(0, self._locomotion_mode - 1)
                        self._lm_api.set_locomotion_mode(self._locomotion_mode)
                        self.get_logger().info(f'→ loco={self._locomotion_mode}')
                    elif k == 'x':
                        self._locomotion_mode += 1
                        self._lm_api.set_locomotion_mode(self._locomotion_mode)
                        self.get_logger().info(f'→ loco={self._locomotion_mode}')
                    elif k in ('w', 's'):
                        self._active_vx = None if self._active_vx == k else k
                        self._update_vel()
                    elif k in ('a', 'd'):
                        self._active_vy = None if self._active_vy == k else k
                        self._update_vel()
                    elif k in ('q', 'e'):
                        self._active_wz = None if self._active_wz == k else k
                        self._update_vel()
        finally:
            termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old)

    def _publish(self):
        m = Twist()
        m.linear.x = self._vx
        m.linear.y = self._vy
        m.angular.z = self._wz
        self.pub.publish(m)


def main():
    import os as _os
    themis_root = _os.path.expanduser('~/THEMIS/THEMIS')
    if themis_root not in sys.path:
        sys.path.insert(0, themis_root)
    rclpy.init()
    node = KeyboardVelControllerNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        rclpy.shutdown()
