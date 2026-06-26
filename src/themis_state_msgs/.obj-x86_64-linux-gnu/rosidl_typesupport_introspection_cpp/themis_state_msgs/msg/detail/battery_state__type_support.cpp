// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/BatteryState.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "themis_state_msgs/msg/detail/battery_state__struct.hpp"
#include "rosidl_typesupport_introspection_cpp/field_types.hpp"
#include "rosidl_typesupport_introspection_cpp/identifier.hpp"
#include "rosidl_typesupport_introspection_cpp/message_introspection.hpp"
#include "rosidl_typesupport_introspection_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_introspection_cpp/visibility_control.h"

namespace themis_state_msgs
{

namespace msg
{

namespace rosidl_typesupport_introspection_cpp
{

void BatteryState_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) themis_state_msgs::msg::BatteryState(_init);
}

void BatteryState_fini_function(void * message_memory)
{
  auto typed_message = static_cast<themis_state_msgs::msg::BatteryState *>(message_memory);
  typed_message->~BatteryState();
}

size_t size_function__BatteryState__battery_state(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__battery_state(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__battery_state(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__battery_state(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__battery_state(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__battery_state(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__battery_state(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__battery_state(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__error_state(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__error_state(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__error_state(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__error_state(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__error_state(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__error_state(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__error_state(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__error_state(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__battery_voltages(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__battery_voltages(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__battery_voltages(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__battery_voltages(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__battery_voltages(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__battery_voltages(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__battery_voltages(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__battery_voltages(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__cell_voltages(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__cell_voltages(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__cell_voltages(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__cell_voltages(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__cell_voltages(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__cell_voltages(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__cell_voltages(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__cell_voltages(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__temperatures(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__temperatures(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__temperatures(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__temperatures(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__temperatures(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__temperatures(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__temperatures(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__temperatures(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__discharge_statuses(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__discharge_statuses(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__discharge_statuses(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__discharge_statuses(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__discharge_statuses(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__discharge_statuses(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__discharge_statuses(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__discharge_statuses(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__charge_statuses(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__charge_statuses(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__charge_statuses(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__charge_statuses(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__charge_statuses(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__charge_statuses(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__charge_statuses(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__charge_statuses(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__max_discharge_currents(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__max_discharge_currents(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__max_discharge_currents(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__max_discharge_currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__max_discharge_currents(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__max_discharge_currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__max_discharge_currents(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__max_discharge_currents(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__max_charge_currents(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__max_charge_currents(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__max_charge_currents(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__max_charge_currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__max_charge_currents(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__max_charge_currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__max_charge_currents(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__max_charge_currents(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BatteryState__present_currents(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BatteryState__present_currents(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BatteryState__present_currents(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BatteryState__present_currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BatteryState__present_currents(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BatteryState__present_currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BatteryState__present_currents(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BatteryState__present_currents(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember BatteryState_message_member_array[10] = {
  {
    "battery_state",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, battery_state),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__battery_state,  // size() function pointer
    get_const_function__BatteryState__battery_state,  // get_const(index) function pointer
    get_function__BatteryState__battery_state,  // get(index) function pointer
    fetch_function__BatteryState__battery_state,  // fetch(index, &value) function pointer
    assign_function__BatteryState__battery_state,  // assign(index, value) function pointer
    resize_function__BatteryState__battery_state  // resize(index) function pointer
  },
  {
    "error_state",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, error_state),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__error_state,  // size() function pointer
    get_const_function__BatteryState__error_state,  // get_const(index) function pointer
    get_function__BatteryState__error_state,  // get(index) function pointer
    fetch_function__BatteryState__error_state,  // fetch(index, &value) function pointer
    assign_function__BatteryState__error_state,  // assign(index, value) function pointer
    resize_function__BatteryState__error_state  // resize(index) function pointer
  },
  {
    "battery_voltages",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, battery_voltages),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__battery_voltages,  // size() function pointer
    get_const_function__BatteryState__battery_voltages,  // get_const(index) function pointer
    get_function__BatteryState__battery_voltages,  // get(index) function pointer
    fetch_function__BatteryState__battery_voltages,  // fetch(index, &value) function pointer
    assign_function__BatteryState__battery_voltages,  // assign(index, value) function pointer
    resize_function__BatteryState__battery_voltages  // resize(index) function pointer
  },
  {
    "cell_voltages",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, cell_voltages),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__cell_voltages,  // size() function pointer
    get_const_function__BatteryState__cell_voltages,  // get_const(index) function pointer
    get_function__BatteryState__cell_voltages,  // get(index) function pointer
    fetch_function__BatteryState__cell_voltages,  // fetch(index, &value) function pointer
    assign_function__BatteryState__cell_voltages,  // assign(index, value) function pointer
    resize_function__BatteryState__cell_voltages  // resize(index) function pointer
  },
  {
    "temperatures",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, temperatures),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__temperatures,  // size() function pointer
    get_const_function__BatteryState__temperatures,  // get_const(index) function pointer
    get_function__BatteryState__temperatures,  // get(index) function pointer
    fetch_function__BatteryState__temperatures,  // fetch(index, &value) function pointer
    assign_function__BatteryState__temperatures,  // assign(index, value) function pointer
    resize_function__BatteryState__temperatures  // resize(index) function pointer
  },
  {
    "discharge_statuses",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, discharge_statuses),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__discharge_statuses,  // size() function pointer
    get_const_function__BatteryState__discharge_statuses,  // get_const(index) function pointer
    get_function__BatteryState__discharge_statuses,  // get(index) function pointer
    fetch_function__BatteryState__discharge_statuses,  // fetch(index, &value) function pointer
    assign_function__BatteryState__discharge_statuses,  // assign(index, value) function pointer
    resize_function__BatteryState__discharge_statuses  // resize(index) function pointer
  },
  {
    "charge_statuses",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, charge_statuses),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__charge_statuses,  // size() function pointer
    get_const_function__BatteryState__charge_statuses,  // get_const(index) function pointer
    get_function__BatteryState__charge_statuses,  // get(index) function pointer
    fetch_function__BatteryState__charge_statuses,  // fetch(index, &value) function pointer
    assign_function__BatteryState__charge_statuses,  // assign(index, value) function pointer
    resize_function__BatteryState__charge_statuses  // resize(index) function pointer
  },
  {
    "max_discharge_currents",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, max_discharge_currents),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__max_discharge_currents,  // size() function pointer
    get_const_function__BatteryState__max_discharge_currents,  // get_const(index) function pointer
    get_function__BatteryState__max_discharge_currents,  // get(index) function pointer
    fetch_function__BatteryState__max_discharge_currents,  // fetch(index, &value) function pointer
    assign_function__BatteryState__max_discharge_currents,  // assign(index, value) function pointer
    resize_function__BatteryState__max_discharge_currents  // resize(index) function pointer
  },
  {
    "max_charge_currents",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, max_charge_currents),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__max_charge_currents,  // size() function pointer
    get_const_function__BatteryState__max_charge_currents,  // get_const(index) function pointer
    get_function__BatteryState__max_charge_currents,  // get(index) function pointer
    fetch_function__BatteryState__max_charge_currents,  // fetch(index, &value) function pointer
    assign_function__BatteryState__max_charge_currents,  // assign(index, value) function pointer
    resize_function__BatteryState__max_charge_currents  // resize(index) function pointer
  },
  {
    "present_currents",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BatteryState, present_currents),  // bytes offset in struct
    nullptr,  // default value
    size_function__BatteryState__present_currents,  // size() function pointer
    get_const_function__BatteryState__present_currents,  // get_const(index) function pointer
    get_function__BatteryState__present_currents,  // get(index) function pointer
    fetch_function__BatteryState__present_currents,  // fetch(index, &value) function pointer
    assign_function__BatteryState__present_currents,  // assign(index, value) function pointer
    resize_function__BatteryState__present_currents  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers BatteryState_message_members = {
  "themis_state_msgs::msg",  // message namespace
  "BatteryState",  // message name
  10,  // number of fields
  sizeof(themis_state_msgs::msg::BatteryState),
  BatteryState_message_member_array,  // message members
  BatteryState_init_function,  // function to initialize message memory (memory has to be allocated)
  BatteryState_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t BatteryState_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &BatteryState_message_members,
  get_message_typesupport_handle_function,
};

}  // namespace rosidl_typesupport_introspection_cpp

}  // namespace msg

}  // namespace themis_state_msgs


namespace rosidl_typesupport_introspection_cpp
{

template<>
ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
get_message_type_support_handle<themis_state_msgs::msg::BatteryState>()
{
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::BatteryState_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, themis_state_msgs, msg, BatteryState)() {
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::BatteryState_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
