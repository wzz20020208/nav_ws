#!/usr/bin/env python3
"""
THEMIS Nav2 导航启动文件
启动所有 Nav2 核心节点: BT Navigator、Planner、Controller、Behavior Server、Waypoint Follower

依赖:
  - RTAB-Map 定位 (提供 /map, map->odom TF)
  - 外部里程计 (提供 odom->base_link TF)
  - 激光雷达 (提供 /scan)

使用方式:
  ros2 launch nav_launch themis_navigation.launch.py
  ros2 launch nav_launch themis_navigation.launch.py \
      params_file:=/path/to/custom_nav2_params.yaml
  ros2 launch nav_launch themis_navigation.launch.py \
      use_sim_time:=false autostart:=false
"""

import os

from ament_index_python.packages import get_package_share_directory

from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument,
    SetEnvironmentVariable,
    LogInfo,
)
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from nav2_common.launch import RewrittenYaml


def generate_launch_description():
    # ── 包路径 ──
    pkg_dir = get_package_share_directory('nav_launch')

    # ── Launch 参数 ──
    params_file = LaunchConfiguration('params_file')
    use_sim_time = LaunchConfiguration('use_sim_time')
    autostart = LaunchConfiguration('autostart')
    log_level = LaunchConfiguration('log_level')

    # ── 生命周期管理节点列表 ──
    lifecycle_nodes = [
        'controller_server',
        'planner_server',
        'behavior_server',
        'bt_navigator',
        'waypoint_follower',
    ]

    # ── TF 重映射 (将全局 TF 话题重映射为相对话题) ──
    remappings = [
        ('/tf', 'tf'),
        ('/tf_static', 'tf_static'),
    ]

    # ── 参数替换 ──
    param_substitutions = {
        'use_sim_time': use_sim_time,
        'autostart': autostart,
    }

    configured_params = RewrittenYaml(
        source_file=params_file,
        root_key='',
        param_rewrites=param_substitutions,
        convert_types=True,
    )

    # ── ROS 日志行缓冲环境变量 ──
    stdout_linebuf_envvar = SetEnvironmentVariable(
        'RCUTILS_LOGGING_BUFFERED_STREAM', '1'
    )

    # ── Launch 参数声明 ──
    declare_params_file_cmd = DeclareLaunchArgument(
        'params_file',
        default_value=os.path.join(pkg_dir, 'config', 'themis_navigation.yaml'),
        description='Nav2 参数 YAML 文件全路径',
    )

    declare_use_sim_time_cmd = DeclareLaunchArgument(
        'use_sim_time',
        default_value='true',
        description='使用仿真时间 (Gazebo/Simulation)',
    )

    declare_autostart_cmd = DeclareLaunchArgument(
        'autostart',
        default_value='true',
        description='自动激活所有 Nav2 生命周期节点',
    )

    declare_log_level_cmd = DeclareLaunchArgument(
        'log_level',
        default_value='info',
        description='ROS 2 日志级别 (debug/info/warn/error)',
    )

    # ══════════════════════════════════════════════════════════
    # Nav2 核心节点
    # ══════════════════════════════════════════════════════════

    # ── 控制器 (DWB 局部路径规划) ──
    controller_server_node = Node(
        package='nav2_controller',
        executable='controller_server',
        name='controller_server',
        output='screen',
        parameters=[configured_params],
        arguments=['--ros-args', '--log-level', log_level],
        remappings=remappings,
    )

    # ── 规划器 (NavFn 全局路径规划) ──
    planner_server_node = Node(
        package='nav2_planner',
        executable='planner_server',
        name='planner_server',
        output='screen',
        parameters=[configured_params],
        arguments=['--ros-args', '--log-level', log_level],
        remappings=remappings,
    )

    # ── 行为服务器 (恢复行为: spin/backup/wait) ──
    behavior_server_node = Node(
        package='nav2_behaviors',
        executable='behavior_server',
        name='behavior_server',
        output='screen',
        parameters=[configured_params],
        arguments=['--ros-args', '--log-level', log_level],
        remappings=remappings,
    )

    # ── 行为树导航器 ──
    bt_navigator_node = Node(
        package='nav2_bt_navigator',
        executable='bt_navigator',
        name='bt_navigator',
        output='screen',
        parameters=[configured_params],
        arguments=['--ros-args', '--log-level', log_level],
        remappings=remappings,
    )

    # ── 航点跟随器 ──
    waypoint_follower_node = Node(
        package='nav2_waypoint_follower',
        executable='waypoint_follower',
        name='waypoint_follower',
        output='screen',
        parameters=[configured_params],
        arguments=['--ros-args', '--log-level', log_level],
        remappings=remappings,
    )

    # ── 生命周期管理器 ──
    lifecycle_manager_node = Node(
        package='nav2_lifecycle_manager',
        executable='lifecycle_manager',
        name='lifecycle_manager_navigation',
        output='screen',
        parameters=[
            {'use_sim_time': use_sim_time},
            {'autostart': autostart},
            {'node_names': lifecycle_nodes},
        ],
        arguments=['--ros-args', '--log-level', log_level],
    )

    # ══════════════════════════════════════════════════════════
    # LaunchDescription 组装
    # ══════════════════════════════════════════════════════════
    ld = LaunchDescription()

    # 环境变量
    ld.add_action(stdout_linebuf_envvar)

    # 参数声明
    ld.add_action(declare_params_file_cmd)
    ld.add_action(declare_use_sim_time_cmd)
    ld.add_action(declare_autostart_cmd)
    ld.add_action(declare_log_level_cmd)

    # 启动信息
    ld.add_action(LogInfo(msg=['[nav2] 启动 THEMIS Nav2 导航堆栈']))
    ld.add_action(LogInfo(msg=['[nav2] 参数文件: ', params_file]))
    ld.add_action(LogInfo(msg=['[nav2] 仿真时间: ', use_sim_time]))

    # 核心节点
    ld.add_action(controller_server_node)
    ld.add_action(planner_server_node)
    ld.add_action(behavior_server_node)
    ld.add_action(bt_navigator_node)
    ld.add_action(waypoint_follower_node)

    # 生命周期管理器 (最后启动, 统一激活所有节点)
    ld.add_action(lifecycle_manager_node)

    return ld
