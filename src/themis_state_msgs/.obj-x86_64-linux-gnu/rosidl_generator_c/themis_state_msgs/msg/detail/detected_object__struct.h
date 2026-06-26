// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/DetectedObject.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'category'
#include "rosidl_runtime_c/string.h"
// Member 'bbox'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/DetectedObject in the package themis_state_msgs.
/**
  * DetectedObject.msg
 */
typedef struct themis_state_msgs__msg__DetectedObject
{
  int32_t id;
  rosidl_runtime_c__String category;
  float score;
  rosidl_runtime_c__float__Sequence bbox;
} themis_state_msgs__msg__DetectedObject;

// Struct for a sequence of themis_state_msgs__msg__DetectedObject.
typedef struct themis_state_msgs__msg__DetectedObject__Sequence
{
  themis_state_msgs__msg__DetectedObject * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__DetectedObject__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT__STRUCT_H_
