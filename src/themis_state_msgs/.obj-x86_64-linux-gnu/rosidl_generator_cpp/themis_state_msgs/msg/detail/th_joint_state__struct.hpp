// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/THJointState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__TH_JOINT_STATE__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__TH_JOINT_STATE__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'head'
// Member 'left_arm'
// Member 'right_arm'
// Member 'left_leg'
// Member 'right_leg'
// Member 'left_hand'
// Member 'right_hand'
#include "themis_state_msgs/msg/detail/joint_array__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__THJointState __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__THJointState __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct THJointState_
{
  using Type = THJointState_<ContainerAllocator>;

  explicit THJointState_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : head(_init),
    left_arm(_init),
    right_arm(_init),
    left_leg(_init),
    right_leg(_init),
    left_hand(_init),
    right_hand(_init)
  {
    (void)_init;
  }

  explicit THJointState_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : head(_alloc, _init),
    left_arm(_alloc, _init),
    right_arm(_alloc, _init),
    left_leg(_alloc, _init),
    right_leg(_alloc, _init),
    left_hand(_alloc, _init),
    right_hand(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _head_type =
    themis_state_msgs::msg::JointArray_<ContainerAllocator>;
  _head_type head;
  using _left_arm_type =
    themis_state_msgs::msg::JointArray_<ContainerAllocator>;
  _left_arm_type left_arm;
  using _right_arm_type =
    themis_state_msgs::msg::JointArray_<ContainerAllocator>;
  _right_arm_type right_arm;
  using _left_leg_type =
    themis_state_msgs::msg::JointArray_<ContainerAllocator>;
  _left_leg_type left_leg;
  using _right_leg_type =
    themis_state_msgs::msg::JointArray_<ContainerAllocator>;
  _right_leg_type right_leg;
  using _left_hand_type =
    themis_state_msgs::msg::JointArray_<ContainerAllocator>;
  _left_hand_type left_hand;
  using _right_hand_type =
    themis_state_msgs::msg::JointArray_<ContainerAllocator>;
  _right_hand_type right_hand;

  // setters for named parameter idiom
  Type & set__head(
    const themis_state_msgs::msg::JointArray_<ContainerAllocator> & _arg)
  {
    this->head = _arg;
    return *this;
  }
  Type & set__left_arm(
    const themis_state_msgs::msg::JointArray_<ContainerAllocator> & _arg)
  {
    this->left_arm = _arg;
    return *this;
  }
  Type & set__right_arm(
    const themis_state_msgs::msg::JointArray_<ContainerAllocator> & _arg)
  {
    this->right_arm = _arg;
    return *this;
  }
  Type & set__left_leg(
    const themis_state_msgs::msg::JointArray_<ContainerAllocator> & _arg)
  {
    this->left_leg = _arg;
    return *this;
  }
  Type & set__right_leg(
    const themis_state_msgs::msg::JointArray_<ContainerAllocator> & _arg)
  {
    this->right_leg = _arg;
    return *this;
  }
  Type & set__left_hand(
    const themis_state_msgs::msg::JointArray_<ContainerAllocator> & _arg)
  {
    this->left_hand = _arg;
    return *this;
  }
  Type & set__right_hand(
    const themis_state_msgs::msg::JointArray_<ContainerAllocator> & _arg)
  {
    this->right_hand = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::THJointState_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::THJointState_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::THJointState_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::THJointState_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__THJointState
    std::shared_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__THJointState
    std::shared_ptr<themis_state_msgs::msg::THJointState_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const THJointState_ & other) const
  {
    if (this->head != other.head) {
      return false;
    }
    if (this->left_arm != other.left_arm) {
      return false;
    }
    if (this->right_arm != other.right_arm) {
      return false;
    }
    if (this->left_leg != other.left_leg) {
      return false;
    }
    if (this->right_leg != other.right_leg) {
      return false;
    }
    if (this->left_hand != other.left_hand) {
      return false;
    }
    if (this->right_hand != other.right_hand) {
      return false;
    }
    return true;
  }
  bool operator!=(const THJointState_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct THJointState_

// alias to use template instance with default allocator
using THJointState =
  themis_state_msgs::msg::THJointState_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__TH_JOINT_STATE__STRUCT_HPP_
