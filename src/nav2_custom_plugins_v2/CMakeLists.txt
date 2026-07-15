cmake_minimum_required(VERSION 3.8)
project(nav2_custom_plugins_v2 LANGUAGES CXX CUDA)

# CUDA
find_package(CUDAToolkit REQUIRED)
message(STATUS "CUDA ${CUDAToolkit_VERSION}, GPU acceleration enabled")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ROS 2
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_core REQUIRED)
find_package(nav2_costmap_2d REQUIRED)
find_package(pluginlib REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# ── 自定义消息 ──
rosidl_generate_interfaces(${PROJECT_NAME}_msgs
  "msg/VelocitySteering.msg"
  DEPENDENCIES builtin_interfaces
)

# ── GPU 加速库 ──
add_library(${PROJECT_NAME}_gpu SHARED
  src/mppi_steering_controller.cpp
  src/mppi_core.cpp
  src/path_manager.cpp
  src/cost_evaluator.cpp
  src/velocity_postprocessor.cpp
  src/gpu_engine.cpp
  src/state_machine.cpp
  src/visualization.cpp
  src/mppi_logger.cpp
  src/mppi_gpu_kernels.cu
)

target_include_directories(${PROJECT_NAME}_gpu PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  ${CUDAToolkit_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}_gpu
  CUDA::cudart
)

rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME}_msgs "rosidl_typesupport_cpp")
target_link_libraries(${PROJECT_NAME}_gpu ${cpp_typesupport_target})

ament_target_dependencies(${PROJECT_NAME}_gpu
  rclcpp
  nav2_core
  nav2_costmap_2d
  pluginlib
  geometry_msgs
  tf2_ros
  tf2_geometry_msgs
  nav_msgs
  visualization_msgs
)

set_target_properties(${PROJECT_NAME}_gpu PROPERTIES
  CUDA_ARCHITECTURES "80;86;89;90;120"
)

# ── Plugin 注册 ──
pluginlib_export_plugin_description_file(nav2_core plugins.xml)

# ── 安装 ──
install(TARGETS ${PROJECT_NAME}_gpu
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY msg/
  DESTINATION share/${PROJECT_NAME}/msg
)

ament_export_dependencies(rosidl_default_runtime)

ament_package()
