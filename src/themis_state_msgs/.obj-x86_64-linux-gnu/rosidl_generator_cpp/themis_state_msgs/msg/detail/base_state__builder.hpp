// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/BaseState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/base_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_BaseState_angular_rate
{
public:
  explicit Init_BaseState_angular_rate(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::BaseState angular_rate(::themis_state_msgs::msg::BaseState::_angular_rate_type arg)
  {
    msg_.angular_rate = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_pitch
{
public:
  explicit Init_BaseState_pitch(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_angular_rate pitch(::themis_state_msgs::msg::BaseState::_pitch_type arg)
  {
    msg_.pitch = std::move(arg);
    return Init_BaseState_angular_rate(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_roll
{
public:
  explicit Init_BaseState_roll(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_pitch roll(::themis_state_msgs::msg::BaseState::_roll_type arg)
  {
    msg_.roll = std::move(arg);
    return Init_BaseState_pitch(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_yaw
{
public:
  explicit Init_BaseState_yaw(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_roll yaw(::themis_state_msgs::msg::BaseState::_yaw_type arg)
  {
    msg_.yaw = std::move(arg);
    return Init_BaseState_roll(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_velocity_z
{
public:
  explicit Init_BaseState_velocity_z(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_yaw velocity_z(::themis_state_msgs::msg::BaseState::_velocity_z_type arg)
  {
    msg_.velocity_z = std::move(arg);
    return Init_BaseState_yaw(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_velocity_y
{
public:
  explicit Init_BaseState_velocity_y(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_velocity_z velocity_y(::themis_state_msgs::msg::BaseState::_velocity_y_type arg)
  {
    msg_.velocity_y = std::move(arg);
    return Init_BaseState_velocity_z(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_velocity_x
{
public:
  explicit Init_BaseState_velocity_x(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_velocity_y velocity_x(::themis_state_msgs::msg::BaseState::_velocity_x_type arg)
  {
    msg_.velocity_x = std::move(arg);
    return Init_BaseState_velocity_y(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_position_z
{
public:
  explicit Init_BaseState_position_z(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_velocity_x position_z(::themis_state_msgs::msg::BaseState::_position_z_type arg)
  {
    msg_.position_z = std::move(arg);
    return Init_BaseState_velocity_x(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_position_y
{
public:
  explicit Init_BaseState_position_y(::themis_state_msgs::msg::BaseState & msg)
  : msg_(msg)
  {}
  Init_BaseState_position_z position_y(::themis_state_msgs::msg::BaseState::_position_y_type arg)
  {
    msg_.position_y = std::move(arg);
    return Init_BaseState_position_z(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

class Init_BaseState_position_x
{
public:
  Init_BaseState_position_x()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_BaseState_position_y position_x(::themis_state_msgs::msg::BaseState::_position_x_type arg)
  {
    msg_.position_x = std::move(arg);
    return Init_BaseState_position_y(msg_);
  }

private:
  ::themis_state_msgs::msg::BaseState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::BaseState>()
{
  return themis_state_msgs::msg::builder::Init_BaseState_position_x();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__BUILDER_HPP_
