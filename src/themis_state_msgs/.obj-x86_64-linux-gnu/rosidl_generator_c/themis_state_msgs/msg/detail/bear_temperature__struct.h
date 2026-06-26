// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/BearTemperature.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'right_leg_bear_temperature'
// Member 'left_leg_bear_temperature'
// Member 'right_arm_bear_temperature'
// Member 'left_arm_bear_temperature'
// Member 'head_bear_temperature'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/BearTemperature in the package themis_state_msgs.
/**
  * BearTemperature.msg
 */
typedef struct themis_state_msgs__msg__BearTemperature
{
  rosidl_runtime_c__double__Sequence right_leg_bear_temperature;
  rosidl_runtime_c__double__Sequence left_leg_bear_temperature;
  rosidl_runtime_c__double__Sequence right_arm_bear_temperature;
  rosidl_runtime_c__double__Sequence left_arm_bear_temperature;
  rosidl_runtime_c__double__Sequence head_bear_temperature;
} themis_state_msgs__msg__BearTemperature;

// Struct for a sequence of themis_state_msgs__msg__BearTemperature.
typedef struct themis_state_msgs__msg__BearTemperature__Sequence
{
  themis_state_msgs__msg__BearTemperature * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__BearTemperature__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__STRUCT_H_
