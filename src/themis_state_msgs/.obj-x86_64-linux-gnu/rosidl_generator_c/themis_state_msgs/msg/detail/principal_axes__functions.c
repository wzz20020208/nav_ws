// generated from rosidl_generator_c/resource/idl__functions.c.em
// with input from themis_state_msgs:msg/PrincipalAxes.idl
// generated code does not contain a copyright notice
#include "themis_state_msgs/msg/detail/principal_axes__functions.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "rcutils/allocator.h"


// Include directives for member types
// Member `header`
#include "std_msgs/msg/detail/header__functions.h"
// Member `centroid`
#include "geometry_msgs/msg/detail/point__functions.h"
// Member `axis_x`
// Member `axis_y`
// Member `axis_z`
#include "geometry_msgs/msg/detail/vector3__functions.h"
// Member `variances`
#include "rosidl_runtime_c/primitives_sequence_functions.h"
// Member `object_category`
#include "rosidl_runtime_c/string_functions.h"

bool
themis_state_msgs__msg__PrincipalAxes__init(themis_state_msgs__msg__PrincipalAxes * msg)
{
  if (!msg) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__init(&msg->header)) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
    return false;
  }
  // centroid
  if (!geometry_msgs__msg__Point__init(&msg->centroid)) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
    return false;
  }
  // axis_x
  if (!geometry_msgs__msg__Vector3__init(&msg->axis_x)) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
    return false;
  }
  // axis_y
  if (!geometry_msgs__msg__Vector3__init(&msg->axis_y)) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
    return false;
  }
  // axis_z
  if (!geometry_msgs__msg__Vector3__init(&msg->axis_z)) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
    return false;
  }
  // variances
  if (!rosidl_runtime_c__float__Sequence__init(&msg->variances, 0)) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
    return false;
  }
  // object_category
  if (!rosidl_runtime_c__String__init(&msg->object_category)) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
    return false;
  }
  // object_id
  return true;
}

void
themis_state_msgs__msg__PrincipalAxes__fini(themis_state_msgs__msg__PrincipalAxes * msg)
{
  if (!msg) {
    return;
  }
  // header
  std_msgs__msg__Header__fini(&msg->header);
  // centroid
  geometry_msgs__msg__Point__fini(&msg->centroid);
  // axis_x
  geometry_msgs__msg__Vector3__fini(&msg->axis_x);
  // axis_y
  geometry_msgs__msg__Vector3__fini(&msg->axis_y);
  // axis_z
  geometry_msgs__msg__Vector3__fini(&msg->axis_z);
  // variances
  rosidl_runtime_c__float__Sequence__fini(&msg->variances);
  // object_category
  rosidl_runtime_c__String__fini(&msg->object_category);
  // object_id
}

bool
themis_state_msgs__msg__PrincipalAxes__are_equal(const themis_state_msgs__msg__PrincipalAxes * lhs, const themis_state_msgs__msg__PrincipalAxes * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__are_equal(
      &(lhs->header), &(rhs->header)))
  {
    return false;
  }
  // centroid
  if (!geometry_msgs__msg__Point__are_equal(
      &(lhs->centroid), &(rhs->centroid)))
  {
    return false;
  }
  // axis_x
  if (!geometry_msgs__msg__Vector3__are_equal(
      &(lhs->axis_x), &(rhs->axis_x)))
  {
    return false;
  }
  // axis_y
  if (!geometry_msgs__msg__Vector3__are_equal(
      &(lhs->axis_y), &(rhs->axis_y)))
  {
    return false;
  }
  // axis_z
  if (!geometry_msgs__msg__Vector3__are_equal(
      &(lhs->axis_z), &(rhs->axis_z)))
  {
    return false;
  }
  // variances
  if (!rosidl_runtime_c__float__Sequence__are_equal(
      &(lhs->variances), &(rhs->variances)))
  {
    return false;
  }
  // object_category
  if (!rosidl_runtime_c__String__are_equal(
      &(lhs->object_category), &(rhs->object_category)))
  {
    return false;
  }
  // object_id
  if (lhs->object_id != rhs->object_id) {
    return false;
  }
  return true;
}

bool
themis_state_msgs__msg__PrincipalAxes__copy(
  const themis_state_msgs__msg__PrincipalAxes * input,
  themis_state_msgs__msg__PrincipalAxes * output)
{
  if (!input || !output) {
    return false;
  }
  // header
  if (!std_msgs__msg__Header__copy(
      &(input->header), &(output->header)))
  {
    return false;
  }
  // centroid
  if (!geometry_msgs__msg__Point__copy(
      &(input->centroid), &(output->centroid)))
  {
    return false;
  }
  // axis_x
  if (!geometry_msgs__msg__Vector3__copy(
      &(input->axis_x), &(output->axis_x)))
  {
    return false;
  }
  // axis_y
  if (!geometry_msgs__msg__Vector3__copy(
      &(input->axis_y), &(output->axis_y)))
  {
    return false;
  }
  // axis_z
  if (!geometry_msgs__msg__Vector3__copy(
      &(input->axis_z), &(output->axis_z)))
  {
    return false;
  }
  // variances
  if (!rosidl_runtime_c__float__Sequence__copy(
      &(input->variances), &(output->variances)))
  {
    return false;
  }
  // object_category
  if (!rosidl_runtime_c__String__copy(
      &(input->object_category), &(output->object_category)))
  {
    return false;
  }
  // object_id
  output->object_id = input->object_id;
  return true;
}

themis_state_msgs__msg__PrincipalAxes *
themis_state_msgs__msg__PrincipalAxes__create()
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__PrincipalAxes * msg = (themis_state_msgs__msg__PrincipalAxes *)allocator.allocate(sizeof(themis_state_msgs__msg__PrincipalAxes), allocator.state);
  if (!msg) {
    return NULL;
  }
  memset(msg, 0, sizeof(themis_state_msgs__msg__PrincipalAxes));
  bool success = themis_state_msgs__msg__PrincipalAxes__init(msg);
  if (!success) {
    allocator.deallocate(msg, allocator.state);
    return NULL;
  }
  return msg;
}

void
themis_state_msgs__msg__PrincipalAxes__destroy(themis_state_msgs__msg__PrincipalAxes * msg)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (msg) {
    themis_state_msgs__msg__PrincipalAxes__fini(msg);
  }
  allocator.deallocate(msg, allocator.state);
}


bool
themis_state_msgs__msg__PrincipalAxes__Sequence__init(themis_state_msgs__msg__PrincipalAxes__Sequence * array, size_t size)
{
  if (!array) {
    return false;
  }
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__PrincipalAxes * data = NULL;

  if (size) {
    data = (themis_state_msgs__msg__PrincipalAxes *)allocator.zero_allocate(size, sizeof(themis_state_msgs__msg__PrincipalAxes), allocator.state);
    if (!data) {
      return false;
    }
    // initialize all array elements
    size_t i;
    for (i = 0; i < size; ++i) {
      bool success = themis_state_msgs__msg__PrincipalAxes__init(&data[i]);
      if (!success) {
        break;
      }
    }
    if (i < size) {
      // if initialization failed finalize the already initialized array elements
      for (; i > 0; --i) {
        themis_state_msgs__msg__PrincipalAxes__fini(&data[i - 1]);
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
themis_state_msgs__msg__PrincipalAxes__Sequence__fini(themis_state_msgs__msg__PrincipalAxes__Sequence * array)
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
      themis_state_msgs__msg__PrincipalAxes__fini(&array->data[i]);
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

themis_state_msgs__msg__PrincipalAxes__Sequence *
themis_state_msgs__msg__PrincipalAxes__Sequence__create(size_t size)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  themis_state_msgs__msg__PrincipalAxes__Sequence * array = (themis_state_msgs__msg__PrincipalAxes__Sequence *)allocator.allocate(sizeof(themis_state_msgs__msg__PrincipalAxes__Sequence), allocator.state);
  if (!array) {
    return NULL;
  }
  bool success = themis_state_msgs__msg__PrincipalAxes__Sequence__init(array, size);
  if (!success) {
    allocator.deallocate(array, allocator.state);
    return NULL;
  }
  return array;
}

void
themis_state_msgs__msg__PrincipalAxes__Sequence__destroy(themis_state_msgs__msg__PrincipalAxes__Sequence * array)
{
  rcutils_allocator_t allocator = rcutils_get_default_allocator();
  if (array) {
    themis_state_msgs__msg__PrincipalAxes__Sequence__fini(array);
  }
  allocator.deallocate(array, allocator.state);
}

bool
themis_state_msgs__msg__PrincipalAxes__Sequence__are_equal(const themis_state_msgs__msg__PrincipalAxes__Sequence * lhs, const themis_state_msgs__msg__PrincipalAxes__Sequence * rhs)
{
  if (!lhs || !rhs) {
    return false;
  }
  if (lhs->size != rhs->size) {
    return false;
  }
  for (size_t i = 0; i < lhs->size; ++i) {
    if (!themis_state_msgs__msg__PrincipalAxes__are_equal(&(lhs->data[i]), &(rhs->data[i]))) {
      return false;
    }
  }
  return true;
}

bool
themis_state_msgs__msg__PrincipalAxes__Sequence__copy(
  const themis_state_msgs__msg__PrincipalAxes__Sequence * input,
  themis_state_msgs__msg__PrincipalAxes__Sequence * output)
{
  if (!input || !output) {
    return false;
  }
  if (output->capacity < input->size) {
    const size_t allocation_size =
      input->size * sizeof(themis_state_msgs__msg__PrincipalAxes);
    rcutils_allocator_t allocator = rcutils_get_default_allocator();
    themis_state_msgs__msg__PrincipalAxes * data =
      (themis_state_msgs__msg__PrincipalAxes *)allocator.reallocate(
      output->data, allocation_size, allocator.state);
    if (!data) {
      return false;
    }
    // If reallocation succeeded, memory may or may not have been moved
    // to fulfill the allocation request, invalidating output->data.
    output->data = data;
    for (size_t i = output->capacity; i < input->size; ++i) {
      if (!themis_state_msgs__msg__PrincipalAxes__init(&output->data[i])) {
        // If initialization of any new item fails, roll back
        // all previously initialized items. Existing items
        // in output are to be left unmodified.
        for (; i-- > output->capacity; ) {
          themis_state_msgs__msg__PrincipalAxes__fini(&output->data[i]);
        }
        return false;
      }
    }
    output->capacity = input->size;
  }
  output->size = input->size;
  for (size_t i = 0; i < input->size; ++i) {
    if (!themis_state_msgs__msg__PrincipalAxes__copy(
        &(input->data[i]), &(output->data[i])))
    {
      return false;
    }
  }
  return true;
}
