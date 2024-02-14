#include "headers.hpp"

class Cliente {
	private:
		SOCKET sckSocket = INVALID_SOCKET;
	public:
		~Cliente();

		bool isRunning = true;
		//Sockets
		bool bConectar(const char* cIP, const char* cPuerto);
		void CerrarConexion();
		//Socket wraps
		int cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false);
		int cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock = false);

		void iniPacket();

		void MainLoop();

		void ProcesarComando(std::vector<std::string> strIn);
};