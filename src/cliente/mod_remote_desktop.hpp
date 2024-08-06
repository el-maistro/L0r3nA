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
		
		std::thread th_RemoteDesktop;
		std::mutex mtx_RemoteDesktop;
		std::mutex mtx_Quality;

		void InitGDI() {
			if (Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL) == Gdiplus::Status::Ok) {
				isGDIon = true;
			}
		}

		void StopGDI() {
			Gdiplus::GdiplusShutdown(gdiplusToken);
		}

		ULONG m_Quality() {
			std::unique_lock<std::mutex> lock(mtx_Quality);
			return uQuality;
		}

	public:
		bool isRunning = false;

		bool m_isRunning() {
			std::unique_lock<std::mutex> lock(mtx_RemoteDesktop);
			return isRunning;
		}

		void m_UpdateQuality(int iNew) {
			//32 default
			std::unique_lock<std::mutex> lock(mtx_Quality);
			uQuality = iNew == 0 ? 32 : static_cast<ULONG>(iNew);
		}

		mod_RemoteDesktop();
		~mod_RemoteDesktop();

		void IniciarLive(int quality);
		void SpawnThread(int quality);
		void DetenerLive();
		std::vector<BYTE> getFrameBytes(ULONG quality);
};

#endif
