// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from themis_state_msgs:msg/BearArray.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "themis_state_msgs/msg/detail/bear_array__rosidl_typesupport_introspection_c.h"
#include "themis_state_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "themis_state_msgs/msg/detail/bear_array__functions.h"
#include "themis_state_msgs/msg/detail/bear_array__struct.h"


// Include directives for member types
// Member `positions`
// Member `velocities`
// Member `currents`
// Member `temperatures`
// Member `voltages`
// Member `errors`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  themis_state_msgs__msg__BearArray__init(message_memory);
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_fini_function(void * message_memory)
{
  themis_state_msgs__msg__BearArray__fini(message_memory);
}

size_t themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__positions(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__positions(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__positions(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__positions(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__positions(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__positions(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__positions(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__positions(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__velocities(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__velocities(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__velocities(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__velocities(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__velocities(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__velocities(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__velocities(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__velocities(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__currents(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__currents(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__currents(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__currents(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__currents(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__currents(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__temperatures(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__temperatures(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__temperatures(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__temperatures(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__temperatures(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__temperatures(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__temperatures(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__temperatures(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__voltages(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__voltages(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__voltages(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__voltages(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__voltages(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__voltages(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__voltages(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__voltages(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__errors(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__errors(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__errors(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__errors(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__errors(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__errors(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__errors(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__errors(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_member_array[6] = {
  {
    "positions",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearArray, positions),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__positions,  // size() function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__positions,  // get_const(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__positions,  // get(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__positions,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__positions,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__positions  // resize(index) function pointer
  },
  {
    "velocities",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearArray, velocities),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__velocities,  // size() function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__velocities,  // get_const(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__velocities,  // get(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__velocities,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__velocities,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__velocities  // resize(index) function pointer
  },
  {
    "currents",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearArray, currents),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__currents,  // size() function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__currents,  // get_const(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__currents,  // get(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__currents,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__currents,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__currents  // resize(index) function pointer
  },
  {
    "temperatures",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearArray, temperatures),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__temperatures,  // size() function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__temperatures,  // get_const(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__temperatures,  // get(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__temperatures,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__temperatures,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__temperatures  // resize(index) function pointer
  },
  {
    "voltages",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearArray, voltages),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__voltages,  // size() function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__voltages,  // get_const(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__voltages,  // get(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__voltages,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__voltages,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__voltages  // resize(index) function pointer
  },
  {
    "errors",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BearArray, errors),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__size_function__BearArray__errors,  // size() function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_const_function__BearArray__errors,  // get_const(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__get_function__BearArray__errors,  // get(index) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__fetch_function__BearArray__errors,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__assign_function__BearArray__errors,  // assign(index, value) function pointer
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__resize_function__BearArray__errors  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_members = {
  "themis_state_msgs__msg",  // message namespace
  "BearArray",  // message name
  6,  // number of fields
  sizeof(themis_state_msgs__msg__BearArray),
  themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_member_array,  // message members
  themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_init_function,  // function to initialize message memory (memory has to be allocated)
  themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_type_support_handle = {
  0,
  &themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, BearArray)() {
  if (!themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_type_support_handle.typesupport_identifier) {
    themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &themis_state_msgs__msg__BearArray__rosidl_typesupport_introspection_c__BearArray_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
