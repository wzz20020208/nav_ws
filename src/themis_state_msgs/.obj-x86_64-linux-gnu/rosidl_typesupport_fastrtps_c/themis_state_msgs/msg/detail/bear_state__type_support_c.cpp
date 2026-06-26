// generated from rosidl_typesupport_fastrtps_c/resource/idl__type_support_c.cpp.em
// with input from themis_state_msgs:msg/BearState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/bear_state__rosidl_typesupport_fastrtps_c.h"


#include <cassert>
#include <limits>
#include <string>
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "themis_state_msgs/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "themis_state_msgs/msg/detail/bear_state__struct.h"
#include "themis_state_msgs/msg/detail/bear_state__functions.h"
#include "fastcdr/Cdr.h"

#ifndef _WIN32
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
# ifdef __clang__
#  pragma clang diagnostic ignored "-Wdeprecated-register"
#  pragma clang diagnostic ignored "-Wreturn-type-c-linkage"
# endif
#endif
#ifndef _WIN32
# pragma GCC diagnostic pop
#endif

// includes and forward declarations of message dependencies and their conversion functions

#if defined(__cplusplus)
extern "C"
{
#endif

#include "themis_state_msgs/msg/detail/bear_array__functions.h"  // head, left_arm, left_hand, left_leg, right_arm, right_hand, right_leg

// forward declare type support functions
size_t get_serialized_size_themis_state_msgs__msg__BearArray(
  const void * untyped_ros_message,
  size_t current_alignment);

size_t max_serialized_size_themis_state_msgs__msg__BearArray(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);

const rosidl_message_type_support_t *
  ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray)();


using _BearState__ros_msg_type = themis_state_msgs__msg__BearState;

static bool _BearState__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const _BearState__ros_msg_type * ros_message = static_cast<const _BearState__ros_msg_type *>(untyped_ros_message);
  // Field name: head
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_serialize(
        &ros_message->head, cdr))
    {
      return false;
    }
  }

  // Field name: left_arm
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_serialize(
        &ros_message->left_arm, cdr))
    {
      return false;
    }
  }

  // Field name: right_arm
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_serialize(
        &ros_message->right_arm, cdr))
    {
      return false;
    }
  }

  // Field name: left_leg
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_serialize(
        &ros_message->left_leg, cdr))
    {
      return false;
    }
  }

  // Field name: right_leg
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_serialize(
        &ros_message->right_leg, cdr))
    {
      return false;
    }
  }

  // Field name: left_hand
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_serialize(
        &ros_message->left_hand, cdr))
    {
      return false;
    }
  }

  // Field name: right_hand
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_serialize(
        &ros_message->right_hand, cdr))
    {
      return false;
    }
  }

  return true;
}

static bool _BearState__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  _BearState__ros_msg_type * ros_message = static_cast<_BearState__ros_msg_type *>(untyped_ros_message);
  // Field name: head
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_deserialize(
        cdr, &ros_message->head))
    {
      return false;
    }
  }

  // Field name: left_arm
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_deserialize(
        cdr, &ros_message->left_arm))
    {
      return false;
    }
  }

  // Field name: right_arm
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_deserialize(
        cdr, &ros_message->right_arm))
    {
      return false;
    }
  }

  // Field name: left_leg
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_deserialize(
        cdr, &ros_message->left_leg))
    {
      return false;
    }
  }

  // Field name: right_leg
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_deserialize(
        cdr, &ros_message->right_leg))
    {
      return false;
    }
  }

  // Field name: left_hand
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_deserialize(
        cdr, &ros_message->left_hand))
    {
      return false;
    }
  }

  // Field name: right_hand
  {
    const message_type_support_callbacks_t * callbacks =
      static_cast<const message_type_support_callbacks_t *>(
      ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(
        rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearArray
      )()->data);
    if (!callbacks->cdr_deserialize(
        cdr, &ros_message->right_hand))
    {
      return false;
    }
  }

  return true;
}  // NOLINT(readability/fn_size)

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_themis_state_msgs
size_t get_serialized_size_themis_state_msgs__msg__BearState(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _BearState__ros_msg_type * ros_message = static_cast<const _BearState__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // field.name head

  current_alignment += get_serialized_size_themis_state_msgs__msg__BearArray(
    &(ros_message->head), current_alignment);
  // field.name left_arm

  current_alignment += get_serialized_size_themis_state_msgs__msg__BearArray(
    &(ros_message->left_arm), current_alignment);
  // field.name right_arm

  current_alignment += get_serialized_size_themis_state_msgs__msg__BearArray(
    &(ros_message->right_arm), current_alignment);
  // field.name left_leg

  current_alignment += get_serialized_size_themis_state_msgs__msg__BearArray(
    &(ros_message->left_leg), current_alignment);
  // field.name right_leg

  current_alignment += get_serialized_size_themis_state_msgs__msg__BearArray(
    &(ros_message->right_leg), current_alignment);
  // field.name left_hand

  current_alignment += get_serialized_size_themis_state_msgs__msg__BearArray(
    &(ros_message->left_hand), current_alignment);
  // field.name right_hand

  current_alignment += get_serialized_size_themis_state_msgs__msg__BearArray(
    &(ros_message->right_hand), current_alignment);

  return current_alignment - initial_alignment;
}

static uint32_t _BearState__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_themis_state_msgs__msg__BearState(
      untyped_ros_message, 0));
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_themis_state_msgs
size_t max_serialized_size_themis_state_msgs__msg__BearState(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  size_t last_member_size = 0;
  (void)last_member_size;
  (void)padding;
  (void)wchar_size;

  full_bounded = true;
  is_plain = true;

  // member: head
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_themis_state_msgs__msg__BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }
  // member: left_arm
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_themis_state_msgs__msg__BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }
  // member: right_arm
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_themis_state_msgs__msg__BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }
  // member: left_leg
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_themis_state_msgs__msg__BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }
  // member: right_leg
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_themis_state_msgs__msg__BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }
  // member: left_hand
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_themis_state_msgs__msg__BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }
  // member: right_hand
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size;
      inner_size =
        max_serialized_size_themis_state_msgs__msg__BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = themis_state_msgs__msg__BearState;
    is_plain =
      (
      offsetof(DataType, right_hand) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static size_t _BearState__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_themis_state_msgs__msg__BearState(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_BearState = {
  "themis_state_msgs::msg",
  "BearState",
  _BearState__cdr_serialize,
  _BearState__cdr_deserialize,
  _BearState__get_serialized_size,
  _BearState__max_serialized_size
};

static rosidl_message_type_support_t _BearState__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_BearState,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BearState)() {
  return &_BearState__type_support;
}

#if defined(__cplusplus)
}
#endif
