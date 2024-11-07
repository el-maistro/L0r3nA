#ifndef __INFO_CHROME
#define __INFO_CHROME 1

#include "headers.hpp"

namespace EnumChromeInfoIDS {
	enum Enum {
		BTN_Profiles = 20000
	};
}

class panelInfoChrome : public wxPanel {
	public:
		panelInfoChrome(wxWindow* pParent, SOCKET sck_socket);

	private:
		SOCKET sckSocket = INVALID_SOCKET;
		void OnListaPerfiles(wxCommandEvent&);

		wxDECLARE_EVENT_TABLE();
};


#endif