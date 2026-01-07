#include "panel_reverse_proxy.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelReverseProxy, wxFrame)
	EVT_TOGGLEBUTTON(EnumIDSProxy::ID_BTN_Toggle, panelReverseProxy::OnToggle)
wxEND_EVENT_TABLE()

panelReverseProxy::panelReverseProxy(wxWindow* pParent, SOCKET sck, std::string _strID, ByteArray c_key) :
   wxFrame(pParent, EnumIDSProxy::ID_Main_Window, "[" + _strID + "] Proxy Inversa", wxDefaultPosition, wxSize(350, 150)) {
	
	this->sckSocket = sck;
	this->enc_key = c_key;

	wxStaticText* label1 = new wxStaticText(this, wxID_ANY, "Puerto:");
	this->txtPort = new wxTextCtrl(this, EnumIDSProxy::ID_TXT_Port, "6666");
	this->btnToggle = new wxToggleButton(this, EnumIDSProxy::ID_BTN_Toggle, "Iniciar");

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(label1);
	main_sizer->Add(txtPort, 0, wxALIGN_CENTER);
	main_sizer->Add(btnToggle, 0, wxALIGN_CENTER);

	this->SetSizer(main_sizer);
	this->SetSizeHints(this->GetSize(), this->GetSize());
}

void panelReverseProxy::OnToggle(wxCommandEvent&) {
	wxString strPort = this->txtPort->GetValue();
	int puerto_escucha = atoi(strPort.c_str());

	if (this->btnToggle->GetValue()) {
		//Iniciar servidor local para proxy
		this->btnToggle->SetLabelText("Detener");
		//Iniciar handler para escuchar en X puerto
		
		p_Servidor->modRerverseProxy->InitHandler(puerto_escucha, this->sckSocket, this->enc_key);
	}else {
		//Detener servidor local para proxy
		this->btnToggle->SetLabelText("Iniciar");

		p_Servidor->modRerverseProxy->StopHandler(puerto_escucha);
	}
}