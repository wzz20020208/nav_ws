// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/BearArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__BearArray __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__BearArray __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct BearArray_
{
  using Type = BearArray_<ContainerAllocator>;

  explicit BearArray_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit BearArray_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _positions_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _positions_type positions;
  using _velocities_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _velocities_type velocities;
  using _currents_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _currents_type currents;
  using _temperatures_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _temperatures_type temperatures;
  using _voltages_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _voltages_type voltages;
  using _errors_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _errors_type errors;

  // setters for named parameter idiom
  Type & set__positions(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->positions = _arg;
    return *this;
  }
  Type & set__velocities(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->velocities = _arg;
    return *this;
  }
  Type & set__currents(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->currents = _arg;
    return *this;
  }
  Type & set__temperatures(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->temperatures = _arg;
    return *this;
  }
  Type & set__voltages(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->voltages = _arg;
    return *this;
  }
  Type & set__errors(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->errors = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::BearArray_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::BearArray_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::BearArray_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::BearArray_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__BearArray
    std::shared_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__BearArray
    std::shared_ptr<themis_state_msgs::msg::BearArray_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const BearArray_ & other) const
  {
    if (this->positions != other.positions) {
      return false;
    }
    if (this->velocities != other.velocities) {
      return false;
    }
    if (this->currents != other.currents) {
      return false;
    }
    if (this->temperatures != other.temperatures) {
      return false;
    }
    if (this->voltages != other.voltages) {
      return false;
    }
    if (this->errors != other.errors) {
      return false;
    }
    return true;
  }
  bool operator!=(const BearArray_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct BearArray_

// alias to use template instance with default allocator
using BearArray =
  themis_state_msgs::msg::BearArray_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_ARRAY__STRUCT_HPP_
