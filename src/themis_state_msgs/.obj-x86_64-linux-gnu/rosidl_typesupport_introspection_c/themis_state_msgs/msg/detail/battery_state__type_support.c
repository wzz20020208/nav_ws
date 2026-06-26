// generated from rosidl_typesupport_introspection_c/resource/idl__type_support.c.em
// with input from themis_state_msgs:msg/BatteryState.idl
// generated code does not contain a copyright notice

#include <stddef.h>
#include "themis_state_msgs/msg/detail/battery_state__rosidl_typesupport_introspection_c.h"
#include "themis_state_msgs/msg/rosidl_typesupport_introspection_c__visibility_control.h"
#include "rosidl_typesupport_introspection_c/field_types.h"
#include "rosidl_typesupport_introspection_c/identifier.h"
#include "rosidl_typesupport_introspection_c/message_introspection.h"
#include "themis_state_msgs/msg/detail/battery_state__functions.h"
#include "themis_state_msgs/msg/detail/battery_state__struct.h"


// Include directives for member types
// Member `battery_state`
// Member `error_state`
// Member `battery_voltages`
// Member `cell_voltages`
// Member `temperatures`
// Member `discharge_statuses`
// Member `charge_statuses`
// Member `max_discharge_currents`
// Member `max_charge_currents`
// Member `present_currents`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

#ifdef __cplusplus
extern "C"
{
#endif

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_init_function(
  void * message_memory, enum rosidl_runtime_c__message_initialization _init)
{
  // TODO(karsten1987): initializers are not yet implemented for typesupport c
  // see https://github.com/ros2/ros2/issues/397
  (void) _init;
  themis_state_msgs__msg__BatteryState__init(message_memory);
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_fini_function(void * message_memory)
{
  themis_state_msgs__msg__BatteryState__fini(message_memory);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__battery_state(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__battery_state(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__battery_state(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__battery_state(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__battery_state(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__battery_state(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__battery_state(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__battery_state(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__error_state(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__error_state(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__error_state(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__error_state(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__error_state(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__error_state(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__error_state(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__error_state(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__battery_voltages(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__battery_voltages(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__battery_voltages(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__battery_voltages(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__battery_voltages(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__battery_voltages(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__battery_voltages(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__battery_voltages(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__cell_voltages(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__cell_voltages(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__cell_voltages(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__cell_voltages(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__cell_voltages(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__cell_voltages(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__cell_voltages(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__cell_voltages(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__temperatures(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__temperatures(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__temperatures(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__temperatures(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__temperatures(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__temperatures(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__temperatures(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__temperatures(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__discharge_statuses(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__discharge_statuses(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__discharge_statuses(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__discharge_statuses(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__discharge_statuses(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__discharge_statuses(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__discharge_statuses(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__discharge_statuses(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__charge_statuses(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__charge_statuses(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__charge_statuses(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__charge_statuses(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__charge_statuses(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__charge_statuses(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__charge_statuses(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__charge_statuses(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__max_discharge_currents(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__max_discharge_currents(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__max_discharge_currents(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__max_discharge_currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__max_discharge_currents(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__max_discharge_currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__max_discharge_currents(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__max_discharge_currents(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__max_charge_currents(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__max_charge_currents(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__max_charge_currents(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__max_charge_currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__max_charge_currents(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__max_charge_currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__max_charge_currents(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__max_charge_currents(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

size_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__present_currents(
  const void * untyped_member)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return member->size;
}

const void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__present_currents(
  const void * untyped_member, size_t index)
{
  const rosidl_runtime_c__double__Sequence * member =
    (const rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void * themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__present_currents(
  void * untyped_member, size_t index)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  return &member->data[index];
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__present_currents(
  const void * untyped_member, size_t index, void * untyped_value)
{
  const double * item =
    ((const double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__present_currents(untyped_member, index));
  double * value =
    (double *)(untyped_value);
  *value = *item;
}

void themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__present_currents(
  void * untyped_member, size_t index, const void * untyped_value)
{
  double * item =
    ((double *)
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__present_currents(untyped_member, index));
  const double * value =
    (const double *)(untyped_value);
  *item = *value;
}

bool themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__present_currents(
  void * untyped_member, size_t size)
{
  rosidl_runtime_c__double__Sequence * member =
    (rosidl_runtime_c__double__Sequence *)(untyped_member);
  rosidl_runtime_c__double__Sequence__fini(member);
  return rosidl_runtime_c__double__Sequence__init(member, size);
}

static rosidl_typesupport_introspection_c__MessageMember themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_member_array[10] = {
  {
    "battery_state",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, battery_state),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__battery_state,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__battery_state,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__battery_state,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__battery_state,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__battery_state,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__battery_state  // resize(index) function pointer
  },
  {
    "error_state",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, error_state),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__error_state,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__error_state,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__error_state,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__error_state,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__error_state,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__error_state  // resize(index) function pointer
  },
  {
    "battery_voltages",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, battery_voltages),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__battery_voltages,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__battery_voltages,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__battery_voltages,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__battery_voltages,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__battery_voltages,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__battery_voltages  // resize(index) function pointer
  },
  {
    "cell_voltages",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, cell_voltages),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__cell_voltages,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__cell_voltages,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__cell_voltages,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__cell_voltages,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__cell_voltages,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__cell_voltages  // resize(index) function pointer
  },
  {
    "temperatures",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, temperatures),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__temperatures,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__temperatures,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__temperatures,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__temperatures,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__temperatures,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__temperatures  // resize(index) function pointer
  },
  {
    "discharge_statuses",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, discharge_statuses),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__discharge_statuses,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__discharge_statuses,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__discharge_statuses,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__discharge_statuses,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__discharge_statuses,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__discharge_statuses  // resize(index) function pointer
  },
  {
    "charge_statuses",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, charge_statuses),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__charge_statuses,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__charge_statuses,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__charge_statuses,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__charge_statuses,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__charge_statuses,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__charge_statuses  // resize(index) function pointer
  },
  {
    "max_discharge_currents",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, max_discharge_currents),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__max_discharge_currents,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__max_discharge_currents,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__max_discharge_currents,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__max_discharge_currents,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__max_discharge_currents,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__max_discharge_currents  // resize(index) function pointer
  },
  {
    "max_charge_currents",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, max_charge_currents),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__max_charge_currents,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__max_charge_currents,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__max_charge_currents,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__max_charge_currents,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__max_charge_currents,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__max_charge_currents  // resize(index) function pointer
  },
  {
    "present_currents",  // name
    rosidl_typesupport_introspection_c__ROS_TYPE_DOUBLE,  // type
    0,  // upper bound of string
    NULL,  // members of sub message
    true,  // is array
    0,  // array size
    false,  // is upper bound
    offsetof(themis_state_msgs__msg__BatteryState, present_currents),  // bytes offset in struct
    NULL,  // default value
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__size_function__BatteryState__present_currents,  // size() function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_const_function__BatteryState__present_currents,  // get_const(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__get_function__BatteryState__present_currents,  // get(index) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__fetch_function__BatteryState__present_currents,  // fetch(index, &value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__assign_function__BatteryState__present_currents,  // assign(index, value) function pointer
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__resize_function__BatteryState__present_currents  // resize(index) function pointer
  }
};

static const rosidl_typesupport_introspection_c__MessageMembers themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_members = {
  "themis_state_msgs__msg",  // message namespace
  "BatteryState",  // message name
  10,  // number of fields
  sizeof(themis_state_msgs__msg__BatteryState),
  themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_member_array,  // message members
  themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_init_function,  // function to initialize message memory (memory has to be allocated)
  themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_fini_function  // function to terminate message instance (will not free memory)
};

// this is not const since it must be initialized on first access
// since C does not allow non-integral compile-time constants
static rosidl_message_type_support_t themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_type_support_handle = {
  0,
  &themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_members,
  get_message_typesupport_handle_function,
};

ROSIDL_TYPESUPPORT_INTROSPECTION_C_EXPORT_themis_state_msgs
const rosidl_message_type_support_t *
ROSIDL_TYPESUPPORT_INTERFACE__MESSAGE_SYMBOL_NAME(rosidl_typesupport_introspection_c, themis_state_msgs, msg, BatteryState)() {
  if (!themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_type_support_handle.typesupport_identifier) {
    themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_type_support_handle.typesupport_identifier =
      rosidl_typesupport_introspection_c__identifier;
  }
  return &themis_state_msgs__msg__BatteryState__rosidl_typesupport_introspection_c__BatteryState_message_type_support_handle;
}
#ifdef __cplusplus
}
#endif
