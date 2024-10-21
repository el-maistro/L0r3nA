#ifndef __MOD_VENTANA
#define __MOD_VENTANA 1

#include "headers.hpp"

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

	private:
		int m_IndexOf(const std::string strTitle);
};

#endif