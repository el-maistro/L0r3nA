#ifndef __KEYLOG_
#define __KEYLOG_

#include "headers.hpp"

class panelKeylogger : public wxPanel {
	public:
		panelKeylogger(wxWindow* pParent);
		wxToggleButton* btn_Iniciar = nullptr;
		wxTextCtrl* txt_Data = nullptr;

		void EnviarComando(std::string strComando, bool isBlock);
	private:

		void OnToggle(wxCommandEvent& event); 
		void OnLimpiar(wxCommandEvent& event);
		void OnGuardarLog(wxCommandEvent& event);

		wxDECLARE_EVENT_TABLE();
};

#endif