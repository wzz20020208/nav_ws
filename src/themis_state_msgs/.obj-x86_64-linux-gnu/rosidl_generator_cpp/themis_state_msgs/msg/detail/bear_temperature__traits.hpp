// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/BearTemperature.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/bear_temperature__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const BearTemperature & msg,
  std::ostream & out)
{
  out << "{";
  // member: right_leg_bear_temperature
  {
    if (msg.right_leg_bear_temperature.size() == 0) {
      out << "right_leg_bear_temperature: []";
    } else {
      out << "right_leg_bear_temperature: [";
      size_t pending_items = msg.right_leg_bear_temperature.size();
      for (auto item : msg.right_leg_bear_temperature) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: left_leg_bear_temperature
  {
    if (msg.left_leg_bear_temperature.size() == 0) {
      out << "left_leg_bear_temperature: []";
    } else {
      out << "left_leg_bear_temperature: [";
      size_t pending_items = msg.left_leg_bear_temperature.size();
      for (auto item : msg.left_leg_bear_temperature) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: right_arm_bear_temperature
  {
    if (msg.right_arm_bear_temperature.size() == 0) {
      out << "right_arm_bear_temperature: []";
    } else {
      out << "right_arm_bear_temperature: [";
      size_t pending_items = msg.right_arm_bear_temperature.size();
      for (auto item : msg.right_arm_bear_temperature) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: left_arm_bear_temperature
  {
    if (msg.left_arm_bear_temperature.size() == 0) {
      out << "left_arm_bear_temperature: []";
    } else {
      out << "left_arm_bear_temperature: [";
      size_t pending_items = msg.left_arm_bear_temperature.size();
      for (auto item : msg.left_arm_bear_temperature) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: head_bear_temperature
  {
    if (msg.head_bear_temperature.size() == 0) {
      out << "head_bear_temperature: []";
    } else {
      out << "head_bear_temperature: [";
      size_t pending_items = msg.head_bear_temperature.size();
      for (auto item : msg.head_bear_temperature) {
        rosidl_generator_traits::value_to_yaml(item, out);
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
  const BearTemperature & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: right_leg_bear_temperature
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.right_leg_bear_temperature.size() == 0) {
      out << "right_leg_bear_temperature: []\n";
    } else {
      out << "right_leg_bear_temperature:\n";
      for (auto item : msg.right_leg_bear_temperature) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: left_leg_bear_temperature
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.left_leg_bear_temperature.size() == 0) {
      out << "left_leg_bear_temperature: []\n";
    } else {
      out << "left_leg_bear_temperature:\n";
      for (auto item : msg.left_leg_bear_temperature) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: right_arm_bear_temperature
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.right_arm_bear_temperature.size() == 0) {
      out << "right_arm_bear_temperature: []\n";
    } else {
      out << "right_arm_bear_temperature:\n";
      for (auto item : msg.right_arm_bear_temperature) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: left_arm_bear_temperature
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.left_arm_bear_temperature.size() == 0) {
      out << "left_arm_bear_temperature: []\n";
    } else {
      out << "left_arm_bear_temperature:\n";
      for (auto item : msg.left_arm_bear_temperature) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: head_bear_temperature
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.head_bear_temperature.size() == 0) {
      out << "head_bear_temperature: []\n";
    } else {
      out << "head_bear_temperature:\n";
      for (auto item : msg.head_bear_temperature) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const BearTemperature & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::BearTemperature & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::BearTemperature & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::BearTemperature>()
{
  return "themis_state_msgs::msg::BearTemperature";
}

template<>
inline const char * name<themis_state_msgs::msg::BearTemperature>()
{
  return "themis_state_msgs/msg/BearTemperature";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::BearTemperature>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::BearTemperature>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<themis_state_msgs::msg::BearTemperature>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BEAR_TEMPERATURE__TRAITS_HPP_
