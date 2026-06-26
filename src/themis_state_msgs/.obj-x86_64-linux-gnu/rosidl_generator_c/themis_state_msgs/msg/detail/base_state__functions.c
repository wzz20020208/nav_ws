// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/BaseState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/base_state__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


bool
themis_state_msgs__msg__BaseState__init(themis_state_msgs__msg__BaseState * msg)
{
  if (!msg) {
    return false;
  }
  // position_x
  // position_y
  // position_z
  // velocity_x
  // velocity_y
  // velocity_z
  // yaw
  // roll
  // pitch
  // angular_rate
  return true;
}

void
themis_state_msgs__msg__BaseState__fini(themis_state_msgs__msg__BaseState * msg)
{
  if (!msg) {
    return;
  }
  // position_x
  // position_y
  // position_z
  // velocity_x
  // velocity_y
  // velocity_z
  // yaw
  // roll
  // pitch
  // angular_rate
}

bool
themis_state_msgs__msg__BaseState__are_equal(const themis_state_msgs__msg__BaseState * lhs, const themis_state_msgs__msg__BaseState * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // position_x
  if (lhs->position_x != rhs->position_x) {
    return false;
  }
  // position_y
  if (lhs->position_y != rhs->position_y) {
    return false;
  }
  // position_z
  if (lhs->position_z != rhs->position_z) {
    return false;
  }
  // velocity_x
  if (lhs->velocity_x != rhs->velocity_x) {
    return false;
  }
  // velocity_y
  if (lhs->velocity_y != rhs->velocity_y) {
    return false;
  }
  // velocity_z
  if (lhs->velocity_z != rhs->velocity_z) {
    return false;
  }
  // yaw
  if (lhs->yaw != rhs->yaw) {
    return false;
  }
  // roll
  if (lhs->roll != rhs->roll) {
    return false;
  }
  // pitch
  if (lhs->pitch != rhs->pitch) {
    return false;
  }
  // angular_rate
  if (lhs->angular_rate != rhs->angular_rate) {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__BaseState__copy(
  const themis_state_msgs__msg__BaseState * input,
  themis_state_msgs__msg__BaseState * output)
{
  if (!input || !output) {
    return false;
  }
  // position_x
  output->position_x = input->position_x;
  // position_y
  output->position_y = input->position_y;
  // position_z
  output->position_z = input->position_z;
  // velocity_x
  output->velocity_x = input->velocity_x;
  // velocity_y
  output->velocity_y = input->velocity_y;
  // velocity_z
  output->velocity_z = input->velocity_z;
  // yaw
  output->yaw = input->yaw;
  // roll
  output->roll = input->roll;
  // pitch
  output->pitch = input->pitch;
  // angular_rate
  output->angular_rate = input->angular_rate;
  return true;
}

themis_state_msgs__msg__BaseState *
themis_state_msgs__msg__BaseState__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BaseState * msg = (themis_state_msgs__msg__BaseState *)allocator.allocate(sizeof(themis_state_msgs__msg__BaseState), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__BaseState));
  bool success = themis_state_msgs__msg__BaseState__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__BaseState__destroy(themis_state_msgs__msg__BaseState * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__BaseState__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__BaseState__Sequence__init(themis_state_msgs__msg__BaseState__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BaseState * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__BaseState *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__BaseState), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__BaseState__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__BaseState__fini(&data[i - 1]);
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
themis_state_msgs__msg__BaseState__Sequence__fini(themis_state_msgs__msg__BaseState__Sequence * array)
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
      themis_state_msgs__msg__BaseState__fini(&array->data[i]);
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

themis_state_msgs__msg__BaseState__Sequence *
themis_state_msgs__msg__BaseState__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__BaseState__Sequence * array = (themis_state_msgs__msg__BaseState__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__BaseState__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__BaseState__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__BaseState__Sequence__destroy(themis_state_msgs__msg__BaseState__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__BaseState__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__BaseState__Sequence__are_equal(const themis_state_msgs__msg__BaseState__Sequence * lhs, const themis_state_msgs__msg__BaseState__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__BaseState__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__BaseState__Sequence__copy(
  const themis_state_msgs__msg__BaseState__Sequence * input,
  themis_state_msgs__msg__BaseState__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__BaseState);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__BaseState * data =
      (themis_state_msgs__msg__BaseState *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__BaseState__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__BaseState__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__BaseState__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
