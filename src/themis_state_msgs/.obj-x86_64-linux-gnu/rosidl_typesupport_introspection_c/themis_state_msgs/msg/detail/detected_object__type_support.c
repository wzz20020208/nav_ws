// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from themis_state_msgs:msg/DetectedObject.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "themis_state_msgs/msg/detail/detected_object__rosidl_typesupport_introspection_c.h"
#include "themis_state_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "themis_state_msgs/msg/detail/detected_object__functions.h"
#include "themis_state_msgs/msg/detail/detected_object__struct.h"


// Include directives for member types
// Member `category`
#include "rosidl_runtime_c/string_functions.h"
// Member `bbox`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  themis_state_msgs__msg__DetectedObject__init(message_memory);
}

void themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_fini_function(void * message_memory)
{
  themis_state_msgs__msg__DetectedObject__fini(message_memory);
}

size_t themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__size_function__DetectedObject__bbox(
  const void * untyped_member)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__get_const_function__DetectedObject__bbox(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__float__Sequence * member =
    (const rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__get_function__DetectedObject__bbox(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__fetch_function__DetectedObject__bbox(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const float * item =
    ((const float *)
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__get_const_function__DetectedObject__bbox(untyped_member, index));
  float * value =
    (float *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__assign_function__DetectedObject__bbox(
  void * untyped_member, size_t index, const void * untyped_value)
{
  float * item =
    ((float *)
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__get_function__DetectedObject__bbox(untyped_member, index));
  const float * value =
    (const float *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__resize_function__DetectedObject__bbox(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__float__Sequence * member =
    (rosidl_runtime_c__float__Sequence *)(untyped_member);
  rosidl_runtime_c__float__Sequence__fini(member);
  return rosidl_runtime_c__float__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_member_array[4] = {
  {
    "id",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_INT32,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__DetectedObject, id),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "category",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_STRING,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__DetectedObject, category),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "score",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    false,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__DetectedObject, score),  // bytes offset in struct
    NULL,  // default value
    NULL,  // size() function pointer
    NULL,  // get_const(index) function pointer
    NULL,  // get(index) function pointer
    NULL,  // fetch(index, &value) function pointer
    NULL,  // assign(index, value) function pointer
    NULL  // resize(index) function pointer
  },
  {
    "bbox",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_FLOAT,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__DetectedObject, bbox),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__size_function__DetectedObject__bbox,  // size() function pointer
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__get_const_function__DetectedObject__bbox,  // get_const(index) function pointer
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__get_function__DetectedObject__bbox,  // get(index) function pointer
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__fetch_function__DetectedObject__bbox,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__assign_function__DetectedObject__bbox,  // assign(index, value) function pointer
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__resize_function__DetectedObject__bbox  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_members = {
  "themis_state_msgs__msg",  // message namespace
  "DetectedObject",  // message name
  4,  // number of fields
  sizeof(themis_state_msgs__msg__DetectedObject),
  themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_member_array,  // message members
  themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_init_function,  // function to initialize message memory (memory has to be allocated)
  themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_type_support_handle = {
  0,
  &themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, DetectedObject)() {
  if (!themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_type_support_handle.typesupport_identifier) {
    themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &themis_state_msgs__msg__DetectedObject__rosidl_typesupport_introspection_c__DetectedObject_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
