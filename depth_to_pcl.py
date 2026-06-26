from launch import LaunchDescription
from launch_ros.actions import ComposableNodeContainer
from launch_ros.descriptions import ComposableNode

def generate_launch_description():
    
    # 创建高效的 C++ 组件容器（内部节点数据零拷贝）
    container = ComposableNodeContainer(
        name='depth_image_proc_container',
        namespace='',
        package='rclcpp_components',
        executable='component_container',
        composable_node_descriptions=[
            
            # 1. 【降维打击】对 Gazebo 发出的深度图进行 2D 抽头降采样
            ComposableNode(
                package='image_proc',
                plugin='image_proc::CropDecimateNode',
                name='depth_downsampler',
                remappings=[
                    # 接收 Gazebo 的原始高分辨率深度图
                    ('in/image_raw', '/chest_camera/chest_camera/depth/image_raw'),
                    ('in/camera_info', '/chest_camera/chest_camera/depth/camera_info'),
                    
                    # 输出降采样后的低分辨率深度图
                    ('out/image_raw', '/chest_camera/depth/image_downsampled'),
                    ('out/camera_info', '/chest_camera/depth/camera_info_downsampled')
                ],
                parameters=[{
                    'decimation_x': 40,  # X方向抽头：每 4 个像素跳跃采样 1 个
                    'decimation_y': 40,  # Y方向抽头：每 4 个像素跳跃采样 1 个
                }]
            ),
            
            # 2. 将轻量级的深度图投影为稀疏点云
            ComposableNode(
                package='depth_image_proc',
                plugin='depth_image_proc::PointCloudXyzNode',
                name='point_cloud_xyz_node',
                remappings=[
                    # 订阅上方节点输出的低分辨率数据
                    ('image_rect', '/chest_camera/depth/image_downsampled'),
                    ('camera_info', '/chest_camera/depth/camera_info_downsampled'),
                    
                    # 对外发布最终的稀疏 3D 点云
                    ('points', '/chest_camera/points_sparse') 
                ]
            )
        ],
        output='screen',
    )

    return LaunchDescription([container])
