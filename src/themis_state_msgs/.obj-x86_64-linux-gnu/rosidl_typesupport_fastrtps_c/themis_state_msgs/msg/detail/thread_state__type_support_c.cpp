// generated from rosidl_typesupport_fastrtps_c/resource/idl__type_support_c.cpp.em
// with input from themis_state_msgs:msg/ThreadState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/thread_state__rosidl_typesupport_fastrtps_c.h"


#include <cassert>
#include <limits>
#include <string>
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "themis_state_msgs/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "themis_state_msgs/msg/detail/thread_state__struct.h"
#include "themis_state_msgs/msg/detail/thread_state__functions.h"
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


// forward declare type support functions


using _ThreadState__ros_msg_type = themis_state_msgs__msg__ThreadState;

static bool _ThreadState__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const _ThreadState__ros_msg_type * ros_message = static_cast<const _ThreadState__ros_msg_type *>(untyped_ros_message);
  // Field name: bear_right_leg
  {
    cdr << ros_message->bear_right_leg;
  }

  // Field name: bear_left_leg
  {
    cdr << ros_message->bear_left_leg;
  }

  // Field name: bear_right_arm
  {
    cdr << ros_message->bear_right_arm;
  }

  // Field name: bear_left_arm
  {
    cdr << ros_message->bear_left_arm;
  }

  // Field name: bear_head
  {
    cdr << ros_message->bear_head;
  }

  // Field name: dxl_right_hand
  {
    cdr << ros_message->dxl_right_hand;
  }

  // Field name: dxl_left_hand
  {
    cdr << ros_message->dxl_left_hand;
  }

  // Field name: sense
  {
    cdr << ros_message->sense;
  }

  // Field name: estimation
  {
    cdr << ros_message->estimation;
  }

  // Field name: control
  {
    cdr << ros_message->control;
  }

  // Field name: locomotion
  {
    cdr << ros_message->locomotion;
  }

  // Field name: command
  {
    cdr << ros_message->command;
  }

  // Field name: data
  {
    cdr << ros_message->data;
  }

  // Field name: posture
  {
    cdr << ros_message->posture;
  }

  return true;
}

static bool _ThreadState__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  _ThreadState__ros_msg_type * ros_message = static_cast<_ThreadState__ros_msg_type *>(untyped_ros_message);
  // Field name: bear_right_leg
  {
    cdr >> ros_message->bear_right_leg;
  }

  // Field name: bear_left_leg
  {
    cdr >> ros_message->bear_left_leg;
  }

  // Field name: bear_right_arm
  {
    cdr >> ros_message->bear_right_arm;
  }

  // Field name: bear_left_arm
  {
    cdr >> ros_message->bear_left_arm;
  }

  // Field name: bear_head
  {
    cdr >> ros_message->bear_head;
  }

  // Field name: dxl_right_hand
  {
    cdr >> ros_message->dxl_right_hand;
  }

  // Field name: dxl_left_hand
  {
    cdr >> ros_message->dxl_left_hand;
  }

  // Field name: sense
  {
    cdr >> ros_message->sense;
  }

  // Field name: estimation
  {
    cdr >> ros_message->estimation;
  }

  // Field name: control
  {
    cdr >> ros_message->control;
  }

  // Field name: locomotion
  {
    cdr >> ros_message->locomotion;
  }

  // Field name: command
  {
    cdr >> ros_message->command;
  }

  // Field name: data
  {
    cdr >> ros_message->data;
  }

  // Field name: posture
  {
    cdr >> ros_message->posture;
  }

  return true;
}  // NOLINT(readability/fn_size)

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_themis_state_msgs
size_t get_serialized_size_themis_state_msgs__msg__ThreadState(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _ThreadState__ros_msg_type * ros_message = static_cast<const _ThreadState__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // field.name bear_right_leg
  {
    size_t item_size = sizeof(ros_message->bear_right_leg);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name bear_left_leg
  {
    size_t item_size = sizeof(ros_message->bear_left_leg);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name bear_right_arm
  {
    size_t item_size = sizeof(ros_message->bear_right_arm);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name bear_left_arm
  {
    size_t item_size = sizeof(ros_message->bear_left_arm);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name bear_head
  {
    size_t item_size = sizeof(ros_message->bear_head);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name dxl_right_hand
  {
    size_t item_size = sizeof(ros_message->dxl_right_hand);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name dxl_left_hand
  {
    size_t item_size = sizeof(ros_message->dxl_left_hand);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name sense
  {
    size_t item_size = sizeof(ros_message->sense);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name estimation
  {
    size_t item_size = sizeof(ros_message->estimation);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name control
  {
    size_t item_size = sizeof(ros_message->control);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name locomotion
  {
    size_t item_size = sizeof(ros_message->locomotion);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name command
  {
    size_t item_size = sizeof(ros_message->command);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name data
  {
    size_t item_size = sizeof(ros_message->data);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name posture
  {
    size_t item_size = sizeof(ros_message->posture);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}

static uint32_t _ThreadState__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_themis_state_msgs__msg__ThreadState(
      untyped_ros_message, 0));
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_themis_state_msgs
size_t max_serialized_size_themis_state_msgs__msg__ThreadState(
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

  // member: bear_right_leg
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: bear_left_leg
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: bear_right_arm
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: bear_left_arm
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: bear_head
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: dxl_right_hand
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: dxl_left_hand
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: sense
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: estimation
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: control
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: locomotion
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: command
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: data
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }
  // member: posture
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = themis_state_msgs__msg__ThreadState;
    is_plain =
      (
      offsetof(DataType, posture) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static size_t _ThreadState__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_themis_state_msgs__msg__ThreadState(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_ThreadState = {
  "themis_state_msgs::msg",
  "ThreadState",
  _ThreadState__cdr_serialize,
  _ThreadState__cdr_deserialize,
  _ThreadState__get_serialized_size,
  _ThreadState__max_serialized_size
};

static rosidl_message_type_support_t _ThreadState__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_ThreadState,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, ThreadState)() {
  return &_ThreadState__type_support;
}

#if defined(__cplusplus)
}
#endif
