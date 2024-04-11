cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(L0r3na LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(wxBUILD_SHARED OFF)

add_subdirectory(cliente)

message(STATUS "Fetching wxWidgets...")

FetchContent_Declare(
   wxWidgets
   #GIT_REPOSITORY https://github.com/wxWidgets/wxWidgets.git
   #GIT_SHALLOW ON
)
FetchContent_MakeAvailable(wxWidgets)

set(SRCS main.cpp
         misc.cpp
         server.cpp
         frame_cliente.cpp
         panel_file_manager.cpp
         file_editor.cpp
         aes256/aes256.cpp)

add_executable(main WIN32 ${SRCS})
target_link_libraries(main PRIVATE wxcore wxpng wxnet wxaui wxhtml winmm)