// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/Foot.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

/// Struct defined in msg/Foot in the package themis_state_msgs.
typedef struct themis_state_msgs__msg__Foot
{
  double x;
  double y;
  double z;
} themis_state_msgs__msg__Foot;

// Struct for a sequence of themis_state_msgs__msg__Foot.
typedef struct themis_state_msgs__msg__Foot__Sequence
{
  themis_state_msgs__msg__Foot * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__Foot__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__STRUCT_H_
