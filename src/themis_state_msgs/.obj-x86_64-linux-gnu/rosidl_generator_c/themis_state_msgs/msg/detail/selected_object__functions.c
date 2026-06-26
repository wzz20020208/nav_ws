// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/SelectedObject.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/selected_object__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `category`
#include "rosidl_runtime_c/string_functions.h"

bool
themis_state_msgs__msg__SelectedObject__init(themis_state_msgs__msg__SelectedObject * msg)
{
  if (!msg) {
    return false;
  }
  // id
  // category
  if (!rosidl_runtime_c__String__init(&msg->category)) {
    themis_state_msgs__msg__SelectedObject__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__SelectedObject__fini(themis_state_msgs__msg__SelectedObject * msg)
{
  if (!msg) {
    return;
  }
  // id
  // category
  rosidl_runtime_c__String__fini(&msg->category);
}

bool
themis_state_msgs__msg__SelectedObject__are_equal(const themis_state_msgs__msg__SelectedObject * lhs, const themis_state_msgs__msg__SelectedObject * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // id
  if (lhs->id != rhs->id) {
    return false;
  }
  // category
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->category), &(rhs->category)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__SelectedObject__copy(
  const themis_state_msgs__msg__SelectedObject * input,
  themis_state_msgs__msg__SelectedObject * output)
{
  if (!input || !output) {
    return false;
  }
  // id
  output->id = input->id;
  // category
  if (!rosidl_runtime_c__String__copy(
      &(input->category), &(output->category)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__SelectedObject *
themis_state_msgs__msg__SelectedObject__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__SelectedObject * msg = (themis_state_msgs__msg__SelectedObject *)allocator.allocate(sizeof(themis_state_msgs__msg__SelectedObject), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__SelectedObject));
  bool success = themis_state_msgs__msg__SelectedObject__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__SelectedObject__destroy(themis_state_msgs__msg__SelectedObject * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__SelectedObject__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__SelectedObject__Sequence__init(themis_state_msgs__msg__SelectedObject__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__SelectedObject * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__SelectedObject *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__SelectedObject), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__SelectedObject__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__SelectedObject__fini(&data[i - 1]);
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
themis_state_msgs__msg__SelectedObject__Sequence__fini(themis_state_msgs__msg__SelectedObject__Sequence * array)
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
      themis_state_msgs__msg__SelectedObject__fini(&array->data[i]);
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

themis_state_msgs__msg__SelectedObject__Sequence *
themis_state_msgs__msg__SelectedObject__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__SelectedObject__Sequence * array = (themis_state_msgs__msg__SelectedObject__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__SelectedObject__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__SelectedObject__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__SelectedObject__Sequence__destroy(themis_state_msgs__msg__SelectedObject__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__SelectedObject__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__SelectedObject__Sequence__are_equal(const themis_state_msgs__msg__SelectedObject__Sequence * lhs, const themis_state_msgs__msg__SelectedObject__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__SelectedObject__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__SelectedObject__Sequence__copy(
  const themis_state_msgs__msg__SelectedObject__Sequence * input,
  themis_state_msgs__msg__SelectedObject__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__SelectedObject);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__SelectedObject * data =
      (themis_state_msgs__msg__SelectedObject *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__SelectedObject__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__SelectedObject__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__SelectedObject__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
