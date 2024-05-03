#ifndef __FILE_EDITOR
#define __FILE_EDITOR

#include "headers.hpp"

class wxEditForm : public wxFrame {
	public:
		wxEditForm(wxWindow* pParent, wxString strNombre, std::string strID);
		wxTextCtrl* p_txtEditor = nullptr;

		//Eventos
		void OnGuardarRemoto(wxCommandEvent& event);
		void OnGuardarLocal(wxCommandEvent& event);
	private:
		std::string strFilename = "";

		wxDECLARE_EVENT_TABLE();
		
};

#endif