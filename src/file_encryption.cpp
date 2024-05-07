#include "file_encryption.hpp"
#include "panel_file_manager.hpp"
#include "misc.hpp"

wxBEGIN_EVENT_TABLE(frameEncryption, wxFrame)
	EVT_BUTTON(EnumIDS::ID_FM_BTN_Random, frameEncryption::OnGenerarPass)
	EVT_BUTTON(EnumIDS::ID_FM_BTN_Crypt_Exec, frameEncryption::OnExecCrypt)
wxEND_EVENT_TABLE()

frameEncryption::frameEncryption(wxWindow* pParent, std::string strPath) :
	wxFrame(pParent, wxID_ANY, "Encriptar/Desencriptar Archivo", wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE) {
	this->p_strPath = strPath;
	
	wxArrayString* selection = new wxArrayString;
	selection->Add("Encriptar");
	selection->Add("Desencriptar");

	this->rdio_Options = new wxRadioBox(this, EnumIDS::ID_FM_Radio_Encriptar, wxEmptyString, wxDefaultPosition,
		wxDefaultSize, *selection, 0, wxRA_SPECIFY_COLS);

	wxStaticText* lbl_File = new wxStaticText(this, wxID_ANY, "Archivo: " + this->p_strPath);
	wxStaticText* lbl_Pass = new wxStaticText(this, wxID_ANY, "Contraseña");
	wxButton* btn_Random_Pass = new wxButton(this, EnumIDS::ID_FM_BTN_Random, "Generar contraseña");
	wxButton* btn_Exec = new wxButton(this, EnumIDS::ID_FM_BTN_Crypt_Exec, ">>>");

	this->txt_Pass = new wxTextCtrl(this, EnumIDS::ID_FM_Text_Password);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	
	sizer->Add(lbl_File, 0, wxEXPAND, 1);
	sizer->Add(this->rdio_Options, 0, wxEXPAND, 1);
	sizer->Add(lbl_Pass, 0, wxEXPAND, 1);
	sizer->Add(this->txt_Pass, 0, wxEXPAND, 1);
	sizer->Add(btn_Random_Pass, 0, wxEXPAND, 1);
	sizer->Add(btn_Exec, 0, wxEXPAND, 1);

	this->SetSizerAndFit(sizer);

	this->SetClientSize(400, 150);

}

void frameEncryption::OnGenerarPass(wxCommandEvent& event) {
	this->txt_Pass->SetValue(RandomPass(40));
}

void frameEncryption::OnExecCrypt(wxCommandEvent& event) {
	std::string strComando = std::to_string(EnumComandos::FM_Crypt_Archivo);
	strComando.append(1, '~');
	strComando += (this->rdio_Options->GetSelection() == 0) ? "0" : "1";
	strComando.append(1, '~');
	strComando += this->p_strPath;
	strComando.append(1, '~');
	strComando += this->txt_Pass->GetValue();

	ListCtrlManager* list_parent = (ListCtrlManager*)this->GetParent();

	if (list_parent) {
		list_parent->itemp->EnviarComando(strComando);
	}
}