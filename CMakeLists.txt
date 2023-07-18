cmake_minimum_required(VERSION 3.10)
project(fractal)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic -Werror)
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  message("OpenMP FOUND")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CPU_SOURCES cpu.cpp shader.cpp shader.hpp)
add_executable(cpu ${CPU_SOURCES})
target_include_directories(cpu PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(cpu glfw GLEW GL)
target_compile_definitions(
  cpu PUBLIC SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/shader/")
if(OPENMP)
  target_compile_definitions(cpu PUBLIC OPENMP)
endif()

set(GPU_SOURCES gpu.cpp shader.cpp shader.hpp)
add_executable(gpu ${GPU_SOURCES})
target_include_directories(gpu PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(gpu glfw GLEW GL)
target_compile_definitions(
  gpu PUBLIC SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/shader/")
