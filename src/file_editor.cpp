#include "file_editor.hpp"
#include "panel_file_manager.hpp"

wxEditForm::wxEditForm(wxWindow* pParent, wxString strNombre, std::string strID)
	: wxFrame(pParent, wxID_ANY, "[REMOTO]" + strNombre, wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE, strID)
{
	this->p_txtEditor = new wxTextCtrl(this, EnumIDS::ID_Panel_FM_Editar_TXT, wxEmptyString, wxDefaultPosition, wxSize(400, 400), wxTE_MULTILINE | wxTE_RICH);

	wxBoxSizer* nsizer = new wxBoxSizer(wxHORIZONTAL);

	nsizer->Add(this->p_txtEditor, 1, wxALL | wxEXPAND, 5);

	wxMenuBar* p_menu = new wxMenuBar();

	wxMenu* p_file = new wxMenu();
	p_file->Append(wxID_ANY, "Guardar [REMOTO]");
	p_file->AppendSeparator();
	p_file->Append(wxID_ANY, "Guardar [LOCAL]");
	p_file->AppendSeparator();
	p_file->Append(wxID_ANY, "Salir");

	wxMenu* p_misc_tools = new wxMenu();
	
	//Encoders :v
	wxMenu* p_encoders = new wxMenu();
	p_encoders->Append(wxID_ANY, "Base64");
	p_encoders->Append(wxID_ANY, "MD5");
	
	p_misc_tools->AppendSubMenu(p_encoders, "Encoders");
	p_misc_tools->AppendSeparator();
	p_misc_tools->Append(wxID_ANY, "Buscar");
	
	p_menu->Append(p_file, "Archivo");
	p_menu->Append(p_misc_tools, "Herramientas");


	this->SetSizer(nsizer);
	this->SetMenuBar(p_menu);

	ListCtrlManager* temp = (ListCtrlManager*)this->GetParent();

	std::string strComando = std::to_string(EnumComandos::FM_Editar_Archivo);
	strComando.append(1, '~');
	strComando += strNombre;
	strComando.append(1, '~');
	strComando += strID;

	temp->itemp->EnviarComando(strComando);
}