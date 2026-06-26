// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/FootPrintArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'current_foot_print'
// Member 'goal_foot_print'
#include "themis_state_msgs/msg/detail/foot_array__struct.h"

/// Struct defined in msg/FootPrintArray in the package themis_state_msgs.
typedef struct themis_state_msgs__msg__FootPrintArray
{
  themis_state_msgs__msg__FootArray__Sequence current_foot_print;
  themis_state_msgs__msg__FootArray__Sequence goal_foot_print;
} themis_state_msgs__msg__FootPrintArray;

// Struct for a sequence of themis_state_msgs__msg__FootPrintArray.
typedef struct themis_state_msgs__msg__FootPrintArray__Sequence
{
  themis_state_msgs__msg__FootPrintArray * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__FootPrintArray__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__STRUCT_H_
