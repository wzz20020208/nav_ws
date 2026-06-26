// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/DetectedObjectArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__STRUCT_HPP_

#include <algorithm>
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "rosidl_runtime_cpp/bounded_vector.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


// Include directives for member types
// Member 'header'
// Member 'camera_stamp'
// Member 'detection_stamp'
#include "std_msgs/msg/detail/header__struct.hpp"
// Member 'objects'
#include "themis_state_msgs/msg/detail/detected_object__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__DetectedObjectArray __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__DetectedObjectArray __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct DetectedObjectArray_
{
  using Type = DetectedObjectArray_<ContainerAllocator>;

  explicit DetectedObjectArray_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init),
    camera_stamp(_init),
    detection_stamp(_init)
  {
    (void)_init;
  }

  explicit DetectedObjectArray_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    camera_stamp(_alloc, _init),
    detection_stamp(_alloc, _init)
  {
    (void)_init;
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _camera_stamp_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _camera_stamp_type camera_stamp;
  using _objects_type =
    std::vector<themis_state_msgs::msg::DetectedObject_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::DetectedObject_<ContainerAllocator>>>;
  _objects_type objects;
  using _detection_stamp_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _detection_stamp_type detection_stamp;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__camera_stamp(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->camera_stamp = _arg;
    return *this;
  }
  Type & set__objects(
    const std::vector<themis_state_msgs::msg::DetectedObject_<ContainerAllocator>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<themis_state_msgs::msg::DetectedObject_<ContainerAllocator>>> & _arg)
  {
    this->objects = _arg;
    return *this;
  }
  Type & set__detection_stamp(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->detection_stamp = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__DetectedObjectArray
    std::shared_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__DetectedObjectArray
    std::shared_ptr<themis_state_msgs::msg::DetectedObjectArray_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const DetectedObjectArray_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->camera_stamp != other.camera_stamp) {
      return false;
    }
    if (this->objects != other.objects) {
      return false;
    }
    if (this->detection_stamp != other.detection_stamp) {
      return false;
    }
    return true;
  }
  bool operator!=(const DetectedObjectArray_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct DetectedObjectArray_

// alias to use template instance with default allocator
using DetectedObjectArray =
  themis_state_msgs::msg::DetectedObjectArray_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__STRUCT_HPP_
