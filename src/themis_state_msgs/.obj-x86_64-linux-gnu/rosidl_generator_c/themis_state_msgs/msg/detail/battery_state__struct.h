// generated from rosidl_generator_c/resource/idl__struct.h.em
// with input from themis_state_msgs:msg/BatteryState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__STRUCT_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__STRUCT_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


// Constants defined in the message

// Include directives for member types
// Member 'battery_state'
// Member 'error_state'
// Member 'battery_voltages'
// Member 'cell_voltages'
// Member 'temperatures'
// Member 'discharge_statuses'
// Member 'charge_statuses'
// Member 'max_discharge_currents'
// Member 'max_charge_currents'
// Member 'present_currents'
#include "rosidl_runtime_c/primitives_sequence.h"

/// Struct defined in msg/BatteryState in the package themis_state_msgs.
/**
  * BatteryState.msg
 */
typedef struct themis_state_msgs__msg__BatteryState
{
  rosidl_runtime_c__double__Sequence battery_state;
  rosidl_runtime_c__double__Sequence error_state;
  rosidl_runtime_c__double__Sequence battery_voltages;
  rosidl_runtime_c__double__Sequence cell_voltages;
  rosidl_runtime_c__double__Sequence temperatures;
  rosidl_runtime_c__double__Sequence discharge_statuses;
  rosidl_runtime_c__double__Sequence charge_statuses;
  rosidl_runtime_c__double__Sequence max_discharge_currents;
  rosidl_runtime_c__double__Sequence max_charge_currents;
  rosidl_runtime_c__double__Sequence present_currents;
} themis_state_msgs__msg__BatteryState;

// Struct for a sequence of themis_state_msgs__msg__BatteryState.
typedef struct themis_state_msgs__msg__BatteryState__Sequence
{
  themis_state_msgs__msg__BatteryState * data;
  /// The number of valid items in data
  size_t size;
  /// The number of allocated items in data
  size_t capacity;
} themis_state_msgs__msg__BatteryState__Sequence;

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__STRUCT_H_
