cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cliente LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)

set(SRCS main.cpp
         misc.cpp
         cliente.cpp)

add_executable(cliente ${SRCS})
target_link_libraries(cliente wsock32 ws2_32)