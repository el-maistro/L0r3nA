#include "file_editor.hpp"
#include "panel_file_manager.hpp"
#include "misc.hpp"

wxBEGIN_EVENT_TABLE(wxEditForm, wxFrame)
	EVT_MENU(EnumIDS::ID_Panel_FM_Editar_Save_Remoto, wxEditForm::OnGuardarRemoto)
	EVT_MENU(EnumIDS::ID_Panel_FM_Editar_Save_Local, wxEditForm::OnGuardarLocal)
wxEND_EVENT_TABLE()

wxEditForm::wxEditForm(wxWindow* pParent, wxString strNombre, std::string strID)
	: wxFrame(pParent, wxID_ANY, "[REMOTO]" + strNombre, wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE, strID)
{
	this->p_txtEditor = new wxTextCtrl(this, EnumIDS::ID_Panel_FM_Editar_TXT, wxEmptyString, wxDefaultPosition, wxSize(500, 600), wxTE_MULTILINE | wxTE_RICH | wxHSCROLL);

	this->strFilename = strNombre;

	wxBoxSizer* nsizer = new wxBoxSizer(wxHORIZONTAL);

	nsizer->Add(this->p_txtEditor, 1, wxALL | wxEXPAND, 5);

	wxMenuBar* p_menu = new wxMenuBar();

	wxMenu* p_file = new wxMenu();
	p_file->Append(EnumIDS::ID_Panel_FM_Editar_Save_Remoto, "Guardar [REMOTO]");
	p_file->AppendSeparator();
	p_file->Append(EnumIDS::ID_Panel_FM_Editar_Save_Local, "Guardar [LOCAL]");
	p_file->AppendSeparator();
	p_file->Append(wxID_CLOSE, "Salir");

	
	//Encoders :v
	//wxMenu* p_encoders = new wxMenu();
	//p_encoders->Append(wxID_ANY, "Base64");
	//p_encoders->Append(wxID_ANY, "MD5");
	
	//wxMenu* p_misc_tools = new wxMenu();
	//p_misc_tools->AppendSubMenu(p_encoders, "Encoders");
	//p_misc_tools->AppendSeparator();
	//p_misc_tools->Append(wxID_ANY, "Buscar");
	
	p_menu->Append(p_file, "Archivo");
	//p_menu->Append(p_misc_tools, "Herramientas");


	this->SetSizer(nsizer);
	this->SetMenuBar(p_menu);

	ListCtrlManager* temp = (ListCtrlManager*)this->GetParent();

	std::string strComando = strNombre;
	strComando.append(1, CMD_DEL);
	strComando += strID;

	if (temp) {
		temp->itemp->EnviarComando(strComando, EnumComandos::FM_Editar_Archivo);
	}else {
		error();
	}
}

void wxEditForm::OnGuardarRemoto(wxCommandEvent& event) {
	std::string strComando = this->strFilename;
	strComando.append(1, CMD_DEL);
	strComando += this->p_txtEditor->GetValue();

	ListCtrlManager* temp = (ListCtrlManager*)this->GetParent();
	
	if (temp) {
		temp->itemp->EnviarComando(strComando, EnumComandos::FM_Editar_Archivo_Guardar_Remoto);
	}
	else {
		error();
	}
}


void wxEditForm::OnGuardarLocal(wxCommandEvent& event) {
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
			std::cout << "[X] No se pudo abrir el archivo "<< dialog.GetPath() << std::endl;
			error();
		}
	}
	
}