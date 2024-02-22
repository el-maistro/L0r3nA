#ifndef _HEADERS
#define _HEADERS 1

#include<wx/wx.h> 
#include<wx/thread.h>
#include<wx/listctrl.h>
#include<wx/treectrl.h>
#include<wx/sizer.h>
#include<wx/aui/aui.h>

#include<ws2tcpip.h>
#include<windows.h>
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

#define PING_TIME 60 * 1


extern bool bSignalFlag;
#define error() std::cout<<"Error["<<GetLastError()<<"]\n";

typedef unsigned int u_int;

namespace EnumIDS {
    enum Enum {
        ID_Escuchar = 100,
        ID_Detener,
        ID_LimpiarLog,
        ID_Interactuar,
        ID_FrameClienteTest,
        TreeCtrl_ID
    };
}

#endif