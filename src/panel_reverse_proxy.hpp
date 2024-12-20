#ifndef __PNL_REVERSE_PROXY
#define __PNL_REVERSE_PROXY 1

#include "headers.hpp"

namespace EnumIDSProxy {
	enum Enum {
		ID_Main_Window = 300,
		ID_BTN_Toggle,
		ID_TXT_Port
	};
}

class panelReverseProxy : public wxPanel {
	public:
		panelReverseProxy(wxWindow* pParent, SOCKET sck);
	private:
		wxToggleButton* btnToggle = nullptr;
		wxTextCtrl* txtPort = nullptr;

		SOCKET sckSocket = INVALID_SOCKET;

		void OnToggle(wxCommandEvent&);
		
		wxDECLARE_EVENT_TABLE();
};

#endif