// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/FootArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/foot_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_FootArray_feet
{
public:
  Init_FootArray_feet()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  ::themis_state_msgs::msg::FootArray feet(::themis_state_msgs::msg::FootArray::_feet_type arg)
  {
    msg_.feet = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::FootArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::FootArray>()
{
  return themis_state_msgs::msg::builder::Init_FootArray_feet();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__BUILDER_HPP_
