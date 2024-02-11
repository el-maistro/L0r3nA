#include "headers.hpp"

class Cliente {
	private:
		SOCKET sckSocket = INVALID_SOCKET;
	public:
		Cliente();
		~Cliente();

		//Sockets
		bool bConectar(const char* cIP, const char* cPuerto);
		void CerrarConexion();
		//Socket wraps
		int cSend(int& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false);
		int cRecv(int& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock = false);

		void iniPacket();

		void MainLoop();
};