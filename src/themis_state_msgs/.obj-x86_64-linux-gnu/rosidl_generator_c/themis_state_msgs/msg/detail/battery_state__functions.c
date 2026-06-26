// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/BatteryState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/battery_state__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


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

bool
themis_state_msgs__msg__BatteryState__init(themis_state_msgs__msg__BatteryState * msg)
{
  if (!msg) {
    return false;
  }
  // battery_state
  if (!rosidl_runtime_c__double__Sequence__init(&msg->battery_state, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // error_state
  if (!rosidl_runtime_c__double__Sequence__init(&msg->error_state, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // battery_voltages
  if (!rosidl_runtime_c__double__Sequence__init(&msg->battery_voltages, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // cell_voltages
  if (!rosidl_runtime_c__double__Sequence__init(&msg->cell_voltages, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // temperatures
  if (!rosidl_runtime_c__double__Sequence__init(&msg->temperatures, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // discharge_statuses
  if (!rosidl_runtime_c__double__Sequence__init(&msg->discharge_statuses, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // charge_statuses
  if (!rosidl_runtime_c__double__Sequence__init(&msg->charge_statuses, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // max_discharge_currents
  if (!rosidl_runtime_c__double__Sequence__init(&msg->max_discharge_currents, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // max_charge_currents
  if (!rosidl_runtime_c__double__Sequence__init(&msg->max_charge_currents, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  // present_currents
  if (!rosidl_runtime_c__double__Sequence__init(&msg->present_currents, 0)) {
    themis_state_msgs__msg__BatteryState__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__BatteryState__fini(themis_state_msgs__msg__BatteryState * msg)
{
  if (!msg) {
    return;
  }
  // battery_state
  rosidl_runtime_c__double__Sequence__fini(&msg->battery_state);
  // error_state
  rosidl_runtime_c__double__Sequence__fini(&msg->error_state);
  // battery_voltages
  rosidl_runtime_c__double__Sequence__fini(&msg->battery_voltages);
  // cell_voltages
  rosidl_runtime_c__double__Sequence__fini(&msg->cell_voltages);
  // temperatures
  rosidl_runtime_c__double__Sequence__fini(&msg->temperatures);
  // discharge_statuses
  rosidl_runtime_c__double__Sequence__fini(&msg->discharge_statuses);
  // charge_statuses
  rosidl_runtime_c__double__Sequence__fini(&msg->charge_statuses);
  // max_discharge_currents
  rosidl_runtime_c__double__Sequence__fini(&msg->max_discharge_currents);
  // max_charge_currents
  rosidl_runtime_c__double__Sequence__fini(&msg->max_charge_currents);
  // present_currents
  rosidl_runtime_c__double__Sequence__fini(&msg->present_currents);
}

bool
themis_state_msgs__msg__BatteryState__are_equal(const themis_state_msgs__msg__BatteryState * lhs, const themis_state_msgs__msg__BatteryState * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // battery_state
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->battery_state), &(rhs->battery_state)))
  {
    return false;
  }
  // error_state
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->error_state), &(rhs->error_state)))
  {
    return false;
  }
  // battery_voltages
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->battery_voltages), &(rhs->battery_voltages)))
  {
    return false;
  }
  // cell_voltages
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->cell_voltages), &(rhs->cell_voltages)))
  {
    return false;
  }
  // temperatures
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->temperatures), &(rhs->temperatures)))
  {
    return false;
  }
  // discharge_statuses
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->discharge_statuses), &(rhs->discharge_statuses)))
  {
    return false;
  }
  // charge_statuses
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->charge_statuses), &(rhs->charge_statuses)))
  {
    return false;
  }
  // max_discharge_currents
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->max_discharge_currents), &(rhs->max_discharge_currents)))
  {
    return false;
  }
  // max_charge_currents
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->max_charge_currents), &(rhs->max_charge_currents)))
  {
    return false;
  }
  // present_currents
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->present_currents), &(rhs->present_currents)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__BatteryState__copy(
  const themis_state_msgs__msg__BatteryState * input,
  themis_state_msgs__msg__BatteryState * output)
{
  if (!input || !output) {
    return false;
  }
  // battery_state
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->battery_state), &(output->battery_state)))
  {
    return false;
  }
  // error_state
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->error_state), &(output->error_state)))
  {
    return false;
  }
  // battery_voltages
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->battery_voltages), &(output->battery_voltages)))
  {
    return false;
  }
  // cell_voltages
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->cell_voltages), &(output->cell_voltages)))
  {
    return false;
  }
  // temperatures
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->temperatures), &(output->temperatures)))
  {
    return false;
  }
  // discharge_statuses
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->discharge_statuses), &(output->discharge_statuses)))
  {
    return false;
  }
  // charge_statuses
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->charge_statuses), &(output->charge_statuses)))
  {
    return false;
  }
  // max_discharge_currents
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->max_discharge_currents), &(output->max_discharge_currents)))
  {
    return false;
  }
  // max_charge_currents
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->max_charge_currents), &(output->max_charge_currents)))
  {
    return false;
  }
  // present_currents
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->present_currents), &(output->present_currents)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__BatteryState *
themis_state_msgs__msg__BatteryState__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BatteryState * msg = (themis_state_msgs__msg__BatteryState *)allocator.allocate(sizeof(themis_state_msgs__msg__BatteryState), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__BatteryState));
  bool success = themis_state_msgs__msg__BatteryState__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__BatteryState__destroy(themis_state_msgs__msg__BatteryState * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__BatteryState__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__BatteryState__Sequence__init(themis_state_msgs__msg__BatteryState__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BatteryState * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__BatteryState *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__BatteryState), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__BatteryState__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__BatteryState__fini(&data[i - 1]);
      }
      allocator.deallocate(data, allocator.state);
      return false;
    }
  }
  array->data = data;
  array->size = size;
  array->capacity = size;
  return true;
}

void
themis_state_msgs__msg__BatteryState__Sequence__fini(themis_state_msgs__msg__BatteryState__Sequence * array)
{
  if (!array) {
    return;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();

  if (array->data) {
    // ensure that data and capacity values are consistent
    assert(array->capacity > 0);
    // finalize all array elements
    for (size_t i = 0; i < array->capacity; ++i) {
      themis_state_msgs__msg__BatteryState__fini(&array->data[i]);
    }
    allocator.deallocate(array->data, allocator.state);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
  } else {
    // ensure that data, size, and capacity values are consistent
    assert(0 == array->size);
    assert(0 == array->capacity);
  }
}

themis_state_msgs__msg__BatteryState__Sequence *
themis_state_msgs__msg__BatteryState__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BatteryState__Sequence * array = (themis_state_msgs__msg__BatteryState__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__BatteryState__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__BatteryState__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__BatteryState__Sequence__destroy(themis_state_msgs__msg__BatteryState__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__BatteryState__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__BatteryState__Sequence__are_equal(const themis_state_msgs__msg__BatteryState__Sequence * lhs, const themis_state_msgs__msg__BatteryState__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__BatteryState__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__BatteryState__Sequence__copy(
  const themis_state_msgs__msg__BatteryState__Sequence * input,
  themis_state_msgs__msg__BatteryState__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__BatteryState);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__BatteryState * data =
      (themis_state_msgs__msg__BatteryState *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__BatteryState__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__BatteryState__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__BatteryState__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
