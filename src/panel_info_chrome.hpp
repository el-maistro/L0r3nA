#ifndef __INFO_CHROME
#define __INFO_CHROME 1

#include "headers.hpp"

namespace EnumChromeInfoIDS {
	enum Enum {
		BTN_Profiles = 20000,
		BTN_Passwords,
		BTN_HistorialN,
		BTN_HistorialD,
		BTN_Busquedas,
		BTN_Cookies
	};
}

struct ColumnData {
	std::string strTitle;
	int iSize;
};

class panelInfoChrome : public wxPanel {
	public:
		panelInfoChrome(wxWindow* pParent, SOCKET sck_socket);

		void m_AgregarDataPerfiles(const std::string& strBuffer);
		void m_ProcesarInfoPerfil(const std::string& strBuffer);

	private:
		SOCKET sckSocket = INVALID_SOCKET;
		
		void OnProcesarBoton(wxCommandEvent& event);
		std::string GetSelectedUserPath();

		wxListCtrl* listCtrlUsers      = nullptr;
		wxListCtrl* listCtrlPasswords  = nullptr;
		wxListCtrl* listCtrlHistorialN = nullptr;
		wxListCtrl* listCtrlHistorialD = nullptr;
		wxListCtrl* listCtrlSearchT    = nullptr;
		wxListCtrl* listCtrlCookies    = nullptr;

		void m_CrearListCtrls();
		void m_InsertarColumnas(std::vector<ColumnData>& columns, wxListCtrl*& list_ctrl);

		wxDECLARE_EVENT_TABLE();
};


#endif