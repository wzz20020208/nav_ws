// generated from rosidl_generator_cpp/resource/idl__builder.hpp.em
// with input from themis_state_msgs:msg/FootPrintArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__BUILDER_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__BUILDER_HPP_

#include <algorithm>
#include <utility>

#include "themis_state_msgs/msg/detail/foot_print_array__struct.hpp"
#include "rosidl_runtime_cpp/message_initialization.hpp"


namespace themis_state_msgs
{

namespace msg
{

namespace builder
{

class Init_FootPrintArray_goal_foot_print
{
public:
  explicit Init_FootPrintArray_goal_foot_print(::themis_state_msgs::msg::FootPrintArray & msg)
  : msg_(msg)
  {}
  ::themis_state_msgs::msg::FootPrintArray goal_foot_print(::themis_state_msgs::msg::FootPrintArray::_goal_foot_print_type arg)
  {
    msg_.goal_foot_print = std::move(arg);
    return std::move(msg_);
  }

private:
  ::themis_state_msgs::msg::FootPrintArray msg_;
};

class Init_FootPrintArray_current_foot_print
{
public:
  Init_FootPrintArray_current_foot_print()
  : msg_(::rosidl_runtime_cpp::MessageInitialization::SKIP)
  {}
  Init_FootPrintArray_goal_foot_print current_foot_print(::themis_state_msgs::msg::FootPrintArray::_current_foot_print_type arg)
  {
    msg_.current_foot_print = std::move(arg);
    return Init_FootPrintArray_goal_foot_print(msg_);
  }

private:
  ::themis_state_msgs::msg::FootPrintArray msg_;
};

}  // namespace builder

}  // namespace msg

template<typename MessageType>
auto build();

template<>
inline
auto build<::themis_state_msgs::msg::FootPrintArray>()
{
  return themis_state_msgs::msg::builder::Init_FootPrintArray_current_foot_print();
}

}  // namespace themis_state_msgs

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__BUILDER_HPP_
