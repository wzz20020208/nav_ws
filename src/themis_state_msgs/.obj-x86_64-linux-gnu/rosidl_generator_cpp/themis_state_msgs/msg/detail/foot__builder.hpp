// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/Foot.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/foot__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_Foot_z
{
public:
  explicit Init_Foot_z(::themis_state_msgs::msg::Foot & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::Foot z(::themis_state_msgs::msg::Foot::_z_type arg)
  {
    msg_.z = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::Foot msg_;
};

class Init_Foot_y
{
public:
  explicit Init_Foot_y(::themis_state_msgs::msg::Foot & msg)
  : msg_(msg)
  {}
  Init_Foot_z y(::themis_state_msgs::msg::Foot::_y_type arg)
  {
    msg_.y = std::move(arg);
    return Init_Foot_z(msg_);
  }

private:
  ::themis_state_msgs::msg::Foot msg_;
};

class Init_Foot_x
{
public:
  Init_Foot_x()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_Foot_y x(::themis_state_msgs::msg::Foot::_x_type arg)
  {
    msg_.x = std::move(arg);
    return Init_Foot_y(msg_);
  }

private:
  ::themis_state_msgs::msg::Foot msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::Foot>()
{
  return themis_state_msgs::msg::builder::Init_Foot_x();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__BUILDER_HPP_
