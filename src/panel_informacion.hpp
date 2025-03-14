#ifndef __PNL_INFORMACION
#define __PNL_INFORMACION 1

#include "headers.hpp"

namespace EnumPanelInfoIDS {
	enum Enum {
		BTN_Chrome = 20000,
	};
}

class panelInformacion : public wxFrame {
	public:
		panelInformacion(wxWindow* _wxParent, SOCKET _sckSocket, std::string _strID);

	private:
		SOCKET sckCliente = INVALID_SOCKET;

		//Eventos
		void OnChromeInfo(wxCommandEvent& event);

		wxDECLARE_EVENT_TABLE();
};

#endif