// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/BearTemperature.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/bear_temperature__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_BearTemperature_head_bear_temperature
{
public:
  explicit Init_BearTemperature_head_bear_temperature(::themis_state_msgs::msg::BearTemperature & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::BearTemperature head_bear_temperature(::themis_state_msgs::msg::BearTemperature::_head_bear_temperature_type arg)
  {
    msg_.head_bear_temperature = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::BearTemperature msg_;
};

class Init_BearTemperature_left_arm_bear_temperature
{
public:
  explicit Init_BearTemperature_left_arm_bear_temperature(::themis_state_msgs::msg::BearTemperature & msg)
  : msg_(msg)
  {}
  Init_BearTemperature_head_bear_temperature left_arm_bear_temperature(::themis_state_msgs::msg::BearTemperature::_left_arm_bear_temperature_type arg)
  {
    msg_.left_arm_bear_temperature = std::move(arg);
    return Init_BearTemperature_head_bear_temperature(msg_);
  }

private:
  ::themis_state_msgs::msg::BearTemperature msg_;
};

class Init_BearTemperature_right_arm_bear_temperature
{
public:
  explicit Init_BearTemperature_right_arm_bear_temperature(::themis_state_msgs::msg::BearTemperature & msg)
  : msg_(msg)
  {}
  Init_BearTemperature_left_arm_bear_temperature right_arm_bear_temperature(::themis_state_msgs::msg::BearTemperature::_right_arm_bear_temperature_type arg)
  {
    msg_.right_arm_bear_temperature = std::move(arg);
    return Init_BearTemperature_left_arm_bear_temperature(msg_);
  }

private:
  ::themis_state_msgs::msg::BearTemperature msg_;
};

class Init_BearTemperature_left_leg_bear_temperature
{
public:
  explicit Init_BearTemperature_left_leg_bear_temperature(::themis_state_msgs::msg::BearTemperature & msg)
  : msg_(msg)
  {}
  Init_BearTemperature_right_arm_bear_temperature left_leg_bear_temperature(::themis_state_msgs::msg::BearTemperature::_left_leg_bear_temperature_type arg)
  {
    msg_.left_leg_bear_temperature = std::move(arg);
    return Init_BearTemperature_right_arm_bear_temperature(msg_);
  }

private:
  ::themis_state_msgs::msg::BearTemperature msg_;
};

class Init_BearTemperature_right_leg_bear_temperature
{
public:
  Init_BearTemperature_right_leg_bear_temperature()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_BearTemperature_left_leg_bear_temperature right_leg_bear_temperature(::themis_state_msgs::msg::BearTemperature::_right_leg_bear_temperature_type arg)
  {
    msg_.right_leg_bear_temperature = std::move(arg);
    return Init_BearTemperature_left_leg_bear_temperature(msg_);
  }

private:
  ::themis_state_msgs::msg::BearTemperature msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::BearTemperature>()
{
  return themis_state_msgs::msg::builder::Init_BearTemperature_right_leg_bear_temperature();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__BUILDER_HPP_
