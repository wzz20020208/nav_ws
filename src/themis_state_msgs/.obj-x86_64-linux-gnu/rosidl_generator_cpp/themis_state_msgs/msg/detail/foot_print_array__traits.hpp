// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/FootPrintArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/foot_print_array__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'current_foot_print'
// Member 'goal_foot_print'
#include "themis_state_msgs/msg/detail/foot_array__traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const FootPrintArray & msg,
  std::ostream & out)
{
  out << "{";
  // member: current_foot_print
  {
    if (msg.current_foot_print.size() == 0) {
      out << "current_foot_print: []";
    } else {
      out << "current_foot_print: [";
      size_t pending_items = msg.current_foot_print.size();
      for (auto item : msg.current_foot_print) {
        to_flow_style_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: goal_foot_print
  {
    if (msg.goal_foot_print.size() == 0) {
      out << "goal_foot_print: []";
    } else {
      out << "goal_foot_print: [";
      size_t pending_items = msg.goal_foot_print.size();
      for (auto item : msg.goal_foot_print) {
        to_flow_style_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const FootPrintArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: current_foot_print
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.current_foot_print.size() == 0) {
      out << "current_foot_print: []\n";
    } else {
      out << "current_foot_print:\n";
      for (auto item : msg.current_foot_print) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }

  // member: goal_foot_print
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.goal_foot_print.size() == 0) {
      out << "goal_foot_print: []\n";
    } else {
      out << "goal_foot_print:\n";
      for (auto item : msg.goal_foot_print) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const FootPrintArray & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::FootPrintArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::FootPrintArray & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::FootPrintArray>()
{
  return "themis_state_msgs::msg::FootPrintArray";
}

template<>
inline const char * name<themis_state_msgs::msg::FootPrintArray>()
{
  return "themis_state_msgs/msg/FootPrintArray";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::FootPrintArray>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::FootPrintArray>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<themis_state_msgs::msg::FootPrintArray>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_PRINT_ARRAY__TRAITS_HPP_
