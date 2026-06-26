// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/BearState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/bear_state__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'head'
// Member 'left_arm'
// Member 'right_arm'
// Member 'left_leg'
// Member 'right_leg'
// Member 'left_hand'
// Member 'right_hand'
#include "themis_state_msgs/msg/detail/bear_array__traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const BearState & msg,
  std::ostream & out)
{
  out << "{";
  // member: head
  {
    out << "head: ";
    to_flow_style_yaml(msg.head, out);
    out << ", ";
  }

  // member: left_arm
  {
    out << "left_arm: ";
    to_flow_style_yaml(msg.left_arm, out);
    out << ", ";
  }

  // member: right_arm
  {
    out << "right_arm: ";
    to_flow_style_yaml(msg.right_arm, out);
    out << ", ";
  }

  // member: left_leg
  {
    out << "left_leg: ";
    to_flow_style_yaml(msg.left_leg, out);
    out << ", ";
  }

  // member: right_leg
  {
    out << "right_leg: ";
    to_flow_style_yaml(msg.right_leg, out);
    out << ", ";
  }

  // member: left_hand
  {
    out << "left_hand: ";
    to_flow_style_yaml(msg.left_hand, out);
    out << ", ";
  }

  // member: right_hand
  {
    out << "right_hand: ";
    to_flow_style_yaml(msg.right_hand, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const BearState & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: head
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "head:\n";
    to_block_style_yaml(msg.head, out, indentation + 2);
  }

  // member: left_arm
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "left_arm:\n";
    to_block_style_yaml(msg.left_arm, out, indentation + 2);
  }

  // member: right_arm
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "right_arm:\n";
    to_block_style_yaml(msg.right_arm, out, indentation + 2);
  }

  // member: left_leg
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "left_leg:\n";
    to_block_style_yaml(msg.left_leg, out, indentation + 2);
  }

  // member: right_leg
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "right_leg:\n";
    to_block_style_yaml(msg.right_leg, out, indentation + 2);
  }

  // member: left_hand
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "left_hand:\n";
    to_block_style_yaml(msg.left_hand, out, indentation + 2);
  }

  // member: right_hand
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "right_hand:\n";
    to_block_style_yaml(msg.right_hand, out, indentation + 2);
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const BearState & msg, bool use_flow_style = false)
{
  std::ostringstream out;
  if (use_flow_style) {
    to_flow_style_yaml(msg, out);
  } else {
    to_block_style_yaml(msg, out);
  }
  return out.str();
}

}  // namespace msg

}  // namespace themis_state_msgs

namespace rosidl_generator_traits
{

[[deprecated("use themis_state_msgs::msg::to_block_style_yaml() instead")]]
inline void to_yaml(
  const themis_state_msgs::msg::BearState & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::BearState & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::BearState>()
{
  return "themis_state_msgs::msg::BearState";
}

template<>
inline const char * name<themis_state_msgs::msg::BearState>()
{
  return "themis_state_msgs/msg/BearState";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::BearState>
  : std::integral_constant<bool, has_fixed_size<themis_state_msgs::msg::BearArray>::value> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::BearState>
  : std::integral_constant<bool, has_bounded_size<themis_state_msgs::msg::BearArray>::value> {};

template<>
struct is_message<themis_state_msgs::msg::BearState>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_STATE__TRAITS_HPP_
