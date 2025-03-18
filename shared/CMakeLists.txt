# shared/CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(SharedCode)

set(CMAKE_CXX_STANDARD 17)

add_library(SharedLib STATIC
    src/NeonController.cpp
)

target_include_directories(SharedLib PUBLIC include)