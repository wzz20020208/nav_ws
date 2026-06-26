#!/usr/bin/env python

# -- coding: utf-8 --

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, QoSReliabilityPolicy, QoSDurabilityPolicy
from geometry_msgs.msg import PoseArray, Pose
from nav_msgs.msg import Path
from sensor_msgs.msg import CameraInfo
from tf2_ros import TransformException, Buffer, TransformListener, LookupException, ConnectivityException, ExtrapolationException

import numpy as np
from std_msgs.msg import Header

class PathToFpvProjector(Node):
    def __init__(self):
        super().__init__('path_to_fpv_projector')

        self.get_logger().info("节点已启动，正在初始化...")

        qos = QoSProfile(
            depth=1,
            reliability=QoSReliabilityPolicy.RELIABLE,
            durability=QoSDurabilityPolicy.VOLATILE
        )

        # 两个相机的内参变量
        self.intrinsics = None      # for zed2i
        self.intrinsics_xm = None   # for zedxm

        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        # FOOT_L 在 map 中的 Z 高度（实时更新）
        self.foot_l_z = 0.0  # 初始值
        self.foot_l_last_update = None

        # --- ZED2i 配置 ---
        self.camera_info_sub = self.create_subscription(
            CameraInfo,
            '/zed2i/zed2i/rgb/color/rect/camera_info',
            self.camera_info_callback,
            qos
        )

        # --- ZEDXM 配置
        self.camera_info_sub_xm = self.create_subscription(
            CameraInfo,
            '/zedxm/zedxm/rgb/color/rect/camera_info',
            #'/zedxm/zedxm/rgb/camera_info',
            self.camera_info_callback_zedxm,
            qos
        )
        self.path_2d_pub_zed2i = self.create_publisher(
            PoseArray,
            '/plan_2d_zed2i',
            qos
        )

        self.path_2d_pub_zedxm = self.create_publisher(
            PoseArray,
            '/plan_2d_zedxm',
            qos
        )

        self.path_sub = self.create_subscription(
            Path,
            '/plan',
            self.path_callback,
            qos
        )

    def project_to_2d(self, point_3d, intrinsics):
        if intrinsics is None or point_3d is None:
            return None

        if point_3d[2] < 0.05:
            return None

        fx, fy, cx, cy = intrinsics['fx'], intrinsics['fy'], intrinsics['cx'], intrinsics['cy']
        x, y, z = point_3d

        u = int((x * fx / z) + cx)
        v = int((y * fy / z) + cy)

        if 0 <= u < intrinsics['width'] and 0 <= v < intrinsics['height']:
            return (u, v)
        return None

    def get_foot_l_z_from_tf(self):
        """实时查询 map → FOOT_L 的 Z 值"""
        try:
            trans = self.tf_buffer.lookup_transform(
                target_frame="FOOT_L",
                source_frame="map",
                time=rclpy.time.Time(),
                timeout=rclpy.duration.Duration(seconds=0.0)
            )
            foot_z = trans.transform.translation.x
            self.foot_l_z = foot_z
            self.foot_l_last_update = self.get_clock().now()
            # self.get_logger().debug(f"更新 FOOT_L Z = {foot_z:.4f}m")
            return foot_z

        except (LookupException, ConnectivityException, ExtrapolationException) as e:
            # 如果失败，使用上一次值或默认
            if self.foot_l_last_update is None:
                self.get_logger().warning(f"首次查询 FOOT_L 失败，使用默认 Z=0: {e}", throttle_duration_sec=5.0)
                return 0.0
            else:
                elapsed = (self.get_clock().now() - self.foot_l_last_update).nanoseconds / 1e9
                if elapsed > 5.0:
                    self.get_logger().warning(f"TF FOOT_L 长时间失效（{elapsed:.1f}s），使用旧值 {self.foot_l_z:.4f}m", throttle_duration_sec=5.0)
                return self.foot_l_z

    def camera_info_callback(self, msg):
        self.intrinsics = {
            'fx': msg.k[0], 'fy': msg.k[4],
            'cx': msg.k[2], 'cy': msg.k[5],
            'width': msg.width, 'height': msg.height
        }

    def camera_info_callback_zedxm(self, msg):
        self.intrinsics_xm = {
            'fx': msg.k[0], 'fy': msg.k[4],
            'cx': msg.k[2], 'cy': msg.k[5],
            'width': msg.width, 'height': msg.height
        }

    def process_and_publish(self, intrinsics, target_frame, publisher, output_frame_id, path_msg):
        if intrinsics is None:
            return

        source_frame = path_msg.header.frame_id
        foot_l_z = self.get_foot_l_z_from_tf()

        try:
            trans = self.tf_buffer.lookup_transform(
                target_frame,
                source_frame,
                rclpy.time.Time(),
                rclpy.duration.Duration(seconds=0.1)
            )

            r = trans.transform.rotation
            t = trans.transform.translation
            rot_matrix = np.array([[1 - 2*(r.y**2 + r.z**2), 2*(r.x*r.y - r.z*r.w), 2*(r.x*r.z + r.y*r.w)],[2*(r.x*r.y + r.z*r.w), 1 - 2*(r.x**2 + r.z**2), 2*(r.y*r.z - r.x*r.w)],[2*(r.x*r.z - r.y*r.w), 2*(r.y*r.z + r.x*r.w), 1 - 2*(r.x**2 + r.y**2)]
            ])
            translation = np.array([t.x, t.y, t.z])

        except TransformException as e:
            return

        out_msg = PoseArray()
        out_msg.header = Header(stamp=self.get_clock().now().to_msg(), frame_id=output_frame_id)
        #print('msg',path_msg.poses)

        for pose_stamped in path_msg.poses:
            pt_original = np.array([
                pose_stamped.pose.position.x,
                pose_stamped.pose.position.y,
                foot_l_z
            ])
            #print('msg',path_msg.poses)
            pt_camera = np.dot(rot_matrix, pt_original) + translation

            pixel = self.project_to_2d(pt_camera, intrinsics)
            if pixel:
                u, v = pixel
                p = Pose()
                p.position.x = float(u)
                p.position.y = float(v)
                p.position.z = pt_camera[2]
                out_msg.poses.append(p)

        if len(out_msg.poses) > 0:
            #print('msg',out_msg)
            publisher.publish(out_msg)

    def path_callback(self, msg):
        # 处理 ZED2i
        self.process_and_publish(
            self.intrinsics,
            'zed2i_left_camera_optical_frame',
            self.path_2d_pub_zed2i,
            'fpv_pixels_zed2i',
            msg
        )

        # 处理 ZEDXM)
        self.process_and_publish(
            self.intrinsics_xm,
            'zedxm_left_camera_optical_frame',
            self.path_2d_pub_zedxm,
            'fpv_pixels_zedxm',
            msg
        )

def main():
    rclpy.init()
    try:
        node = PathToFpvProjector()
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        rclpy.shutdown()

if __name__ == '__main__':
    main()