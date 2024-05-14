#include "panel_keylogger.hpp"

wxBEGIN_EVENT_TABLE(panelKeylogger, wxPanel)
	EVT_TOGGLEBUTTON(EnumIDS::ID_KL_BTN_Toggle, panelKeylogger::OnToggle)
wxEND_EVENT_TABLE()

panelKeylogger::panelKeylogger(wxWindow* pParent) :
	wxPanel(pParent, EnumIDS::ID_KL_Panel) {
	this->btn_Iniciar = new wxToggleButton(this, EnumIDS::ID_KL_BTN_Toggle, "Iniciar");


}

void panelKeylogger::OnToggle(wxCommandEvent& event) {
	bool isSel = this->btn_Iniciar->GetValue();
	if (isSel) {
		//Iniciar keylogger
		this->btn_Iniciar->SetLabelText(wxT("Detener"));
	}else {
		//Apagar keylogger
		this->btn_Iniciar->SetLabelText(wxT("Iniciar"));
	}
}