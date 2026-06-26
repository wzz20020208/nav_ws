// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/BearArray.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "themis_state_msgs/msg/detail/bear_array__struct.hpp"
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

void BearArray_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) themis_state_msgs::msg::BearArray(_init);
}

void BearArray_fini_function(void * message_memory)
{
  auto typed_message = static_cast<themis_state_msgs::msg::BearArray *>(message_memory);
  typed_message->~BearArray();
}

size_t size_function__BearArray__positions(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearArray__positions(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__BearArray__positions(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearArray__positions(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__BearArray__positions(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__BearArray__positions(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__BearArray__positions(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__BearArray__positions(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearArray__velocities(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearArray__velocities(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__BearArray__velocities(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearArray__velocities(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__BearArray__velocities(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__BearArray__velocities(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__BearArray__velocities(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__BearArray__velocities(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearArray__currents(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearArray__currents(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__BearArray__currents(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearArray__currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__BearArray__currents(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__BearArray__currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__BearArray__currents(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__BearArray__currents(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearArray__temperatures(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearArray__temperatures(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__BearArray__temperatures(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearArray__temperatures(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__BearArray__temperatures(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__BearArray__temperatures(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__BearArray__temperatures(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__BearArray__temperatures(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearArray__voltages(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearArray__voltages(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__BearArray__voltages(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearArray__voltages(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__BearArray__voltages(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__BearArray__voltages(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__BearArray__voltages(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__BearArray__voltages(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

size_t size_function__BearArray__errors(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<float> *>(untyped_member);
  return member->size();
}

const void * get_const_function__BearArray__errors(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<float> *>(untyped_member);
  return &member[index];
}

void * get_function__BearArray__errors(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<float> *>(untyped_member);
  return &member[index];
}

void fetch_function__BearArray__errors(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const float *>(
    get_const_function__BearArray__errors(untyped_member, index));
  auto & value = *reinterpret_cast<float *>(untyped_value);
  value = item;
}

void assign_function__BearArray__errors(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<float *>(
    get_function__BearArray__errors(untyped_member, index));
  const auto & value = *reinterpret_cast<const float *>(untyped_value);
  item = value;
}

void resize_function__BearArray__errors(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<float> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember BearArray_message_member_array[6] = {
  {
    "positions",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearArray, positions),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearArray__positions,  // size() function pointer
    get_const_function__BearArray__positions,  // get_const(index) function pointer
    get_function__BearArray__positions,  // get(index) function pointer
    fetch_function__BearArray__positions,  // fetch(index, &value) function pointer
    assign_function__BearArray__positions,  // assign(index, value) function pointer
    resize_function__BearArray__positions  // resize(index) function pointer
  },
  {
    "velocities",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearArray, velocities),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearArray__velocities,  // size() function pointer
    get_const_function__BearArray__velocities,  // get_const(index) function pointer
    get_function__BearArray__velocities,  // get(index) function pointer
    fetch_function__BearArray__velocities,  // fetch(index, &value) function pointer
    assign_function__BearArray__velocities,  // assign(index, value) function pointer
    resize_function__BearArray__velocities  // resize(index) function pointer
  },
  {
    "currents",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearArray, currents),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearArray__currents,  // size() function pointer
    get_const_function__BearArray__currents,  // get_const(index) function pointer
    get_function__BearArray__currents,  // get(index) function pointer
    fetch_function__BearArray__currents,  // fetch(index, &value) function pointer
    assign_function__BearArray__currents,  // assign(index, value) function pointer
    resize_function__BearArray__currents  // resize(index) function pointer
  },
  {
    "temperatures",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearArray, temperatures),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearArray__temperatures,  // size() function pointer
    get_const_function__BearArray__temperatures,  // get_const(index) function pointer
    get_function__BearArray__temperatures,  // get(index) function pointer
    fetch_function__BearArray__temperatures,  // fetch(index, &value) function pointer
    assign_function__BearArray__temperatures,  // assign(index, value) function pointer
    resize_function__BearArray__temperatures  // resize(index) function pointer
  },
  {
    "voltages",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearArray, voltages),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearArray__voltages,  // size() function pointer
    get_const_function__BearArray__voltages,  // get_const(index) function pointer
    get_function__BearArray__voltages,  // get(index) function pointer
    fetch_function__BearArray__voltages,  // fetch(index, &value) function pointer
    assign_function__BearArray__voltages,  // assign(index, value) function pointer
    resize_function__BearArray__voltages  // resize(index) function pointer
  },
  {
    "errors",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::BearArray, errors),  // bytes offset in struct
    nullptr,  // default value
    size_function__BearArray__errors,  // size() function pointer
    get_const_function__BearArray__errors,  // get_const(index) function pointer
    get_function__BearArray__errors,  // get(index) function pointer
    fetch_function__BearArray__errors,  // fetch(index, &value) function pointer
    assign_function__BearArray__errors,  // assign(index, value) function pointer
    resize_function__BearArray__errors  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers BearArray_message_members = {
  "themis_state_msgs::msg",  // message namespace
  "BearArray",  // message name
  6,  // number of fields
  sizeof(themis_state_msgs::msg::BearArray),
  BearArray_message_member_array,  // message members
  BearArray_init_function,  // function to initialize message memory (memory has to be allocated)
  BearArray_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t BearArray_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &BearArray_message_members,
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
get_message_type_support_handle<themis_state_msgs::msg::BearArray>()
{
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::BearArray_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, themis_state_msgs, msg, BearArray)() {
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::BearArray_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
