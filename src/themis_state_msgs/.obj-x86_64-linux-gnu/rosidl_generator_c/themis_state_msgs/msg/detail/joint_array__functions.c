// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/JointArray.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/joint_array__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `position`
// Member `velocity`
// Member `torque`
#include "rosidl_runtime_c/primitives_sequence_functions.h"

bool
themis_state_msgs__msg__JointArray__init(themis_state_msgs__msg__JointArray * msg)
{
  if (!msg) {
    return false;
  }
  // position
  if (!rosidl_runtime_c__float__Sequence__init(&msg->position, 0)) {
    themis_state_msgs__msg__JointArray__fini(msg);
    return false;
  }
  // velocity
  if (!rosidl_runtime_c__float__Sequence__init(&msg->velocity, 0)) {
    themis_state_msgs__msg__JointArray__fini(msg);
    return false;
  }
  // torque
  if (!rosidl_runtime_c__float__Sequence__init(&msg->torque, 0)) {
    themis_state_msgs__msg__JointArray__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__JointArray__fini(themis_state_msgs__msg__JointArray * msg)
{
  if (!msg) {
    return;
  }
  // position
  rosidl_runtime_c__float__Sequence__fini(&msg->position);
  // velocity
  rosidl_runtime_c__float__Sequence__fini(&msg->velocity);
  // torque
  rosidl_runtime_c__float__Sequence__fini(&msg->torque);
}

bool
themis_state_msgs__msg__JointArray__are_equal(const themis_state_msgs__msg__JointArray * lhs, const themis_state_msgs__msg__JointArray * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // position
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->position), &(rhs->position)))
  {
    return false;
  }
  // velocity
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->velocity), &(rhs->velocity)))
  {
    return false;
  }
  // torque
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->torque), &(rhs->torque)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__JointArray__copy(
  const themis_state_msgs__msg__JointArray * input,
  themis_state_msgs__msg__JointArray * output)
{
  if (!input || !output) {
    return false;
  }
  // position
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->position), &(output->position)))
  {
    return false;
  }
  // velocity
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->velocity), &(output->velocity)))
  {
    return false;
  }
  // torque
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->torque), &(output->torque)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__JointArray *
themis_state_msgs__msg__JointArray__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__JointArray * msg = (themis_state_msgs__msg__JointArray *)allocator.allocate(sizeof(themis_state_msgs__msg__JointArray), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__JointArray));
  bool success = themis_state_msgs__msg__JointArray__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__JointArray__destroy(themis_state_msgs__msg__JointArray * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__JointArray__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__JointArray__Sequence__init(themis_state_msgs__msg__JointArray__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__JointArray * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__JointArray *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__JointArray), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__JointArray__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__JointArray__fini(&data[i - 1]);
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
themis_state_msgs__msg__JointArray__Sequence__fini(themis_state_msgs__msg__JointArray__Sequence * array)
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
      themis_state_msgs__msg__JointArray__fini(&array->data[i]);
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

themis_state_msgs__msg__JointArray__Sequence *
themis_state_msgs__msg__JointArray__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__JointArray__Sequence * array = (themis_state_msgs__msg__JointArray__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__JointArray__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__JointArray__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__JointArray__Sequence__destroy(themis_state_msgs__msg__JointArray__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__JointArray__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__JointArray__Sequence__are_equal(const themis_state_msgs__msg__JointArray__Sequence * lhs, const themis_state_msgs__msg__JointArray__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__JointArray__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__JointArray__Sequence__copy(
  const themis_state_msgs__msg__JointArray__Sequence * input,
  themis_state_msgs__msg__JointArray__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__JointArray);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__JointArray * data =
      (themis_state_msgs__msg__JointArray *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__JointArray__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__JointArray__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__JointArray__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
