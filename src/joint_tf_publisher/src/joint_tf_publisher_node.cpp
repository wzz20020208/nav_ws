#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2/LinearMath/Matrix3x3.h>
#include <tf2/LinearMath/Vector3.h>
#include <cmath>

struct JointDef {
    std::string parent;
    std::string child;
    std::array<double, 3> xyz;
    std::array<double, 3> rpy;
    std::array<double, 3> axis;
};

const std::map<std::string, JointDef> URDF_JOINTS = {
    // Right leg
    {"HIP_YAW_R",    {"BASE_LINK", "HIP_R",       {-0.183378, -0.0625, -0.288378}, {0.0, 0.785398, 0.0}, {0, 0, 1}}},
    {"HIP_ROLL_R",   {"HIP_R", "HIP_ABAD_R",      {0.141, 0.0, 0.0},              {1.5708, 0.0, -1.5708}, {0, 0, 1}}},
    {"HIP_PITCH_R",  {"HIP_ABAD_R", "FEMUR_R",    {0.029, 0.0, 0.0},              {-1.5708, 0.785398, -1.5708}, {0, 0, 1}}},
    {"KNEE_PITCH_R", {"FEMUR_R", "TIBIA_R",       {0.375, 0.0, 0.0},              {0.0, 0.0, 0.0}, {0, 0, 1}}},
    {"ANKLE_PITCH_R",{"TIBIA_R", "ANKLE_R",       {0.375, 0.0, 0.0},              {0.0, 0.0, 0.0}, {0, 0, 1}}},
    {"ANKLE_ROLL_R", {"ANKLE_R", "FOOT_R",        {0.0, 0.0, 0.0},               {-1.5708, 0.0, 0.0}, {0, 0, 1}}},
    // Left leg
    {"HIP_YAW_L",    {"BASE_LINK", "HIP_L",       {-0.183378, 0.0625, -0.288378}, {0.0, 0.785398, 0.0}, {0, 0, 1}}},
    {"HIP_ROLL_L",   {"HIP_L", "HIP_ABAD_L",      {0.141, 0.0, 0.0},              {1.5708, 0.0, -1.5708}, {0, 0, 1}}},
    {"HIP_PITCH_L",  {"HIP_ABAD_L", "FEMUR_L",    {-0.029, 0.0, 0.0},             {-1.5708, 0.785398, -1.5708}, {0, 0, 1}}},
    {"KNEE_PITCH_L", {"FEMUR_L", "TIBIA_L",       {0.375, 0.0, 0.0},              {0.0, 0.0, 0.0}, {0, 0, 1}}},
    {"ANKLE_PITCH_L",{"TIBIA_L", "ANKLE_L",       {0.375, 0.0, 0.0},              {0.0, 0.0, 0.0}, {0, 0, 1}}},
    {"ANKLE_ROLL_L", {"ANKLE_L", "FOOT_L",        {0.0, 0.0, 0.0},               {-1.5708, 0.0, 0.0}, {0, 0, 1}}},
    // Right arm
    {"SHOULDER_PITCH_R", {"BASE_LINK", "UPPERSHOULDER_R", {-0.065085, -0.1567, 0.113475}, {1.5708, 0.0, 0.0}, {0, 0, 1}}},
    {"SHOULDER_ROLL_R",  {"UPPERSHOULDER_R", "LOWERSHOULDER_R", {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"SHOULDER_YAW_R",   {"LOWERSHOULDER_R", "UPPERARM_R",     {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"ELBOW_PITCH_R",    {"UPPERARM_R", "ELBOW_R",            {0.0, -0.015, 0.22495}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"ELBOW_YAW_R",      {"ELBOW_R", "FOREARM_R",            {0.15495, -0.015, 0.0}, {3.14134, -1.5708, 0.0}, {0, 0, 1}}},
    {"WRIST_PITCH_R",    {"FOREARM_R", "UPPERWRIST_R",       {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"WRIST_YAW_R",      {"UPPERWRIST_R", "LOWERWRIST_R",    {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    // Left arm
    {"SHOULDER_PITCH_L", {"BASE_LINK", "UPPERSHOULDER_L", {-0.065085, 0.1567, 0.113475}, {-1.5708, 0.0, 0.0}, {0, 0, 1}}},
    {"SHOULDER_ROLL_L",  {"UPPERSHOULDER_L", "LOWERSHOULDER_L", {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"SHOULDER_YAW_L",   {"LOWERSHOULDER_L", "UPPERARM_L",     {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"ELBOW_PITCH_L",    {"UPPERARM_L", "ELBOW_L",            {0.0, 0.015, 0.22495}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"ELBOW_YAW_L",      {"ELBOW_L", "FOREARM_L",            {0.15495, 0.015, 0.0}, {3.14134, -1.5708, 0.0}, {0, 0, 1}}},
    {"WRIST_PITCH_L",    {"FOREARM_L", "UPPERWRIST_L",       {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    {"WRIST_YAW_L",      {"UPPERWRIST_L", "LOWERWRIST_L",    {0.0, 0.0, 0.0}, {3.14159, -1.5708, 0.0}, {0, 0, 1}}},
    // Head
    {"HEAD_YAW",   {"BASE_LINK", "NECK",  {-0.069374, 0.0, 0.230836}, {0.0, 0.0, 0.0}, {0, 0, 1}}},
    {"HEAD_PITCH", {"NECK", "HEAD",      {0.0, 0.0, 0.0},            {1.5708, 0.0, 0.0}, {0, 0, 1}}},
};

class JointTFPublisherNode : public rclcpp::Node {
public:
    JointTFPublisherNode() : Node("joint_tf_publisher"), tf_broadcaster_(this), static_tf_broadcaster_(this) {
        subscription_ = this->create_subscription<sensor_msgs::msg::JointState>(
            "/joint_states", 10,
            std::bind(&JointTFPublisherNode::listener_callback, this, std::placeholders::_1));

        // Publish static camera transforms (from SDF: Simulation/models/themis/model.sdf)
        // 1. BASE_LINK -> chest_depth_camera_optical_link
        geometry_msgs::msg::TransformStamped camera_optical_tf;
        camera_optical_tf.header.stamp = this->now();
        camera_optical_tf.header.frame_id = "BASE_LINK";
        camera_optical_tf.child_frame_id = "chest_depth_camera_optical_link";
        camera_optical_tf.transform.translation.x = 0.0334;
        camera_optical_tf.transform.translation.y = 0.0;
        camera_optical_tf.transform.translation.z = 0.01613;
        tf2::Quaternion q_optical = zyxToQuaternion(-2.0944, 0.0, -1.5708);
        camera_optical_tf.transform.rotation.x = q_optical.x();
        camera_optical_tf.transform.rotation.y = q_optical.y();
        camera_optical_tf.transform.rotation.z = q_optical.z();
        camera_optical_tf.transform.rotation.w = q_optical.w();
        static_tf_broadcaster_.sendTransform(camera_optical_tf);

        // 2. BASE_LINK -> chest_depth_camera_link
        geometry_msgs::msg::TransformStamped camera_link_tf;
        camera_link_tf.header.stamp = this->now();
        camera_link_tf.header.frame_id = "BASE_LINK";
        camera_link_tf.child_frame_id = "chest_depth_camera_link";
        camera_link_tf.transform.translation.x = 0.0334;
        camera_link_tf.transform.translation.y = 0.0;
        camera_link_tf.transform.translation.z = 0.01613;
        tf2::Quaternion q_link = zyxToQuaternion(0.0, 0.5236, 0.0);
        camera_link_tf.transform.rotation.x = q_link.x();
        camera_link_tf.transform.rotation.y = q_link.y();
        camera_link_tf.transform.rotation.z = q_link.z();
        camera_link_tf.transform.rotation.w = q_link.w();
        static_tf_broadcaster_.sendTransform(camera_link_tf);

        RCLCPP_INFO(this->get_logger(), "Joint TF Publisher started, listening to /joint_states");
    }

private:
    tf2_ros::TransformBroadcaster tf_broadcaster_;
    tf2_ros::StaticTransformBroadcaster static_tf_broadcaster_;
    rclcpp::Subscription<sensor_msgs::msg::JointState>::SharedPtr subscription_;

    void listener_callback(const sensor_msgs::msg::JointState::SharedPtr msg) {
        std::map<std::string, double> joint_positions;
        for (size_t i = 0; i < msg->name.size(); ++i) {
            joint_positions[msg->name[i]] = (i < msg->position.size()) ? msg->position[i] : 0.0;
        }

        rclcpp::Time now = msg->header.stamp;

        for (const auto& [joint_name, joint_info] : URDF_JOINTS) {
            double angle = 0.0;
            if (joint_name == "SHOULDER_PITCH_L") {
                auto it = joint_positions.find(joint_name);
                if (it != joint_positions.end()) {
                    angle = it->second;
                }
            } else {
                auto it = joint_positions.find(joint_name);
                if (it != joint_positions.end()) {
                    angle = it->second;
                }
            }

            geometry_msgs::msg::TransformStamped tf;
            tf.header.stamp = now;
            tf.header.frame_id = joint_info.parent;
            tf.child_frame_id = joint_info.child;

            tf.transform.translation.x = joint_info.xyz[0];
            tf.transform.translation.y = joint_info.xyz[1];
            tf.transform.translation.z = joint_info.xyz[2];

            tf2::Quaternion q_initial = zyxToQuaternion(joint_info.rpy[0], joint_info.rpy[1], joint_info.rpy[2]);
            tf2::Quaternion q_joint = axisAngleToQuaternion(joint_info.axis, angle);
            tf2::Quaternion q_final = multiplyQuaternions(q_initial, q_joint);

            tf.transform.rotation.x = q_final.x();
            tf.transform.rotation.y = q_final.y();
            tf.transform.rotation.z = q_final.z();
            tf.transform.rotation.w = q_final.w();

            tf_broadcaster_.sendTransform(tf);
        }
    }

    tf2::Quaternion zyxToQuaternion(double roll, double pitch, double yaw) {
        double cr = std::cos(roll * 0.5), sr = std::sin(roll * 0.5);
        double cp = std::cos(pitch * 0.5), sp = std::sin(pitch * 0.5);
        double cy = std::cos(yaw * 0.5), sy = std::sin(yaw * 0.5);
        return tf2::Quaternion(
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        );
    }

    tf2::Quaternion axisAngleToQuaternion(const std::array<double, 3>& axis, double angle) {
        double norm = std::sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
        if (norm < 1e-6) return tf2::Quaternion(0, 0, 0, 1);
        double ax = axis[0]/norm, ay = axis[1]/norm, az = axis[2]/norm;
        double half = angle * 0.5;
        double s = std::sin(half);
        return tf2::Quaternion(ax * s, ay * s, az * s, std::cos(half));
    }

    tf2::Quaternion multiplyQuaternions(const tf2::Quaternion& q1, const tf2::Quaternion& q2) {
        return tf2::Quaternion(
            q1.w() * q2.x() + q1.x() * q2.w() + q1.y() * q2.z() - q1.z() * q2.y(),
            q1.w() * q2.y() - q1.x() * q2.z() + q1.y() * q2.w() + q1.z() * q2.x(),
            q1.w() * q2.z() + q1.x() * q2.y() - q1.y() * q2.x() + q1.z() * q2.w(),
            q1.w() * q2.w() - q1.x() * q2.x() - q1.y() * q2.y() - q1.z() * q2.z()
        );
    }
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JointTFPublisherNode>());
    rclcpp::shutdown();
    return 0;
}