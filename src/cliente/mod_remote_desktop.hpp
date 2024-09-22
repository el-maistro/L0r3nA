#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"
#include<shlwapi.h>
#include<gdiplus.h>

#define KEYEVENTF_KEDOWN 0x0000

struct rect_Monitor {
	int resWidth;
	int resHeight;
	int xStart;
	int yStart;
	rect_Monitor()
		:resWidth(0), resHeight(0), xStart(0), yStart(0) {}
};

struct Monitor {
	char szDevice[CCHDEVICENAME];
	rect_Monitor rectData;
};

namespace EnumRemoteMouse {
	enum Enum {
		_LEFT_DOWN = 1,
		_LEFT_UP,
		_RIGHT_DOWN,
		_RIGHT_UP,
		_MIDDLE_DOWN,
		_MIDDLE_UP,
		_DOUBLE_LEFT,
		_DOUBLE_RIGHT,
		_DOUBLE_MIDDLE,
		_WHEEL_DOWN,
		_WHEEL_UP
	};
}

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
		std::mutex mtx_Monitores;

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

		//Mouse remoto
		void m_RemoteMouse(int x, int y, int monitor_index, int mouse_action);
		void m_RemoteTeclado(char key, bool isDown);

		//Comparacion de imagenes (incompleto)
		bool m_AreEqual(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2);
		std::vector<char> m_Diff(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2);

		//Funciones para monitores
		std::vector<Monitor> m_ListaMonitores();
		Monitor m_GetMonitor(int index);
		void m_Agregar_Monitor(Monitor& new_monitor);
		void m_Clear_Monitores();
		std::vector<Monitor> m_GetVectorCopy();

		mod_RemoteDesktop();
		~mod_RemoteDesktop();

		void IniciarLive(int quality, int monitor_index);
		void SpawnThread(int quality, int monitor_index);
		void DetenerLive();
		std::vector<char> getFrameBytes(ULONG quality, int index);
};

#endif
