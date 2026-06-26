// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/FootArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'feet'
#include "themis_state_msgs/msg/detail/foot__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__FootArray __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__FootArray __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct FootArray_
{
  using Type = FootArray_<ContainerAllocator>;

  explicit FootArray_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit FootArray_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _feet_type =
    std::vector<themis_state_msgs::msg::Foot_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::Foot_<ContainerAllocator>>>;
  _feet_type feet;

  // setters for named parameter idiom
  Type & set__feet(
    const std::vector<themis_state_msgs::msg::Foot_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::Foot_<ContainerAllocator>>> & _arg)
  {
    this->feet = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::FootArray_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::FootArray_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::FootArray_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::FootArray_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__FootArray
    std::shared_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__FootArray
    std::shared_ptr<themis_state_msgs::msg::FootArray_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const FootArray_ & other) const
  {
    if (this->feet != other.feet) {
      return false;
    }
    return true;
  }
  bool operator!=(const FootArray_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct FootArray_

// alias to use template instance with default allocator
using FootArray =
  themis_state_msgs::msg::FootArray_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__STRUCT_HPP_
