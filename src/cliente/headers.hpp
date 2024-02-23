#ifndef _HEADERS
#define _HEADERS 1

#include<ws2tcpip.h>
#include<windows.h>
#include<iostream>
#include<mutex>
#include<atomic>
#include<thread>
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

#endif