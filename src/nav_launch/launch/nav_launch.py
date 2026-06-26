#!/usr/bin/env python3
import os
from launch import LaunchDescription
from launch.actions import ExecuteProcess, TimerAction, RegisterEventHandler
from launch.event_handlers import OnProcessStart

def generate_launch_description():
    home_dir = os.path.expanduser('~/THEMIS/THEMIS')
    install_dir = os.path.join(home_dir, 'nav_ws/install')
    src_dir = os.path.join(home_dir, 'nav_ws/src')

    # 基础节点（不需要等待的）
    memory_manager = ExecuteProcess(
        cmd=['bash', '-c', 'cd ' + home_dir + ' && python3 -m Play.Navigation.memory_manager'],
        name='memory_manager'
    )

    memory_pub = ExecuteProcess(
        cmd=['bash', '-c', 'cd ' + home_dir + ' && python3 -m Play.Navigation.memory_pub'],
        name='memory_pub'
    )

    velocity_controller = ExecuteProcess(
        cmd=['bash', '-c', 'PYTHONPATH=' + home_dir + ':$PYTHONPATH ' + install_dir + '/velocity_controller/bin/velocity_controller'],
        name='velocity_controller'
    )

    # TF 相关节点（Rviz2 需要这些）
    joint_tf_publisher = ExecuteProcess(
        cmd=[install_dir + '/joint_tf_publisher/lib/joint_tf_publisher/joint_tf_publisher_node',
             '--ros-args', '-p', 'use_sim_time:=true'],
        name='joint_tf_publisher'
    )

    base_state_to_odom = ExecuteProcess(
        cmd=[install_dir + '/base_state_to_odom/lib/base_state_to_odom/base_state_to_odom_node',
             '--ros-args', '-p', 'use_sim_time:=true'],
        name='base_state_to_odom'
    )

    tf_launch = ExecuteProcess(
        cmd=['bash', '-c', '. /opt/ros/humble/setup.bash && . ' + install_dir + '/setup.bash && ros2 launch ' + src_dir + '/depth_to_pcl/launch/tf_launch.py'],
        name='tf_launch'
    )

    depth_to_pcl = ExecuteProcess(
        cmd=['bash', '-c', '. /opt/ros/humble/setup.bash && . ' + install_dir + '/setup.bash && ros2 launch ' + src_dir + '/depth_to_pcl/launch/depth_to_pcl_launch.py'],
        name='depth_to_pcl'
    )

    # Rviz2 - 延迟启动，等 TF 节点准备好
    rviz2 = TimerAction(
        period=5.0,  # 延迟 5 秒启动
        actions=[
            ExecuteProcess(
                cmd=['bash', '-c',
                     '. /opt/ros/humble/setup.bash && '
                     'export LIBGL_ALWAYS_SOFTWARE=1 && '  # 防止 OpenGL 问题
                     'ros2 run rviz2 rviz2 -d ' +
                     install_dir + '/nav_launch/share/nav_launch/rviz/simulation.rviz'],
                name='rviz2'
            )
        ]
    )

    return LaunchDescription([
        memory_manager,
        memory_pub,
        velocity_controller,
        joint_tf_publisher,
        base_state_to_odom,
        tf_launch,
        depth_to_pcl,
        rviz2,  # 最后启动
    ])
