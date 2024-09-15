#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"
#include<shlwapi.h>
#include<gdiplus.h>

class mod_RemoteDesktop {
	private:
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		ULONG uQuality = 0;
		bool isGDIon = false;
		bool isMouseOn = false;
		
		std::thread th_RemoteDesktop;
		std::mutex mtx_RemoteDesktop;
		std::mutex mtx_RemoteSettings;


		void InitGDI();

		void StopGDI();

		ULONG m_Quality();
		bool  m_Vmouse();

	public:
		bool isRunning = false;

		bool m_isRunning();

		void m_UpdateQuality(int iNew);
		void m_UpdateVmouse(bool isVisible);

		bool m_AreEqual(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2);
		std::vector<char> m_Diff(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2);

		mod_RemoteDesktop();
		~mod_RemoteDesktop();

		void IniciarLive(int quality);
		void SpawnThread(int quality);
		void DetenerLive();
		std::vector<char> getFrameBytes(ULONG quality);
};

#endif
