cmake_minimum_required(VERSION 4.1)
project(matrix)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_COLOR_DIAGNOSTICS 1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include" gtest)

add_subdirectory(samples)
add_subdirectory(gtest)
add_subdirectory(tests)
