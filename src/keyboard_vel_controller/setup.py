import os
from setuptools import setup

package_name = 'keyboard_vel_controller'

setup(
    name=package_name,
    version='0.0.1',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
        # Install wrapper script to lib/<pkg>/ for ros2 run compatibility
        (os.path.join('lib', package_name), ['scripts/keyboard_vel_controller']),
    ],
    install_requires=['rclpy', 'geometry_msgs'],
    zip_safe=True,
    maintainer='Themis',
    maintainer_email='themis@westwoodrobotics.io',
    description='Keyboard speed controller with high/low speed modes, publishes to /cmd_vel',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'keyboard_vel_controller = keyboard_vel_controller.keyboard_vel_controller_node:main',
        ],
    },
)
