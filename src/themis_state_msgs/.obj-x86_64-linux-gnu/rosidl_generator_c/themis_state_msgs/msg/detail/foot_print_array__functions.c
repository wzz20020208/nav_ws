// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/FootPrintArray.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/foot_print_array__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `current_foot_print`
// Member `goal_foot_print`
#include "themis_state_msgs/msg/detail/foot_array__functions.h"

bool
themis_state_msgs__msg__FootPrintArray__init(themis_state_msgs__msg__FootPrintArray * msg)
{
  if (!msg) {
    return false;
  }
  // current_foot_print
  if (!themis_state_msgs__msg__FootArray__Sequence__init(&msg->current_foot_print, 0)) {
    themis_state_msgs__msg__FootPrintArray__fini(msg);
    return false;
  }
  // goal_foot_print
  if (!themis_state_msgs__msg__FootArray__Sequence__init(&msg->goal_foot_print, 0)) {
    themis_state_msgs__msg__FootPrintArray__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__FootPrintArray__fini(themis_state_msgs__msg__FootPrintArray * msg)
{
  if (!msg) {
    return;
  }
  // current_foot_print
  themis_state_msgs__msg__FootArray__Sequence__fini(&msg->current_foot_print);
  // goal_foot_print
  themis_state_msgs__msg__FootArray__Sequence__fini(&msg->goal_foot_print);
}

bool
themis_state_msgs__msg__FootPrintArray__are_equal(const themis_state_msgs__msg__FootPrintArray * lhs, const themis_state_msgs__msg__FootPrintArray * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // current_foot_print
  if (!themis_state_msgs__msg__FootArray__Sequence__are_equal(
      &(lhs->current_foot_print), &(rhs->current_foot_print)))
  {
    return false;
  }
  // goal_foot_print
  if (!themis_state_msgs__msg__FootArray__Sequence__are_equal(
      &(lhs->goal_foot_print), &(rhs->goal_foot_print)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__FootPrintArray__copy(
  const themis_state_msgs__msg__FootPrintArray * input,
  themis_state_msgs__msg__FootPrintArray * output)
{
  if (!input || !output) {
    return false;
  }
  // current_foot_print
  if (!themis_state_msgs__msg__FootArray__Sequence__copy(
      &(input->current_foot_print), &(output->current_foot_print)))
  {
    return false;
  }
  // goal_foot_print
  if (!themis_state_msgs__msg__FootArray__Sequence__copy(
      &(input->goal_foot_print), &(output->goal_foot_print)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__FootPrintArray *
themis_state_msgs__msg__FootPrintArray__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__FootPrintArray * msg = (themis_state_msgs__msg__FootPrintArray *)allocator.allocate(sizeof(themis_state_msgs__msg__FootPrintArray), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__FootPrintArray));
  bool success = themis_state_msgs__msg__FootPrintArray__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__FootPrintArray__destroy(themis_state_msgs__msg__FootPrintArray * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__FootPrintArray__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__FootPrintArray__Sequence__init(themis_state_msgs__msg__FootPrintArray__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__FootPrintArray * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__FootPrintArray *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__FootPrintArray), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__FootPrintArray__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__FootPrintArray__fini(&data[i - 1]);
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
themis_state_msgs__msg__FootPrintArray__Sequence__fini(themis_state_msgs__msg__FootPrintArray__Sequence * array)
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
      themis_state_msgs__msg__FootPrintArray__fini(&array->data[i]);
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

themis_state_msgs__msg__FootPrintArray__Sequence *
themis_state_msgs__msg__FootPrintArray__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__FootPrintArray__Sequence * array = (themis_state_msgs__msg__FootPrintArray__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__FootPrintArray__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__FootPrintArray__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__FootPrintArray__Sequence__destroy(themis_state_msgs__msg__FootPrintArray__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__FootPrintArray__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__FootPrintArray__Sequence__are_equal(const themis_state_msgs__msg__FootPrintArray__Sequence * lhs, const themis_state_msgs__msg__FootPrintArray__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__FootPrintArray__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__FootPrintArray__Sequence__copy(
  const themis_state_msgs__msg__FootPrintArray__Sequence * input,
  themis_state_msgs__msg__FootPrintArray__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__FootPrintArray);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__FootPrintArray * data =
      (themis_state_msgs__msg__FootPrintArray *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__FootPrintArray__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__FootPrintArray__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__FootPrintArray__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
