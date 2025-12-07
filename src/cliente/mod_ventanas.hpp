#ifdef __MOD_WM

#ifndef __MOD_VENTANA
#define __MOD_VENTANA 1

#include "headers.hpp"
#include "mod_dynamic_load.hpp"

struct VentanaInfo {
	HWND hwnd;
	std::string strTitle;
	bool active;
};

class mod_AdminVentanas {
	public:
		std::vector<VentanaInfo> m_ListaVentanas();
		std::vector<VentanaInfo> vcVentanas;

		void m_WindowMSG(const std::string strTitle, int iMessage);
		mod_AdminVentanas(st_User32_WM& _user32);
		
		st_User32_WM USER32;
	private:
		int m_IndexOf(const std::string strTitle);
};

#endif

#endif