cmake_minimum_required(VERSION 3.10)
project(zissenn)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

add_executable(main main.cpp)
