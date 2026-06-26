cmake_minimum_required(VERSION 3.8)
project(th02_a7_description)

find_package(ament_cmake REQUIRED)

install(DIRECTORY
  launch
  meshes
  urdf
  urdf_camera
  DESTINATION share/${PROJECT_NAME}
)

# 导出并打包
ament_package()
