// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/FootArray.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "themis_state_msgs/msg/detail/foot_array__struct.hpp"
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

void FootArray_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) themis_state_msgs::msg::FootArray(_init);
}

void FootArray_fini_function(void * message_memory)
{
  auto typed_message = static_cast<themis_state_msgs::msg::FootArray *>(message_memory);
  typed_message->~FootArray();
}

size_t size_function__FootArray__feet(const void * untyped_member)
{
  const auto * member = reinterpret_cast<const std::vector<themis_state_msgs::msg::Foot> *>(untyped_member);
  return member->size();
}

const void * get_const_function__FootArray__feet(const void * untyped_member, size_t index)
{
  const auto & member =
    *reinterpret_cast<const std::vector<themis_state_msgs::msg::Foot> *>(untyped_member);
  return &member[index];
}

void * get_function__FootArray__feet(void * untyped_member, size_t index)
{
  auto & member =
    *reinterpret_cast<std::vector<themis_state_msgs::msg::Foot> *>(untyped_member);
  return &member[index];
}

void fetch_function__FootArray__feet(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const auto & item = *reinterpret_cast<const themis_state_msgs::msg::Foot *>(
    get_const_function__FootArray__feet(untyped_member, index));
  auto & value = *reinterpret_cast<themis_state_msgs::msg::Foot *>(untyped_value);
  value = item;
}

void assign_function__FootArray__feet(
  void * untyped_member, size_t index, const void * untyped_value)
{
  auto & item = *reinterpret_cast<themis_state_msgs::msg::Foot *>(
    get_function__FootArray__feet(untyped_member, index));
  const auto & value = *reinterpret_cast<const themis_state_msgs::msg::Foot *>(untyped_value);
  item = value;
}

void resize_function__FootArray__feet(void * untyped_member, size_t size)
{
  auto * member =
    reinterpret_cast<std::vector<themis_state_msgs::msg::Foot> *>(untyped_member);
  member->resize(size);
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember FootArray_message_member_array[1] = {
  {
    "feet",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    ::rosidl_typesupport_introspection_cpp::get_message_type_support_handle<themis_state_msgs::msg::Foot>(),  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::FootArray, feet),  // bytes offset in struct
    nullptr,  // default value
    size_function__FootArray__feet,  // size() function pointer
    get_const_function__FootArray__feet,  // get_const(index) function pointer
    get_function__FootArray__feet,  // get(index) function pointer
    fetch_function__FootArray__feet,  // fetch(index, &value) function pointer
    assign_function__FootArray__feet,  // assign(index, value) function pointer
    resize_function__FootArray__feet  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers FootArray_message_members = {
  "themis_state_msgs::msg",  // message namespace
  "FootArray",  // message name
  1,  // number of fields
  sizeof(themis_state_msgs::msg::FootArray),
  FootArray_message_member_array,  // message members
  FootArray_init_function,  // function to initialize message memory (memory has to be allocated)
  FootArray_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t FootArray_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &FootArray_message_members,
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
get_message_type_support_handle<themis_state_msgs::msg::FootArray>()
{
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::FootArray_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, themis_state_msgs, msg, FootArray)() {
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::FootArray_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
