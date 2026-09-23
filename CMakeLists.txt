cmake_minimum_required(VERSION 3.20)
project(chess_engine_cpp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-Wall -Wextra -Wpedantic)

add_executable(chess_engine src/main.cpp src/board.cpp src/movegen.cpp src/move.cpp)
target_include_directories(chess_engine PRIVATE include)