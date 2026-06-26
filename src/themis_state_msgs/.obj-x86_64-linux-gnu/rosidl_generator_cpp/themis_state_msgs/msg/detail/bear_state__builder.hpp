// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/BearState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/bear_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_BearState_right_hand
{
public:
  explicit Init_BearState_right_hand(::themis_state_msgs::msg::BearState & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::BearState right_hand(::themis_state_msgs::msg::BearState::_right_hand_type arg)
  {
    msg_.right_hand = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::BearState msg_;
};

class Init_BearState_left_hand
{
public:
  explicit Init_BearState_left_hand(::themis_state_msgs::msg::BearState & msg)
  : msg_(msg)
  {}
  Init_BearState_right_hand left_hand(::themis_state_msgs::msg::BearState::_left_hand_type arg)
  {
    msg_.left_hand = std::move(arg);
    return Init_BearState_right_hand(msg_);
  }

private:
  ::themis_state_msgs::msg::BearState msg_;
};

class Init_BearState_right_leg
{
public:
  explicit Init_BearState_right_leg(::themis_state_msgs::msg::BearState & msg)
  : msg_(msg)
  {}
  Init_BearState_left_hand right_leg(::themis_state_msgs::msg::BearState::_right_leg_type arg)
  {
    msg_.right_leg = std::move(arg);
    return Init_BearState_left_hand(msg_);
  }

private:
  ::themis_state_msgs::msg::BearState msg_;
};

class Init_BearState_left_leg
{
public:
  explicit Init_BearState_left_leg(::themis_state_msgs::msg::BearState & msg)
  : msg_(msg)
  {}
  Init_BearState_right_leg left_leg(::themis_state_msgs::msg::BearState::_left_leg_type arg)
  {
    msg_.left_leg = std::move(arg);
    return Init_BearState_right_leg(msg_);
  }

private:
  ::themis_state_msgs::msg::BearState msg_;
};

class Init_BearState_right_arm
{
public:
  explicit Init_BearState_right_arm(::themis_state_msgs::msg::BearState & msg)
  : msg_(msg)
  {}
  Init_BearState_left_leg right_arm(::themis_state_msgs::msg::BearState::_right_arm_type arg)
  {
    msg_.right_arm = std::move(arg);
    return Init_BearState_left_leg(msg_);
  }

private:
  ::themis_state_msgs::msg::BearState msg_;
};

class Init_BearState_left_arm
{
public:
  explicit Init_BearState_left_arm(::themis_state_msgs::msg::BearState & msg)
  : msg_(msg)
  {}
  Init_BearState_right_arm left_arm(::themis_state_msgs::msg::BearState::_left_arm_type arg)
  {
    msg_.left_arm = std::move(arg);
    return Init_BearState_right_arm(msg_);
  }

private:
  ::themis_state_msgs::msg::BearState msg_;
};

class Init_BearState_head
{
public:
  Init_BearState_head()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_BearState_left_arm head(::themis_state_msgs::msg::BearState::_head_type arg)
  {
    msg_.head = std::move(arg);
    return Init_BearState_left_arm(msg_);
  }

private:
  ::themis_state_msgs::msg::BearState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::BearState>()
{
  return themis_state_msgs::msg::builder::Init_BearState_head();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__BUILDER_HPP_
