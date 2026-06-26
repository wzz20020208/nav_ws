// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/ThreadState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/thread_state__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const ThreadState & msg,
  std::ostream & out)
{
  out << "{";
  // member: bear_right_leg
  {
    out << "bear_right_leg: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_right_leg, out);
    out << ", ";
  }

  // member: bear_left_leg
  {
    out << "bear_left_leg: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_left_leg, out);
    out << ", ";
  }

  // member: bear_right_arm
  {
    out << "bear_right_arm: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_right_arm, out);
    out << ", ";
  }

  // member: bear_left_arm
  {
    out << "bear_left_arm: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_left_arm, out);
    out << ", ";
  }

  // member: bear_head
  {
    out << "bear_head: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_head, out);
    out << ", ";
  }

  // member: dxl_right_hand
  {
    out << "dxl_right_hand: ";
    rosidl_generator_traits::value_to_yaml(msg.dxl_right_hand, out);
    out << ", ";
  }

  // member: dxl_left_hand
  {
    out << "dxl_left_hand: ";
    rosidl_generator_traits::value_to_yaml(msg.dxl_left_hand, out);
    out << ", ";
  }

  // member: sense
  {
    out << "sense: ";
    rosidl_generator_traits::value_to_yaml(msg.sense, out);
    out << ", ";
  }

  // member: estimation
  {
    out << "estimation: ";
    rosidl_generator_traits::value_to_yaml(msg.estimation, out);
    out << ", ";
  }

  // member: control
  {
    out << "control: ";
    rosidl_generator_traits::value_to_yaml(msg.control, out);
    out << ", ";
  }

  // member: locomotion
  {
    out << "locomotion: ";
    rosidl_generator_traits::value_to_yaml(msg.locomotion, out);
    out << ", ";
  }

  // member: command
  {
    out << "command: ";
    rosidl_generator_traits::value_to_yaml(msg.command, out);
    out << ", ";
  }

  // member: data
  {
    out << "data: ";
    rosidl_generator_traits::value_to_yaml(msg.data, out);
    out << ", ";
  }

  // member: posture
  {
    out << "posture: ";
    rosidl_generator_traits::value_to_yaml(msg.posture, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const ThreadState & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: bear_right_leg
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bear_right_leg: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_right_leg, out);
    out << "\n";
  }

  // member: bear_left_leg
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bear_left_leg: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_left_leg, out);
    out << "\n";
  }

  // member: bear_right_arm
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bear_right_arm: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_right_arm, out);
    out << "\n";
  }

  // member: bear_left_arm
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bear_left_arm: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_left_arm, out);
    out << "\n";
  }

  // member: bear_head
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "bear_head: ";
    rosidl_generator_traits::value_to_yaml(msg.bear_head, out);
    out << "\n";
  }

  // member: dxl_right_hand
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "dxl_right_hand: ";
    rosidl_generator_traits::value_to_yaml(msg.dxl_right_hand, out);
    out << "\n";
  }

  // member: dxl_left_hand
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "dxl_left_hand: ";
    rosidl_generator_traits::value_to_yaml(msg.dxl_left_hand, out);
    out << "\n";
  }

  // member: sense
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "sense: ";
    rosidl_generator_traits::value_to_yaml(msg.sense, out);
    out << "\n";
  }

  // member: estimation
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "estimation: ";
    rosidl_generator_traits::value_to_yaml(msg.estimation, out);
    out << "\n";
  }

  // member: control
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "control: ";
    rosidl_generator_traits::value_to_yaml(msg.control, out);
    out << "\n";
  }

  // member: locomotion
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "locomotion: ";
    rosidl_generator_traits::value_to_yaml(msg.locomotion, out);
    out << "\n";
  }

  // member: command
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "command: ";
    rosidl_generator_traits::value_to_yaml(msg.command, out);
    out << "\n";
  }

  // member: data
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "data: ";
    rosidl_generator_traits::value_to_yaml(msg.data, out);
    out << "\n";
  }

  // member: posture
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "posture: ";
    rosidl_generator_traits::value_to_yaml(msg.posture, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const ThreadState & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::ThreadState & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::ThreadState & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::ThreadState>()
{
  return "themis_state_msgs::msg::ThreadState";
}

template<>
inline const char * name<themis_state_msgs::msg::ThreadState>()
{
  return "themis_state_msgs/msg/ThreadState";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::ThreadState>
  : std::integral_constant<bool, true> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::ThreadState>
  : std::integral_constant<bool, true> {};

template<>
struct is_message<themis_state_msgs::msg::ThreadState>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__THREAD_STATE__TRAITS_HPP_
