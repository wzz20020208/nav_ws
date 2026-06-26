// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/FootPrintArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'current_foot_print'
// Member 'goal_foot_print'
#include "themis_state_msgs/msg/detail/foot_array__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__FootPrintArray __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__FootPrintArray __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct FootPrintArray_
{
  using Type = FootPrintArray_<ContainerAllocator>;

  explicit FootPrintArray_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit FootPrintArray_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _current_foot_print_type =
    std::vector<themis_state_msgs::msg::FootArray_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::FootArray_<ContainerAllocator>>>;
  _current_foot_print_type current_foot_print;
  using _goal_foot_print_type =
    std::vector<themis_state_msgs::msg::FootArray_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::FootArray_<ContainerAllocator>>>;
  _goal_foot_print_type goal_foot_print;

  // setters for named parameter idiom
  Type & set__current_foot_print(
    const std::vector<themis_state_msgs::msg::FootArray_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::FootArray_<ContainerAllocator>>> & _arg)
  {
    this->current_foot_print = _arg;
    return *this;
  }
  Type & set__goal_foot_print(
    const std::vector<themis_state_msgs::msg::FootArray_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::FootArray_<ContainerAllocator>>> & _arg)
  {
    this->goal_foot_print = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::FootPrintArray_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::FootPrintArray_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::FootPrintArray_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::FootPrintArray_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__FootPrintArray
    std::shared_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__FootPrintArray
    std::shared_ptr<themis_state_msgs::msg::FootPrintArray_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const FootPrintArray_ & other) const
  {
    if (this->current_foot_print != other.current_foot_print) {
      return false;
    }
    if (this->goal_foot_print != other.goal_foot_print) {
      return false;
    }
    return true;
  }
  bool operator!=(const FootPrintArray_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct FootPrintArray_

// alias to use template instance with default allocator
using FootPrintArray =
  themis_state_msgs::msg::FootPrintArray_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__STRUCT_HPP_
