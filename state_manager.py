import sys
import termios
import tty
from Play.Others import locomotion as lm_api
from Play.Others import wbc as wbc_api

def main():
    old_settings = termios.tcgetattr(sys.stdin)
    tty.setcbreak(sys.stdin.fileno())

    mode = 1
    lm_api.set_locomotion_mode(mode)
    print('State manager started - WASD to move, Q/E mode, ESC to quit\n')

    try:
        keys = {}
        while True:
            if sys.stdin.readable():
                try:
                    key = sys.stdin.read(1)
                except:
                    key = ''

                if key == '\x1b':  # ESC
                    break
                elif key in 'wasdWASD':
                    keys[key.lower()] = True
                elif key == 'q':
                    mode = max(0, mode - 1)
                    lm_api.set_locomotion_mode(mode)
                elif key == 'e':
                    mode += 1
                    lm_api.set_locomotion_mode(mode)
                elif key == 's':
                    keys.clear()

            lin_x = 0.1 if keys.get('w') else (-0.1 if keys.get('s') else 0.0)
            lin_y = 0.1 if keys.get('a') else (-0.1 if keys.get('d') else 0.0)
            ang_z = 0.0

            v = (lin_x, lin_y)
            w = ang_z
            lm_api.set_walking_velocity(v, w)

            q, dq, u = wbc_api.get_joint_states(0)
            status = f'\rM={mode} V=({lin_x:.2f},{lin_y:.2f},{ang_z:.2f}) Q={q} DQ={dq} U={u}   '
            print(status, end='', flush=True)

    finally:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old_settings)
        print('\nShutting down...')

if __name__ == '__main__':
    main()