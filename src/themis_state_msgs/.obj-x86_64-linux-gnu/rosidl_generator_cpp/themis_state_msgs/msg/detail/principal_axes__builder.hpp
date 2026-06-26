// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/PrincipalAxes.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/principal_axes__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_PrincipalAxes_object_id
{
public:
  explicit Init_PrincipalAxes_object_id(::themis_state_msgs::msg::PrincipalAxes & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::PrincipalAxes object_id(::themis_state_msgs::msg::PrincipalAxes::_object_id_type arg)
  {
    msg_.object_id = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

class Init_PrincipalAxes_object_category
{
public:
  explicit Init_PrincipalAxes_object_category(::themis_state_msgs::msg::PrincipalAxes & msg)
  : msg_(msg)
  {}
  Init_PrincipalAxes_object_id object_category(::themis_state_msgs::msg::PrincipalAxes::_object_category_type arg)
  {
    msg_.object_category = std::move(arg);
    return Init_PrincipalAxes_object_id(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

class Init_PrincipalAxes_variances
{
public:
  explicit Init_PrincipalAxes_variances(::themis_state_msgs::msg::PrincipalAxes & msg)
  : msg_(msg)
  {}
  Init_PrincipalAxes_object_category variances(::themis_state_msgs::msg::PrincipalAxes::_variances_type arg)
  {
    msg_.variances = std::move(arg);
    return Init_PrincipalAxes_object_category(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

class Init_PrincipalAxes_axis_z
{
public:
  explicit Init_PrincipalAxes_axis_z(::themis_state_msgs::msg::PrincipalAxes & msg)
  : msg_(msg)
  {}
  Init_PrincipalAxes_variances axis_z(::themis_state_msgs::msg::PrincipalAxes::_axis_z_type arg)
  {
    msg_.axis_z = std::move(arg);
    return Init_PrincipalAxes_variances(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

class Init_PrincipalAxes_axis_y
{
public:
  explicit Init_PrincipalAxes_axis_y(::themis_state_msgs::msg::PrincipalAxes & msg)
  : msg_(msg)
  {}
  Init_PrincipalAxes_axis_z axis_y(::themis_state_msgs::msg::PrincipalAxes::_axis_y_type arg)
  {
    msg_.axis_y = std::move(arg);
    return Init_PrincipalAxes_axis_z(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

class Init_PrincipalAxes_axis_x
{
public:
  explicit Init_PrincipalAxes_axis_x(::themis_state_msgs::msg::PrincipalAxes & msg)
  : msg_(msg)
  {}
  Init_PrincipalAxes_axis_y axis_x(::themis_state_msgs::msg::PrincipalAxes::_axis_x_type arg)
  {
    msg_.axis_x = std::move(arg);
    return Init_PrincipalAxes_axis_y(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

class Init_PrincipalAxes_centroid
{
public:
  explicit Init_PrincipalAxes_centroid(::themis_state_msgs::msg::PrincipalAxes & msg)
  : msg_(msg)
  {}
  Init_PrincipalAxes_axis_x centroid(::themis_state_msgs::msg::PrincipalAxes::_centroid_type arg)
  {
    msg_.centroid = std::move(arg);
    return Init_PrincipalAxes_axis_x(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

class Init_PrincipalAxes_header
{
public:
  Init_PrincipalAxes_header()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_PrincipalAxes_centroid header(::themis_state_msgs::msg::PrincipalAxes::_header_type arg)
  {
    msg_.header = std::move(arg);
    return Init_PrincipalAxes_centroid(msg_);
  }

private:
  ::themis_state_msgs::msg::PrincipalAxes msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::PrincipalAxes>()
{
  return themis_state_msgs::msg::builder::Init_PrincipalAxes_header();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__BUILDER_HPP_
