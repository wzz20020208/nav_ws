// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/BaseState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/BaseState in the package themis_state_msgs.
/**
  * BaseState.msg
 */
typedef struct themis_state_msgs__msg__BaseState
{
  double position_x;
  double position_y;
  double position_z;
  double velocity_x;
  double velocity_y;
  double velocity_z;
  double yaw;
  double roll;
  double pitch;
  double angular_rate;
} themis_state_msgs__msg__BaseState;

// Struct for a sequence of themis_state_msgs__msg__BaseState.
typedef struct themis_state_msgs__msg__BaseState__Sequence
{
  themis_state_msgs__msg__BaseState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__BaseState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__STRUCT_H_
