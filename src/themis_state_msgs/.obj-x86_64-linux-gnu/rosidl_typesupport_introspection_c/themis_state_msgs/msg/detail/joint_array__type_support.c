// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from themis_state_msgs:msg/JointArray.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "themis_state_msgs/msg/detail/joint_array__rosidl_typesupport_introspection_c.h"
#include "themis_state_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "themis_state_msgs/msg/detail/joint_array__functions.h"
#include "themis_state_msgs/msg/detail/joint_array__struct.h"


// Include directives for member types
// Member `position`
// Member `velocity`
// Member `torque`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  themis_state_msgs__msg__JointArray__init(message_memory);
}

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_fini_function(void * message_memory)
{
  themis_state_msgs__msg__JointArray__fini(message_memory);
}

size_t themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__size_function__JointArray__position(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__position(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__position(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__fetch_function__JointArray__position(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__position(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__assign_function__JointArray__position(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__position(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__resize_function__JointArray__position(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__size_function__JointArray__velocity(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__velocity(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__velocity(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__fetch_function__JointArray__velocity(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__velocity(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__assign_function__JointArray__velocity(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__velocity(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__resize_function__JointArray__velocity(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__size_function__JointArray__torque(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__torque(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__torque(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__fetch_function__JointArray__torque(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__torque(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__assign_function__JointArray__torque(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__torque(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__resize_function__JointArray__torque(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_member_array[3] = {
  {
    "position",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__JointArray, position),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__size_function__JointArray__position,  // size() function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__position,  // get_const(index) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__position,  // get(index) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__fetch_function__JointArray__position,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__assign_function__JointArray__position,  // assign(index, value) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__resize_function__JointArray__position  // resize(index) function pointer
  },
  {
    "velocity",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__JointArray, velocity),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__size_function__JointArray__velocity,  // size() function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__velocity,  // get_const(index) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__velocity,  // get(index) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__fetch_function__JointArray__velocity,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__assign_function__JointArray__velocity,  // assign(index, value) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__resize_function__JointArray__velocity  // resize(index) function pointer
  },
  {
    "torque",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__JointArray, torque),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__size_function__JointArray__torque,  // size() function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_const_function__JointArray__torque,  // get_const(index) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__get_function__JointArray__torque,  // get(index) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__fetch_function__JointArray__torque,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__assign_function__JointArray__torque,  // assign(index, value) function pointer
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__resize_function__JointArray__torque  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_members = {
  "themis_state_msgs__msg",  // message namespace
  "JointArray",  // message name
  3,  // number of fields
  sizeof(themis_state_msgs__msg__JointArray),
  themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_member_array,  // message members
  themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_init_function,  // function to initialize message memory (memory has to be allocated)
  themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_type_support_handle = {
  0,
  &themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)() {
  if (!themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_type_support_handle.typesupport_identifier) {
    themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &themis_state_msgs__msg__JointArray__rosidl_typesupport_introspection_c__JointArray_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
