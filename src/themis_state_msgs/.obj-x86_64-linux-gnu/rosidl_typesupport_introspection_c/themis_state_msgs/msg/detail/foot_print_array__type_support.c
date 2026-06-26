// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from themis_state_msgs:msg/FootPrintArray.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "themis_state_msgs/msg/detail/foot_print_array__rosidl_typesupport_introspection_c.h"
#include "themis_state_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "themis_state_msgs/msg/detail/foot_print_array__functions.h"
#include "themis_state_msgs/msg/detail/foot_print_array__struct.h"


// Include directives for member types
// Member `current_foot_print`
// Member `goal_foot_print`
#include "themis_state_msgs/msg/foot_array.h"
// Member `current_foot_print`
// Member `goal_foot_print`
#include "themis_state_msgs/msg/detail/foot_array__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  themis_state_msgs__msg__FootPrintArray__init(message_memory);
}

void themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_fini_function(void * message_memory)
{
  themis_state_msgs__msg__FootPrintArray__fini(message_memory);
}

size_t themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__size_function__FootPrintArray__current_foot_print(
  const void * untyped_member)
{
  const themis_state_msgs__msg__FootArray__Sequence * member =
    (const themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_const_function__FootPrintArray__current_foot_print(
  const void * untyped_member, size_t index)
{
  const themis_state_msgs__msg__FootArray__Sequence * member =
    (const themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_function__FootPrintArray__current_foot_print(
  void * untyped_member, size_t index)
{
  themis_state_msgs__msg__FootArray__Sequence * member =
    (themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__fetch_function__FootPrintArray__current_foot_print(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const themis_state_msgs__msg__FootArray * item =
    ((const themis_state_msgs__msg__FootArray *)
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_const_function__FootPrintArray__current_foot_print(untyped_member, index));
  themis_state_msgs__msg__FootArray * value =
    (themis_state_msgs__msg__FootArray *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__assign_function__FootPrintArray__current_foot_print(
  void * untyped_member, size_t index, const void * untyped_value)
{
  themis_state_msgs__msg__FootArray * item =
    ((themis_state_msgs__msg__FootArray *)
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_function__FootPrintArray__current_foot_print(untyped_member, index));
  const themis_state_msgs__msg__FootArray * value =
    (const themis_state_msgs__msg__FootArray *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__resize_function__FootPrintArray__current_foot_print(
  void * untyped_member, size_t size)
{
  themis_state_msgs__msg__FootArray__Sequence * member =
    (themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  themis_state_msgs__msg__FootArray__Sequence__fini(member);
  return themis_state_msgs__msg__FootArray__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__size_function__FootPrintArray__goal_foot_print(
  const void * untyped_member)
{
  const themis_state_msgs__msg__FootArray__Sequence * member =
    (const themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_const_function__FootPrintArray__goal_foot_print(
  const void * untyped_member, size_t index)
{
  const themis_state_msgs__msg__FootArray__Sequence * member =
    (const themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_function__FootPrintArray__goal_foot_print(
  void * untyped_member, size_t index)
{
  themis_state_msgs__msg__FootArray__Sequence * member =
    (themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__fetch_function__FootPrintArray__goal_foot_print(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const themis_state_msgs__msg__FootArray * item =
    ((const themis_state_msgs__msg__FootArray *)
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_const_function__FootPrintArray__goal_foot_print(untyped_member, index));
  themis_state_msgs__msg__FootArray * value =
    (themis_state_msgs__msg__FootArray *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__assign_function__FootPrintArray__goal_foot_print(
  void * untyped_member, size_t index, const void * untyped_value)
{
  themis_state_msgs__msg__FootArray * item =
    ((themis_state_msgs__msg__FootArray *)
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_function__FootPrintArray__goal_foot_print(untyped_member, index));
  const themis_state_msgs__msg__FootArray * value =
    (const themis_state_msgs__msg__FootArray *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__resize_function__FootPrintArray__goal_foot_print(
  void * untyped_member, size_t size)
{
  themis_state_msgs__msg__FootArray__Sequence * member =
    (themis_state_msgs__msg__FootArray__Sequence *)(untyped_member);
  themis_state_msgs__msg__FootArray__Sequence__fini(member);
  return themis_state_msgs__msg__FootArray__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_member_array[2] = {
  {
    "current_foot_print",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__FootPrintArray, current_foot_print),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__size_function__FootPrintArray__current_foot_print,  // size() function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_const_function__FootPrintArray__current_foot_print,  // get_const(index) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_function__FootPrintArray__current_foot_print,  // get(index) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__fetch_function__FootPrintArray__current_foot_print,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__assign_function__FootPrintArray__current_foot_print,  // assign(index, value) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__resize_function__FootPrintArray__current_foot_print  // resize(index) function pointer
  },
  {
    "goal_foot_print",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__FootPrintArray, goal_foot_print),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__size_function__FootPrintArray__goal_foot_print,  // size() function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_const_function__FootPrintArray__goal_foot_print,  // get_const(index) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__get_function__FootPrintArray__goal_foot_print,  // get(index) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__fetch_function__FootPrintArray__goal_foot_print,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__assign_function__FootPrintArray__goal_foot_print,  // assign(index, value) function pointer
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__resize_function__FootPrintArray__goal_foot_print  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_members = {
  "themis_state_msgs__msg",  // message namespace
  "FootPrintArray",  // message name
  2,  // number of fields
  sizeof(themis_state_msgs__msg__FootPrintArray),
  themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_member_array,  // message members
  themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_init_function,  // function to initialize message memory (memory has to be allocated)
  themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_type_support_handle = {
  0,
  &themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, FootPrintArray)() {
  themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, FootArray)();
  themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, FootArray)();
  if (!themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_type_support_handle.typesupport_identifier) {
    themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &themis_state_msgs__msg__FootPrintArray__rosidl_typesupport_introspection_c__FootPrintArray_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
