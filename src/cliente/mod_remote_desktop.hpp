#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"
#include "mod_dynamic_load.hpp"

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

struct Pixel {
	BYTE R;
	BYTE G;
	BYTE B;
};

struct Pixel_Data {
	int x;
	int y;
	Pixel data;
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
		GdiplusStartupInput gdiplusStartupInput;
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

		//TESTING BORRAR
		Gdiplus::Bitmap* oldBitmap = nullptr;

		std::vector<Monitor> vc_Monitores;
		
		static BOOL MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT rectMonitor, LPARAM lparam);
		
	public:
		bool isRunning = false;
		st_User32_RD USER32;
		st_Gdi32 GDI32;
		st_GdiPlus GDIPLUS;
		st_Ole32 OLE32;

		bool m_isRunning();

		//Cambio en tiempo real de calidad y mostrar puntero
		void m_UpdateQuality(int iNew);
		void m_UpdateVmouse(bool isVisible);

		//Mouse remoto
		void m_RemoteMouse(int x, int y, int monitor_index, int mouse_action);
		void m_RemoteTeclado(char key, bool isDown);

		//Comparacion de imagenes (incompleto)
		int BitmapDiff(Gdiplus::Bitmap*& _oldBitmap, Gdiplus::Bitmap*& _newBitmap, std::vector<Pixel_Data>& _outPixels);
		
		//Funciones para monitores
		std::vector<Monitor> m_ListaMonitores();
		Monitor m_GetMonitor(int index);
		void m_Agregar_Monitor(Monitor& new_monitor);
		void m_Clear_Monitores();
		std::vector<Monitor> m_GetVectorCopy();

		mod_RemoteDesktop(st_User32_RD& _user32, st_Gdi32& _gdi32, st_GdiPlus& _gdiplus, st_Ole32& _ole32);
		~mod_RemoteDesktop();

		void IniciarLive(int quality, int monitor_index);
		void SpawnThread(int quality, int monitor_index);
		void DetenerLive();
		Gdiplus::Bitmap* getFrameBitmap(ULONG quality, int index);
		std::vector<char> getBitmapBytes(Gdiplus::Bitmap*& _in, ULONG _quality);
		void pixelSerialize(const std::vector<Pixel_Data>& _vcin, std::vector<char>& _vcout);
		int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
};

#endif
