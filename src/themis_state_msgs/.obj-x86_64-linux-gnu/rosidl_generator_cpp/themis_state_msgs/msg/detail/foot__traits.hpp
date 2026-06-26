// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/Foot.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/foot__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const Foot & msg,
  std::ostream & out)
{
  out << "{";
  // member: x
  {
    out << "x: ";
    rosidl_generator_traits::value_to_yaml(msg.x, out);
    out << ", ";
  }

  // member: y
  {
    out << "y: ";
    rosidl_generator_traits::value_to_yaml(msg.y, out);
    out << ", ";
  }

  // member: z
  {
    out << "z: ";
    rosidl_generator_traits::value_to_yaml(msg.z, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const Foot & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: x
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "x: ";
    rosidl_generator_traits::value_to_yaml(msg.x, out);
    out << "\n";
  }

  // member: y
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "y: ";
    rosidl_generator_traits::value_to_yaml(msg.y, out);
    out << "\n";
  }

  // member: z
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "z: ";
    rosidl_generator_traits::value_to_yaml(msg.z, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const Foot & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::Foot & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::Foot & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::Foot>()
{
  return "themis_state_msgs::msg::Foot";
}

template<>
inline const char * name<themis_state_msgs::msg::Foot>()
{
  return "themis_state_msgs/msg/Foot";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::Foot>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::Foot>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<themis_state_msgs::msg::Foot>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__FOOT__TRAITS_HPP_
