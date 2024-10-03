#include "file_editor.hpp"
#include "panel_file_manager.hpp"
#include "misc.hpp"

wxBEGIN_EVENT_TABLE(wxEditForm, wxFrame)
	EVT_MENU(EditorIDS::Edit_Save_Remoto, wxEditForm::OnGuardarRemoto)
	EVT_MENU(EditorIDS::Edit_Save_Local, wxEditForm::OnGuardarLocal)
	EVT_MENU(EditorIDS::Edit_Menu_Buscar, wxEditForm::OnBuscar)
	EVT_MENU(EditorIDS::Edit_Menu_Remplazar, wxEditForm::OnRemplazar)
	EVT_FIND(wxID_ANY, wxEditForm::OnBuscarDialog)
	EVT_FIND_NEXT(wxID_ANY, wxEditForm::OnBuscarDialog)
	EVT_FIND_REPLACE(wxID_ANY, wxEditForm::OnBuscarDialog)
	EVT_FIND_REPLACE_ALL(wxID_ANY, wxEditForm::OnBuscarDialog)
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
	/*wxTextEntryDialog dialog(this, "Buscar", "Buscar texto", "itnik", wxOK | wxCANCEL);
	if (dialog.ShowModal() == wxID_OK) {
		//dialog.GetValue()
		int npos = this->p_txtEditor->FindText(0, this->p_txtEditor->GetLength(), dialog.GetValue(), 0, 0);
		if (npos >= 0) {
			//this->p_txtEditor->SetInsertionPoint(npos);
			this->p_txtEditor->SetEmptySelection(npos);
			this->p_txtEditor->SetCurrentPos(npos);
		}else {
			std::string strMsg = "No se pudo encontrar: " + dialog.GetValue();
			wxMessageBox(strMsg, "Resultado");
		}
	}*/

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