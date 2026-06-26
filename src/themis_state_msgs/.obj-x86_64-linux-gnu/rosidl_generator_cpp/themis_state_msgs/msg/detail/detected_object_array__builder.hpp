// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/DetectedObjectArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/detected_object_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_DetectedObjectArray_detection_stamp
{
public:
  explicit Init_DetectedObjectArray_detection_stamp(::themis_state_msgs::msg::DetectedObjectArray & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::DetectedObjectArray detection_stamp(::themis_state_msgs::msg::DetectedObjectArray::_detection_stamp_type arg)
  {
    msg_.detection_stamp = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObjectArray msg_;
};

class Init_DetectedObjectArray_objects
{
public:
  explicit Init_DetectedObjectArray_objects(::themis_state_msgs::msg::DetectedObjectArray & msg)
  : msg_(msg)
  {}
  Init_DetectedObjectArray_detection_stamp objects(::themis_state_msgs::msg::DetectedObjectArray::_objects_type arg)
  {
    msg_.objects = std::move(arg);
    return Init_DetectedObjectArray_detection_stamp(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObjectArray msg_;
};

class Init_DetectedObjectArray_camera_stamp
{
public:
  explicit Init_DetectedObjectArray_camera_stamp(::themis_state_msgs::msg::DetectedObjectArray & msg)
  : msg_(msg)
  {}
  Init_DetectedObjectArray_objects camera_stamp(::themis_state_msgs::msg::DetectedObjectArray::_camera_stamp_type arg)
  {
    msg_.camera_stamp = std::move(arg);
    return Init_DetectedObjectArray_objects(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObjectArray msg_;
};

class Init_DetectedObjectArray_header
{
public:
  Init_DetectedObjectArray_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_DetectedObjectArray_camera_stamp header(::themis_state_msgs::msg::DetectedObjectArray::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_DetectedObjectArray_camera_stamp(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObjectArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::DetectedObjectArray>()
{
  return themis_state_msgs::msg::builder::Init_DetectedObjectArray_header();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__BUILDER_HPP_
