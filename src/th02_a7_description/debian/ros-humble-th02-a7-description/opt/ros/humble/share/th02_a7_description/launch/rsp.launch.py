import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node
import xacro

def generate_launch_description():

    pkg_path = get_package_share_directory('th02_a7_description')

    xacro_file = os.path.join(pkg_path, 'urdf', 'th02_a7_with_zed.urdf.xacro')

    robot_description_config = xacro.process_file(xacro_file).toxml()

    rsp_node = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        name='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description_config,
            'publish_frequency': 50.0
        }]
    )

    return LaunchDescription([
        rsp_node,
        # jsp_node
    ])
