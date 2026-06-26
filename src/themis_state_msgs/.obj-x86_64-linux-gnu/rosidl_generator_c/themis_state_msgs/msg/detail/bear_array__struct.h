// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/BearArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'positions'
// Member 'velocities'
// Member 'currents'
// Member 'temperatures'
// Member 'voltages'
// Member 'errors'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/BearArray in the package themis_state_msgs.
/**
  * JointArray.msg
 */
typedef struct themis_state_msgs__msg__BearArray
{
  rosidl_runtime_c__float__Sequence positions;
  rosidl_runtime_c__float__Sequence velocities;
  rosidl_runtime_c__float__Sequence currents;
  rosidl_runtime_c__float__Sequence temperatures;
  rosidl_runtime_c__float__Sequence voltages;
  rosidl_runtime_c__float__Sequence errors;
} themis_state_msgs__msg__BearArray;

// Struct for a sequence of themis_state_msgs__msg__BearArray.
typedef struct themis_state_msgs__msg__BearArray__Sequence
{
  themis_state_msgs__msg__BearArray * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__BearArray__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__STRUCT_H_
