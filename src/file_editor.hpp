#ifndef __FILE_EDITOR
#define __FILE_EDITOR

#include "headers.hpp"
#include <wx/fdrepdlg.h>

namespace EditorIDS {
	enum Enum {
		Edit_Text = 100,
		Edit_Save_Remoto,
		Edit_Save_Local,
		Edit_Menu_Buscar,
		Edit_Menu_Remplazar,
		Edit_Menu_Encoders
	};
}

class wxEditForm : public wxFrame {
	public:
		wxEditForm(wxWindow* pParent, wxString strNombre, std::string strID);
		wxStyledTextCtrl* p_txtEditor = nullptr;

		//Eventos
		void OnGuardarRemoto(wxCommandEvent& event);
		void OnGuardarLocal(wxCommandEvent& event);
		void OnBuscar(wxCommandEvent& event);
		void OnRemplazar(wxCommandEvent& event);

		//Dialog busqueda y remplazo
		void OnBuscarDialog(wxFindDialogEvent& event);
	private:
		wxFindReplaceData m_findData;
		std::string strFilename = "";

		wxDECLARE_EVENT_TABLE();
		
};

#endif