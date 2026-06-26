// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/SelectedObject.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__STRUCT_H_

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

/// Struct defined in msg/SelectedObject in the package themis_state_msgs.
/**
  * SelectedObject.msg
 */
typedef struct themis_state_msgs__msg__SelectedObject
{
  int32_t id;
  rosidl_runtime_c__String category;
} themis_state_msgs__msg__SelectedObject;

// Struct for a sequence of themis_state_msgs__msg__SelectedObject.
typedef struct themis_state_msgs__msg__SelectedObject__Sequence
{
  themis_state_msgs__msg__SelectedObject * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__SelectedObject__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__STRUCT_H_
