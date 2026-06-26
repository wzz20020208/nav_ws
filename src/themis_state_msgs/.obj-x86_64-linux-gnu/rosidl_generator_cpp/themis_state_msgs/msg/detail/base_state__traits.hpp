// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/BaseState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/base_state__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const BaseState & msg,
  std::ostream & out)
{
  out << "{";
  // member: position_x
  {
    out << "position_x: ";
    rosidl_generator_traits::value_to_yaml(msg.position_x, out);
    out << ", ";
  }

  // member: position_y
  {
    out << "position_y: ";
    rosidl_generator_traits::value_to_yaml(msg.position_y, out);
    out << ", ";
  }

  // member: position_z
  {
    out << "position_z: ";
    rosidl_generator_traits::value_to_yaml(msg.position_z, out);
    out << ", ";
  }

  // member: velocity_x
  {
    out << "velocity_x: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_x, out);
    out << ", ";
  }

  // member: velocity_y
  {
    out << "velocity_y: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_y, out);
    out << ", ";
  }

  // member: velocity_z
  {
    out << "velocity_z: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_z, out);
    out << ", ";
  }

  // member: yaw
  {
    out << "yaw: ";
    rosidl_generator_traits::value_to_yaml(msg.yaw, out);
    out << ", ";
  }

  // member: roll
  {
    out << "roll: ";
    rosidl_generator_traits::value_to_yaml(msg.roll, out);
    out << ", ";
  }

  // member: pitch
  {
    out << "pitch: ";
    rosidl_generator_traits::value_to_yaml(msg.pitch, out);
    out << ", ";
  }

  // member: angular_rate
  {
    out << "angular_rate: ";
    rosidl_generator_traits::value_to_yaml(msg.angular_rate, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const BaseState & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: position_x
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "position_x: ";
    rosidl_generator_traits::value_to_yaml(msg.position_x, out);
    out << "\n";
  }

  // member: position_y
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "position_y: ";
    rosidl_generator_traits::value_to_yaml(msg.position_y, out);
    out << "\n";
  }

  // member: position_z
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "position_z: ";
    rosidl_generator_traits::value_to_yaml(msg.position_z, out);
    out << "\n";
  }

  // member: velocity_x
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "velocity_x: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_x, out);
    out << "\n";
  }

  // member: velocity_y
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "velocity_y: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_y, out);
    out << "\n";
  }

  // member: velocity_z
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "velocity_z: ";
    rosidl_generator_traits::value_to_yaml(msg.velocity_z, out);
    out << "\n";
  }

  // member: yaw
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "yaw: ";
    rosidl_generator_traits::value_to_yaml(msg.yaw, out);
    out << "\n";
  }

  // member: roll
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "roll: ";
    rosidl_generator_traits::value_to_yaml(msg.roll, out);
    out << "\n";
  }

  // member: pitch
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "pitch: ";
    rosidl_generator_traits::value_to_yaml(msg.pitch, out);
    out << "\n";
  }

  // member: angular_rate
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "angular_rate: ";
    rosidl_generator_traits::value_to_yaml(msg.angular_rate, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const BaseState & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::BaseState & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::BaseState & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::BaseState>()
{
  return "themis_state_msgs::msg::BaseState";
}

template<>
inline const char * name<themis_state_msgs::msg::BaseState>()
{
  return "themis_state_msgs/msg/BaseState";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::BaseState>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::BaseState>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<themis_state_msgs::msg::BaseState>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BASE_STATE__TRAITS_HPP_
