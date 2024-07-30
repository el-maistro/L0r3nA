#ifndef __REMOTE_DESKTOP
#define __REMOTE_DESKTOP

#include "headers.hpp"

class frameRemoteDesktop : public wxFrame {
	public:
		frameRemoteDesktop(wxWindow* pParent);

	private:
		SOCKET sckCliente = INVALID_SOCKET;
		void Onclose(wxCloseEvent& event);

		wxDECLARE_EVENT_TABLE();
};

#endif