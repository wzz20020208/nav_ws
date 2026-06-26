// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/ThreadState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__ThreadState __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__ThreadState __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct ThreadState_
{
  using Type = ThreadState_<ContainerAllocator>;

  explicit ThreadState_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->bear_right_leg = 0;
      this->bear_left_leg = 0;
      this->bear_right_arm = 0;
      this->bear_left_arm = 0;
      this->bear_head = 0;
      this->dxl_right_hand = 0;
      this->dxl_left_hand = 0;
      this->sense = 0;
      this->estimation = 0;
      this->control = 0;
      this->locomotion = 0;
      this->command = 0;
      this->data = 0;
      this->posture = 0;
    }
  }

  explicit ThreadState_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  {
    (void)_alloc;
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->bear_right_leg = 0;
      this->bear_left_leg = 0;
      this->bear_right_arm = 0;
      this->bear_left_arm = 0;
      this->bear_head = 0;
      this->dxl_right_hand = 0;
      this->dxl_left_hand = 0;
      this->sense = 0;
      this->estimation = 0;
      this->control = 0;
      this->locomotion = 0;
      this->command = 0;
      this->data = 0;
      this->posture = 0;
    }
  }

  // field types and members
  using _bear_right_leg_type =
    int8_t;
  _bear_right_leg_type bear_right_leg;
  using _bear_left_leg_type =
    int8_t;
  _bear_left_leg_type bear_left_leg;
  using _bear_right_arm_type =
    int8_t;
  _bear_right_arm_type bear_right_arm;
  using _bear_left_arm_type =
    int8_t;
  _bear_left_arm_type bear_left_arm;
  using _bear_head_type =
    int8_t;
  _bear_head_type bear_head;
  using _dxl_right_hand_type =
    int8_t;
  _dxl_right_hand_type dxl_right_hand;
  using _dxl_left_hand_type =
    int8_t;
  _dxl_left_hand_type dxl_left_hand;
  using _sense_type =
    int8_t;
  _sense_type sense;
  using _estimation_type =
    int8_t;
  _estimation_type estimation;
  using _control_type =
    int8_t;
  _control_type control;
  using _locomotion_type =
    int8_t;
  _locomotion_type locomotion;
  using _command_type =
    int8_t;
  _command_type command;
  using _data_type =
    int8_t;
  _data_type data;
  using _posture_type =
    int8_t;
  _posture_type posture;

  // setters for named parameter idiom
  Type & set__bear_right_leg(
    const int8_t & _arg)
  {
    this->bear_right_leg = _arg;
    return *this;
  }
  Type & set__bear_left_leg(
    const int8_t & _arg)
  {
    this->bear_left_leg = _arg;
    return *this;
  }
  Type & set__bear_right_arm(
    const int8_t & _arg)
  {
    this->bear_right_arm = _arg;
    return *this;
  }
  Type & set__bear_left_arm(
    const int8_t & _arg)
  {
    this->bear_left_arm = _arg;
    return *this;
  }
  Type & set__bear_head(
    const int8_t & _arg)
  {
    this->bear_head = _arg;
    return *this;
  }
  Type & set__dxl_right_hand(
    const int8_t & _arg)
  {
    this->dxl_right_hand = _arg;
    return *this;
  }
  Type & set__dxl_left_hand(
    const int8_t & _arg)
  {
    this->dxl_left_hand = _arg;
    return *this;
  }
  Type & set__sense(
    const int8_t & _arg)
  {
    this->sense = _arg;
    return *this;
  }
  Type & set__estimation(
    const int8_t & _arg)
  {
    this->estimation = _arg;
    return *this;
  }
  Type & set__control(
    const int8_t & _arg)
  {
    this->control = _arg;
    return *this;
  }
  Type & set__locomotion(
    const int8_t & _arg)
  {
    this->locomotion = _arg;
    return *this;
  }
  Type & set__command(
    const int8_t & _arg)
  {
    this->command = _arg;
    return *this;
  }
  Type & set__data(
    const int8_t & _arg)
  {
    this->data = _arg;
    return *this;
  }
  Type & set__posture(
    const int8_t & _arg)
  {
    this->posture = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::ThreadState_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::ThreadState_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::ThreadState_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::ThreadState_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__ThreadState
    std::shared_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__ThreadState
    std::shared_ptr<themis_state_msgs::msg::ThreadState_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const ThreadState_ & other) const
  {
    if (this->bear_right_leg != other.bear_right_leg) {
      return false;
    }
    if (this->bear_left_leg != other.bear_left_leg) {
      return false;
    }
    if (this->bear_right_arm != other.bear_right_arm) {
      return false;
    }
    if (this->bear_left_arm != other.bear_left_arm) {
      return false;
    }
    if (this->bear_head != other.bear_head) {
      return false;
    }
    if (this->dxl_right_hand != other.dxl_right_hand) {
      return false;
    }
    if (this->dxl_left_hand != other.dxl_left_hand) {
      return false;
    }
    if (this->sense != other.sense) {
      return false;
    }
    if (this->estimation != other.estimation) {
      return false;
    }
    if (this->control != other.control) {
      return false;
    }
    if (this->locomotion != other.locomotion) {
      return false;
    }
    if (this->command != other.command) {
      return false;
    }
    if (this->data != other.data) {
      return false;
    }
    if (this->posture != other.posture) {
      return false;
    }
    return true;
  }
  bool operator!=(const ThreadState_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct ThreadState_

// alias to use template instance with default allocator
using ThreadState =
  themis_state_msgs::msg::ThreadState_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__STRUCT_HPP_
