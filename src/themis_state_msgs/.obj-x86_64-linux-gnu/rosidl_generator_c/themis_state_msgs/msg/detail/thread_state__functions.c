// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/ThreadState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/thread_state__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


bool
themis_state_msgs__msg__ThreadState__init(themis_state_msgs__msg__ThreadState * msg)
{
  if (!msg) {
    return false;
  }
  // bear_right_leg
  // bear_left_leg
  // bear_right_arm
  // bear_left_arm
  // bear_head
  // dxl_right_hand
  // dxl_left_hand
  // sense
  // estimation
  // control
  // locomotion
  // command
  // data
  // posture
  return true;
}

void
themis_state_msgs__msg__ThreadState__fini(themis_state_msgs__msg__ThreadState * msg)
{
  if (!msg) {
    return;
  }
  // bear_right_leg
  // bear_left_leg
  // bear_right_arm
  // bear_left_arm
  // bear_head
  // dxl_right_hand
  // dxl_left_hand
  // sense
  // estimation
  // control
  // locomotion
  // command
  // data
  // posture
}

bool
themis_state_msgs__msg__ThreadState__are_equal(const themis_state_msgs__msg__ThreadState * lhs, const themis_state_msgs__msg__ThreadState * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // bear_right_leg
  if (lhs->bear_right_leg != rhs->bear_right_leg) {
    return false;
  }
  // bear_left_leg
  if (lhs->bear_left_leg != rhs->bear_left_leg) {
    return false;
  }
  // bear_right_arm
  if (lhs->bear_right_arm != rhs->bear_right_arm) {
    return false;
  }
  // bear_left_arm
  if (lhs->bear_left_arm != rhs->bear_left_arm) {
    return false;
  }
  // bear_head
  if (lhs->bear_head != rhs->bear_head) {
    return false;
  }
  // dxl_right_hand
  if (lhs->dxl_right_hand != rhs->dxl_right_hand) {
    return false;
  }
  // dxl_left_hand
  if (lhs->dxl_left_hand != rhs->dxl_left_hand) {
    return false;
  }
  // sense
  if (lhs->sense != rhs->sense) {
    return false;
  }
  // estimation
  if (lhs->estimation != rhs->estimation) {
    return false;
  }
  // control
  if (lhs->control != rhs->control) {
    return false;
  }
  // locomotion
  if (lhs->locomotion != rhs->locomotion) {
    return false;
  }
  // command
  if (lhs->command != rhs->command) {
    return false;
  }
  // data
  if (lhs->data != rhs->data) {
    return false;
  }
  // posture
  if (lhs->posture != rhs->posture) {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__ThreadState__copy(
  const themis_state_msgs__msg__ThreadState * input,
  themis_state_msgs__msg__ThreadState * output)
{
  if (!input || !output) {
    return false;
  }
  // bear_right_leg
  output->bear_right_leg = input->bear_right_leg;
  // bear_left_leg
  output->bear_left_leg = input->bear_left_leg;
  // bear_right_arm
  output->bear_right_arm = input->bear_right_arm;
  // bear_left_arm
  output->bear_left_arm = input->bear_left_arm;
  // bear_head
  output->bear_head = input->bear_head;
  // dxl_right_hand
  output->dxl_right_hand = input->dxl_right_hand;
  // dxl_left_hand
  output->dxl_left_hand = input->dxl_left_hand;
  // sense
  output->sense = input->sense;
  // estimation
  output->estimation = input->estimation;
  // control
  output->control = input->control;
  // locomotion
  output->locomotion = input->locomotion;
  // command
  output->command = input->command;
  // data
  output->data = input->data;
  // posture
  output->posture = input->posture;
  return true;
}

themis_state_msgs__msg__ThreadState *
themis_state_msgs__msg__ThreadState__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__ThreadState * msg = (themis_state_msgs__msg__ThreadState *)allocator.allocate(sizeof(themis_state_msgs__msg__ThreadState), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__ThreadState));
  bool success = themis_state_msgs__msg__ThreadState__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__ThreadState__destroy(themis_state_msgs__msg__ThreadState * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__ThreadState__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__ThreadState__Sequence__init(themis_state_msgs__msg__ThreadState__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__ThreadState * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__ThreadState *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__ThreadState), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__ThreadState__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__ThreadState__fini(&data[i - 1]);
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
themis_state_msgs__msg__ThreadState__Sequence__fini(themis_state_msgs__msg__ThreadState__Sequence * array)
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
      themis_state_msgs__msg__ThreadState__fini(&array->data[i]);
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

themis_state_msgs__msg__ThreadState__Sequence *
themis_state_msgs__msg__ThreadState__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__ThreadState__Sequence * array = (themis_state_msgs__msg__ThreadState__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__ThreadState__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__ThreadState__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__ThreadState__Sequence__destroy(themis_state_msgs__msg__ThreadState__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__ThreadState__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__ThreadState__Sequence__are_equal(const themis_state_msgs__msg__ThreadState__Sequence * lhs, const themis_state_msgs__msg__ThreadState__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__ThreadState__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__ThreadState__Sequence__copy(
  const themis_state_msgs__msg__ThreadState__Sequence * input,
  themis_state_msgs__msg__ThreadState__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__ThreadState);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__ThreadState * data =
      (themis_state_msgs__msg__ThreadState *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__ThreadState__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__ThreadState__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__ThreadState__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
