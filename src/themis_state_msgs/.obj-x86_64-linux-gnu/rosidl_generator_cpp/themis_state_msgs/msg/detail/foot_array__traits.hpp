// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/FootArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/foot_array__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'feet'
#include "themis_state_msgs/msg/detail/foot__traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const FootArray & msg,
  std::ostream & out)
{
  out << "{";
  // member: feet
  {
    if (msg.feet.size() == 0) {
      out << "feet: []";
    } else {
      out << "feet: [";
      size_t pending_items = msg.feet.size();
      for (auto item : msg.feet) {
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
  const FootArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: feet
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.feet.size() == 0) {
      out << "feet: []\n";
    } else {
      out << "feet:\n";
      for (auto item : msg.feet) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const FootArray & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::FootArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::FootArray & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::FootArray>()
{
  return "themis_state_msgs::msg::FootArray";
}

template<>
inline const char * name<themis_state_msgs::msg::FootArray>()
{
  return "themis_state_msgs/msg/FootArray";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::FootArray>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::FootArray>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<themis_state_msgs::msg::FootArray>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT_ARRAY__TRAITS_HPP_
