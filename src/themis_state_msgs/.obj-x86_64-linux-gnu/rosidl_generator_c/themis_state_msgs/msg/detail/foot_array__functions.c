// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/FootArray.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/foot_array__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `feet`
#include "themis_state_msgs/msg/detail/foot__functions.h"

bool
themis_state_msgs__msg__FootArray__init(themis_state_msgs__msg__FootArray * msg)
{
  if (!msg) {
    return false;
  }
  // feet
  if (!themis_state_msgs__msg__Foot__Sequence__init(&msg->feet, 0)) {
    themis_state_msgs__msg__FootArray__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__FootArray__fini(themis_state_msgs__msg__FootArray * msg)
{
  if (!msg) {
    return;
  }
  // feet
  themis_state_msgs__msg__Foot__Sequence__fini(&msg->feet);
}

bool
themis_state_msgs__msg__FootArray__are_equal(const themis_state_msgs__msg__FootArray * lhs, const themis_state_msgs__msg__FootArray * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // feet
  if (!themis_state_msgs__msg__Foot__Sequence__are_equal(
      &(lhs->feet), &(rhs->feet)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__FootArray__copy(
  const themis_state_msgs__msg__FootArray * input,
  themis_state_msgs__msg__FootArray * output)
{
  if (!input || !output) {
    return false;
  }
  // feet
  if (!themis_state_msgs__msg__Foot__Sequence__copy(
      &(input->feet), &(output->feet)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__FootArray *
themis_state_msgs__msg__FootArray__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__FootArray * msg = (themis_state_msgs__msg__FootArray *)allocator.allocate(sizeof(themis_state_msgs__msg__FootArray), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__FootArray));
  bool success = themis_state_msgs__msg__FootArray__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__FootArray__destroy(themis_state_msgs__msg__FootArray * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__FootArray__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__FootArray__Sequence__init(themis_state_msgs__msg__FootArray__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__FootArray * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__FootArray *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__FootArray), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__FootArray__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__FootArray__fini(&data[i - 1]);
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
themis_state_msgs__msg__FootArray__Sequence__fini(themis_state_msgs__msg__FootArray__Sequence * array)
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
      themis_state_msgs__msg__FootArray__fini(&array->data[i]);
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

themis_state_msgs__msg__FootArray__Sequence *
themis_state_msgs__msg__FootArray__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__FootArray__Sequence * array = (themis_state_msgs__msg__FootArray__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__FootArray__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__FootArray__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__FootArray__Sequence__destroy(themis_state_msgs__msg__FootArray__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__FootArray__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__FootArray__Sequence__are_equal(const themis_state_msgs__msg__FootArray__Sequence * lhs, const themis_state_msgs__msg__FootArray__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__FootArray__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__FootArray__Sequence__copy(
  const themis_state_msgs__msg__FootArray__Sequence * input,
  themis_state_msgs__msg__FootArray__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__FootArray);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__FootArray * data =
      (themis_state_msgs__msg__FootArray *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__FootArray__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__FootArray__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__FootArray__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
