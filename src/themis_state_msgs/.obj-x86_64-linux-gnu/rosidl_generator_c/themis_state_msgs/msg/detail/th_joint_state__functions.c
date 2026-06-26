// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/THJointState.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/th_joint_state__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `head`
// Member `left_arm`
// Member `right_arm`
// Member `left_leg`
// Member `right_leg`
// Member `left_hand`
// Member `right_hand`
#include "themis_state_msgs/msg/detail/joint_array__functions.h"

bool
themis_state_msgs__msg__THJointState__init(themis_state_msgs__msg__THJointState * msg)
{
  if (!msg) {
    return false;
  }
  // head
  if (!themis_state_msgs__msg__JointArray__init(&msg->head)) {
    themis_state_msgs__msg__THJointState__fini(msg);
    return false;
  }
  // left_arm
  if (!themis_state_msgs__msg__JointArray__init(&msg->left_arm)) {
    themis_state_msgs__msg__THJointState__fini(msg);
    return false;
  }
  // right_arm
  if (!themis_state_msgs__msg__JointArray__init(&msg->right_arm)) {
    themis_state_msgs__msg__THJointState__fini(msg);
    return false;
  }
  // left_leg
  if (!themis_state_msgs__msg__JointArray__init(&msg->left_leg)) {
    themis_state_msgs__msg__THJointState__fini(msg);
    return false;
  }
  // right_leg
  if (!themis_state_msgs__msg__JointArray__init(&msg->right_leg)) {
    themis_state_msgs__msg__THJointState__fini(msg);
    return false;
  }
  // left_hand
  if (!themis_state_msgs__msg__JointArray__init(&msg->left_hand)) {
    themis_state_msgs__msg__THJointState__fini(msg);
    return false;
  }
  // right_hand
  if (!themis_state_msgs__msg__JointArray__init(&msg->right_hand)) {
    themis_state_msgs__msg__THJointState__fini(msg);
    return false;
  }
  return true;
}

void
themis_state_msgs__msg__THJointState__fini(themis_state_msgs__msg__THJointState * msg)
{
  if (!msg) {
    return;
  }
  // head
  themis_state_msgs__msg__JointArray__fini(&msg->head);
  // left_arm
  themis_state_msgs__msg__JointArray__fini(&msg->left_arm);
  // right_arm
  themis_state_msgs__msg__JointArray__fini(&msg->right_arm);
  // left_leg
  themis_state_msgs__msg__JointArray__fini(&msg->left_leg);
  // right_leg
  themis_state_msgs__msg__JointArray__fini(&msg->right_leg);
  // left_hand
  themis_state_msgs__msg__JointArray__fini(&msg->left_hand);
  // right_hand
  themis_state_msgs__msg__JointArray__fini(&msg->right_hand);
}

bool
themis_state_msgs__msg__THJointState__are_equal(const themis_state_msgs__msg__THJointState * lhs, const themis_state_msgs__msg__THJointState * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // head
  if (!themis_state_msgs__msg__JointArray__are_equal(
      &(lhs->head), &(rhs->head)))
  {
    return false;
  }
  // left_arm
  if (!themis_state_msgs__msg__JointArray__are_equal(
      &(lhs->left_arm), &(rhs->left_arm)))
  {
    return false;
  }
  // right_arm
  if (!themis_state_msgs__msg__JointArray__are_equal(
      &(lhs->right_arm), &(rhs->right_arm)))
  {
    return false;
  }
  // left_leg
  if (!themis_state_msgs__msg__JointArray__are_equal(
      &(lhs->left_leg), &(rhs->left_leg)))
  {
    return false;
  }
  // right_leg
  if (!themis_state_msgs__msg__JointArray__are_equal(
      &(lhs->right_leg), &(rhs->right_leg)))
  {
    return false;
  }
  // left_hand
  if (!themis_state_msgs__msg__JointArray__are_equal(
      &(lhs->left_hand), &(rhs->left_hand)))
  {
    return false;
  }
  // right_hand
  if (!themis_state_msgs__msg__JointArray__are_equal(
      &(lhs->right_hand), &(rhs->right_hand)))
  {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__THJointState__copy(
  const themis_state_msgs__msg__THJointState * input,
  themis_state_msgs__msg__THJointState * output)
{
  if (!input || !output) {
    return false;
  }
  // head
  if (!themis_state_msgs__msg__JointArray__copy(
      &(input->head), &(output->head)))
  {
    return false;
  }
  // left_arm
  if (!themis_state_msgs__msg__JointArray__copy(
      &(input->left_arm), &(output->left_arm)))
  {
    return false;
  }
  // right_arm
  if (!themis_state_msgs__msg__JointArray__copy(
      &(input->right_arm), &(output->right_arm)))
  {
    return false;
  }
  // left_leg
  if (!themis_state_msgs__msg__JointArray__copy(
      &(input->left_leg), &(output->left_leg)))
  {
    return false;
  }
  // right_leg
  if (!themis_state_msgs__msg__JointArray__copy(
      &(input->right_leg), &(output->right_leg)))
  {
    return false;
  }
  // left_hand
  if (!themis_state_msgs__msg__JointArray__copy(
      &(input->left_hand), &(output->left_hand)))
  {
    return false;
  }
  // right_hand
  if (!themis_state_msgs__msg__JointArray__copy(
      &(input->right_hand), &(output->right_hand)))
  {
    return false;
  }
  return true;
}

themis_state_msgs__msg__THJointState *
themis_state_msgs__msg__THJointState__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__THJointState * msg = (themis_state_msgs__msg__THJointState *)allocator.allocate(sizeof(themis_state_msgs__msg__THJointState), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__THJointState));
  bool success = themis_state_msgs__msg__THJointState__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__THJointState__destroy(themis_state_msgs__msg__THJointState * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__THJointState__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__THJointState__Sequence__init(themis_state_msgs__msg__THJointState__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__THJointState * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__THJointState *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__THJointState), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__THJointState__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__THJointState__fini(&data[i - 1]);
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
themis_state_msgs__msg__THJointState__Sequence__fini(themis_state_msgs__msg__THJointState__Sequence * array)
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
      themis_state_msgs__msg__THJointState__fini(&array->data[i]);
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

themis_state_msgs__msg__THJointState__Sequence *
themis_state_msgs__msg__THJointState__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__THJointState__Sequence * array = (themis_state_msgs__msg__THJointState__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__THJointState__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__THJointState__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__THJointState__Sequence__destroy(themis_state_msgs__msg__THJointState__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__THJointState__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__THJointState__Sequence__are_equal(const themis_state_msgs__msg__THJointState__Sequence * lhs, const themis_state_msgs__msg__THJointState__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__THJointState__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__THJointState__Sequence__copy(
  const themis_state_msgs__msg__THJointState__Sequence * input,
  themis_state_msgs__msg__THJointState__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__THJointState);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__THJointState * data =
      (themis_state_msgs__msg__THJointState *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__THJointState__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__THJointState__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__THJointState__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
