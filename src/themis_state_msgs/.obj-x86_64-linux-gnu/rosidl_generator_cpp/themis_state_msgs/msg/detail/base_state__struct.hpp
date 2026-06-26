// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/BaseState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__BaseState __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__BaseState __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct BaseState_
{
  using Type = BaseState_<ContainerAllocator>;

  explicit BaseState_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->position_x = 0.0;
      this->position_y = 0.0;
      this->position_z = 0.0;
      this->velocity_x = 0.0;
      this->velocity_y = 0.0;
      this->velocity_z = 0.0;
      this->yaw = 0.0;
      this->roll = 0.0;
      this->pitch = 0.0;
      this->angular_rate = 0.0;
    }
  }

  explicit BaseState_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->position_x = 0.0;
      this->position_y = 0.0;
      this->position_z = 0.0;
      this->velocity_x = 0.0;
      this->velocity_y = 0.0;
      this->velocity_z = 0.0;
      this->yaw = 0.0;
      this->roll = 0.0;
      this->pitch = 0.0;
      this->angular_rate = 0.0;
    }
  }

  // field types and members
  using _position_x_type =
    double;
  _position_x_type position_x;
  using _position_y_type =
    double;
  _position_y_type position_y;
  using _position_z_type =
    double;
  _position_z_type position_z;
  using _velocity_x_type =
    double;
  _velocity_x_type velocity_x;
  using _velocity_y_type =
    double;
  _velocity_y_type velocity_y;
  using _velocity_z_type =
    double;
  _velocity_z_type velocity_z;
  using _yaw_type =
    double;
  _yaw_type yaw;
  using _roll_type =
    double;
  _roll_type roll;
  using _pitch_type =
    double;
  _pitch_type pitch;
  using _angular_rate_type =
    double;
  _angular_rate_type angular_rate;

  // setters for named parameter idiom
  Type & set__position_x(
    const double & _arg)
  {
    this->position_x = _arg;
    return *this;
  }
  Type & set__position_y(
    const double & _arg)
  {
    this->position_y = _arg;
    return *this;
  }
  Type & set__position_z(
    const double & _arg)
  {
    this->position_z = _arg;
    return *this;
  }
  Type & set__velocity_x(
    const double & _arg)
  {
    this->velocity_x = _arg;
    return *this;
  }
  Type & set__velocity_y(
    const double & _arg)
  {
    this->velocity_y = _arg;
    return *this;
  }
  Type & set__velocity_z(
    const double & _arg)
  {
    this->velocity_z = _arg;
    return *this;
  }
  Type & set__yaw(
    const double & _arg)
  {
    this->yaw = _arg;
    return *this;
  }
  Type & set__roll(
    const double & _arg)
  {
    this->roll = _arg;
    return *this;
  }
  Type & set__pitch(
    const double & _arg)
  {
    this->pitch = _arg;
    return *this;
  }
  Type & set__angular_rate(
    const double & _arg)
  {
    this->angular_rate = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::BaseState_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::BaseState_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::BaseState_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::BaseState_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__BaseState
    std::shared_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__BaseState
    std::shared_ptr<themis_state_msgs::msg::BaseState_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const BaseState_ & other) const
  {
    if (this->position_x != other.position_x) {
      return false;
    }
    if (this->position_y != other.position_y) {
      return false;
    }
    if (this->position_z != other.position_z) {
      return false;
    }
    if (this->velocity_x != other.velocity_x) {
      return false;
    }
    if (this->velocity_y != other.velocity_y) {
      return false;
    }
    if (this->velocity_z != other.velocity_z) {
      return false;
    }
    if (this->yaw != other.yaw) {
      return false;
    }
    if (this->roll != other.roll) {
      return false;
    }
    if (this->pitch != other.pitch) {
      return false;
    }
    if (this->angular_rate != other.angular_rate) {
      return false;
    }
    return true;
  }
  bool operator!=(const BaseState_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct BaseState_

// alias to use template instance with default allocator
using BaseState =
  themis_state_msgs::msg::BaseState_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__STRUCT_HPP_
