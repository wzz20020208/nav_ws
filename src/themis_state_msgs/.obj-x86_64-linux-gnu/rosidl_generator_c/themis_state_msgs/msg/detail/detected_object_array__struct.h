// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/DetectedObjectArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'header'
// Member 'camera_stamp'
// Member 'detection_stamp'
#include "std_msgs/msg/detail/header__struct.h"
// Member 'objects'
#include "themis_state_msgs/msg/detail/detected_object__struct.h"

/// Struct defined in msg/DetectedObjectArray in the package themis_state_msgs.
/**
  * DetectedObjectArray.msg
 */
typedef struct themis_state_msgs__msg__DetectedObjectArray
{
  std_msgs__msg__Header header;
  std_msgs__msg__Header camera_stamp;
  themis_state_msgs__msg__DetectedObject__Sequence objects;
  std_msgs__msg__Header detection_stamp;
} themis_state_msgs__msg__DetectedObjectArray;

// Struct for a sequence of themis_state_msgs__msg__DetectedObjectArray.
typedef struct themis_state_msgs__msg__DetectedObjectArray__Sequence
{
  themis_state_msgs__msg__DetectedObjectArray * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__DetectedObjectArray__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__STRUCT_H_
