// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/THJointState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__TH_JOINT_STATE__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__TH_JOINT_STATE__STRUCT_H_

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
#include "themis_state_msgs/msg/detail/joint_array__struct.h"

/// Struct defined in msg/THJointState in the package themis_state_msgs.
/**
  * JointState.msg
 */
typedef struct themis_state_msgs__msg__THJointState
{
  themis_state_msgs__msg__JointArray head;
  themis_state_msgs__msg__JointArray left_arm;
  themis_state_msgs__msg__JointArray right_arm;
  themis_state_msgs__msg__JointArray left_leg;
  themis_state_msgs__msg__JointArray right_leg;
  themis_state_msgs__msg__JointArray left_hand;
  themis_state_msgs__msg__JointArray right_hand;
} themis_state_msgs__msg__THJointState;

// Struct for a sequence of themis_state_msgs__msg__THJointState.
typedef struct themis_state_msgs__msg__THJointState__Sequence
{
  themis_state_msgs__msg__THJointState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__THJointState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__TH_JOINT_STATE__STRUCT_H_
