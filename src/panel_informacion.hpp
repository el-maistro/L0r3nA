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
		panelInformacion(wxWindow* _wxParent, SOCKET _sckSocket, std::string _strID, ByteArray c_key);

	private:
		SOCKET sckCliente = INVALID_SOCKET;
		ByteArray enc_key;
		std::string strdID = "";
		//Eventos
		void OnChromeInfo(wxCommandEvent& event);

		wxDECLARE_EVENT_TABLE();
};

#endif