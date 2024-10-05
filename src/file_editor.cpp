#include "file_editor.hpp"
#include "panel_file_manager.hpp"
#include "misc.hpp"

wxBEGIN_EVENT_TABLE(wxEditForm, wxFrame)
	EVT_MENU(EditorIDS::Edit_Save_Remoto, wxEditForm::OnGuardarRemoto)
	EVT_MENU(EditorIDS::Edit_Save_Local, wxEditForm::OnGuardarLocal)
	EVT_MENU(EditorIDS::Edit_Menu_Buscar, wxEditForm::OnBuscar)
	EVT_MENU(EditorIDS::Edit_Menu_Remplazar, wxEditForm::OnRemplazar)
	EVT_MENU(EditorIDS::Edit_Menu_Encoders, wxEditForm::OnEncoders)
	EVT_FIND(wxID_ANY, wxEditForm::OnBuscarDialog)
	EVT_FIND_NEXT(wxID_ANY, wxEditForm::OnBuscarDialog)
	EVT_FIND_REPLACE(wxID_ANY, wxEditForm::OnBuscarDialog)
	EVT_FIND_REPLACE_ALL(wxID_ANY, wxEditForm::OnBuscarDialog)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxEncoders, wxFrame)
	EVT_BUTTON(EditorIDS::ENC_Process, wxEncoders::OnProcesar)
wxEND_EVENT_TABLE()

wxEditForm::wxEditForm(wxWindow* pParent, wxString strNombre, std::string strID)
	: wxFrame(pParent, wxID_ANY, "[REMOTO]" + strNombre, wxDefaultPosition, wxSize(600, 600), wxDEFAULT_FRAME_STYLE, strID)
{
	this->p_txtEditor = new wxStyledTextCtrl(this, EditorIDS::Edit_Text, wxDefaultPosition, wxDefaultSize);
	this->p_txtEditor->SetCaretLineVisible(true);
	this->p_txtEditor->SetCaretLineBackground(wxColour(0, 200, 255));
	this->p_txtEditor->SetMarginType(0, wxSTC_MARGIN_NUMBER);
	this->p_txtEditor->SetMarginWidth(0, 40);
	
	this->strFilename = strNombre;

	wxBoxSizer* nsizer = new wxBoxSizer(wxHORIZONTAL);

	nsizer->Add(this->p_txtEditor, 1, wxALL | wxEXPAND, 5);

	wxMenuBar* p_menu = new wxMenuBar();

	wxMenu* p_file = new wxMenu();
	p_file->Append(EditorIDS::Edit_Save_Remoto, "Guardar [REMOTO]");
	p_file->AppendSeparator();
	p_file->Append(EditorIDS::Edit_Save_Local, "Guardar [LOCAL]");
	p_file->AppendSeparator();
	p_file->Append(wxID_CLOSE, "Salir");

	wxMenu* p_misc_tools = new wxMenu();
	p_misc_tools->Append(EditorIDS::Edit_Menu_Encoders, "Encoders");
	p_misc_tools->AppendSeparator();
	p_misc_tools->Append(EditorIDS::Edit_Menu_Buscar, "Buscar");
	p_misc_tools->Append(EditorIDS::Edit_Menu_Remplazar, "Buscar y remplazar");
	
	p_menu->Append(p_file, "Archivo");
	p_menu->Append(p_misc_tools, "Herramientas");

	this->SetSizer(nsizer);
	this->SetMenuBar(p_menu);

	/*ListCtrlManager* temp = (ListCtrlManager*)this->GetParent();

	std::string strComando = strNombre;
	strComando.append(1, CMD_DEL);
	strComando += strID;

	if (temp) {
		temp->itemp->EnviarComando(strComando, EnumComandos::FM_Editar_Archivo);
	}else {
		DEBUG_MSG("No existe el panel principal");
	}*/
}

void wxEditForm::OnGuardarRemoto(wxCommandEvent&) {
	std::string strComando = this->strFilename;
	strComando.append(1, CMD_DEL);
	strComando += this->p_txtEditor->GetValue();

	ListCtrlManager* temp = (ListCtrlManager*)this->GetParent();
	
	if (temp) {
		temp->itemp->EnviarComando(strComando, EnumComandos::FM_Editar_Archivo_Guardar_Remoto);
	}
	else {
		DEBUG_MSG("No existe el panel principal");
	}
}

void wxEditForm::OnGuardarLocal(wxCommandEvent&) {
	std::vector<std::string> vcPath = strSplit(this->strFilename, '\\', 100);
	std::string strOutFile = vcPath[vcPath.size() - 1];

	wxFileDialog dialog(this, "Guardar archivo", wxEmptyString, strOutFile, wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_OK) {
		std::ofstream outFile(std::string(dialog.GetPath()), std::ios::binary);

		if (outFile.is_open()) {
			std::string strBuffer = this->p_txtEditor->GetValue();
			outFile.write(strBuffer.c_str(), strBuffer.size());

			outFile.flush();
			outFile.close();
		}else {
			ERROR("[X] No se pudo abrir el archivo " << dialog.GetPath());
		}
	}
	
}

void wxEditForm::OnBuscar(wxCommandEvent&) {
	wxFindReplaceDialog* find_dlg = new wxFindReplaceDialog(this, &this->m_findData, "Buscar", wxFR_NOWHOLEWORD);
	find_dlg->Show(true);
}

void wxEditForm::OnRemplazar(wxCommandEvent& event) {
	wxFindReplaceDialog* find_dlg = new wxFindReplaceDialog(this, &this->m_findData, "Buscar y remplazar", wxFR_REPLACEDIALOG);
	find_dlg->Show(true);
}

void wxEditForm::OnBuscarDialog(wxFindDialogEvent& event) {
	wxEventType type = event.GetEventType();
	int flags = event.GetFlags();
	wxString str = event.GetFindString();
	bool isDown =          (flags & wxFR_DOWN)      ? true : false;
	bool whole_word =      (flags & wxFR_WHOLEWORD) ? true : false;
	bool isCaseSensitive = (flags & wxFR_MATCHCASE) ? true : false;
	int posActual = this->p_txtEditor->GetCurrentPos();
	int posResultado = -1;

	if(type == wxEVT_FIND || type == wxEVT_FIND_NEXT){
		if (type == wxEVT_FIND_NEXT && isDown) { posActual += str.size(); }
		if (isDown) {
			posResultado = this->p_txtEditor->FindText(posActual, this->p_txtEditor->GetLength(), str, 0, 0);
		}else {
			posResultado = this->p_txtEditor->FindText(0, posActual,str, 0, 0);
		}
		
		if (posResultado >= 0) {
			this->p_txtEditor->SetEmptySelection(posResultado);
			this->p_txtEditor->SetCurrentPos(posResultado);
			this->p_txtEditor->SetFocus();
		}else {
			wxMessageBox("No se encontraron resultados", "Buscar", wxICON_ERROR);
		}

	}else if (type == wxEVT_FIND_REPLACE || type == wxEVT_FIND_REPLACE_ALL) {
		posResultado = this->p_txtEditor->FindText(posActual, this->p_txtEditor->GetLength(), str, 0, 0);
		if (posResultado >= 0) {
			int strNewSize = event.GetReplaceString().size();
			
			this->p_txtEditor->DeleteRange(posResultado, str.size());
			this->p_txtEditor->InsertText(posResultado, event.GetReplaceString());
			this->p_txtEditor->SetEmptySelection(posResultado + strNewSize);
			this->p_txtEditor->SetCurrentPos(posResultado + strNewSize);
			if (type == wxEVT_FIND_REPLACE_ALL) {
				OnBuscarDialog(event);
			}
		}else {
			if (type == wxEVT_FIND_REPLACE) {
				wxMessageBox("No se encontraron resultados", "Buscar y remplazar", wxICON_ERROR);
			}
		}
	}
}

void wxEditForm::OnEncoders(wxCommandEvent& event) {
	wxEncoders* frame_encoders = new wxEncoders(this);
	frame_encoders->Show(true);
}

wxEncoders::wxEncoders(wxWindow* pParent) :
	wxFrame(pParent, wxID_ANY, "Funciones de encoding/hashing", wxDefaultPosition, wxDefaultSize) {
	
	wxStaticText* lblIn = new wxStaticText(this, wxID_ANY, "Entrada:");
	wxStaticText* lblOut = new wxStaticText(this, wxID_ANY, "Salida:");


	wxArrayString cmbOpciones;
	cmbOpciones.push_back(wxString("base64Encode"));
	cmbOpciones.push_back(wxString("base64Decode"));
	cmbOpciones.push_back(wxString("ROT13"));

	this->cmbOpcion = new wxComboBox(this, EditorIDS::ENC_Combo, "Seleccionar funcion", wxDefaultPosition, wxDefaultSize, cmbOpciones, wxCB_READONLY);

	this->txtIn = new wxTextCtrl(this, EditorIDS::ENC_Text_In, wxEmptyString, wxDefaultPosition, wxSize(200,100), wxTE_MULTILINE | wxTE_RICH);
	this->txtOut = new wxTextCtrl(this, EditorIDS::ENC_Text_Out, wxEmptyString, wxDefaultPosition, wxSize(200, 100), wxTE_MULTILINE | wxTE_RICH);
	wxButton* btnProcess = new wxButton(this, EditorIDS::ENC_Process, "Procesar", wxDefaultPosition, wxDefaultSize);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->Add(this->cmbOpcion, 0, wxEXPAND | wxALL, 1);
	main_sizer->Add(lblIn);
	main_sizer->Add(this->txtIn);
	main_sizer->Add(lblOut);
	main_sizer->Add(this->txtOut);
	main_sizer->Add(btnProcess, 0, wxEXPAND | wxALL, 1);

	this->SetSizerAndFit(main_sizer);
	this->SetSizeHints(216, 321, 216, 321);
}

void wxEncoders::OnProcesar(wxCommandEvent& event) {
	wxString strOpcion = this->cmbOpcion->GetValue();
	if (strOpcion != "" && strOpcion != "Seleccionar funcion") {
		wxString strIn = this->txtIn->GetValue();
		wxString strOut = this->strProcesar(strIn, strOpcion);
		this->txtOut->SetValue(strOut);
	}
	DEBUG_MSG(strOpcion);
}

wxString wxEncoders::strProcesar(const wxString& strIn, const wxString& metodo) {
	wxString strOut = "";
	if (metodo == "base64Encode") {
		strOut = base64_encode(strIn.ToStdString());
	}else if (metodo == "base64Decode") {
		strOut = base64_decode(strIn.ToStdString());
	}else if (metodo == "ROT13") {
		strOut = this->ROT13(strIn);
	}
	DEBUG_MSG(strOut);
	return strOut;
}

wxString wxEncoders::ROT13(const wxString& in) {
	// rot13.com
	// (c <= 'Z' ? 90 : 122) >= (c = c.charCodeAt(0) + i) ? c : c - 26)
		
	wxString out = "";
	for (int i = 0; i < in.size(); i++) {
		char c = in[i];
		int iLetra = static_cast<int>(c);
		if (iLetra == 32) {
			out += " ";
		}else {
			int iLimit = c <= 'Z' ? 90 : 122;
			int iLowest = iLimit == 90 ? 64 : 96;
			int nuevo = (iLimit) >= (iLetra + 13) ? iLetra + 13 : iLowest + ((iLetra + 13) - iLimit);
			out += static_cast<char>(nuevo);
		}
	}
	return out;
}