// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/BatteryState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/battery_state__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_BatteryState_present_currents
{
public:
  explicit Init_BatteryState_present_currents(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::BatteryState present_currents(::themis_state_msgs::msg::BatteryState::_present_currents_type arg)
  {
    msg_.present_currents = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_max_charge_currents
{
public:
  explicit Init_BatteryState_max_charge_currents(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_present_currents max_charge_currents(::themis_state_msgs::msg::BatteryState::_max_charge_currents_type arg)
  {
    msg_.max_charge_currents = std::move(arg);
    return Init_BatteryState_present_currents(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_max_discharge_currents
{
public:
  explicit Init_BatteryState_max_discharge_currents(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_max_charge_currents max_discharge_currents(::themis_state_msgs::msg::BatteryState::_max_discharge_currents_type arg)
  {
    msg_.max_discharge_currents = std::move(arg);
    return Init_BatteryState_max_charge_currents(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_charge_statuses
{
public:
  explicit Init_BatteryState_charge_statuses(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_max_discharge_currents charge_statuses(::themis_state_msgs::msg::BatteryState::_charge_statuses_type arg)
  {
    msg_.charge_statuses = std::move(arg);
    return Init_BatteryState_max_discharge_currents(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_discharge_statuses
{
public:
  explicit Init_BatteryState_discharge_statuses(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_charge_statuses discharge_statuses(::themis_state_msgs::msg::BatteryState::_discharge_statuses_type arg)
  {
    msg_.discharge_statuses = std::move(arg);
    return Init_BatteryState_charge_statuses(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_temperatures
{
public:
  explicit Init_BatteryState_temperatures(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_discharge_statuses temperatures(::themis_state_msgs::msg::BatteryState::_temperatures_type arg)
  {
    msg_.temperatures = std::move(arg);
    return Init_BatteryState_discharge_statuses(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_cell_voltages
{
public:
  explicit Init_BatteryState_cell_voltages(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_temperatures cell_voltages(::themis_state_msgs::msg::BatteryState::_cell_voltages_type arg)
  {
    msg_.cell_voltages = std::move(arg);
    return Init_BatteryState_temperatures(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_battery_voltages
{
public:
  explicit Init_BatteryState_battery_voltages(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_cell_voltages battery_voltages(::themis_state_msgs::msg::BatteryState::_battery_voltages_type arg)
  {
    msg_.battery_voltages = std::move(arg);
    return Init_BatteryState_cell_voltages(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_error_state
{
public:
  explicit Init_BatteryState_error_state(::themis_state_msgs::msg::BatteryState & msg)
  : msg_(msg)
  {}
  Init_BatteryState_battery_voltages error_state(::themis_state_msgs::msg::BatteryState::_error_state_type arg)
  {
    msg_.error_state = std::move(arg);
    return Init_BatteryState_battery_voltages(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

class Init_BatteryState_battery_state
{
public:
  Init_BatteryState_battery_state()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_BatteryState_error_state battery_state(::themis_state_msgs::msg::BatteryState::_battery_state_type arg)
  {
    msg_.battery_state = std::move(arg);
    return Init_BatteryState_error_state(msg_);
  }

private:
  ::themis_state_msgs::msg::BatteryState msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::BatteryState>()
{
  return themis_state_msgs::msg::builder::Init_BatteryState_battery_state();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__BUILDER_HPP_
