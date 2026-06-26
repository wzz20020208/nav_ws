// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/JointArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__JOINT_ARRAY__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__JOINT_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/joint_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_JointArray_torque
{
public:
  explicit Init_JointArray_torque(::themis_state_msgs::msg::JointArray & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::JointArray torque(::themis_state_msgs::msg::JointArray::_torque_type arg)
  {
    msg_.torque = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::JointArray msg_;
};

class Init_JointArray_velocity
{
public:
  explicit Init_JointArray_velocity(::themis_state_msgs::msg::JointArray & msg)
  : msg_(msg)
  {}
  Init_JointArray_torque velocity(::themis_state_msgs::msg::JointArray::_velocity_type arg)
  {
    msg_.velocity = std::move(arg);
    return Init_JointArray_torque(msg_);
  }

private:
  ::themis_state_msgs::msg::JointArray msg_;
};

class Init_JointArray_position
{
public:
  Init_JointArray_position()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_JointArray_velocity position(::themis_state_msgs::msg::JointArray::_position_type arg)
  {
    msg_.position = std::move(arg);
    return Init_JointArray_velocity(msg_);
  }

private:
  ::themis_state_msgs::msg::JointArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::JointArray>()
{
  return themis_state_msgs::msg::builder::Init_JointArray_position();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__JOINT_ARRAY__BUILDER_HPP_
