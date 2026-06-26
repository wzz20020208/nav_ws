// generated from rosidl_generator_cpp/resource/idl__traits.hpp.em
// with input from themis_state_msgs:msg/DetectedObjectArray.idl
// generated code does not contain a copyright notice

#ifndef THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__TRAITS_HPP_
#define THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__TRAITS_HPP_

#include <stdint.h>

#include <sstream>
#include <string>
#include <type_traits>

#include "themis_state_msgs/msg/detail/detected_object_array__struct.hpp"
#include "rosidl_runtime_cpp/traits.hpp"

// Include directives for member types
// Member 'header'
// Member 'camera_stamp'
// Member 'detection_stamp'
#include "std_msgs/msg/detail/header__traits.hpp"
// Member 'objects'
#include "themis_state_msgs/msg/detail/detected_object__traits.hpp"

namespace themis_state_msgs
{

namespace msg
{

inline void to_flow_style_yaml(
  const DetectedObjectArray & msg,
  std::ostream & out)
{
  out << "{";
  // member: header
  {
    out << "header: ";
    to_flow_style_yaml(msg.header, out);
    out << ", ";
  }

  // member: camera_stamp
  {
    out << "camera_stamp: ";
    to_flow_style_yaml(msg.camera_stamp, out);
    out << ", ";
  }

  // member: objects
  {
    if (msg.objects.size() == 0) {
      out << "objects: []";
    } else {
      out << "objects: [";
      size_t pending_items = msg.objects.size();
      for (auto item : msg.objects) {
        to_flow_style_yaml(item, out);
        if (--pending_items > 0) {
          out << ", ";
        }
      }
      out << "]";
    }
    out << ", ";
  }

  // member: detection_stamp
  {
    out << "detection_stamp: ";
    to_flow_style_yaml(msg.detection_stamp, out);
  }
  out << "}";
}  // NOLINT(readability/fn_size)

inline void to_block_style_yaml(
  const DetectedObjectArray & msg,
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

  // member: camera_stamp
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "camera_stamp:\n";
    to_block_style_yaml(msg.camera_stamp, out, indentation + 2);
  }

  // member: objects
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    if (msg.objects.size() == 0) {
      out << "objects: []\n";
    } else {
      out << "objects:\n";
      for (auto item : msg.objects) {
        if (indentation > 0) {
          out << std::string(indentation, ' ');
        }
        out << "-\n";
        to_block_style_yaml(item, out, indentation + 2);
      }
    }
  }

  // member: detection_stamp
  {
    if (indentation > 0) {
      out << std::string(indentation, ' ');
    }
    out << "detection_stamp:\n";
    to_block_style_yaml(msg.detection_stamp, out, indentation + 2);
  }
}  // NOLINT(readability/fn_size)

inline std::string to_yaml(const DetectedObjectArray & msg, bool use_flow_style = false)
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
  const themis_state_msgs::msg::DetectedObjectArray & msg,
  std::ostream & out, size_t indentation = 0)
{
  themis_state_msgs::msg::to_block_style_yaml(msg, out, indentation);
}

[[deprecated("use themis_state_msgs::msg::to_yaml() instead")]]
inline std::string to_yaml(const themis_state_msgs::msg::DetectedObjectArray & msg)
{
  return themis_state_msgs::msg::to_yaml(msg);
}

template<>
inline const char * data_type<themis_state_msgs::msg::DetectedObjectArray>()
{
  return "themis_state_msgs::msg::DetectedObjectArray";
}

template<>
inline const char * name<themis_state_msgs::msg::DetectedObjectArray>()
{
  return "themis_state_msgs/msg/DetectedObjectArray";
}

template<>
struct has_fixed_size<themis_state_msgs::msg::DetectedObjectArray>
  : std::integral_constant<bool, false> {};

template<>
struct has_bounded_size<themis_state_msgs::msg::DetectedObjectArray>
  : std::integral_constant<bool, false> {};

template<>
struct is_message<themis_state_msgs::msg::DetectedObjectArray>
  : std::true_type {};

}  // namespace rosidl_generator_traits

#endif  // THEMIS_STATE_MSGS__MSG__DETAIL__DETECTED_OBJECT_ARRAY__TRAITS_HPP_
