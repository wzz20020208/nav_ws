// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/SelectedObject.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/selected_object__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_SelectedObject_category
{
public:
  explicit Init_SelectedObject_category(::themis_state_msgs::msg::SelectedObject & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::SelectedObject category(::themis_state_msgs::msg::SelectedObject::_category_type arg)
  {
    msg_.category = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::SelectedObject msg_;
};

class Init_SelectedObject_id
{
public:
  Init_SelectedObject_id()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_SelectedObject_category id(::themis_state_msgs::msg::SelectedObject::_id_type arg)
  {
    msg_.id = std::move(arg);
    return Init_SelectedObject_category(msg_);
  }

private:
  ::themis_state_msgs::msg::SelectedObject msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::SelectedObject>()
{
  return themis_state_msgs::msg::builder::Init_SelectedObject_id();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__SELECTED_OBJECT__BUILDER_HPP_
