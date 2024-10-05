#ifndef __FILE_EDITOR
#define __FILE_EDITOR

#include "headers.hpp"
#include <wx/fdrepdlg.h>
#include "base64/base64.h"

namespace EditorIDS {
	enum Enum {
		Edit_Text = 100,
		Edit_Save_Remoto,
		Edit_Save_Local,
		Edit_Menu_Buscar,
		Edit_Menu_Remplazar,
		Edit_Menu_Encoders,
		ENC_Combo,
		ENC_Text_In,
		ENC_Text_Out,
		ENC_Process
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
		void OnEncoders(wxCommandEvent& event);

		//Dialog busqueda y remplazo
		void OnBuscarDialog(wxFindDialogEvent& event);

		//Agregar texto
		void AgregarTexto(const char*& cBuffer);
	private:
		wxFindReplaceData m_findData;
		std::string strFilename = "";

		wxDECLARE_EVENT_TABLE();
		
};

class wxEncoders : public wxFrame {
	public:
		wxEncoders(wxWindow* pParent);

		void OnProcesar(wxCommandEvent& event);

	private:
		wxTextCtrl* txtIn = nullptr;
		wxTextCtrl* txtOut = nullptr;
		wxComboBox* cmbOpcion = nullptr;
		wxString strProcesar(const wxString& in, const wxString& metodo);
		wxString ROT13(const wxString& in);
		wxDECLARE_EVENT_TABLE();
};

#endif