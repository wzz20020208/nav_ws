cmake_minimum_required(VERSION 3.8)
project(base_state_to_odom)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(themis_state_msgs REQUIRED)

# C++ node
add_executable(base_state_to_odom_node src/base_state_to_odom_node.cpp)
ament_target_dependencies(base_state_to_odom_node rclcpp nav_msgs tf2 tf2_ros themis_state_msgs)

install(TARGETS base_state_to_odom_node
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
