// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from themis_state_msgs:msg/SelectedObject.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__FUNCTIONS_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "themis_state_msgs/msg/rosidl_generator_c__visibility_control.h"

#include "themis_state_msgs/msg/detail/selected_object__struct.h"

/// Initialize msg/SelectedObject message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * themis_state_msgs__msg__SelectedObject
 * )) before or use
 * themis_state_msgs__msg__SelectedObject__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__SelectedObject__init(themis_state_msgs__msg__SelectedObject * msg);

/// Finalize msg/SelectedObject message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__SelectedObject__fini(themis_state_msgs__msg__SelectedObject * msg);

/// Create msg/SelectedObject message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * themis_state_msgs__msg__SelectedObject__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
themis_state_msgs__msg__SelectedObject *
themis_state_msgs__msg__SelectedObject__create();

/// Destroy msg/SelectedObject message.
/**
 * It calls
 * themis_state_msgs__msg__SelectedObject__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__SelectedObject__destroy(themis_state_msgs__msg__SelectedObject * msg);

/// Check for msg/SelectedObject message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__SelectedObject__are_equal(const themis_state_msgs__msg__SelectedObject * lhs, const themis_state_msgs__msg__SelectedObject * rhs);

/// Copy a msg/SelectedObject message.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source message pointer.
 * \param[out] output The target message pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer is null
 *   or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__SelectedObject__copy(
  const themis_state_msgs__msg__SelectedObject * input,
  themis_state_msgs__msg__SelectedObject * output);

/// Initialize array of msg/SelectedObject messages.
/**
 * It allocates the memory for the number of elements and calls
 * themis_state_msgs__msg__SelectedObject__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__SelectedObject__Sequence__init(themis_state_msgs__msg__SelectedObject__Sequence * array, size_t size);

/// Finalize array of msg/SelectedObject messages.
/**
 * It calls
 * themis_state_msgs__msg__SelectedObject__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__SelectedObject__Sequence__fini(themis_state_msgs__msg__SelectedObject__Sequence * array);

/// Create array of msg/SelectedObject messages.
/**
 * It allocates the memory for the array and calls
 * themis_state_msgs__msg__SelectedObject__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
themis_state_msgs__msg__SelectedObject__Sequence *
themis_state_msgs__msg__SelectedObject__Sequence__create(size_t size);

/// Destroy array of msg/SelectedObject messages.
/**
 * It calls
 * themis_state_msgs__msg__SelectedObject__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__SelectedObject__Sequence__destroy(themis_state_msgs__msg__SelectedObject__Sequence * array);

/// Check for msg/SelectedObject message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__SelectedObject__Sequence__are_equal(const themis_state_msgs__msg__SelectedObject__Sequence * lhs, const themis_state_msgs__msg__SelectedObject__Sequence * rhs);

/// Copy an array of msg/SelectedObject messages.
/**
 * This functions performs a deep copy, as opposed to the shallow copy that
 * plain assignment yields.
 *
 * \param[in] input The source array pointer.
 * \param[out] output The target array pointer, which must
 *   have been initialized before calling this function.
 * \return true if successful, or false if either pointer
 *   is null or memory allocation fails.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__SelectedObject__Sequence__copy(
  const themis_state_msgs__msg__SelectedObject__Sequence * input,
  themis_state_msgs__msg__SelectedObject__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__FUNCTIONS_H_
