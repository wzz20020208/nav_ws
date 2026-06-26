// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/BearTemperature.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "themis_state_msgs/msg/detail/bear_temperature__struct.hpp"
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

void BearTemperature_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) themis_state_msgs::msg::BearTemperature(_init);
}

void BearTemperature_fini_function(void * message_memory)
{
  auto typed_message = static_cast<themis_state_msgs::msg::BearTemperature *>(message_memory);
  typed_message->~BearTemperature();
}

size_t size_function__BearTemperature__right_leg_bear_temperature(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearTemperature__right_leg_bear_temperature(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BearTemperature__right_leg_bear_temperature(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearTemperature__right_leg_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BearTemperature__right_leg_bear_temperature(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BearTemperature__right_leg_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BearTemperature__right_leg_bear_temperature(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BearTemperature__right_leg_bear_temperature(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearTemperature__left_leg_bear_temperature(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearTemperature__left_leg_bear_temperature(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BearTemperature__left_leg_bear_temperature(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearTemperature__left_leg_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BearTemperature__left_leg_bear_temperature(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BearTemperature__left_leg_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BearTemperature__left_leg_bear_temperature(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BearTemperature__left_leg_bear_temperature(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearTemperature__right_arm_bear_temperature(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearTemperature__right_arm_bear_temperature(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BearTemperature__right_arm_bear_temperature(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearTemperature__right_arm_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BearTemperature__right_arm_bear_temperature(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BearTemperature__right_arm_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BearTemperature__right_arm_bear_temperature(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BearTemperature__right_arm_bear_temperature(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearTemperature__left_arm_bear_temperature(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearTemperature__left_arm_bear_temperature(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BearTemperature__left_arm_bear_temperature(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearTemperature__left_arm_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BearTemperature__left_arm_bear_temperature(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BearTemperature__left_arm_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BearTemperature__left_arm_bear_temperature(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BearTemperature__left_arm_bear_temperature(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearTemperature__head_bear_temperature(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<double> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearTemperature__head_bear_temperature(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<double> *>(untyped_member);
  return &member[index];
}

void * get_function__BearTemperature__head_bear_temperature(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<double> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearTemperature__head_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const double *>(
    get_const_function__BearTemperature__head_bear_temperature(untyped_member, index));
  auto & value = *reinterpret_cast<double *>(untyped_value);
  value = item;
}

void assign_function__BearTemperature__head_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<double *>(
    get_function__BearTemperature__head_bear_temperature(untyped_member, index));
  const auto & value = *reinterpret_cast<const double *>(untyped_value);
  item = value;
}

void resize_function__BearTemperature__head_bear_temperature(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<double> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember BearTemperature_message_member_array[5] = {
  {
    "right_leg_bear_temperature",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearTemperature, right_leg_bear_temperature),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearTemperature__right_leg_bear_temperature,  // size() function pointer
    get_const_function__BearTemperature__right_leg_bear_temperature,  // get_const(index) function pointer
    get_function__BearTemperature__right_leg_bear_temperature,  // get(index) function pointer
    fetch_function__BearTemperature__right_leg_bear_temperature,  // fetch(index, &value) function pointer
    assign_function__BearTemperature__right_leg_bear_temperature,  // assign(index, value) function pointer
    resize_function__BearTemperature__right_leg_bear_temperature  // resize(index) function pointer
  },
  {
    "left_leg_bear_temperature",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearTemperature, left_leg_bear_temperature),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearTemperature__left_leg_bear_temperature,  // size() function pointer
    get_const_function__BearTemperature__left_leg_bear_temperature,  // get_const(index) function pointer
    get_function__BearTemperature__left_leg_bear_temperature,  // get(index) function pointer
    fetch_function__BearTemperature__left_leg_bear_temperature,  // fetch(index, &value) function pointer
    assign_function__BearTemperature__left_leg_bear_temperature,  // assign(index, value) function pointer
    resize_function__BearTemperature__left_leg_bear_temperature  // resize(index) function pointer
  },
  {
    "right_arm_bear_temperature",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearTemperature, right_arm_bear_temperature),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearTemperature__right_arm_bear_temperature,  // size() function pointer
    get_const_function__BearTemperature__right_arm_bear_temperature,  // get_const(index) function pointer
    get_function__BearTemperature__right_arm_bear_temperature,  // get(index) function pointer
    fetch_function__BearTemperature__right_arm_bear_temperature,  // fetch(index, &value) function pointer
    assign_function__BearTemperature__right_arm_bear_temperature,  // assign(index, value) function pointer
    resize_function__BearTemperature__right_arm_bear_temperature  // resize(index) function pointer
  },
  {
    "left_arm_bear_temperature",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearTemperature, left_arm_bear_temperature),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearTemperature__left_arm_bear_temperature,  // size() function pointer
    get_const_function__BearTemperature__left_arm_bear_temperature,  // get_const(index) function pointer
    get_function__BearTemperature__left_arm_bear_temperature,  // get(index) function pointer
    fetch_function__BearTemperature__left_arm_bear_temperature,  // fetch(index, &value) function pointer
    assign_function__BearTemperature__left_arm_bear_temperature,  // assign(index, value) function pointer
    resize_function__BearTemperature__left_arm_bear_temperature  // resize(index) function pointer
  },
  {
    "head_bear_temperature",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearTemperature, head_bear_temperature),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearTemperature__head_bear_temperature,  // size() function pointer
    get_const_function__BearTemperature__head_bear_temperature,  // get_const(index) function pointer
    get_function__BearTemperature__head_bear_temperature,  // get(index) function pointer
    fetch_function__BearTemperature__head_bear_temperature,  // fetch(index, &value) function pointer
    assign_function__BearTemperature__head_bear_temperature,  // assign(index, value) function pointer
    resize_function__BearTemperature__head_bear_temperature  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers BearTemperature_message_members = {
  "themis_state_msgs::msg",  // message namespace
  "BearTemperature",  // message name
  5,  // number of fields
  sizeof(themis_state_msgs::msg::BearTemperature),
  BearTemperature_message_member_array,  // message members
  BearTemperature_init_function,  // function to initialize message memory (memory has to be allocated)
  BearTemperature_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t BearTemperature_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &BearTemperature_message_members,
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
get_message_type_support_handle<themis_state_msgs::msg::BearTemperature>()
{
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::BearTemperature_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, themis_state_msgs, msg, BearTemperature)() {
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::BearTemperature_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
