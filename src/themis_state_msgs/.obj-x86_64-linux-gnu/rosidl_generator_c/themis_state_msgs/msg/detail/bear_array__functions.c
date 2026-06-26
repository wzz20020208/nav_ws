// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/BearArray.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/bear_array__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `positions`
// Member `velocities`
// Member `currents`
// Member `temperatures`
// Member `voltages`
// Member `errors`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

bool
themis_state_msgs__msg__BearArray__init(themis_state_msgs__msg__BearArray * msg)
{
  if (!msg) {
    return false;
  }
  // positions
  if (!rosidl_runtime_c__float__Sequence__init(&msg->positions, 0)) {
    themis_state_msgs__msg__BearArray__fini(msg);
    return false;
  }
  // velocities
  if (!rosidl_runtime_c__float__Sequence__init(&msg->velocities, 0)) {
    themis_state_msgs__msg__BearArray__fini(msg);
    return false;
  }
  // currents
  if (!rosidl_runtime_c__float__Sequence__init(&msg->currents, 0)) {
    themis_state_msgs__msg__BearArray__fini(msg);
    return false;
  }
  // temperatures
  if (!rosidl_runtime_c__float__Sequence__init(&msg->temperatures, 0)) {
    themis_state_msgs__msg__BearArray__fini(msg);
    return false;
  }
  // voltages
  if (!rosidl_runtime_c__float__Sequence__init(&msg->voltages, 0)) {
    themis_state_msgs__msg__BearArray__fini(msg);
    return false;
  }
  // errors
  if (!rosidl_runtime_c__float__Sequence__init(&msg->errors, 0)) {
    themis_state_msgs__msg__BearArray__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__BearArray__fini(themis_state_msgs__msg__BearArray * msg)
{
  if (!msg) {
    return;
  }
  // positions
  rosidl_runtime_c__float__Sequence__fini(&msg->positions);
  // velocities
  rosidl_runtime_c__float__Sequence__fini(&msg->velocities);
  // currents
  rosidl_runtime_c__float__Sequence__fini(&msg->currents);
  // temperatures
  rosidl_runtime_c__float__Sequence__fini(&msg->temperatures);
  // voltages
  rosidl_runtime_c__float__Sequence__fini(&msg->voltages);
  // errors
  rosidl_runtime_c__float__Sequence__fini(&msg->errors);
}

bool
themis_state_msgs__msg__BearArray__are_equal(const themis_state_msgs__msg__BearArray * lhs, const themis_state_msgs__msg__BearArray * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // positions
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->positions), &(rhs->positions)))
  {
    return false;
  }
  // velocities
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->velocities), &(rhs->velocities)))
  {
    return false;
  }
  // currents
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->currents), &(rhs->currents)))
  {
    return false;
  }
  // temperatures
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->temperatures), &(rhs->temperatures)))
  {
    return false;
  }
  // voltages
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->voltages), &(rhs->voltages)))
  {
    return false;
  }
  // errors
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->errors), &(rhs->errors)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__BearArray__copy(
  const themis_state_msgs__msg__BearArray * input,
  themis_state_msgs__msg__BearArray * output)
{
  if (!input || !output) {
    return false;
  }
  // positions
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->positions), &(output->positions)))
  {
    return false;
  }
  // velocities
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->velocities), &(output->velocities)))
  {
    return false;
  }
  // currents
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->currents), &(output->currents)))
  {
    return false;
  }
  // temperatures
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->temperatures), &(output->temperatures)))
  {
    return false;
  }
  // voltages
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->voltages), &(output->voltages)))
  {
    return false;
  }
  // errors
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->errors), &(output->errors)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__BearArray *
themis_state_msgs__msg__BearArray__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BearArray * msg = (themis_state_msgs__msg__BearArray *)allocator.allocate(sizeof(themis_state_msgs__msg__BearArray), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__BearArray));
  bool success = themis_state_msgs__msg__BearArray__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__BearArray__destroy(themis_state_msgs__msg__BearArray * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__BearArray__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__BearArray__Sequence__init(themis_state_msgs__msg__BearArray__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BearArray * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__BearArray *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__BearArray), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__BearArray__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__BearArray__fini(&data[i - 1]);
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
themis_state_msgs__msg__BearArray__Sequence__fini(themis_state_msgs__msg__BearArray__Sequence * array)
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
      themis_state_msgs__msg__BearArray__fini(&array->data[i]);
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

themis_state_msgs__msg__BearArray__Sequence *
themis_state_msgs__msg__BearArray__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BearArray__Sequence * array = (themis_state_msgs__msg__BearArray__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__BearArray__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__BearArray__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__BearArray__Sequence__destroy(themis_state_msgs__msg__BearArray__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__BearArray__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__BearArray__Sequence__are_equal(const themis_state_msgs__msg__BearArray__Sequence * lhs, const themis_state_msgs__msg__BearArray__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__BearArray__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__BearArray__Sequence__copy(
  const themis_state_msgs__msg__BearArray__Sequence * input,
  themis_state_msgs__msg__BearArray__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__BearArray);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__BearArray * data =
      (themis_state_msgs__msg__BearArray *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__BearArray__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__BearArray__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__BearArray__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
