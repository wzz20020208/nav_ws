// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/PrincipalAxes.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/principal_axes__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
#include "std_msgs/msg/detail/header__traits.hpp"
// Member 'centroid'
#include "geometry_msgs/msg/detail/point__traits.hpp"
// Member 'axis_x'
// Member 'axis_y'
// Member 'axis_z'
#include "geometry_msgs/msg/detail/vector3__traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const PrincipalAxes & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: centroid
  {
    out << "centroid: ";
    to_flow_style_yaml(msg.centroid, out);
    out << ", ";
  }

  // member: axis_x
  {
    out << "axis_x: ";
    to_flow_style_yaml(msg.axis_x, out);
    out << ", ";
  }

  // member: axis_y
  {
    out << "axis_y: ";
    to_flow_style_yaml(msg.axis_y, out);
    out << ", ";
  }

  // member: axis_z
  {
    out << "axis_z: ";
    to_flow_style_yaml(msg.axis_z, out);
    out << ", ";
  }

  // member: variances
  {
    if (msg.variances.size() == 0) {
      out << "variances: []";
    } else {
      out << "variances: [";
      size_t pending_items = msg.variances.size();
      for (auto item : msg.variances) {
        rosidl_generator_traits::value_to_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: object_category
  {
    out << "object_category: ";
    rosidl_generator_traits::value_to_yaml(msg.object_category, out);
    out << ", ";
  }

  // member: object_id
  {
    out << "object_id: ";
    rosidl_generator_traits::value_to_yaml(msg.object_id, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const PrincipalAxes & msg,
  std::ostream & out, size_t indentation = 0)
{
  // member: header
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "header:\n";
    to_block_style_yaml(msg.header, out, indentation + 2);
  }

  // member: centroid
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "centroid:\n";
    to_block_style_yaml(msg.centroid, out, indentation + 2);
  }

  // member: axis_x
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "axis_x:\n";
    to_block_style_yaml(msg.axis_x, out, indentation + 2);
  }

  // member: axis_y
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "axis_y:\n";
    to_block_style_yaml(msg.axis_y, out, indentation + 2);
  }

  // member: axis_z
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "axis_z:\n";
    to_block_style_yaml(msg.axis_z, out, indentation + 2);
  }

  // member: variances
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.variances.size() == 0) {
      out << "variances: []\n";
    } else {
      out << "variances:\n";
      for (auto item : msg.variances) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "- ";
        rosidl_generator_traits::value_to_yaml(item, out);
        out << "\n";
      }
    }
  }

  // member: object_category
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "object_category: ";
    rosidl_generator_traits::value_to_yaml(msg.object_category, out);
    out << "\n";
  }

  // member: object_id
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "object_id: ";
    rosidl_generator_traits::value_to_yaml(msg.object_id, out);
    out << "\n";
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const PrincipalAxes & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::PrincipalAxes & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::PrincipalAxes & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::PrincipalAxes>()
{
  return "themis_state_msgs::msg::PrincipalAxes";
}

template<>
inline const char * name<themis_state_msgs::msg::PrincipalAxes>()
{
  return "themis_state_msgs/msg/PrincipalAxes";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::PrincipalAxes>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::PrincipalAxes>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<themis_state_msgs::msg::PrincipalAxes>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__PRINCIPAL_AXES__TRAITS_HPP_
