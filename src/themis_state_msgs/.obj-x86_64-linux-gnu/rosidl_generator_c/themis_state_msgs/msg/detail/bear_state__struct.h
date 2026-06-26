// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/BearState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'head'
// Member 'left_arm'
// Member 'right_arm'
// Member 'left_leg'
// Member 'right_leg'
// Member 'left_hand'
// Member 'right_hand'
#include "themis_state_msgs/msg/detail/bear_array__struct.h"

/// Struct defined in msg/BearState in the package themis_state_msgs.
/**
  * JointState.msg
 */
typedef struct themis_state_msgs__msg__BearState
{
  themis_state_msgs__msg__BearArray head;
  themis_state_msgs__msg__BearArray left_arm;
  themis_state_msgs__msg__BearArray right_arm;
  themis_state_msgs__msg__BearArray left_leg;
  themis_state_msgs__msg__BearArray right_leg;
  themis_state_msgs__msg__BearArray left_hand;
  themis_state_msgs__msg__BearArray right_hand;
} themis_state_msgs__msg__BearState;

// Struct for a sequence of themis_state_msgs__msg__BearState.
typedef struct themis_state_msgs__msg__BearState__Sequence
{
  themis_state_msgs__msg__BearState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__BearState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__STRUCT_H_
