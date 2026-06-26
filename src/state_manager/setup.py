from setuptools import setup

setup(
    name='state_manager',
    version='0.0.1',
    packages=['state_manager'],
    data_files=[
        ('share/ament_index/resource_index/packages', ['resource/' + 'state_manager']),
        ('share/' + 'state_manager', ['package.xml']),
    ],
    install_requires=['rclpy'],
    zip_safe=True,
    maintainer='Themis',
    maintainer_email='themis@westwoodrobotics.io',
    description='State manager for robot locomotion control',
    license='Apache-2.0',
    entry_points={
        'console_scripts': [
            'state_manager = state_manager.state_manager_node:main',
        ],
    },
)