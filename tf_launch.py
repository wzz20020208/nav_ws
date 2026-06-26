import os
from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    # ⚠️ 把这里改成你那个 1700 行 URDF 文件的绝对路径
    urdf_file_path = '/home/themis/THEMIS/THEMIS/Simulation/models/themis/urdf/TH02-A7.urdf'
    
    with open(urdf_file_path, 'r') as infp:
        robot_desc = infp.read()

    return LaunchDescription([
        # 节点 1：发布所有 revolute (旋转) 关节的默认角度 (0度)
        # 如果没有这个节点，ROS 不知道关节转了多少度，会拒绝发布 TF！
        Node(
            package='joint_state_publisher',
            executable='joint_state_publisher',
            name='joint_state_publisher',
            parameters=[{'robot_description': robot_desc}],
        ),
        
        # 节点 2：读取 URDF 和关节角度，暴风暴雨般地计算并发布整个 TF 树！
        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            parameters=[{'robot_description': robot_desc}],
        )
    ])
