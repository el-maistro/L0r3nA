#ifndef __PANEL_WM
#define __PANEL_WM 1

#include "headers.hpp"

class panelWManager : public wxPanel {
	public:
		panelWManager(wxWindow* pParent, SOCKET sckCliente);

	private:
		SOCKET sckCliente = INVALID_SOCKET;
};

#endif
