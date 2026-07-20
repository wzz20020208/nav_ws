cmake_minimum_required(VERSION 3.5)
project(themis_state_msgs)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()


find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)

set(msg_files
  "msg/BatteryState.msg"
  "msg/BearTemperature.msg"
  "msg/ThreadState.msg"
  "msg/BaseState.msg"
  "msg/THJointState.msg"
  "msg/DetectedObject.msg"
  "msg/DetectedObjectArray.msg"
  "msg/SelectedObject.msg"
  "msg/PrincipalAxes.msg"
  "msg/JointArray.msg"
  "msg/BearState.msg"
  "msg/BearArray.msg"
  "msg/Foot.msg"
  "msg/FootArray.msg"
  "msg/FootPrintArray.msg"
  "msg/RobotStateSnapshot.msg"
  "msg/NavigationCommand.msg"
)


rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES std_msgs geometry_msgs sensor_msgs
)


ament_export_dependencies(rosidl_default_runtime)

ament_package()
