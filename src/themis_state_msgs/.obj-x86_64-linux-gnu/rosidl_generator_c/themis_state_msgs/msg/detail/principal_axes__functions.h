// generated from rosidl_generator_c/resource/idl__functions.h.em
// with input from themis_state_msgs:msg/PrincipalAxes.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__FUNCTIONS_H_
#define THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__FUNCTIONS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdlib.h>

#include "rosidl_runtime_c/visibility_control.h"
#include "themis_state_msgs/msg/rosidl_generator_c__visibility_control.h"

#include "themis_state_msgs/msg/detail/principal_axes__struct.h"

/// Initialize msg/PrincipalAxes message.
/**
 * If the init function is called twice for the same message without
 * calling fini inbetween previously allocated memory will be leaked.
 * \param[in,out] msg The previously allocated message pointer.
 * Fields without a default value will not be initialized by this function.
 * You might want to call memset(msg, 0, sizeof(
 * themis_state_msgs__msg__PrincipalAxes
 * )) before or use
 * themis_state_msgs__msg__PrincipalAxes__create()
 * to allocate and initialize the message.
 * \return true if initialization was successful, otherwise false
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__PrincipalAxes__init(themis_state_msgs__msg__PrincipalAxes * msg);

/// Finalize msg/PrincipalAxes message.
/**
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__PrincipalAxes__fini(themis_state_msgs__msg__PrincipalAxes * msg);

/// Create msg/PrincipalAxes message.
/**
 * It allocates the memory for the message, sets the memory to zero, and
 * calls
 * themis_state_msgs__msg__PrincipalAxes__init().
 * \return The pointer to the initialized message if successful,
 * otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
themis_state_msgs__msg__PrincipalAxes *
themis_state_msgs__msg__PrincipalAxes__create();

/// Destroy msg/PrincipalAxes message.
/**
 * It calls
 * themis_state_msgs__msg__PrincipalAxes__fini()
 * and frees the memory of the message.
 * \param[in,out] msg The allocated message pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__PrincipalAxes__destroy(themis_state_msgs__msg__PrincipalAxes * msg);

/// Check for msg/PrincipalAxes message equality.
/**
 * \param[in] lhs The message on the left hand size of the equality operator.
 * \param[in] rhs The message on the right hand size of the equality operator.
 * \return true if messages are equal, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__PrincipalAxes__are_equal(const themis_state_msgs__msg__PrincipalAxes * lhs, const themis_state_msgs__msg__PrincipalAxes * rhs);

/// Copy a msg/PrincipalAxes message.
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
themis_state_msgs__msg__PrincipalAxes__copy(
  const themis_state_msgs__msg__PrincipalAxes * input,
  themis_state_msgs__msg__PrincipalAxes * output);

/// Initialize array of msg/PrincipalAxes messages.
/**
 * It allocates the memory for the number of elements and calls
 * themis_state_msgs__msg__PrincipalAxes__init()
 * for each element of the array.
 * \param[in,out] array The allocated array pointer.
 * \param[in] size The size / capacity of the array.
 * \return true if initialization was successful, otherwise false
 * If the array pointer is valid and the size is zero it is guaranteed
 # to return true.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__PrincipalAxes__Sequence__init(themis_state_msgs__msg__PrincipalAxes__Sequence * array, size_t size);

/// Finalize array of msg/PrincipalAxes messages.
/**
 * It calls
 * themis_state_msgs__msg__PrincipalAxes__fini()
 * for each element of the array and frees the memory for the number of
 * elements.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__PrincipalAxes__Sequence__fini(themis_state_msgs__msg__PrincipalAxes__Sequence * array);

/// Create array of msg/PrincipalAxes messages.
/**
 * It allocates the memory for the array and calls
 * themis_state_msgs__msg__PrincipalAxes__Sequence__init().
 * \param[in] size The size / capacity of the array.
 * \return The pointer to the initialized array if successful, otherwise NULL
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
themis_state_msgs__msg__PrincipalAxes__Sequence *
themis_state_msgs__msg__PrincipalAxes__Sequence__create(size_t size);

/// Destroy array of msg/PrincipalAxes messages.
/**
 * It calls
 * themis_state_msgs__msg__PrincipalAxes__Sequence__fini()
 * on the array,
 * and frees the memory of the array.
 * \param[in,out] array The initialized array pointer.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
void
themis_state_msgs__msg__PrincipalAxes__Sequence__destroy(themis_state_msgs__msg__PrincipalAxes__Sequence * array);

/// Check for msg/PrincipalAxes message array equality.
/**
 * \param[in] lhs The message array on the left hand size of the equality operator.
 * \param[in] rhs The message array on the right hand size of the equality operator.
 * \return true if message arrays are equal in size and content, otherwise false.
 */
ROSIDL_GENERATOR_C_PUBLIC_themis_state_msgs
bool
themis_state_msgs__msg__PrincipalAxes__Sequence__are_equal(const themis_state_msgs__msg__PrincipalAxes__Sequence * lhs, const themis_state_msgs__msg__PrincipalAxes__Sequence * rhs);

/// Copy an array of msg/PrincipalAxes messages.
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
themis_state_msgs__msg__PrincipalAxes__Sequence__copy(
  const themis_state_msgs__msg__PrincipalAxes__Sequence * input,
  themis_state_msgs__msg__PrincipalAxes__Sequence * output);

#ifdef __cplusplus
}
#endif

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__FUNCTIONS_H_
