cmake_minimum_required(VERSION 3.8)
project(nav2_custom_plugins_parallel LANGUAGES CXX CUDA)

# CUDA 加速支持
find_package(CUDAToolkit REQUIRED)
message(STATUS "CUDA found (version ${CUDAToolkit_VERSION}), GPU acceleration enabled")

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# 1. 寻找系统依赖
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav2_core REQUIRED)
find_package(nav2_behaviors REQUIRED)
find_package(nav2_costmap_2d REQUIRED)
find_package(pluginlib REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(visualization_msgs REQUIRED)

find_package(behaviortree_cpp_v3 REQUIRED)
find_package(std_msgs REQUIRED)

# 2. 编译共享动态链接库 (Shared Library)
add_library(${PROJECT_NAME}_core SHARED
    src/mpc_controller.cpp
    src/mppi_controller.cpp
    src/is_progress_diagnosis.cpp
    src/escape_obstacle.cpp
  )

# 3. 设置头文件路径
target_include_directories(${PROJECT_NAME}_core PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# 4. 链接依赖项
ament_target_dependencies(${PROJECT_NAME}_core
  rclcpp
  nav2_behaviors
  nav2_core
  nav2_costmap_2d
  pluginlib
  geometry_msgs
  tf2_ros
  nav_msgs
  visualization_msgs
  behaviortree_cpp_v3
  std_msgs
)

# 5. GPU 加速库
add_library(${PROJECT_NAME}_gpu SHARED
    src/mppi_gpu_controller.cpp
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

ament_target_dependencies(${PROJECT_NAME}_gpu
  rclcpp
  nav2_core
  nav2_costmap_2d
  pluginlib
  geometry_msgs
  tf2_ros
  nav_msgs
  visualization_msgs
)

# 设置 CUDA 架构 (Ampere / Ada / Hopper / Blackwell)
set_target_properties(${PROJECT_NAME}_gpu PROPERTIES
  CUDA_ARCHITECTURES "80;86;89;90;120"
)

install(TARGETS ${PROJECT_NAME}_gpu
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

# 6. 导出 pluginlib XML 文件 (核心步骤！)
pluginlib_export_plugin_description_file(nav2_core plugins.xml)

# 7. 安装规则
install(TARGETS ${PROJECT_NAME}_core
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include
)

install(DIRECTORY behavior_trees/
  DESTINATION share/${PROJECT_NAME}/behavior_trees
)

add_executable(test_plugin src/test_plugin.cpp)
ament_target_dependencies(test_plugin rclcpp nav2_core pluginlib)

install(TARGETS test_plugin
  DESTINATION lib/${PROJECT_NAME}
)

# 8. costmap_reader 作为独立可执行工具
add_executable(costmap_reader_tool src/costmap_reader.cpp)
ament_target_dependencies(costmap_reader_tool
  rclcpp
  nav2_costmap_2d
  nav_msgs
  visualization_msgs
  tf2_ros
)

install(TARGETS costmap_reader_tool
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()