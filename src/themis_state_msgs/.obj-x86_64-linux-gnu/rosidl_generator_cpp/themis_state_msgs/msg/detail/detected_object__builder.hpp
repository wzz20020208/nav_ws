// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/DetectedObject.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/detected_object__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_DetectedObject_bbox
{
public:
  explicit Init_DetectedObject_bbox(::themis_state_msgs::msg::DetectedObject & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::DetectedObject bbox(::themis_state_msgs::msg::DetectedObject::_bbox_type arg)
  {
    msg_.bbox = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObject msg_;
};

class Init_DetectedObject_score
{
public:
  explicit Init_DetectedObject_score(::themis_state_msgs::msg::DetectedObject & msg)
  : msg_(msg)
  {}
  Init_DetectedObject_bbox score(::themis_state_msgs::msg::DetectedObject::_score_type arg)
  {
    msg_.score = std::move(arg);
    return Init_DetectedObject_bbox(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObject msg_;
};

class Init_DetectedObject_category
{
public:
  explicit Init_DetectedObject_category(::themis_state_msgs::msg::DetectedObject & msg)
  : msg_(msg)
  {}
  Init_DetectedObject_score category(::themis_state_msgs::msg::DetectedObject::_category_type arg)
  {
    msg_.category = std::move(arg);
    return Init_DetectedObject_score(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObject msg_;
};

class Init_DetectedObject_id
{
public:
  Init_DetectedObject_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_DetectedObject_category id(::themis_state_msgs::msg::DetectedObject::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_DetectedObject_category(msg_);
  }

private:
  ::themis_state_msgs::msg::DetectedObject msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::DetectedObject>()
{
  return themis_state_msgs::msg::builder::Init_DetectedObject_id();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT__BUILDER_HPP_
