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

class panelInfoChrome : public wxFrame {
	public:
		panelInfoChrome(wxWindow* pParent, SOCKET sck_socket, wxString _title, std::string _strID, ByteArray c_key);

		void m_AgregarDataPerfiles(const std::string& strBuffer);
		void m_ProcesarInfoPerfil(const std::string& strBuffer);

	private:
		SOCKET sckSocket = INVALID_SOCKET;
		ByteArray enc_key;
		
		void OnProcesarBoton(wxCommandEvent& event);
		std::string GetSelectedUserPath();
		std::string GetSelectedUserName();
		std::string strDelim1 = ":[<>]:";
		std::string strDelim2 = ":[<->]:";


		wxListCtrl* listCtrlUsers      = nullptr;
		wxListCtrl* listCtrlPasswords  = nullptr;
		wxListCtrl* listCtrlHistorialN = nullptr;
		wxListCtrl* listCtrlHistorialD = nullptr;
		wxListCtrl* listCtrlSearchT    = nullptr;
		wxListCtrl* listCtrlCookies    = nullptr;

		wxStaticText* lblUsers         = nullptr;
		wxStaticText* lblPasswords     = nullptr;
		wxStaticText* lblHistorialN    = nullptr;
		wxStaticText* lblHistorialD    = nullptr;
		wxStaticText* lblSearchT       = nullptr;
		wxStaticText* lblCookies       = nullptr;

		void m_CrearListCtrls();
		void m_InsertarColumnas(std::vector<ColumnData>& columns, wxListCtrl*& list_ctrl);
		void m_InsertarDatos(const std::vector<std::vector<std::string>>& rows, wxListCtrl*& list_ctrl);

		wxDECLARE_EVENT_TABLE();
};


#endif