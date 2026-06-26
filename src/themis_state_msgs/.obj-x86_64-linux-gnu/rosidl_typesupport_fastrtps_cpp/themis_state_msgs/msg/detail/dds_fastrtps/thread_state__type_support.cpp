// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/ThreadState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/thread_state__rosidl_typesupport_fastrtps_cpp.hpp"
#include "themis_state_msgs/msg/detail/thread_state__struct.hpp"

#include <limits>
#include <stdexcept>
#include <string>
#include "rosidl_typesupport_cpp/message_type_support.hpp"
#include "rosidl_typesupport_fastrtps_cpp/identifier.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support_decl.hpp"
#include "rosidl_typesupport_fastrtps_cpp/wstring_conversion.hpp"
#include "fastcdr/Cdr.h"


// forward declaration of message dependencies and their conversion functions

namespace themis_state_msgs
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
cdr_serialize(
  const themis_state_msgs::msg::ThreadState & ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Member: bear_right_leg
  cdr << ros_message.bear_right_leg;
  // Member: bear_left_leg
  cdr << ros_message.bear_left_leg;
  // Member: bear_right_arm
  cdr << ros_message.bear_right_arm;
  // Member: bear_left_arm
  cdr << ros_message.bear_left_arm;
  // Member: bear_head
  cdr << ros_message.bear_head;
  // Member: dxl_right_hand
  cdr << ros_message.dxl_right_hand;
  // Member: dxl_left_hand
  cdr << ros_message.dxl_left_hand;
  // Member: sense
  cdr << ros_message.sense;
  // Member: estimation
  cdr << ros_message.estimation;
  // Member: control
  cdr << ros_message.control;
  // Member: locomotion
  cdr << ros_message.locomotion;
  // Member: command
  cdr << ros_message.command;
  // Member: data
  cdr << ros_message.data;
  // Member: posture
  cdr << ros_message.posture;
  return true;
}

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  themis_state_msgs::msg::ThreadState & ros_message)
{
  // Member: bear_right_leg
  cdr >> ros_message.bear_right_leg;

  // Member: bear_left_leg
  cdr >> ros_message.bear_left_leg;

  // Member: bear_right_arm
  cdr >> ros_message.bear_right_arm;

  // Member: bear_left_arm
  cdr >> ros_message.bear_left_arm;

  // Member: bear_head
  cdr >> ros_message.bear_head;

  // Member: dxl_right_hand
  cdr >> ros_message.dxl_right_hand;

  // Member: dxl_left_hand
  cdr >> ros_message.dxl_left_hand;

  // Member: sense
  cdr >> ros_message.sense;

  // Member: estimation
  cdr >> ros_message.estimation;

  // Member: control
  cdr >> ros_message.control;

  // Member: locomotion
  cdr >> ros_message.locomotion;

  // Member: command
  cdr >> ros_message.command;

  // Member: data
  cdr >> ros_message.data;

  // Member: posture
  cdr >> ros_message.posture;

  return true;
}  // NOLINT(readability/fn_size)

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
get_serialized_size(
  const themis_state_msgs::msg::ThreadState & ros_message,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Member: bear_right_leg
  {
    size_t item_size = sizeof(ros_message.bear_right_leg);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: bear_left_leg
  {
    size_t item_size = sizeof(ros_message.bear_left_leg);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: bear_right_arm
  {
    size_t item_size = sizeof(ros_message.bear_right_arm);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: bear_left_arm
  {
    size_t item_size = sizeof(ros_message.bear_left_arm);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: bear_head
  {
    size_t item_size = sizeof(ros_message.bear_head);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: dxl_right_hand
  {
    size_t item_size = sizeof(ros_message.dxl_right_hand);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: dxl_left_hand
  {
    size_t item_size = sizeof(ros_message.dxl_left_hand);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: sense
  {
    size_t item_size = sizeof(ros_message.sense);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: estimation
  {
    size_t item_size = sizeof(ros_message.estimation);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: control
  {
    size_t item_size = sizeof(ros_message.control);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: locomotion
  {
    size_t item_size = sizeof(ros_message.locomotion);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: command
  {
    size_t item_size = sizeof(ros_message.command);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: data
  {
    size_t item_size = sizeof(ros_message.data);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // Member: posture
  {
    size_t item_size = sizeof(ros_message.posture);
    current_alignment += item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
max_serialized_size_ThreadState(
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


  // Member: bear_right_leg
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: bear_left_leg
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: bear_right_arm
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: bear_left_arm
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: bear_head
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: dxl_right_hand
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: dxl_left_hand
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: sense
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: estimation
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: control
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: locomotion
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: command
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: data
  {
    size_t array_size = 1;

    last_member_size = array_size * sizeof(uint8_t);
    current_alignment += array_size * sizeof(uint8_t);
  }

  // Member: posture
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
    using DataType = themis_state_msgs::msg::ThreadState;
    is_plain =
      (
      offsetof(DataType, posture) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static bool _ThreadState__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  auto typed_message =
    static_cast<const themis_state_msgs::msg::ThreadState *>(
    untyped_ros_message);
  return cdr_serialize(*typed_message, cdr);
}

static bool _ThreadState__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  auto typed_message =
    static_cast<themis_state_msgs::msg::ThreadState *>(
    untyped_ros_message);
  return cdr_deserialize(cdr, *typed_message);
}

static uint32_t _ThreadState__get_serialized_size(
  const void * untyped_ros_message)
{
  auto typed_message =
    static_cast<const themis_state_msgs::msg::ThreadState *>(
    untyped_ros_message);
  return static_cast<uint32_t>(get_serialized_size(*typed_message, 0));
}

static size_t _ThreadState__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_ThreadState(full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}

static message_type_support_callbacks_t _ThreadState__callbacks = {
  "themis_state_msgs::msg",
  "ThreadState",
  _ThreadState__cdr_serialize,
  _ThreadState__cdr_deserialize,
  _ThreadState__get_serialized_size,
  _ThreadState__max_serialized_size
};

static rosidl_message_type_support_t _ThreadState__handle = {
  rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
  &_ThreadState__callbacks,
  get_message_typesupport_handle_function,
};

}  // namespace typesupport_fastrtps_cpp

}  // namespace msg

}  // namespace themis_state_msgs

namespace rosidl_typesupport_fastrtps_cpp
{

template<>
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
get_message_type_support_handle<themis_state_msgs::msg::ThreadState>()
{
  return &themis_state_msgs::msg::typesupport_fastrtps_cpp::_ThreadState__handle;
}

}  // namespace rosidl_typesupport_fastrtps_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, themis_state_msgs, msg, ThreadState)() {
  return &themis_state_msgs::msg::typesupport_fastrtps_cpp::_ThreadState__handle;
}

#ifdef __cplusplus
}
#endif
