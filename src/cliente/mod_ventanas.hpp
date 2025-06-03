#ifndef __MOD_VENTANA
#define __MOD_VENTANA 1

#include "headers.hpp"

struct VentanaInfo {
	HWND hwnd;
	std::string strTitle;
	bool active;
};

struct st_User32_WM {
	//IsWindowVisible
	typedef BOOL(WINAPI* LPISWINDOWVISIBLE)(HWND);
	LPISWINDOWVISIBLE pIsWindowVisible = nullptr;

	//GetWindowTextA
	typedef int(WINAPI* LPGETWINDOWTEXTA)(HWND, LPSTR, int);
	LPGETWINDOWTEXTA pGetWindowTextA = nullptr;

	//GetForegroundWindow
	typedef HWND(WINAPI* LPGETFOREGROUNDWINDOW)();
	LPGETFOREGROUNDWINDOW pGetForegroundWindow = nullptr;

	//EnumWindows
	typedef BOOL(WINAPI* LPENUMWINDOWS)(WNDENUMPROC, LPARAM);
	LPENUMWINDOWS pEnumWindows = nullptr;

	//FindWindowA
	typedef HWND(WINAPI* LPFINDWINDOWA)(LPCSTR, LPCSTR);
	LPFINDWINDOWA pFindWindowA = nullptr;

	//ShowWindow
	typedef BOOL(WINAPI* LPSHOWWINDOW)(HWND, int);
	LPSHOWWINDOW pShowWindow = nullptr;
};

class mod_AdminVentanas {
	public:
		std::vector<VentanaInfo> m_ListaVentanas();
		std::vector<VentanaInfo> vcVentanas;

		void m_WindowMSG(const std::string strTitle, int iMessage);

		mod_AdminVentanas(HMODULE _user32DLL);

		st_User32_WM USER32;
	private:
		HMODULE hUser32DLL = NULL;
		
		int m_IndexOf(const std::string strTitle);
};

#endif