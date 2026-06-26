#include <rclcpp/rclcpp.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_ros/transform_broadcaster.h>
#include <geometry_msgs/msg/transform_stamped.hpp>
#include <themis_state_msgs/msg/base_state.hpp>

class BaseStateToOdom : public rclcpp::Node
{
public:
    BaseStateToOdom() : Node("base_state_to_odom")
    {
        declare_parameter<std::string>("base_frame", "BASE_LINK");
        declare_parameter<std::string>("odom_frame", "odom");
        declare_parameter<std::string>("base_state_topic", "/base_state");

        std::string base_frame = get_parameter("base_frame").as_string();
        std::string odom_frame = get_parameter("odom_frame").as_string();
        std::string base_state_topic = get_parameter("base_state_topic").as_string();

        odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/odom", 10);
        tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

        sub_ = create_subscription<themis_state_msgs::msg::BaseState>(
            base_state_topic, 10,
            [this, odom_frame, base_frame](const themis_state_msgs::msg::BaseState::SharedPtr msg) {
                this->publishOdom(msg, odom_frame, base_frame);
            });
    }

private:
    void publishOdom(const themis_state_msgs::msg::BaseState::SharedPtr msg,
                     const std::string& odom_frame,
                     const std::string& base_frame)
    {
        nav_msgs::msg::Odometry odom;
        odom.header.stamp = this->now();
        odom.header.frame_id = odom_frame;
        odom.child_frame_id = base_frame;

        odom.pose.pose.position.x = msg->position_x;
        odom.pose.pose.position.y = msg->position_y;
        odom.pose.pose.position.z = msg->position_z;

        tf2::Quaternion q;
        q.setRPY(msg->roll, msg->pitch, msg->yaw);
        odom.pose.pose.orientation.x = q.x();
        odom.pose.pose.orientation.y = q.y();
        odom.pose.pose.orientation.z = q.z();
        odom.pose.pose.orientation.w = q.w();

        odom.twist.twist.linear.x = msg->velocity_x;
        odom.twist.twist.linear.y = msg->velocity_y;
        odom.twist.twist.linear.z = msg->velocity_z;
        odom.twist.twist.angular.z = msg->angular_rate;

        odom_pub_->publish(odom);

        geometry_msgs::msg::TransformStamped tf;
        tf.header.stamp = odom.header.stamp;
        tf.header.frame_id = odom_frame;
        tf.child_frame_id = base_frame;
        tf.transform.translation.x = msg->position_x;
        tf.transform.translation.y = msg->position_y;
        tf.transform.translation.z = msg->position_z;
        tf.transform.rotation = odom.pose.pose.orientation;

        tf_broadcaster_->sendTransform(tf);
    }

    rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
    rclcpp::Subscription<themis_state_msgs::msg::BaseState>::SharedPtr sub_;
    std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;
};

int main(int argc, char** argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<BaseStateToOdom>());
    rclcpp::shutdown();
    return 0;
}