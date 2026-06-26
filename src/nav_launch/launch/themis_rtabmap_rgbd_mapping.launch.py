#!/usr/bin/env python3
"""
THEMIS RTAB-Map RGB-D 建图模式启动文件
基于 rtabmap_launch 官方 launch，使用 ZED XM RGB-D 相机 + 外部融合里程计。

等效命令:
  ros2 launch rtabmap_launch rtabmap.launch.py \
      localization:=false \
      depth:=true \
      subscribe_rgbd:=true \
      rgb_topic:=/zedxm/zedxm/rgb/color/rect/image \
      depth_topic:=/zedxm/zedxm/depth/depth_registered \
      rgb_camera_info_topic:=/zedxm/zedxm/rgb/color/rect/camera_info \
      imu_topic:=/base_imu \
      odom_topic:=/fused_odom \
      visual_odometry:=false \
      approx_sync:=true \
      frame_id:=BASE_LINK \
      odom_frame_id:=ODOM \
      map_frame_id:=MAP \
      database_path:=~/.ros/themis_map.db \
      rtabmap_args:="<yaml_params>"

用法:
  ros2 launch nav_launch themis_rtabmap_rgbd_mapping.launch.py                    # 建图
  ros2 launch nav_launch themis_rtabmap_rgbd_mapping.launch.py \
      database_path:=/path/to/map.db                                               # 指定数据库
  ros2 launch nav_launch themis_rtabmap_rgbd_mapping.launch.py \
      delete_db:=true                                                              # 清空旧地图
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
    """将参数字典转为 RTAB-Map CLI 格式: --Key Value"""
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
    yaml_path = os.path.join(pkg_dir, 'config', 'themis_rtabmap_rgbd_mapping.yaml')

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
            description='地图数据库保存路径 (建图和定位共用)'),

        # ── 额外 CLI 参数 (默认清空旧地图, 建图时常用) ──
        DeclareLaunchArgument(
            'extra_args', default_value='--delete_db_on_start',
            description='额外 RTAB-Map CLI 参数 (默认 --delete_db_on_start 清空旧数据库, 设为空字符串保留)'),

        LogInfo(msg=['[rtabmap] RGB-D mapping config: ', yaml_path]),
        LogInfo(msg=['[rtabmap] Mode: RGB-D Mapping (ZED XM + fused odom)']),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(rtabmap_launch),
            launch_arguments={
                # ── 输入源: RGB-D (内部同步, 无需外部 rgbd_sync) ──
                'depth':               'true',
                'subscribe_rgbd':      'false',
                'subscribe_depth':     'true',
                'subscribe_scan':      'false',
                'subscribe_scan_cloud':'false',

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
                'odom_topic':      launch_cfg.get('odom_topic', '/fused_odom'),

                # ── 坐标系 ──
                'frame_id':      launch_cfg.get('frame_id', 'BASE_LINK'),
                'odom_frame_id': launch_cfg.get('odom_frame_id', 'ODOM'),
                'map_frame_id':  launch_cfg.get('map_frame_id', 'MAP'),
                'publish_tf':    launch_cfg.get('publish_tf', 'true'),

                # ── 同步 (增大队列, 容忍 RGB/Depth 频率不一致) ──
                'approx_sync':         'true',
                'sync_queue_size':     '30',
                'topic_queue_size':    '30',

                # ── 地图数据库 ──
                'database_path': LaunchConfiguration('database_path'),

                # ── RTAB-Map 参数 (YAML 自动注入) + 额外 CLI 参数 ──
                'args': [yaml_args, ' ', LaunchConfiguration('extra_args')],

                # ── 建图 / 定位 ──
                'localization': 'false',

                # ── 仿真 & GUI ──
                'use_sim_time': 'true',
                'rtabmap_viz':  'true',
                'rviz':         'false',
            }.items(),
        ),
    ])
