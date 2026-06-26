// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/BearArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/bear_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_BearArray_errors
{
public:
  explicit Init_BearArray_errors(::themis_state_msgs::msg::BearArray & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::BearArray errors(::themis_state_msgs::msg::BearArray::_errors_type arg)
  {
    msg_.errors = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::BearArray msg_;
};

class Init_BearArray_voltages
{
public:
  explicit Init_BearArray_voltages(::themis_state_msgs::msg::BearArray & msg)
  : msg_(msg)
  {}
  Init_BearArray_errors voltages(::themis_state_msgs::msg::BearArray::_voltages_type arg)
  {
    msg_.voltages = std::move(arg);
    return Init_BearArray_errors(msg_);
  }

private:
  ::themis_state_msgs::msg::BearArray msg_;
};

class Init_BearArray_temperatures
{
public:
  explicit Init_BearArray_temperatures(::themis_state_msgs::msg::BearArray & msg)
  : msg_(msg)
  {}
  Init_BearArray_voltages temperatures(::themis_state_msgs::msg::BearArray::_temperatures_type arg)
  {
    msg_.temperatures = std::move(arg);
    return Init_BearArray_voltages(msg_);
  }

private:
  ::themis_state_msgs::msg::BearArray msg_;
};

class Init_BearArray_currents
{
public:
  explicit Init_BearArray_currents(::themis_state_msgs::msg::BearArray & msg)
  : msg_(msg)
  {}
  Init_BearArray_temperatures currents(::themis_state_msgs::msg::BearArray::_currents_type arg)
  {
    msg_.currents = std::move(arg);
    return Init_BearArray_temperatures(msg_);
  }

private:
  ::themis_state_msgs::msg::BearArray msg_;
};

class Init_BearArray_velocities
{
public:
  explicit Init_BearArray_velocities(::themis_state_msgs::msg::BearArray & msg)
  : msg_(msg)
  {}
  Init_BearArray_currents velocities(::themis_state_msgs::msg::BearArray::_velocities_type arg)
  {
    msg_.velocities = std::move(arg);
    return Init_BearArray_currents(msg_);
  }

private:
  ::themis_state_msgs::msg::BearArray msg_;
};

class Init_BearArray_positions
{
public:
  Init_BearArray_positions()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_BearArray_velocities positions(::themis_state_msgs::msg::BearArray::_positions_type arg)
  {
    msg_.positions = std::move(arg);
    return Init_BearArray_velocities(msg_);
  }

private:
  ::themis_state_msgs::msg::BearArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::BearArray>()
{
  return themis_state_msgs::msg::builder::Init_BearArray_positions();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__BUILDER_HPP_
