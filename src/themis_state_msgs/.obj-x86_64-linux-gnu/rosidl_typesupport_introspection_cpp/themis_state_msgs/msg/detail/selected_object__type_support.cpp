// generated from rosidl_typesupport_introspection_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/SelectedObject.idl
// generated code does not contain a copyright notice

#include "array"
#include "cstddef"
#include "string"
#include "vector"
#include "rosidl_runtime_c/message_type_support_struct.h"
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_interface/macros.h"
#include "themis_state_msgs/msg/detail/selected_object__struct.hpp"
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

void SelectedObject_init_function(
  void * message_memory, rosidl_runtime_cpp::MessageInitialization _init)
{
  new (message_memory) themis_state_msgs::msg::SelectedObject(_init);
}

void SelectedObject_fini_function(void * message_memory)
{
  auto typed_message = static_cast<themis_state_msgs::msg::SelectedObject *>(message_memory);
  typed_message->~SelectedObject();
}

static const ::rosidl_typesupport_introspection_cpp::MessageMember SelectedObject_message_member_array[2] = {
  {
    "id",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::SelectedObject, id),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  },
  {
    "category",  // name
    ::rosidl_typesupport_introspection_cpp::ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    nullptr,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs::msg::SelectedObject, category),  // bytes offset in struct
    nullptr,  // default value
    nullptr,  // size() function pointer
    nullptr,  // get_const(index) function pointer
    nullptr,  // get(index) function pointer
    nullptr,  // fetch(index, &value) function pointer
    nullptr,  // assign(index, value) function pointer
    nullptr  // resize(index) function pointer
  }
};

static const ::rosidl_typesupport_introspection_cpp::MessageMembers SelectedObject_message_members = {
  "themis_state_msgs::msg",  // message namespace
  "SelectedObject",  // message name
  2,  // number of fields
  sizeof(themis_state_msgs::msg::SelectedObject),
  SelectedObject_message_member_array,  // message members
  SelectedObject_init_function,  // function to initialize message memory (memory has to be allocated)
  SelectedObject_fini_function  // function to terminate message instance (will not free memory)
};

static const rosidl_message_type_support_t SelectedObject_message_type_support_handle = {
  ::rosidl_typesupport_introspection_cpp::typesupport_identifier,
  &SelectedObject_message_members,
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
get_message_type_support_handle<themis_state_msgs::msg::SelectedObject>()
{
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::SelectedObject_message_type_support_handle;
}

}  // namespace rosidl_typesupport_introspection_cpp

#ifdef __cplusplus
extern "C"
{
#endif

ROSIDL_TYPESUPPORT_INTROSPECTION_CPP_PUBLIC
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_cpp, themis_state_msgs, msg, SelectedObject)() {
  return &::themis_state_msgs::msg::rosidl_typesupport_introspection_cpp::SelectedObject_message_type_support_handle;
}

#ifdef __cplusplus
}
#endif
