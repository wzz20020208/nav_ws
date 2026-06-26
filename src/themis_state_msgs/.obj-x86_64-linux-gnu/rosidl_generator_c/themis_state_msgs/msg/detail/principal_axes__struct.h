// NOLINT: This file starts with a BOM since it contain non-ASCII characters
// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/PrincipalAxes.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__STRUCT_H_

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
#include "std_msgs/msg/detail/header__struct.h"
// Member 'centroid'
#include "geometry_msgs/msg/detail/point__struct.h"
// Member 'axis_x'
// Member 'axis_y'
// Member 'axis_z'
#include "geometry_msgs/msg/detail/vector3__struct.h"
// Member 'variances'
#include "rosidl_runtime_c/primitives_sequence.h"
// Member 'object_category'
#include "rosidl_runtime_c/string.h"

/// Struct defined in msg/PrincipalAxes in the package themis_state_msgs.
/**
  * 新增功能：为 PrincipalAxes 添加 header 字段
 */
typedef struct themis_state_msgs__msg__PrincipalAxes
{
  std_msgs__msg__Header header;
  geometry_msgs__msg__Point centroid;
  geometry_msgs__msg__Vector3 axis_x;
  geometry_msgs__msg__Vector3 axis_y;
  geometry_msgs__msg__Vector3 axis_z;
  rosidl_runtime_c__float__Sequence variances;
  rosidl_runtime_c__String object_category;
  int32_t object_id;
} themis_state_msgs__msg__PrincipalAxes;

// Struct for a sequence of themis_state_msgs__msg__PrincipalAxes.
typedef struct themis_state_msgs__msg__PrincipalAxes__Sequence
{
  themis_state_msgs__msg__PrincipalAxes * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__PrincipalAxes__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__STRUCT_H_
