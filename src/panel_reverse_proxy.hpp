#ifndef __PNL_REVERSE_PROXY
#define __PNL_REVERSE_PROXY 1

#include "headers.hpp"

class panelReverseProxy : public wxPanel {
	public:
		panelReverseProxy(wxWindow* pParent, SOCKET sck);
	private:
		SOCKET sckSocket = INVALID_SOCKET;
		//wxDECLARE_EVENT_TABLE();
};

#endif