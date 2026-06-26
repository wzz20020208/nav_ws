#!/usr/bin/env python3
"""
THEMIS RTAB-Map 定位模式启动文件 — 纯视觉定位 (RGB-D)
基于 rtabmap_launch 官方 launch，参数由 YAML 自动注入。

等效命令:
  ros2 launch rtabmap_launch rtabmap.launch.py \
      localization:=true \
      depth:=true \
      subscribe_depth:=true \
      rgb_topic:=/chest_camera/chest_camera/image_raw \
      depth_topic:=/chest_camera/chest_camera/depth/image_raw \
      rgb_camera_info_topic:=/chest_camera/chest_camera/camera_info \
      imu_topic:=/base_imu \
      odom_topic:=/odom \
      visual_odometry:=false \
      approx_sync:=true \
      frame_id:=BASE_LINK \
      odom_frame_id:=odom \
      map_frame_id:=map \
      database_path:=~/.ros/themis_map.db \
      rtabmap_args:="<yaml_params>"

用法:
  ros2 launch nav_launch themis_rtabmap_localization.launch.py                    # 定位
  ros2 launch nav_launch themis_rtabmap_localization.launch.py \
      database_path:=/path/to/map.db                                               # 指定数据库
"""

import os
import yaml

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription, LogInfo
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration, PathJoinSubstitution
from ament_index_python.packages import get_package_share_directory


def _load_rtabmap_params(yaml_path: str) -> dict:
    """从 YAML 提取 rtabmap 节点 ros__parameters 字典。"""
    with open(yaml_path, 'r') as f:
        data = yaml.safe_load(f)
    for scope in data.values():
        if isinstance(scope, dict) and 'rtabmap' in scope:
            return scope['rtabmap'].get('ros__parameters', {})
    return {}


def _load_launch_config(yaml_path: str) -> dict:
    """从 YAML 提取 launch 配置 (话题、坐标系等)。"""
    with open(yaml_path, 'r') as f:
        data = yaml.safe_load(f)
    for scope in data.values():
        if isinstance(scope, dict) and 'launch' in scope:
            return scope['launch']
    return {}


def _params_to_cli(params: dict) -> str:
    """将参数字典转为 RTAB-Map CLI 格式: --Key Value

    RTAB-Map 解析器只认 '--' 前缀判断 key, '-1.3' 这种负数值
    不会被误判为 flag (因为只检查 '--' 不检查 '-'), 空格分隔即可.
    """
    parts = []
    for k, v in params.items():
        # 跳过 launch 层面的参数 (由 launch_arguments 单独控制)
        if k in ('subscribe_rgbd', 'subscribe_depth', 'subscribe_scan',
                 'subscribe_scan_cloud', 'subscribe_odom'):
            continue
        parts.append(f'--{k}')
        parts.append(str(v))
    return ' '.join(parts)


def generate_launch_description():
    pkg_dir = get_package_share_directory('nav_launch')
    yaml_path = os.path.join(pkg_dir, 'config', 'themis_rtabmap_localization.yaml')

    params = _load_rtabmap_params(yaml_path)
    yaml_args = _params_to_cli(params)
    launch_cfg = _load_launch_config(yaml_path)

    rtabmap_launch = PathJoinSubstitution([
        get_package_share_directory('rtabmap_launch'),
        'launch', 'rtabmap.launch.py'
    ])

    return LaunchDescription([
        # ── 数据库路径 ──
        DeclareLaunchArgument(
            'database_path', default_value='~/.ros/themis_map.db',
            description='已有地图数据库路径 (默认与建图共用)'),

        # ── 额外 CLI 参数 (定位模式默认不删库) ──
        DeclareLaunchArgument(
            'extra_args', default_value='',
            description='额外 RTAB-Map CLI 参数 (默认空字符串保留数据库)'),

        # ── 初始位姿 (格式: "x y z roll pitch yaw" 或 "x y z qx qy qz qw") ──
        DeclareLaunchArgument(
            'initial_pose', default_value='',
            description='手动设置初始位姿'),

        # ── 仿真时间 (仿真环境需要 true, 实车需要 false) ──
        DeclareLaunchArgument(
            'use_sim_time', default_value='true',
            description='使用仿真时间 (Gazebo/rosbag: true, 实车: false)'),

        # ── 定位模式 (默认 true, 设为 false 可建图测试) ──
        DeclareLaunchArgument(
            'localization', default_value='true',
            description='定位模式: true=不创建新节点仅定位, false=增量建图'),

        LogInfo(msg=['[rtabmap] localization config: ', yaml_path]),
        LogInfo(msg=['[rtabmap] Mode: Visual Localization (RGB-D)']),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(rtabmap_launch),
            launch_arguments={
                # ── 输入源: RGB-D (内部同步, 无需外部 rgbd_sync) ──
                'depth':                'true',
                'subscribe_rgbd':       'false',
                'subscribe_depth':      'true',
                'subscribe_scan':       'false',
                'subscribe_scan_cloud': 'false',

                # ── 原始话题 (rtabmap_launch 内部 rgbd_sync 订阅) ──
                'rgb_topic':              launch_cfg.get('rgb_topic',
                                            '/chest_camera/chest_camera/image_raw'),
                'camera_info_topic':      launch_cfg.get('camera_info_topic',
                                            '/chest_camera/chest_camera/camera_info'),
                'depth_topic':            launch_cfg.get('depth_topic',
                                            '/chest_camera/chest_camera/depth/image_raw'),

                # ── IMU ──
                'imu_topic': launch_cfg.get('imu_topic', '/base_imu'),

                # ── 里程计: 外部 odom, 不用视觉里程计 ──
                'visual_odometry': 'false',
                'odom_topic':      launch_cfg.get('odom_topic', '/odom'),

                # ── 坐标系 ──
                'frame_id':      launch_cfg.get('frame_id', 'BASE_LINK'),
                'odom_frame_id': launch_cfg.get('odom_frame_id', 'odom'),
                'map_frame_id':  launch_cfg.get('map_frame_id', 'map'),
                'publish_tf':    launch_cfg.get('publish_tf', 'true'),

                # ── 同步 (增大队列, 容忍 RGB/Depth 频率不一致) ──
                'approx_sync':      'true',
                'sync_queue_size':  '30',
                'topic_queue_size': '30',

                # ── 地图数据库 ──
                'database_path': LaunchConfiguration('database_path'),

                # ── RTAB-Map 参数 (YAML 自动注入) + 额外 CLI 参数 ──
                'args': [yaml_args, ' ', LaunchConfiguration('extra_args')],

                # ── 定位模式 ──
                'localization': LaunchConfiguration('localization'),

                # ── 初始位姿 ──
                'initial_pose': LaunchConfiguration('initial_pose'),

                # ── 仿真 & GUI ──
                'use_sim_time': LaunchConfiguration('use_sim_time'),
                'rtabmap_viz':  'true',
                'rviz':         'false',
            }.items(),
        ),
    ])
