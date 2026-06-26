import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
import xacro
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration 

def generate_launch_description():

    urdf_file_path = os.path.expanduser('~/THEMIS/THEMIS/Simulation/models/themis/urdf/TH02-A7.urdf')
    
    with open(urdf_file_path, 'r') as infp:
        robot_desc = infp.read()

    robot_description_config = robot_desc

    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
    	output='screen',
        parameters=[{
            'robot_description': robot_description_config,
            'publish_frequency': 50.0,
            'use_sim_time': True  
        }]
    )

    return LaunchDescription([
        rsp_node,
        # jsp_node
    ])
