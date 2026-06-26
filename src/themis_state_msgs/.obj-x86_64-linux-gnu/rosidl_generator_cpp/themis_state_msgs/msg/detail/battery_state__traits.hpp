// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/BatteryState.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/battery_state__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const BatteryState & msg,
  std::ostream & out)
{
  out << "{";
  // member: battery_state
  {
    if (msg.battery_state.size() == 0) {
      out << "battery_state: []";
    } else {
      out << "battery_state: [";
      size_t pending_items = msg.battery_state.size();
      for (auto item : msg.battery_state) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: error_state
  {
    if (msg.error_state.size() == 0) {
      out << "error_state: []";
    } else {
      out << "error_state: [";
      size_t pending_items = msg.error_state.size();
      for (auto item : msg.error_state) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: battery_voltages
  {
    if (msg.battery_voltages.size() == 0) {
      out << "battery_voltages: []";
    } else {
      out << "battery_voltages: [";
      size_t pending_items = msg.battery_voltages.size();
      for (auto item : msg.battery_voltages) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: cell_voltages
  {
    if (msg.cell_voltages.size() == 0) {
      out << "cell_voltages: []";
    } else {
      out << "cell_voltages: [";
      size_t pending_items = msg.cell_voltages.size();
      for (auto item : msg.cell_voltages) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: temperatures
  {
    if (msg.temperatures.size() == 0) {
      out << "temperatures: []";
    } else {
      out << "temperatures: [";
      size_t pending_items = msg.temperatures.size();
      for (auto item : msg.temperatures) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: discharge_statuses
  {
    if (msg.discharge_statuses.size() == 0) {
      out << "discharge_statuses: []";
    } else {
      out << "discharge_statuses: [";
      size_t pending_items = msg.discharge_statuses.size();
      for (auto item : msg.discharge_statuses) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: charge_statuses
  {
    if (msg.charge_statuses.size() == 0) {
      out << "charge_statuses: []";
    } else {
      out << "charge_statuses: [";
      size_t pending_items = msg.charge_statuses.size();
      for (auto item : msg.charge_statuses) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: max_discharge_currents
  {
    if (msg.max_discharge_currents.size() == 0) {
      out << "max_discharge_currents: []";
    } else {
      out << "max_discharge_currents: [";
      size_t pending_items = msg.max_discharge_currents.size();
      for (auto item : msg.max_discharge_currents) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: max_charge_currents
  {
    if (msg.max_charge_currents.size() == 0) {
      out << "max_charge_currents: []";
    } else {
      out << "max_charge_currents: [";
      size_t pending_items = msg.max_charge_currents.size();
      for (auto item : msg.max_charge_currents) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: present_currents
  {
    if (msg.present_currents.size() == 0) {
      out << "present_currents: []";
    } else {
      out << "present_currents: [";
      size_t pending_items = msg.present_currents.size();
      for (auto item : msg.present_currents) {
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
  const BatteryState & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: battery_state
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.battery_state.size() == 0) {
      out << "battery_state: []\n";
    } else {
      out << "battery_state:\n";
      for (auto item : msg.battery_state) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: error_state
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.error_state.size() == 0) {
      out << "error_state: []\n";
    } else {
      out << "error_state:\n";
      for (auto item : msg.error_state) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: battery_voltages
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.battery_voltages.size() == 0) {
      out << "battery_voltages: []\n";
    } else {
      out << "battery_voltages:\n";
      for (auto item : msg.battery_voltages) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: cell_voltages
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.cell_voltages.size() == 0) {
      out << "cell_voltages: []\n";
    } else {
      out << "cell_voltages:\n";
      for (auto item : msg.cell_voltages) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: temperatures
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.temperatures.size() == 0) {
      out << "temperatures: []\n";
    } else {
      out << "temperatures:\n";
      for (auto item : msg.temperatures) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: discharge_statuses
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.discharge_statuses.size() == 0) {
      out << "discharge_statuses: []\n";
    } else {
      out << "discharge_statuses:\n";
      for (auto item : msg.discharge_statuses) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: charge_statuses
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.charge_statuses.size() == 0) {
      out << "charge_statuses: []\n";
    } else {
      out << "charge_statuses:\n";
      for (auto item : msg.charge_statuses) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: max_discharge_currents
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.max_discharge_currents.size() == 0) {
      out << "max_discharge_currents: []\n";
    } else {
      out << "max_discharge_currents:\n";
      for (auto item : msg.max_discharge_currents) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: max_charge_currents
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.max_charge_currents.size() == 0) {
      out << "max_charge_currents: []\n";
    } else {
      out << "max_charge_currents:\n";
      for (auto item : msg.max_charge_currents) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: present_currents
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.present_currents.size() == 0) {
      out << "present_currents: []\n";
    } else {
      out << "present_currents:\n";
      for (auto item : msg.present_currents) {
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

inline std::string to_yaml(const BatteryState & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::BatteryState & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::BatteryState & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::BatteryState>()
{
  return "themis_state_msgs::msg::BatteryState";
}

template<>
inline const char * name<themis_state_msgs::msg::BatteryState>()
{
  return "themis_state_msgs/msg/BatteryState";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::BatteryState>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::BatteryState>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<themis_state_msgs::msg::BatteryState>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__BATTERY_STATE__TRAITS_HPP_
