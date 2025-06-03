#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"
#define KEYEVENTF_KEDOWN 0x0000

struct st_User32_RD {
	//SendInput
	typedef UINT(WINAPI* LPSENDINPUT)(UINT, LPINPUT, int);
	LPSENDINPUT pSendInput = nullptr;

	//GetDC
	typedef HDC(WINAPI* LPGETDC)(HWND);
	LPGETDC pGetDC = nullptr;

	//GetDesktopWindow
	typedef HWND(WINAPI* LPGETDESKTOPWINDOW)();
	LPGETDESKTOPWINDOW pGetDesktopWindow = nullptr;

	//EnumDisplayMonitors
	typedef BOOL(WINAPI* LPENUMDISPLAYMONITORS)(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
	LPENUMDISPLAYMONITORS pEnumDisplayMonitors = nullptr;

	//GetMonitorInfoA
	typedef BOOL(WINAPI* LPGETMONITORINFOA)(HMONITOR, LPMONITORINFO);
	LPGETMONITORINFOA pGetMonitorInfoA = nullptr;

	//GetCursorInfo 
	typedef BOOL(WINAPI* LPGETCURSORINFO)(PCURSORINFO);
	LPGETCURSORINFO pGetCursorInfo = nullptr;
	
	//GetWindowRect 
	typedef BOOL(WINAPI* LPGETWINDOWRECT)(HWND, LPRECT);
	LPGETWINDOWRECT pGetWindowRect = nullptr;

	//GetIconInfo
	typedef BOOL(WINAPI* LPGETICONINFO)(HICON, PICONINFO);
	LPGETICONINFO pGetIconInfo = nullptr;

	//DrawIconEx
	typedef BOOL(WINAPI* LPDRAWICONEX)(HDC, int, int, HICON, int, int, UINT, HBRUSH, UINT);
	LPDRAWICONEX pDrawIconEx = nullptr;
};

struct st_Gdi32 {
	//CreateCompatibleDC
	typedef HDC(WINAPI* LPCREATECOMPATIBLEDC)(HDC);
	LPCREATECOMPATIBLEDC pCreateCompatibleDC = nullptr;

	//CreateCompatibleBitmap
	typedef HBITMAP(WINAPI* LPCREATECOMPATIBLEBITMAP)(HDC, int, int);
	LPCREATECOMPATIBLEBITMAP pCreateCompatibleBitmap = nullptr;

	//SelectObject
	typedef HGDIOBJ(WINAPI* LPSELECTOBJECT)(HDC, HGDIOBJ);
	LPSELECTOBJECT pSelectObject = nullptr;

	//GetObjectA
	typedef int(WINAPI* LPGETOBJECTA)(HANDLE, int, LPVOID);
	LPGETOBJECTA pGetObjectA = nullptr;

	//BitBlt
	typedef BOOL(WINAPI* LPBITBLT)(HDC, int, int, int, int, HDC, int, int, DWORD);
	LPBITBLT pBitBlt = nullptr;
};

struct st_Ole32 {
	//CreateStreamOnHGlobal
	typedef HRESULT(WINAPI* LPCREATESTREAMONHGLOBAL)(HGLOBAL, BOOL, LPSTREAM*);
	LPCREATESTREAMONHGLOBAL pCreateStreamOnHGlobal = nullptr;
};

struct st_GdiPlus {
	//GdiplusStartup
	typedef Gdiplus::Status(WINAPI* LPGDIPLUSSTARTUP)(ULONG_PTR*, const Gdiplus::GdiplusStartupInput*, Gdiplus::GdiplusStartupOutput*);
	LPGDIPLUSSTARTUP pGdiplusStartup = nullptr;

	//GdiplusShutdown
	typedef void(WINAPI* LPGDIPLUSSHUTDOWN)(ULONG_PTR);
	LPGDIPLUSSHUTDOWN pGdiplusShutdown = nullptr;
};

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

		//TESTING BORRAR
		Gdiplus::Bitmap* oldBitmap = nullptr;

		std::vector<Monitor> vc_Monitores;
		
		static BOOL MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT rectMonitor, LPARAM lparam);
		
		HMODULE hUser32DLL = NULL;
		HMODULE hKernel32 = NULL;
		HMODULE hGdi32DLL = NULL;
		HMODULE hOle32 = NULL;
		HMODULE hGdiPlusDLL = NULL;
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
		int BitmapDiff(std::shared_ptr<Gdiplus::Bitmap>& _oldBitmap, std::shared_ptr<Gdiplus::Bitmap>& _newBitmap, std::vector<Pixel_Data>& _outPixels);
		
		//Funciones para monitores
		std::vector<Monitor> m_ListaMonitores();
		Monitor m_GetMonitor(int index);
		void m_Agregar_Monitor(Monitor& new_monitor);
		void m_Clear_Monitores();
		std::vector<Monitor> m_GetVectorCopy();

		mod_RemoteDesktop(HMODULE _user32DLL);
		~mod_RemoteDesktop();

		void IniciarLive(int quality, int monitor_index);
		void SpawnThread(int quality, int monitor_index);
		void DetenerLive();
		std::shared_ptr<Gdiplus::Bitmap> getFrameBitmap(ULONG quality, int index);
		std::vector<char> getBitmapBytes(std::shared_ptr<Gdiplus::Bitmap>& _in, ULONG _quality);
		void pixelSerialize(const std::vector<Pixel_Data>& _vcin, std::vector<char>& _vcout);
};

#endif
