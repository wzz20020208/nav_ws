#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <pcl_conversions/pcl_conversions/pcl_conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <cv_bridge/cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.hpp>
#include <memory>
#include <limits>

using std::placeholders::_1;

class DepthToPCL : public rclcpp::Node
{
public:
  DepthToPCL() : Node("depth_to_pcl_node"), downsample_factor_(4), min_depth_(1.0f), max_depth_(10.0f), invert_depth_(false)
  {
    declare_parameter("downsample_factor", 4);
    declare_parameter("target_frame", "chest_depth_camera_link");
    declare_parameter("min_depth", 1.0);
    declare_parameter("max_depth", 10.0);
    declare_parameter("invert_depth", false);

    get_parameter("downsample_factor", downsample_factor_);
    get_parameter("target_frame", target_frame_);
    get_parameter("min_depth", min_depth_);
    get_parameter("max_depth", max_depth_);
    get_parameter("invert_depth", invert_depth_);

    image_sub_ = image_transport::create_subscription(
        this,
        "/chest_camera/chest_camera/depth/image_raw",
        std::bind(&DepthToPCL::depthCallback, this, _1),
        "raw");

    pcl_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>("PCL_output", rclcpp::QoS(1).reliable());

    RCLCPP_INFO(get_logger(), "DepthToPCL node started");
    RCLCPP_INFO(get_logger(), "Subscribing to: /chest_camera/chest_camera/depth/image_raw");
    RCLCPP_INFO(get_logger(), "Publishing to: PCL_output");
    RCLCPP_INFO(get_logger(), "Target frame: %s", target_frame_.c_str());
  }

  void depthCallback(const sensor_msgs::msg::Image::ConstSharedPtr & msg)
  {
    cv_bridge::CvImagePtr cv_ptr;
    try
    {
      cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::TYPE_32FC1);
    }
    catch (cv_bridge::Exception& e)
    {
      RCLCPP_ERROR(get_logger(), "cv_bridge exception: %s", e.what());
      return;
    }

    cv::Mat depth = cv_ptr->image;
    int orig_rows = depth.rows;
    int orig_cols = depth.cols;
    int new_rows = orig_rows / downsample_factor_;
    int new_cols = orig_cols / downsample_factor_;

    cv::Mat downsampled;
    cv::resize(depth, downsampled, cv::Size(new_cols, new_rows), 0, 0, cv::INTER_AREA);

    pcl::PointCloud<pcl::PointXYZ> cloud;
    cloud.width = new_cols;
    cloud.height = new_rows;
    cloud.points.resize(cloud.width * cloud.height);
    cloud.header.frame_id = target_frame_;
    cloud.header.stamp = pcl_conversions::toPCL(msg->header).stamp;

    float fx = 1.0f, fy = 1.0f, cx = static_cast<float>(new_cols) / 2.0f, cy = static_cast<float>(new_rows) / 2.0f;
    for (int v = 0; v < new_rows; ++v)
    {
      for (int u = 0; u < new_cols; ++u)
      {
      float z = downsampled.at<float>(v, u);
      if (invert_depth_) {
        z = max_depth_ - z;
      }
      if (std::isnan(z) || z <= min_depth_ || z > max_depth_)
        {
          cloud.points[v * new_cols + u].x = cloud.points[v * new_cols + u].y = cloud.points[v * new_cols + u].z = std::numeric_limits<float>::quiet_NaN();
        }
        else
        {
          float normalized_z = (z - min_depth_) / (max_depth_ - min_depth_);
          normalized_z = std::max(0.0f, std::min(1.0f, normalized_z));
          cloud.points[v * new_cols + u].x = (static_cast<float>(u) - cx) * normalized_z / fx;
          cloud.points[v * new_cols + u].y = (static_cast<float>(new_rows - 1 - v) - cy) * normalized_z / fy;
          cloud.points[v * new_cols + u].z = normalized_z;
        }
      }
    }

    sensor_msgs::msg::PointCloud2 output;
    pcl::toROSMsg(cloud, output);
    output.header = msg->header;
    output.header.frame_id = target_frame_;
    pcl_pub_->publish(output);
  }

private:
  image_transport::Subscriber image_sub_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pcl_pub_;
  int downsample_factor_;
  float min_depth_;
  float max_depth_;
  bool invert_depth_;
  std::string target_frame_;
};

int main(int argc, char** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DepthToPCL>());
  rclcpp::shutdown();
  return 0;
}