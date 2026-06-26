cmake_minimum_required(VERSION 3.8)
project(depth_to_pcl)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake_python REQUIRED)

find_package(ament_cmake REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(pcl_conversions REQUIRED)
find_package(image_transport REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(PCL REQUIRED)

add_executable(depth_to_pcl_node src/depth_to_pcl_node.cpp)
ament_target_dependencies(depth_to_pcl_node rclcpp sensor_msgs pcl_conversions image_transport cv_bridge PCL)

target_include_directories(depth_to_pcl_node PUBLIC
  ${PCL_INCLUDE_DIRS}
  /opt/ros/humble/include
)

target_link_libraries(depth_to_pcl_node ${PCL_LIBRARIES})

install(TARGETS depth_to_pcl_node
  DESTINATION lib/${PROJECT_NAME}
)

# Install launch files
install(DIRECTORY launch
        DESTINATION share/${PROJECT_NAME})

ament_package()