// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/BearTemperature.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/bear_temperature__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `right_leg_bear_temperature`
// Member `left_leg_bear_temperature`
// Member `right_arm_bear_temperature`
// Member `left_arm_bear_temperature`
// Member `head_bear_temperature`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

bool
themis_state_msgs__msg__BearTemperature__init(themis_state_msgs__msg__BearTemperature * msg)
{
  if (!msg) {
    return false;
  }
  // right_leg_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__init(&msg->right_leg_bear_temperature, 0)) {
    themis_state_msgs__msg__BearTemperature__fini(msg);
    return false;
  }
  // left_leg_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__init(&msg->left_leg_bear_temperature, 0)) {
    themis_state_msgs__msg__BearTemperature__fini(msg);
    return false;
  }
  // right_arm_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__init(&msg->right_arm_bear_temperature, 0)) {
    themis_state_msgs__msg__BearTemperature__fini(msg);
    return false;
  }
  // left_arm_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__init(&msg->left_arm_bear_temperature, 0)) {
    themis_state_msgs__msg__BearTemperature__fini(msg);
    return false;
  }
  // head_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__init(&msg->head_bear_temperature, 0)) {
    themis_state_msgs__msg__BearTemperature__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__BearTemperature__fini(themis_state_msgs__msg__BearTemperature * msg)
{
  if (!msg) {
    return;
  }
  // right_leg_bear_temperature
  rosidl_runtime_c__double__Sequence__fini(&msg->right_leg_bear_temperature);
  // left_leg_bear_temperature
  rosidl_runtime_c__double__Sequence__fini(&msg->left_leg_bear_temperature);
  // right_arm_bear_temperature
  rosidl_runtime_c__double__Sequence__fini(&msg->right_arm_bear_temperature);
  // left_arm_bear_temperature
  rosidl_runtime_c__double__Sequence__fini(&msg->left_arm_bear_temperature);
  // head_bear_temperature
  rosidl_runtime_c__double__Sequence__fini(&msg->head_bear_temperature);
}

bool
themis_state_msgs__msg__BearTemperature__are_equal(const themis_state_msgs__msg__BearTemperature * lhs, const themis_state_msgs__msg__BearTemperature * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // right_leg_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->right_leg_bear_temperature), &(rhs->right_leg_bear_temperature)))
  {
    return false;
  }
  // left_leg_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->left_leg_bear_temperature), &(rhs->left_leg_bear_temperature)))
  {
    return false;
  }
  // right_arm_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->right_arm_bear_temperature), &(rhs->right_arm_bear_temperature)))
  {
    return false;
  }
  // left_arm_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->left_arm_bear_temperature), &(rhs->left_arm_bear_temperature)))
  {
    return false;
  }
  // head_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__are_equal(
      &(lhs->head_bear_temperature), &(rhs->head_bear_temperature)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__BearTemperature__copy(
  const themis_state_msgs__msg__BearTemperature * input,
  themis_state_msgs__msg__BearTemperature * output)
{
  if (!input || !output) {
    return false;
  }
  // right_leg_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->right_leg_bear_temperature), &(output->right_leg_bear_temperature)))
  {
    return false;
  }
  // left_leg_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->left_leg_bear_temperature), &(output->left_leg_bear_temperature)))
  {
    return false;
  }
  // right_arm_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->right_arm_bear_temperature), &(output->right_arm_bear_temperature)))
  {
    return false;
  }
  // left_arm_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->left_arm_bear_temperature), &(output->left_arm_bear_temperature)))
  {
    return false;
  }
  // head_bear_temperature
  if (!rosidl_runtime_c__double__Sequence__copy(
      &(input->head_bear_temperature), &(output->head_bear_temperature)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__BearTemperature *
themis_state_msgs__msg__BearTemperature__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BearTemperature * msg = (themis_state_msgs__msg__BearTemperature *)allocator.allocate(sizeof(themis_state_msgs__msg__BearTemperature), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__BearTemperature));
  bool success = themis_state_msgs__msg__BearTemperature__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__BearTemperature__destroy(themis_state_msgs__msg__BearTemperature * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__BearTemperature__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__BearTemperature__Sequence__init(themis_state_msgs__msg__BearTemperature__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BearTemperature * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__BearTemperature *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__BearTemperature), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__BearTemperature__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__BearTemperature__fini(&data[i - 1]);
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
themis_state_msgs__msg__BearTemperature__Sequence__fini(themis_state_msgs__msg__BearTemperature__Sequence * array)
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
      themis_state_msgs__msg__BearTemperature__fini(&array->data[i]);
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

themis_state_msgs__msg__BearTemperature__Sequence *
themis_state_msgs__msg__BearTemperature__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BearTemperature__Sequence * array = (themis_state_msgs__msg__BearTemperature__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__BearTemperature__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__BearTemperature__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__BearTemperature__Sequence__destroy(themis_state_msgs__msg__BearTemperature__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__BearTemperature__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__BearTemperature__Sequence__are_equal(const themis_state_msgs__msg__BearTemperature__Sequence * lhs, const themis_state_msgs__msg__BearTemperature__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__BearTemperature__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__BearTemperature__Sequence__copy(
  const themis_state_msgs__msg__BearTemperature__Sequence * input,
  themis_state_msgs__msg__BearTemperature__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__BearTemperature);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__BearTemperature * data =
      (themis_state_msgs__msg__BearTemperature *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__BearTemperature__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__BearTemperature__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__BearTemperature__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
