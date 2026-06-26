cmake_minimum_required(VERSION 3.8)
project(joint_tf_publisher)

if(CMAKE_CXX_STANDARD LESS 14)
    set(CMAKE_CXX_STANDARD 14)
endif()

if(NOT CMAKE_CXX_STANDARD_REQUIRED)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(geometry_msgs REQUIRED)

add_executable(joint_tf_publisher_node src/joint_tf_publisher_node.cpp)
ament_target_dependencies(joint_tf_publisher_node rclcpp sensor_msgs tf2_ros geometry_msgs)

install(TARGETS joint_tf_publisher_node DESTINATION lib/${PROJECT_NAME})

ament_package()