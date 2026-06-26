from setuptools import setup

setup(
    name='velocity_controller',
    version='0.0.1',
    packages=['velocity_controller'],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + 'velocity_controller']),
        ('share/' + 'velocity_controller', ['package.xml']),
    ],
    install_requires=['rclpy', 'geometry_msgs'],
    zip_safe=True,
    maintainer='Themis',
    maintainer_email='themis@westwoodrobotics.io',
    description='Velocity controller for robot locomotion',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'velocity_controller = velocity_controller.velocity_controller_node:main',
        ],
    },
)