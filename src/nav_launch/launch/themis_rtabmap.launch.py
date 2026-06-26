#!/usr/bin/env python3
"""
THEMIS RTAB-Map 启动文件 — 点云 SLAM 模式
基于 rtabmap_launch 官方 launch，所有参数由 themis_rtabmap.yaml 统一管理。

话题 & 坐标系配置: 见 YAML 中 launch: 段
RTAB-Map 核心参数:      见 YAML 中 ros__parameters: 段

用法:
  ros2 launch nav_launch themis_rtabmap.launch.py                        # 建图
  ros2 launch nav_launch themis_rtabmap.launch.py localization:=true     # 定位
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
        parts.append(f'--{k}')
        parts.append(str(v))
    return ' '.join(parts)


def generate_launch_description():
    pkg_dir = get_package_share_directory('nav_launch')
    yaml_path = os.path.join(pkg_dir, 'config', 'themis_rtabmap.yaml')

    params = _load_rtabmap_params(yaml_path)
    yaml_args = _params_to_cli(params)
    launch_cfg = _load_launch_config(yaml_path)

    rtabmap_launch = PathJoinSubstitution([
        get_package_share_directory('rtabmap_launch'),
        'launch', 'rtabmap.launch.py'
    ])

    return LaunchDescription([
        DeclareLaunchArgument(
            'database_path', default_value='~/.ros/themis_map.db',
            description='地图数据库保存路径 (建图和定位共用)'),

        DeclareLaunchArgument(
            'extra_args', default_value='--delete_db_on_start',
            description='额外 RTAB-Map CLI 参数 (默认清空旧地图)'),

        DeclareLaunchArgument(
            'localization', default_value='false',
            description='定位模式'),

        LogInfo(msg=['[rtabmap] config: ', yaml_path]),

        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(rtabmap_launch),
            launch_arguments={
                # ── 输入源: 点云 ──
                'depth':               'false',
                'subscribe_scan_cloud': 'true',
                'subscribe_scan':       'false',
                'scan_cloud_topic':     launch_cfg.get('scan_cloud_topic', '/chest_camera/points_sparse'),

                # ── 里程计: 外部 odom, 不用视觉里程计 ──
                'visual_odometry': 'false',
                'odom_topic':      launch_cfg.get('odom_topic', '/odom'),

                # ── 坐标系 ──
                'frame_id':      launch_cfg.get('frame_id', 'BASE_LINK'),
                'odom_frame_id': launch_cfg.get('odom_frame_id', 'odom'),
                'map_frame_id':  launch_cfg.get('map_frame_id', 'map'),

                # ── 同步 ──
                'approx_sync': 'true',

                # ── 地图数据库 ──
                'database_path': LaunchConfiguration('database_path'),

                # ── RTAB-Map 参数 (YAML 自动注入) ──
                'args':          [yaml_args, ' ', LaunchConfiguration('extra_args')],

                # ── 建图 / 定位 ──
                'localization': LaunchConfiguration('localization'),

                # ── 仿真 & GUI ──
                'use_sim_time': 'true',
                'rtabmap_viz':  'true',
                'rviz':         'false',
            }.items(),
        ),
    ])
