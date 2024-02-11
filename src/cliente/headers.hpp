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


extern bool bSignalFlag;
#define error() std::cout<<"Error["<<GetLastError()<<"]\n";

typedef unsigned int u_int;

#define ___DEBUG_