// generated from rosidl_typesupport_fastrtps_cpp/resource/idl__type_support.cpp.em
// with input from themis_state_msgs:msg/BearState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/bear_state__rosidl_typesupport_fastrtps_cpp.hpp"
#include "themis_state_msgs/msg/detail/bear_state__struct.hpp"

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
bool cdr_serialize(
  const themis_state_msgs::msg::BearArray &,
  eprosima::fastcdr::Cdr &);
bool cdr_deserialize(
  eprosima::fastcdr::Cdr &,
  themis_state_msgs::msg::BearArray &);
size_t get_serialized_size(
  const themis_state_msgs::msg::BearArray &,
  size_t current_alignment);
size_t
max_serialized_size_BearArray(
  bool & full_bounded,
  bool & is_plain,
  size_t current_alignment);
}  // namespace typesupport_fastrtps_cpp
}  // namespace msg
}  // namespace themis_state_msgs

// functions for themis_state_msgs::msg::BearArray already declared above

// functions for themis_state_msgs::msg::BearArray already declared above

// functions for themis_state_msgs::msg::BearArray already declared above

// functions for themis_state_msgs::msg::BearArray already declared above

// functions for themis_state_msgs::msg::BearArray already declared above

// functions for themis_state_msgs::msg::BearArray already declared above


namespace themis_state_msgs
{

namespace msg
{

namespace typesupport_fastrtps_cpp
{

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
cdr_serialize(
  const themis_state_msgs::msg::BearState & ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  // Member: head
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_serialize(
    ros_message.head,
    cdr);
  // Member: left_arm
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_serialize(
    ros_message.left_arm,
    cdr);
  // Member: right_arm
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_serialize(
    ros_message.right_arm,
    cdr);
  // Member: left_leg
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_serialize(
    ros_message.left_leg,
    cdr);
  // Member: right_leg
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_serialize(
    ros_message.right_leg,
    cdr);
  // Member: left_hand
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_serialize(
    ros_message.left_hand,
    cdr);
  // Member: right_hand
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_serialize(
    ros_message.right_hand,
    cdr);
  return true;
}

bool
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  themis_state_msgs::msg::BearState & ros_message)
{
  // Member: head
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_deserialize(
    cdr, ros_message.head);

  // Member: left_arm
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_deserialize(
    cdr, ros_message.left_arm);

  // Member: right_arm
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_deserialize(
    cdr, ros_message.right_arm);

  // Member: left_leg
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_deserialize(
    cdr, ros_message.left_leg);

  // Member: right_leg
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_deserialize(
    cdr, ros_message.right_leg);

  // Member: left_hand
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_deserialize(
    cdr, ros_message.left_hand);

  // Member: right_hand
  themis_state_msgs::msg::typesupport_fastrtps_cpp::cdr_deserialize(
    cdr, ros_message.right_hand);

  return true;
}  // NOLINT(readability/fn_size)

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
get_serialized_size(
  const themis_state_msgs::msg::BearState & ros_message,
  size_t current_alignment)
{
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // Member: head

  current_alignment +=
    themis_state_msgs::msg::typesupport_fastrtps_cpp::get_serialized_size(
    ros_message.head, current_alignment);
  // Member: left_arm

  current_alignment +=
    themis_state_msgs::msg::typesupport_fastrtps_cpp::get_serialized_size(
    ros_message.left_arm, current_alignment);
  // Member: right_arm

  current_alignment +=
    themis_state_msgs::msg::typesupport_fastrtps_cpp::get_serialized_size(
    ros_message.right_arm, current_alignment);
  // Member: left_leg

  current_alignment +=
    themis_state_msgs::msg::typesupport_fastrtps_cpp::get_serialized_size(
    ros_message.left_leg, current_alignment);
  // Member: right_leg

  current_alignment +=
    themis_state_msgs::msg::typesupport_fastrtps_cpp::get_serialized_size(
    ros_message.right_leg, current_alignment);
  // Member: left_hand

  current_alignment +=
    themis_state_msgs::msg::typesupport_fastrtps_cpp::get_serialized_size(
    ros_message.left_hand, current_alignment);
  // Member: right_hand

  current_alignment +=
    themis_state_msgs::msg::typesupport_fastrtps_cpp::get_serialized_size(
    ros_message.right_hand, current_alignment);

  return current_alignment - initial_alignment;
}

size_t
ROSIDL_TYPESUPPORT_FASTRTPS_CPP_PUBLIC_themis_state_msgs
max_serialized_size_BearState(
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


  // Member: head
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size =
        themis_state_msgs::msg::typesupport_fastrtps_cpp::max_serialized_size_BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Member: left_arm
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size =
        themis_state_msgs::msg::typesupport_fastrtps_cpp::max_serialized_size_BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Member: right_arm
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size =
        themis_state_msgs::msg::typesupport_fastrtps_cpp::max_serialized_size_BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Member: left_leg
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size =
        themis_state_msgs::msg::typesupport_fastrtps_cpp::max_serialized_size_BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Member: right_leg
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size =
        themis_state_msgs::msg::typesupport_fastrtps_cpp::max_serialized_size_BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Member: left_hand
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size =
        themis_state_msgs::msg::typesupport_fastrtps_cpp::max_serialized_size_BearArray(
        inner_full_bounded, inner_is_plain, current_alignment);
      last_member_size += inner_size;
      current_alignment += inner_size;
      full_bounded &= inner_full_bounded;
      is_plain &= inner_is_plain;
    }
  }

  // Member: right_hand
  {
    size_t array_size = 1;


    last_member_size = 0;
    for (size_t index = 0; index < array_size; ++index) {
      bool inner_full_bounded;
      bool inner_is_plain;
      size_t inner_size =
        themis_state_msgs::msg::typesupport_fastrtps_cpp::max_serialized_size_BearArray(
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
    using DataType = themis_state_msgs::msg::BearState;
    is_plain =
      (
      offsetof(DataType, right_hand) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static bool _BearState__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  auto typed_message =
    static_cast<const themis_state_msgs::msg::BearState *>(
    untyped_ros_message);
  return cdr_serialize(*typed_message, cdr);
}

static bool _BearState__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  auto typed_message =
    static_cast<themis_state_msgs::msg::BearState *>(
    untyped_ros_message);
  return cdr_deserialize(cdr, *typed_message);
}

static uint32_t _BearState__get_serialized_size(
  const void * untyped_ros_message)
{
  auto typed_message =
    static_cast<const themis_state_msgs::msg::BearState *>(
    untyped_ros_message);
  return static_cast<uint32_t>(get_serialized_size(*typed_message, 0));
}

static size_t _BearState__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_BearState(full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}

static message_type_support_callbacks_t _BearState__callbacks = {
  "themis_state_msgs::msg",
  "BearState",
  _BearState__cdr_serialize,
  _BearState__cdr_deserialize,
  _BearState__get_serialized_size,
  _BearState__max_serialized_size
};

static rosidl_message_type_support_t _BearState__handle = {
  rosidl_typesupport_fastrtps_cpp::typesupport_identifier,
  &_BearState__callbacks,
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
get_message_type_support_handle<themis_state_msgs::msg::BearState>()
{
  return &themis_state_msgs::msg::typesupport_fastrtps_cpp::_BearState__handle;
}

}  // namespace rosidl_typesupport_fastrtps_cpp

#ifdef __cplusplus
extern "C"
{
#endif

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_cpp, themis_state_msgs, msg, BearState)() {
  return &themis_state_msgs::msg::typesupport_fastrtps_cpp::_BearState__handle;
}

#ifdef __cplusplus
}
#endif
