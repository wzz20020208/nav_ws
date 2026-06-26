// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/FootPrintArray.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "themis_state_msgs/msg/detail/foot_print_array__struct.hpp"
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

void FootPrintArray_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) themis_state_msgs::msg::FootPrintArray(_init);
}

void FootPrintArray_fini_function(void * message_memory)
{
  auto typed_message = static_cast<themis_state_msgs::msg::FootPrintArray *>(message_memory);
  typed_message->~FootPrintArray();
}

size_t size_function__FootPrintArray__current_foot_print(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  return member->size();
}

const void * get_const_function__FootPrintArray__current_foot_print(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  return &member[index];
}

void * get_function__FootPrintArray__current_foot_print(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  return &member[index];
}

void fetch_function__FootPrintArray__current_foot_print(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const themis_state_msgs::msg::FootArray *>(
    get_const_function__FootPrintArray__current_foot_print(untyped_member, index));
  auto & value = *reinterpret_cast<themis_state_msgs::msg::FootArray *>(untyped_value);
  value = item;
}

void assign_function__FootPrintArray__current_foot_print(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<themis_state_msgs::msg::FootArray *>(
    get_function__FootPrintArray__current_foot_print(untyped_member, index));
  const auto & value = *reinterpret_cast<const themis_state_msgs::msg::FootArray *>(untyped_value);
  item = value;
}

void resize_function__FootPrintArray__current_foot_print(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  member->resize(size);
}

size_t size_function__FootPrintArray__goal_foot_print(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  return member->size();
}

const void * get_const_function__FootPrintArray__goal_foot_print(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  return &member[index];
}

void * get_function__FootPrintArray__goal_foot_print(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  return &member[index];
}

void fetch_function__FootPrintArray__goal_foot_print(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const themis_state_msgs::msg::FootArray *>(
    get_const_function__FootPrintArray__goal_foot_print(untyped_member, index));
  auto & value = *reinterpret_cast<themis_state_msgs::msg::FootArray *>(untyped_value);
  value = item;
}

void assign_function__FootPrintArray__goal_foot_print(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<themis_state_msgs::msg::FootArray *>(
    get_function__FootPrintArray__goal_foot_print(untyped_member, index));
  const auto & value = *reinterpret_cast<const themis_state_msgs::msg::FootArray *>(untyped_value);
  item = value;
}

void resize_function__FootPrintArray__goal_foot_print(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<themis_state_msgs::msg::FootArray> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember FootPrintArray_message_member_array[2] = {
  {
    "current_foot_print",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<themis_state_msgs::msg::FootArray>(),  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::FootPrintArray, current_foot_print),  // bytes offset in struct
    nullptr,  // default value
    size_function__FootPrintArray__current_foot_print,  // size() function pointer
    get_const_function__FootPrintArray__current_foot_print,  // get_const(index) function pointer
    get_function__FootPrintArray__current_foot_print,  // get(index) function pointer
    fetch_function__FootPrintArray__current_foot_print,  // fetch(index, &value) function pointer
    assign_function__FootPrintArray__current_foot_print,  // assign(index, value) function pointer
    resize_function__FootPrintArray__current_foot_print  // resize(index) function pointer
  },
  {
    "goal_foot_print",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<themis_state_msgs::msg::FootArray>(),  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::FootPrintArray, goal_foot_print),  // bytes offset in struct
    nullptr,  // default value
    size_function__FootPrintArray__goal_foot_print,  // size() function pointer
    get_const_function__FootPrintArray__goal_foot_print,  // get_const(index) function pointer
    get_function__FootPrintArray__goal_foot_print,  // get(index) function pointer
    fetch_function__FootPrintArray__goal_foot_print,  // fetch(index, &value) function pointer
    assign_function__FootPrintArray__goal_foot_print,  // assign(index, value) function pointer
    resize_function__FootPrintArray__goal_foot_print  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers FootPrintArray_message_members = {
  "themis_state_msgs::msg",  // message namespace
  "FootPrintArray",  // message name
  2,  // number of fields
  sizeof(themis_state_msgs::msg::FootPrintArray),
  FootPrintArray_message_member_array,  // message members
  FootPrintArray_init_function,  // function to initialize message memory (memory has to be allocated)
  FootPrintArray_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t FootPrintArray_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &FootPrintArray_message_members,
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
get_message_type_support_handle<themis_state_msgs::msg::FootPrintArray>()
{
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::FootPrintArray_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, themis_state_msgs, msg, FootPrintArray)() {
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::FootPrintArray_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
