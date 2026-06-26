// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/BearTemperature.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__BearTemperature __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__BearTemperature __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct BearTemperature_
{
  using Type = BearTemperature_<ContainerAllocator>;

  explicit BearTemperature_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
  }

  explicit BearTemperature_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_init;
    (void)_alloc;
  }

  // field types and members
  using _right_leg_bear_temperature_type =
    std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>>;
  _right_leg_bear_temperature_type right_leg_bear_temperature;
  using _left_leg_bear_temperature_type =
    std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>>;
  _left_leg_bear_temperature_type left_leg_bear_temperature;
  using _right_arm_bear_temperature_type =
    std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>>;
  _right_arm_bear_temperature_type right_arm_bear_temperature;
  using _left_arm_bear_temperature_type =
    std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>>;
  _left_arm_bear_temperature_type left_arm_bear_temperature;
  using _head_bear_temperature_type =
    std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>>;
  _head_bear_temperature_type head_bear_temperature;

  // setters for named parameter idiom
  Type & set__right_leg_bear_temperature(
    const std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>> & _arg)
  {
    this->right_leg_bear_temperature = _arg;
    return *this;
  }
  Type & set__left_leg_bear_temperature(
    const std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>> & _arg)
  {
    this->left_leg_bear_temperature = _arg;
    return *this;
  }
  Type & set__right_arm_bear_temperature(
    const std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>> & _arg)
  {
    this->right_arm_bear_temperature = _arg;
    return *this;
  }
  Type & set__left_arm_bear_temperature(
    const std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>> & _arg)
  {
    this->left_arm_bear_temperature = _arg;
    return *this;
  }
  Type & set__head_bear_temperature(
    const std::vector<double, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<double>> & _arg)
  {
    this->head_bear_temperature = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::BearTemperature_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::BearTemperature_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::BearTemperature_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::BearTemperature_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__BearTemperature
    std::shared_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__BearTemperature
    std::shared_ptr<themis_state_msgs::msg::BearTemperature_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const BearTemperature_ & other) const
  {
    if (this->right_leg_bear_temperature != other.right_leg_bear_temperature) {
      return false;
    }
    if (this->left_leg_bear_temperature != other.left_leg_bear_temperature) {
      return false;
    }
    if (this->right_arm_bear_temperature != other.right_arm_bear_temperature) {
      return false;
    }
    if (this->left_arm_bear_temperature != other.left_arm_bear_temperature) {
      return false;
    }
    if (this->head_bear_temperature != other.head_bear_temperature) {
      return false;
    }
    return true;
  }
  bool operator!=(const BearTemperature_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct BearTemperature_

// alias to use template instance with default allocator
using BearTemperature =
  themis_state_msgs::msg::BearTemperature_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__STRUCT_HPP_
