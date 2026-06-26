// generated from rosidl_typesupport_fastrtps_c/resource/idl__type_support_c.cpp.em
// with input from themis_state_msgs:msg/BatteryState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/battery_state__rosidl_typesupport_fastrtps_c.h"


#include <cassert>
#include <limits>
#include <string>
#include "rosidl_typesupport_fastrtps_c/identifier.h"
#include "rosidl_typesupport_fastrtps_c/wstring_conversion.hpp"
#include "rosidl_typesupport_fastrtps_cpp/message_type_support.h"
#include "themis_state_msgs/msg/rosidl_typesupport_fastrtps_c__visibility_control.h"
#include "themis_state_msgs/msg/detail/battery_state__struct.h"
#include "themis_state_msgs/msg/detail/battery_state__functions.h"
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

#include "rosidl_runtime_c/primitives_sequence.h"  // battery_state, battery_voltages, cell_voltages, charge_statuses, discharge_statuses, error_state, max_charge_currents, max_discharge_currents, present_currents, temperatures
#include "rosidl_runtime_c/primitives_sequence_functions.h"  // battery_state, battery_voltages, cell_voltages, charge_statuses, discharge_statuses, error_state, max_charge_currents, max_discharge_currents, present_currents, temperatures

// forward declare type support functions


using _BatteryState__ros_msg_type = themis_state_msgs__msg__BatteryState;

static bool _BatteryState__cdr_serialize(
  const void * untyped_ros_message,
  eprosima::fastcdr::Cdr & cdr)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  const _BatteryState__ros_msg_type * ros_message = static_cast<const _BatteryState__ros_msg_type *>(untyped_ros_message);
  // Field name: battery_state
  {
    size_t size = ros_message->battery_state.size;
    auto array_ptr = ros_message->battery_state.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: error_state
  {
    size_t size = ros_message->error_state.size;
    auto array_ptr = ros_message->error_state.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: battery_voltages
  {
    size_t size = ros_message->battery_voltages.size;
    auto array_ptr = ros_message->battery_voltages.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: cell_voltages
  {
    size_t size = ros_message->cell_voltages.size;
    auto array_ptr = ros_message->cell_voltages.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: temperatures
  {
    size_t size = ros_message->temperatures.size;
    auto array_ptr = ros_message->temperatures.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: discharge_statuses
  {
    size_t size = ros_message->discharge_statuses.size;
    auto array_ptr = ros_message->discharge_statuses.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: charge_statuses
  {
    size_t size = ros_message->charge_statuses.size;
    auto array_ptr = ros_message->charge_statuses.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: max_discharge_currents
  {
    size_t size = ros_message->max_discharge_currents.size;
    auto array_ptr = ros_message->max_discharge_currents.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: max_charge_currents
  {
    size_t size = ros_message->max_charge_currents.size;
    auto array_ptr = ros_message->max_charge_currents.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  // Field name: present_currents
  {
    size_t size = ros_message->present_currents.size;
    auto array_ptr = ros_message->present_currents.data;
    cdr << static_cast<uint32_t>(size);
    cdr.serializeArray(array_ptr, size);
  }

  return true;
}

static bool _BatteryState__cdr_deserialize(
  eprosima::fastcdr::Cdr & cdr,
  void * untyped_ros_message)
{
  if (!untyped_ros_message) {
    fprintf(stderr, "ros message handle is null\n");
    return false;
  }
  _BatteryState__ros_msg_type * ros_message = static_cast<_BatteryState__ros_msg_type *>(untyped_ros_message);
  // Field name: battery_state
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->battery_state.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->battery_state);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->battery_state, size)) {
      fprintf(stderr, "failed to create array for field 'battery_state'");
      return false;
    }
    auto array_ptr = ros_message->battery_state.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: error_state
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->error_state.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->error_state);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->error_state, size)) {
      fprintf(stderr, "failed to create array for field 'error_state'");
      return false;
    }
    auto array_ptr = ros_message->error_state.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: battery_voltages
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->battery_voltages.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->battery_voltages);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->battery_voltages, size)) {
      fprintf(stderr, "failed to create array for field 'battery_voltages'");
      return false;
    }
    auto array_ptr = ros_message->battery_voltages.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: cell_voltages
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->cell_voltages.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->cell_voltages);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->cell_voltages, size)) {
      fprintf(stderr, "failed to create array for field 'cell_voltages'");
      return false;
    }
    auto array_ptr = ros_message->cell_voltages.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: temperatures
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->temperatures.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->temperatures);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->temperatures, size)) {
      fprintf(stderr, "failed to create array for field 'temperatures'");
      return false;
    }
    auto array_ptr = ros_message->temperatures.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: discharge_statuses
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->discharge_statuses.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->discharge_statuses);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->discharge_statuses, size)) {
      fprintf(stderr, "failed to create array for field 'discharge_statuses'");
      return false;
    }
    auto array_ptr = ros_message->discharge_statuses.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: charge_statuses
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->charge_statuses.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->charge_statuses);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->charge_statuses, size)) {
      fprintf(stderr, "failed to create array for field 'charge_statuses'");
      return false;
    }
    auto array_ptr = ros_message->charge_statuses.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: max_discharge_currents
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->max_discharge_currents.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->max_discharge_currents);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->max_discharge_currents, size)) {
      fprintf(stderr, "failed to create array for field 'max_discharge_currents'");
      return false;
    }
    auto array_ptr = ros_message->max_discharge_currents.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: max_charge_currents
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->max_charge_currents.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->max_charge_currents);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->max_charge_currents, size)) {
      fprintf(stderr, "failed to create array for field 'max_charge_currents'");
      return false;
    }
    auto array_ptr = ros_message->max_charge_currents.data;
    cdr.deserializeArray(array_ptr, size);
  }

  // Field name: present_currents
  {
    uint32_t cdrSize;
    cdr >> cdrSize;
    size_t size = static_cast<size_t>(cdrSize);

    // Check there are at least 'size' remaining bytes in the CDR stream before resizing
    auto old_state = cdr.getState();
    bool correct_size = cdr.jump(size);
    cdr.setState(old_state);
    if (!correct_size) {
      fprintf(stderr, "sequence size exceeds remaining buffer\n");
      return false;
    }

    if (ros_message->present_currents.data) {
      rosidl_runtime_c__double__Sequence__fini(&ros_message->present_currents);
    }
    if (!rosidl_runtime_c__double__Sequence__init(&ros_message->present_currents, size)) {
      fprintf(stderr, "failed to create array for field 'present_currents'");
      return false;
    }
    auto array_ptr = ros_message->present_currents.data;
    cdr.deserializeArray(array_ptr, size);
  }

  return true;
}  // NOLINT(readability/fn_size)

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_themis_state_msgs
size_t get_serialized_size_themis_state_msgs__msg__BatteryState(
  const void * untyped_ros_message,
  size_t current_alignment)
{
  const _BatteryState__ros_msg_type * ros_message = static_cast<const _BatteryState__ros_msg_type *>(untyped_ros_message);
  (void)ros_message;
  size_t initial_alignment = current_alignment;

  const size_t padding = 4;
  const size_t wchar_size = 4;
  (void)padding;
  (void)wchar_size;

  // field.name battery_state
  {
    size_t array_size = ros_message->battery_state.size;
    auto array_ptr = ros_message->battery_state.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name error_state
  {
    size_t array_size = ros_message->error_state.size;
    auto array_ptr = ros_message->error_state.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name battery_voltages
  {
    size_t array_size = ros_message->battery_voltages.size;
    auto array_ptr = ros_message->battery_voltages.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name cell_voltages
  {
    size_t array_size = ros_message->cell_voltages.size;
    auto array_ptr = ros_message->cell_voltages.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name temperatures
  {
    size_t array_size = ros_message->temperatures.size;
    auto array_ptr = ros_message->temperatures.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name discharge_statuses
  {
    size_t array_size = ros_message->discharge_statuses.size;
    auto array_ptr = ros_message->discharge_statuses.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name charge_statuses
  {
    size_t array_size = ros_message->charge_statuses.size;
    auto array_ptr = ros_message->charge_statuses.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name max_discharge_currents
  {
    size_t array_size = ros_message->max_discharge_currents.size;
    auto array_ptr = ros_message->max_discharge_currents.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name max_charge_currents
  {
    size_t array_size = ros_message->max_charge_currents.size;
    auto array_ptr = ros_message->max_charge_currents.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }
  // field.name present_currents
  {
    size_t array_size = ros_message->present_currents.size;
    auto array_ptr = ros_message->present_currents.data;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);
    (void)array_ptr;
    size_t item_size = sizeof(array_ptr[0]);
    current_alignment += array_size * item_size +
      eprosima::fastcdr::Cdr::alignment(current_alignment, item_size);
  }

  return current_alignment - initial_alignment;
}

static uint32_t _BatteryState__get_serialized_size(const void * untyped_ros_message)
{
  return static_cast<uint32_t>(
    get_serialized_size_themis_state_msgs__msg__BatteryState(
      untyped_ros_message, 0));
}

ROSIDL_TYPESUPPORT_FASTRTPS_C_PUBLIC_themis_state_msgs
size_t max_serialized_size_themis_state_msgs__msg__BatteryState(
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

  // member: battery_state
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: error_state
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: battery_voltages
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: cell_voltages
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: temperatures
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: discharge_statuses
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: charge_statuses
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: max_discharge_currents
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: max_charge_currents
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }
  // member: present_currents
  {
    size_t array_size = 0;
    full_bounded = false;
    is_plain = false;
    current_alignment += padding +
      eprosima::fastcdr::Cdr::alignment(current_alignment, padding);

    last_member_size = array_size * sizeof(uint64_t);
    current_alignment += array_size * sizeof(uint64_t) +
      eprosima::fastcdr::Cdr::alignment(current_alignment, sizeof(uint64_t));
  }

  size_t ret_val = current_alignment - initial_alignment;
  if (is_plain) {
    // All members are plain, and type is not empty.
    // We still need to check that the in-memory alignment
    // is the same as the CDR mandated alignment.
    using DataType = themis_state_msgs__msg__BatteryState;
    is_plain =
      (
      offsetof(DataType, present_currents) +
      last_member_size
      ) == ret_val;
  }

  return ret_val;
}

static size_t _BatteryState__max_serialized_size(char & bounds_info)
{
  bool full_bounded;
  bool is_plain;
  size_t ret_val;

  ret_val = max_serialized_size_themis_state_msgs__msg__BatteryState(
    full_bounded, is_plain, 0);

  bounds_info =
    is_plain ? ROSIDL_TYPESUPPORT_FASTRTPS_PLAIN_TYPE :
    full_bounded ? ROSIDL_TYPESUPPORT_FASTRTPS_BOUNDED_TYPE : ROSIDL_TYPESUPPORT_FASTRTPS_UNBOUNDED_TYPE;
  return ret_val;
}


static message_type_support_callbacks_t __callbacks_BatteryState = {
  "themis_state_msgs::msg",
  "BatteryState",
  _BatteryState__cdr_serialize,
  _BatteryState__cdr_deserialize,
  _BatteryState__get_serialized_size,
  _BatteryState__max_serialized_size
};

static rosidl_message_type_support_t _BatteryState__type_support = {
  rosidl_typesupport_fastrtps_c__identifier,
  &__callbacks_BatteryState,
  get_message_typesupport_handle_function,
};

const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_fastrtps_c, themis_state_msgs, msg, BatteryState)() {
  return &_BatteryState__type_support;
}

#if defined(__cplusplus)
}
#endif
