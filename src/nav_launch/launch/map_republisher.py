#!/usr/bin/env python3
"""加载静态地图 + 持续发布 map→odom TF (10Hz) + 地图 (1Hz)"""

import yaml
import numpy as np
import rclpy
from rclpy.node import Node
from rclpy.parameter import Parameter
from rclpy.qos import QoSProfile, ReliabilityPolicy, DurabilityPolicy
from nav_msgs.msg import OccupancyGrid
from geometry_msgs.msg import TransformStamped, Pose
from tf2_ros import TransformBroadcaster


MAP_YAML = "/home/wzz2002/THEMIS/THEMIS/nav_ws/src/map/map.yaml"


def load_map():
    with open(MAP_YAML) as f:
        data = yaml.safe_load(f)
    import os
    pgm_path = os.path.join(os.path.dirname(MAP_YAML), data['image'])
    img = _read_pgm(pgm_path)
    msg = OccupancyGrid()
    msg.info.resolution = data['resolution']
    msg.info.width = img.shape[1]
    msg.info.height = img.shape[0]
    msg.info.origin = Pose()
    msg.info.origin.position.x = data['origin'][0]
    msg.info.origin.position.y = data['origin'][1]
    msg.info.origin.position.z = 0.0
    msg.info.origin.orientation.w = 1.0
    free_val = int(data['free_thresh'] * 255)
    occ_val = int(data['occupied_thresh'] * 255)
    out = np.full(img.shape, -1, dtype=np.int8)
    out[img <= free_val] = 0
    out[img >= occ_val] = 100
    if data.get('negate', 0):
        free_mask = out == 0
        occ_mask = out == 100
        out[free_mask] = 100
        out[occ_mask] = 0
    msg.data = out.ravel().tolist()
    msg.header.frame_id = 'map'
    return msg


def _read_pgm(path):
    with open(path, 'rb') as f:
        header = f.readline().strip()
        if header == b'P5':
            return _read_p5(f)
        elif header == b'P2':
            return _read_p2(f)
        raise ValueError(f"Unsupported PGM format: {header}")


def _read_p5(f):
    while True:
        line = f.readline().strip()
        if not line.startswith(b'#'):
            break
    w, h = map(int, line.split())
    maxval = int(f.readline().strip())
    data = np.frombuffer(f.read(), dtype=np.uint8).reshape(h, w)
    if maxval <= 255:
        return data
    return (data.astype(np.float32) * 255.0 / maxval).astype(np.uint8)


def _read_p2(f):
    data = []
    for line in f:
        line = line.strip()
        if line.startswith(b'#'):
            continue
        data.extend(int(x) for x in line.split())
    w, h = data[1], data[0]
    maxval = data[2]
    pixels = np.array(data[3:], dtype=np.float32)
    if maxval <= 255:
        return pixels.reshape(h, w).astype(np.uint8)
    return (pixels * 255.0 / maxval).reshape(h, w).astype(np.uint8)


class MapRepublisher(Node):
    def __init__(self):
        super().__init__('map_republisher',
                         parameter_overrides=[Parameter('use_sim_time', Parameter.Type.BOOL, True)])
        self.tf_pub = TransformBroadcaster(self)

        map_qos = QoSProfile(
            reliability=ReliabilityPolicy.RELIABLE,
            durability=DurabilityPolicy.TRANSIENT_LOCAL,
            depth=1)
        self.map_pub = self.create_publisher(OccupancyGrid, '/map', map_qos)

        self.map_msg = load_map()
        self.get_logger().info(f"Loaded map: {self.map_msg.info.width}x{self.map_msg.info.height}")

        self.tf_timer = self.create_timer(0.1, self.publish_tf)
        self.map_timer = self.create_timer(1.0, self.publish_map)

    def publish_tf(self):
        t = TransformStamped()
        t.header.stamp = self.get_clock().now().to_msg()
        t.header.frame_id = 'map'
        t.child_frame_id = 'odom'
        t.transform.rotation.w = 1.0
        self.tf_pub.sendTransform(t)

    def publish_map(self):
        self.map_msg.header.stamp = self.get_clock().now().to_msg()
        self.map_pub.publish(self.map_msg)


def main():
    rclpy.init()
    rclpy.spin(MapRepublisher())
    rclpy.shutdown()

if __name__ == '__main__':
    main()
