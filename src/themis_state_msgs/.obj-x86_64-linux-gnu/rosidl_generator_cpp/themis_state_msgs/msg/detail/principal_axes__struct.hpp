// generated from rosidl_generator_cpp/resource/idl__struct.hpp.em
// with input from themis_state_msgs:msg/PrincipalAxes.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__STRUCT_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__STRUCT_HPP_

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
#include "std_msgs/msg/detail/header__struct.hpp"
// Member 'centroid'
#include "geometry_msgs/msg/detail/point__struct.hpp"
// Member 'axis_x'
// Member 'axis_y'
// Member 'axis_z'
#include "geometry_msgs/msg/detail/vector3__struct.hpp"

#ifndef _WIN32
# define DEPRECATED__themis_state_msgs__msg__PrincipalAxes __attribute__((deprecated))
#else
# define DEPRECATED__themis_state_msgs__msg__PrincipalAxes __declspec(deprecated)
#endif

namespace themis_state_msgs
{

namespace msg
{

// message struct
template<class ContainerAllocator>
struct PrincipalAxes_
{
  using Type = PrincipalAxes_<ContainerAllocator>;

  explicit PrincipalAxes_(rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_init),
    centroid(_init),
    axis_x(_init),
    axis_y(_init),
    axis_z(_init)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->object_category = "";
      this->object_id = 0l;
    }
  }

  explicit PrincipalAxes_(const ContainerAllocator & _alloc, rosidl_runtime_cpp::MessageInitialization _init = rosidl_runtime_cpp::MessageInitialization::ALL)
  : header(_alloc, _init),
    centroid(_alloc, _init),
    axis_x(_alloc, _init),
    axis_y(_alloc, _init),
    axis_z(_alloc, _init),
    object_category(_alloc)
  {
    if (rosidl_runtime_cpp::MessageInitialization::ALL == _init ||
      rosidl_runtime_cpp::MessageInitialization::ZERO == _init)
    {
      this->object_category = "";
      this->object_id = 0l;
    }
  }

  // field types and members
  using _header_type =
    std_msgs::msg::Header_<ContainerAllocator>;
  _header_type header;
  using _centroid_type =
    geometry_msgs::msg::Point_<ContainerAllocator>;
  _centroid_type centroid;
  using _axis_x_type =
    geometry_msgs::msg::Vector3_<ContainerAllocator>;
  _axis_x_type axis_x;
  using _axis_y_type =
    geometry_msgs::msg::Vector3_<ContainerAllocator>;
  _axis_y_type axis_y;
  using _axis_z_type =
    geometry_msgs::msg::Vector3_<ContainerAllocator>;
  _axis_z_type axis_z;
  using _variances_type =
    std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>>;
  _variances_type variances;
  using _object_category_type =
    std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>>;
  _object_category_type object_category;
  using _object_id_type =
    int32_t;
  _object_id_type object_id;

  // setters for named parameter idiom
  Type & set__header(
    const std_msgs::msg::Header_<ContainerAllocator> & _arg)
  {
    this->header = _arg;
    return *this;
  }
  Type & set__centroid(
    const geometry_msgs::msg::Point_<ContainerAllocator> & _arg)
  {
    this->centroid = _arg;
    return *this;
  }
  Type & set__axis_x(
    const geometry_msgs::msg::Vector3_<ContainerAllocator> & _arg)
  {
    this->axis_x = _arg;
    return *this;
  }
  Type & set__axis_y(
    const geometry_msgs::msg::Vector3_<ContainerAllocator> & _arg)
  {
    this->axis_y = _arg;
    return *this;
  }
  Type & set__axis_z(
    const geometry_msgs::msg::Vector3_<ContainerAllocator> & _arg)
  {
    this->axis_z = _arg;
    return *this;
  }
  Type & set__variances(
    const std::vector<float, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<float>> & _arg)
  {
    this->variances = _arg;
    return *this;
  }
  Type & set__object_category(
    const std::basic_string<char, std::char_traits<char>, typename std::allocator_traits<ContainerAllocator>::template rebind_alloc<char>> & _arg)
  {
    this->object_category = _arg;
    return *this;
  }
  Type & set__object_id(
    const int32_t & _arg)
  {
    this->object_id = _arg;
    return *this;
  }

  // constant declarations

  // pointer types
  using RawPtr =
    themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator> *;
  using ConstRawPtr =
    const themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator> *;
  using SharedPtr =
    std::shared_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator>>;
  using ConstSharedPtr =
    std::shared_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator> const>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator>>>
  using UniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator>, Deleter>;

  using UniquePtr = UniquePtrWithDeleter<>;

  template<typename Deleter = std::default_delete<
      themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator>>>
  using ConstUniquePtrWithDeleter =
    std::unique_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator> const, Deleter>;
  using ConstUniquePtr = ConstUniquePtrWithDeleter<>;

  using WeakPtr =
    std::weak_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator>>;
  using ConstWeakPtr =
    std::weak_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator> const>;

  // pointer types similar to ROS 1, use SharedPtr / ConstSharedPtr instead
  // NOTE: Can't use 'using' here because GNU C++ can't parse attributes properly
  typedef DEPRECATED__themis_state_msgs__msg__PrincipalAxes
    std::shared_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator>>
    Ptr;
  typedef DEPRECATED__themis_state_msgs__msg__PrincipalAxes
    std::shared_ptr<themis_state_msgs::msg::PrincipalAxes_<ContainerAllocator> const>
    ConstPtr;

  // comparison operators
  bool operator==(const PrincipalAxes_ & other) const
  {
    if (this->header != other.header) {
      return false;
    }
    if (this->centroid != other.centroid) {
      return false;
    }
    if (this->axis_x != other.axis_x) {
      return false;
    }
    if (this->axis_y != other.axis_y) {
      return false;
    }
    if (this->axis_z != other.axis_z) {
      return false;
    }
    if (this->variances != other.variances) {
      return false;
    }
    if (this->object_category != other.object_category) {
      return false;
    }
    if (this->object_id != other.object_id) {
      return false;
    }
    return true;
  }
  bool operator!=(const PrincipalAxes_ & other) const
  {
    return !this->operator==(other);
  }
};  // struct PrincipalAxes_

// alias to use template instance with default allocator
using PrincipalAxes =
  themis_state_msgs::msg::PrincipalAxes_<std::allocator<void>>;

// constant definitions

}  // namespace msg

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__STRUCT_HPP_
