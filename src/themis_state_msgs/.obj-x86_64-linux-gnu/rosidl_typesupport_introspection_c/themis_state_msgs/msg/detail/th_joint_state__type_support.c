// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from themis_state_msgs:msg/THJointState.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "themis_state_msgs/msg/detail/th_joint_state__rosidl_typesupport_introspection_c.h"
#include "themis_state_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "themis_state_msgs/msg/detail/th_joint_state__functions.h"
#include "themis_state_msgs/msg/detail/th_joint_state__struct.h"


// Include directives for member types
// Member `head`
// Member `left_arm`
// Member `right_arm`
// Member `left_leg`
// Member `right_leg`
// Member `left_hand`
// Member `right_hand`
#include "themis_state_msgs/msg/joint_array.h"
// Member `head`
// Member `left_arm`
// Member `right_arm`
// Member `left_leg`
// Member `right_leg`
// Member `left_hand`
// Member `right_hand`
#include "themis_state_msgs/msg/detail/joint_array__rosidl_typesupport_introspection_c.h"

#ifdef __cplusplus
extern "C"
{
#endif

void themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  themis_state_msgs__msg__THJointState__init(message_memory);
}

void themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_fini_function(void * message_memory)
{
  themis_state_msgs__msg__THJointState__fini(message_memory);
}

static rosidl_typesupport_introspection_c__MessageMember themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[7] = {
  {
    "head",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__THJointState, head),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "left_arm",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__THJointState, left_arm),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "right_arm",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__THJointState, right_arm),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "left_leg",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__THJointState, left_leg),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "right_leg",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__THJointState, right_leg),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "left_hand",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__THJointState, left_hand),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "right_hand",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_MESSAGE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message (initialized later)
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__THJointState, right_hand),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_members = {
  "themis_state_msgs__msg",  // message namespace
  "THJointState",  // message name
  7,  // number of fields
  sizeof(themis_state_msgs__msg__THJointState),
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array,  // message members
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_init_function,  // function to initialize message memory (memory has to be allocated)
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_type_support_handle = {
  0,
  &themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, THJointState)() {
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[0].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)();
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[1].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)();
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[2].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)();
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[3].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)();
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[4].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)();
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[5].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)();
  themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_member_array[6].members_ =
    ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, JointArray)();
  if (!themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_type_support_handle.typesupport_identifier) {
    themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &themis_state_msgs__msg__THJointState__rosidl_typesupport_introspection_c__THJointState_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
