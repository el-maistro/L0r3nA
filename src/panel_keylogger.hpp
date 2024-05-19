#ifndef __KEYLOG_
#define __KEYLOG_

#include "headers.hpp"

class panelKeylogger : public wxPanel {
	public:
		panelKeylogger(wxWindow* pParent);
		wxToggleButton* btn_Iniciar = nullptr;
		wxTextCtrl* txt_Data = nullptr;
		SOCKET sckCliente = INVALID_SOCKET;

	private:

		void OnToggle(wxCommandEvent& event); 
		void OnLimpiar(wxCommandEvent& event);
		void OnGuardarLog(wxCommandEvent& event);

		wxDECLARE_EVENT_TABLE();
};

#endif