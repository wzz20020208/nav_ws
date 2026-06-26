from launch import LaunchDescription
from launch.actions import ExecuteProcess


def generate_launch_description():
    # Static transform: BASE_LINK -> chest_camera_link
    return LaunchDescription([
        ExecuteProcess(
            cmd=['bash', '-c', '. /opt/ros/humble/setup.bash && ros2 run tf2_ros static_transform_publisher 0 0 0 0 0 0 BASE_LINK chest_camera_link'],
            name='camera_tf_publisher'
        ),
    ])