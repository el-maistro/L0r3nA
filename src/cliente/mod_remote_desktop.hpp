#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"
#include<shlwapi.h>
#include<gdiplus.h>

struct rect_Monitor {
	int resWidth;
	int resHeight;
	int xStart;
	int yStart;
};

struct Monitor {
	char szDevice[CCHDEVICENAME];
	rect_Monitor rectData;
};

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

		std::vector<Monitor> vc_Monitores;
		
		static BOOL MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT rectMonitor, LPARAM lparam);

	public:
		bool isRunning = false;

		bool m_isRunning();

		//Cambio en tiempo real de calidad y mostrar puntero
		void m_UpdateQuality(int iNew);
		void m_UpdateVmouse(bool isVisible);

		//Comparacion de imagenes (incompleto)
		bool m_AreEqual(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2);
		std::vector<char> m_Diff(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2);

		std::vector<Monitor> m_ListaMonitores();

		mod_RemoteDesktop();
		~mod_RemoteDesktop();

		void IniciarLive(int quality);
		void SpawnThread(int quality);
		void DetenerLive();
		std::vector<char> getFrameBytes(ULONG quality);
};

#endif
