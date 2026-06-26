// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from themis_state_msgs:msg/BearTemperature.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "themis_state_msgs/msg/detail/bear_temperature__rosidl_typesupport_introspection_c.h"
#include "themis_state_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "themis_state_msgs/msg/detail/bear_temperature__functions.h"
#include "themis_state_msgs/msg/detail/bear_temperature__struct.h"


// Include directives for member types
// Member `right_leg_bear_temperature`
// Member `left_leg_bear_temperature`
// Member `right_arm_bear_temperature`
// Member `left_arm_bear_temperature`
// Member `head_bear_temperature`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  themis_state_msgs__msg__BearTemperature__init(message_memory);
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_fini_function(void * message_memory)
{
  themis_state_msgs__msg__BearTemperature__fini(message_memory);
}

size_t themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__right_leg_bear_temperature(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__right_leg_bear_temperature(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__right_leg_bear_temperature(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__right_leg_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__right_leg_bear_temperature(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__right_leg_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__right_leg_bear_temperature(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__right_leg_bear_temperature(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__left_leg_bear_temperature(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__left_leg_bear_temperature(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__left_leg_bear_temperature(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__left_leg_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__left_leg_bear_temperature(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__left_leg_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__left_leg_bear_temperature(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__left_leg_bear_temperature(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__right_arm_bear_temperature(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__right_arm_bear_temperature(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__right_arm_bear_temperature(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__right_arm_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__right_arm_bear_temperature(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__right_arm_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__right_arm_bear_temperature(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__right_arm_bear_temperature(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__left_arm_bear_temperature(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__left_arm_bear_temperature(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__left_arm_bear_temperature(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__left_arm_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__left_arm_bear_temperature(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__left_arm_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__left_arm_bear_temperature(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__left_arm_bear_temperature(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__head_bear_temperature(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__head_bear_temperature(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__head_bear_temperature(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__head_bear_temperature(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__head_bear_temperature(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__head_bear_temperature(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__head_bear_temperature(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__head_bear_temperature(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_member_array[5] = {
  {
    "right_leg_bear_temperature",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearTemperature, right_leg_bear_temperature),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__right_leg_bear_temperature,  // size() function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__right_leg_bear_temperature,  // get_const(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__right_leg_bear_temperature,  // get(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__right_leg_bear_temperature,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__right_leg_bear_temperature,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__right_leg_bear_temperature  // resize(index) function pointer
  },
  {
    "left_leg_bear_temperature",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearTemperature, left_leg_bear_temperature),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__left_leg_bear_temperature,  // size() function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__left_leg_bear_temperature,  // get_const(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__left_leg_bear_temperature,  // get(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__left_leg_bear_temperature,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__left_leg_bear_temperature,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__left_leg_bear_temperature  // resize(index) function pointer
  },
  {
    "right_arm_bear_temperature",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearTemperature, right_arm_bear_temperature),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__right_arm_bear_temperature,  // size() function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__right_arm_bear_temperature,  // get_const(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__right_arm_bear_temperature,  // get(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__right_arm_bear_temperature,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__right_arm_bear_temperature,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__right_arm_bear_temperature  // resize(index) function pointer
  },
  {
    "left_arm_bear_temperature",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearTemperature, left_arm_bear_temperature),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__left_arm_bear_temperature,  // size() function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__left_arm_bear_temperature,  // get_const(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__left_arm_bear_temperature,  // get(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__left_arm_bear_temperature,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__left_arm_bear_temperature,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__left_arm_bear_temperature  // resize(index) function pointer
  },
  {
    "head_bear_temperature",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearTemperature, head_bear_temperature),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__size_function__BearTemperature__head_bear_temperature,  // size() function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_const_function__BearTemperature__head_bear_temperature,  // get_const(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__get_function__BearTemperature__head_bear_temperature,  // get(index) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__fetch_function__BearTemperature__head_bear_temperature,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__assign_function__BearTemperature__head_bear_temperature,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__resize_function__BearTemperature__head_bear_temperature  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_members = {
  "themis_state_msgs__msg",  // message namespace
  "BearTemperature",  // message name
  5,  // number of fields
  sizeof(themis_state_msgs__msg__BearTemperature),
  themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_member_array,  // message members
  themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_init_function,  // function to initialize message memory (memory has to be allocated)
  themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_type_support_handle = {
  0,
  &themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, BearTemperature)() {
  if (!themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_type_support_handle.typesupport_identifier) {
    themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &themis_state_msgs__msg__BearTemperature__rosidl_typesupport_introspection_c__BearTemperature_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
