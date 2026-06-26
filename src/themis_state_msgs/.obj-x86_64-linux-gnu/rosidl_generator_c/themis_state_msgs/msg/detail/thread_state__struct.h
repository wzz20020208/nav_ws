// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/ThreadState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/ThreadState in the package themis_state_msgs.
/**
  * ThreadState msg
 */
typedef struct themis_state_msgs__msg__ThreadState
{
  int8_t bear_right_leg;
  int8_t bear_left_leg;
  int8_t bear_right_arm;
  int8_t bear_left_arm;
  int8_t bear_head;
  int8_t dxl_right_hand;
  int8_t dxl_left_hand;
  int8_t sense;
  int8_t estimation;
  int8_t control;
  int8_t locomotion;
  int8_t command;
  int8_t data;
  int8_t posture;
} themis_state_msgs__msg__ThreadState;

// Struct for a sequence of themis_state_msgs__msg__ThreadState.
typedef struct themis_state_msgs__msg__ThreadState__Sequence
{
  themis_state_msgs__msg__ThreadState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__ThreadState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__STRUCT_H_
