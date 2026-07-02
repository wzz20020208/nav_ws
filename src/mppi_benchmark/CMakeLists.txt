cmake_minimum_required(VERSION 3.18)
project(mppi_benchmark LANGUAGES CXX CUDA)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CUDA_STANDARD 17)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

# GPU 架构: RTX 5060 = sm_120, 兼容 sm_89 (Ada/Blackwell)
set(CMAKE_CUDA_ARCHITECTURES 89 120)

# ── OpenMP ──
find_package(OpenMP)
if(OpenMP_CXX_FOUND)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(include)

# ── GPU library ──
add_library(mppi_cost_gpu STATIC
  src/mppi_cost_gpu.cu
)
target_include_directories(mppi_cost_gpu PUBLIC include)
set_target_properties(mppi_cost_gpu PROPERTIES
  CUDA_SEPARABLE_COMPILATION OFF)

# ── CPU library ──
add_library(mppi_cost_cpu STATIC
  src/mppi_cost_cpu.cpp
)
target_include_directories(mppi_cost_cpu PUBLIC include)

# ── Benchmark executable ──
add_executable(mppi_benchmark
  src/benchmark_main.cpp
)
target_link_libraries(mppi_benchmark
  mppi_cost_gpu
  mppi_cost_cpu
)
if(OpenMP_CXX_FOUND)
  target_link_libraries(mppi_benchmark OpenMP::OpenMP_CXX)
endif()
