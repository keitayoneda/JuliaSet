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

file(GLOB MAIN_SOURCES "*.cpp")
add_executable(main ${MAIN_SOURCES})
target_include_directories(main PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(main glfw GLEW GL)
target_compile_definitions(
  main PUBLIC SHADER_DIR="${CMAKE_CURRENT_LIST_DIR}/shader/")
