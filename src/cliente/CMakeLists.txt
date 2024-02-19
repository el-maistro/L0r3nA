cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(cliente LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_EXE_LINKER_FLAGS " -static")
#set(CMAKE_CXX_COMPILER x86_64-w64-mingw32-gcc )
#set(CMAKE_CXX_FLAGS "-Wl,-Bstatic -s -w -ffunction-sections -fdata-sections -Wno-write-strings -fno-exceptions -fmerge-all-constants -static-libgcc -Wl,-Bstatic " )

#set(CMAKE_BUILD_TYPE Debug)

set(SRCS main.cpp
         misc.cpp
         cliente.cpp)


link_libraries(netapi32 wsock32 ws2_32)
add_executable(cliente ${SRCS})
#target_link_libraries(cliente)
