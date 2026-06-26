// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/ThreadState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/thread_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_ThreadState_posture
{
public:
  explicit Init_ThreadState_posture(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::ThreadState posture(::themis_state_msgs::msg::ThreadState::_posture_type arg)
  {
    msg_.posture = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_data
{
public:
  explicit Init_ThreadState_data(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_posture data(::themis_state_msgs::msg::ThreadState::_data_type arg)
  {
    msg_.data = std::move(arg);
    return Init_ThreadState_posture(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_command
{
public:
  explicit Init_ThreadState_command(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_data command(::themis_state_msgs::msg::ThreadState::_command_type arg)
  {
    msg_.command = std::move(arg);
    return Init_ThreadState_data(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_locomotion
{
public:
  explicit Init_ThreadState_locomotion(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_command locomotion(::themis_state_msgs::msg::ThreadState::_locomotion_type arg)
  {
    msg_.locomotion = std::move(arg);
    return Init_ThreadState_command(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_control
{
public:
  explicit Init_ThreadState_control(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_locomotion control(::themis_state_msgs::msg::ThreadState::_control_type arg)
  {
    msg_.control = std::move(arg);
    return Init_ThreadState_locomotion(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_estimation
{
public:
  explicit Init_ThreadState_estimation(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_control estimation(::themis_state_msgs::msg::ThreadState::_estimation_type arg)
  {
    msg_.estimation = std::move(arg);
    return Init_ThreadState_control(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_sense
{
public:
  explicit Init_ThreadState_sense(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_estimation sense(::themis_state_msgs::msg::ThreadState::_sense_type arg)
  {
    msg_.sense = std::move(arg);
    return Init_ThreadState_estimation(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_dxl_left_hand
{
public:
  explicit Init_ThreadState_dxl_left_hand(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_sense dxl_left_hand(::themis_state_msgs::msg::ThreadState::_dxl_left_hand_type arg)
  {
    msg_.dxl_left_hand = std::move(arg);
    return Init_ThreadState_sense(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_dxl_right_hand
{
public:
  explicit Init_ThreadState_dxl_right_hand(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_dxl_left_hand dxl_right_hand(::themis_state_msgs::msg::ThreadState::_dxl_right_hand_type arg)
  {
    msg_.dxl_right_hand = std::move(arg);
    return Init_ThreadState_dxl_left_hand(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_bear_head
{
public:
  explicit Init_ThreadState_bear_head(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_dxl_right_hand bear_head(::themis_state_msgs::msg::ThreadState::_bear_head_type arg)
  {
    msg_.bear_head = std::move(arg);
    return Init_ThreadState_dxl_right_hand(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_bear_left_arm
{
public:
  explicit Init_ThreadState_bear_left_arm(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_bear_head bear_left_arm(::themis_state_msgs::msg::ThreadState::_bear_left_arm_type arg)
  {
    msg_.bear_left_arm = std::move(arg);
    return Init_ThreadState_bear_head(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_bear_right_arm
{
public:
  explicit Init_ThreadState_bear_right_arm(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_bear_left_arm bear_right_arm(::themis_state_msgs::msg::ThreadState::_bear_right_arm_type arg)
  {
    msg_.bear_right_arm = std::move(arg);
    return Init_ThreadState_bear_left_arm(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_bear_left_leg
{
public:
  explicit Init_ThreadState_bear_left_leg(::themis_state_msgs::msg::ThreadState & msg)
  : msg_(msg)
  {}
  Init_ThreadState_bear_right_arm bear_left_leg(::themis_state_msgs::msg::ThreadState::_bear_left_leg_type arg)
  {
    msg_.bear_left_leg = std::move(arg);
    return Init_ThreadState_bear_right_arm(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

class Init_ThreadState_bear_right_leg
{
public:
  Init_ThreadState_bear_right_leg()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_ThreadState_bear_left_leg bear_right_leg(::themis_state_msgs::msg::ThreadState::_bear_right_leg_type arg)
  {
    msg_.bear_right_leg = std::move(arg);
    return Init_ThreadState_bear_left_leg(msg_);
  }

private:
  ::themis_state_msgs::msg::ThreadState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::ThreadState>()
{
  return themis_state_msgs::msg::builder::Init_ThreadState_bear_right_leg();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__BUILDER_HPP_
