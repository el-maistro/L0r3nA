#include "panel_camara.hpp"
#include "frame_client.hpp"
#include "misc.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelCamara, wxPanel)
	EVT_BUTTON(EnumCamMenu::ID_Refrescar_Lista, panelCamara::OnRefrescarLista)
	EVT_BUTTON(EnumCamMenu::ID_Spawn_Frame, panelCamara::OnManageCam)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(panelPictureBox, wxPanel)
	EVT_CLOSE(panelPictureBox::OnClose)
	EVT_MENU(EnumCamMenu::ID_SingleShot, panelPictureBox::OnSingleShot)
	EVT_MENU(EnumCamMenu::ID_Iniciar_Live, panelPictureBox::OnLive)
	EVT_MENU(EnumCamMenu::ID_Detener_Live, panelPictureBox::OnStopLive)
	EVT_MENU(EnumCamMenu::ID_Guardar_Frame, panelPictureBox::OnGuardarFrame)
wxEND_EVENT_TABLE()

panelPictureBox::panelPictureBox(wxWindow* pParent, wxString strTitle, int iCamIndex ) :
	wxFrame(pParent, EnumCamMenu::ID_Picture_Frame, strTitle + " - Index " + std::to_string(iCamIndex), wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE, "CAM" + std::to_string(iCamIndex)) {
	
	this->imageCtrl = new wxStaticBitmap(this, wxID_ANY, wxBitmap(600, 300));

	wxMenuBar* menuBar = new wxMenuBar();
	
	wxMenu* camMenu = new wxMenu();

	camMenu->Append(EnumCamMenu::ID_SingleShot, "Captura unica");
	camMenu->AppendSeparator();
	camMenu->Append(EnumCamMenu::ID_Iniciar_Live, "Iniciar Live");
	camMenu->Append(EnumCamMenu::ID_Detener_Live, "Detener Live");
	camMenu->AppendSeparator();
	camMenu->Append(EnumCamMenu::ID_Guardar_Frame, "Guardar Captura");

	camMenu->Enable(EnumCamMenu::ID_Detener_Live, false);

	menuBar->Append(camMenu, "Camara");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->imageCtrl, 1, wxEXPAND | wxALL, 5);
	this->SetSizer(sizer);
	this->Layout();

	this->SetMenuBar(menuBar);
}

void panelPictureBox::OnDrawBuffer(const char* cBuffer, int iBufferSize) {
	if (iBufferSize > 0) {
		wxMemoryInputStream imgStream(cBuffer, iBufferSize);
		wxImage::AddHandler(new wxJPEGHandler);

		wxImage img(imgStream, wxBITMAP_TYPE_JPEG);

		if (img.IsOk()) {
			wxBitmap bmp_Obj(img);
			
			if (this->imageCtrl) {
				this->imageCtrl->SetBitmap(bmp_Obj);
				this->GetSizer()->Layout();
				this->GetSizer()->Fit(this);
				this->Refresh(false);
				this->Update();
			} else {
				DEBUG_MSG("imageCtrl no esta inicializado");
			}
		}
	}
}

void panelPictureBox::OnClose(wxCloseEvent& event) {
	Destroy();
}

void panelPictureBox::OnSingleShot(wxCommandEvent& event) {
	//Enviar comando para obtener captura sencilla
	std::vector<std::string> vcTmp = strSplit(this->GetTitle().ToStdString(), ' ', 10);
	std::string strComando = vcTmp[vcTmp.size() - 1];

	p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false, EnumComandos::CM_Single);
}

void panelPictureBox::OnStopLive(wxCommandEvent& event) {
	wxMenuBar* menuBar = this->GetMenuBar();
	if (menuBar) {
		menuBar->Enable(EnumCamMenu::ID_Detener_Live, false);
		menuBar->Enable(EnumCamMenu::ID_Iniciar_Live, true);
	}
	//Enviar comando para iniciar live
	std::vector<std::string> vcTmp = strSplit(this->GetTitle().ToStdString(), ' ', 10);
	std::string strComando = vcTmp[vcTmp.size() - 1];

	p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false, EnumComandos::CM_Live_Stop);
}

void panelPictureBox::OnLive(wxCommandEvent& event) {
	wxMenuBar* menuBar = this->GetMenuBar();
	if (menuBar) {
		menuBar->Enable(EnumCamMenu::ID_Detener_Live, true);
		menuBar->Enable(EnumCamMenu::ID_Iniciar_Live, false);
	}
	//Enviar comando para iniciar live
	std::vector<std::string> vcTmp = strSplit(this->GetTitle().ToStdString(), ' ', 10);
	std::string strComando = vcTmp[vcTmp.size() - 1];

	p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false, EnumComandos::CM_Live_Start);
}

void panelPictureBox::OnGuardarFrame(wxCommandEvent& event) {
	wxFileDialog dialog(this, "Guardar frame", wxEmptyString, "captura.jpg", wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_OK) {
		if (!this->imageCtrl->GetBitmap().SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
			wxMessageBox("No se pudo guardar " + dialog.GetPath(), "Error", wxID_OK);
		}
	}
}

panelCamara::panelCamara(wxWindow* pParent):
	wxPanel(pParent, wxID_ANY){

	wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
	if (wxTree) {
		wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
		if (panel_cliente) {
			FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
			if (frame_cliente) {
				this->sckCliente = frame_cliente->sckCliente;
				this->strID = frame_cliente->strClienteID;
			}
		}
	}

	this->cam_Devices = new wxComboBox(this, EnumCamMenu::ID_Combo_Devices, "...", wxDefaultPosition, wxSize(200, 20));
	wxButton* btn_Listar = new wxButton(this, EnumCamMenu::ID_Refrescar_Lista, "Refrescar lista");
	wxButton* btn_ManageCam = new wxButton(this, EnumCamMenu::ID_Spawn_Frame, "Administrar");
	
	wxBoxSizer* nSizer = new wxBoxSizer(wxHORIZONTAL);
	nSizer->Add(this->cam_Devices, 0, wxALL, 1);
	nSizer->AddSpacer(20);
	nSizer->Add(btn_Listar, 0, wxALL, 1);
	nSizer->Add(btn_ManageCam, 0, wxALL, 1);
	
	this->SetSizerAndFit(nSizer);
}

void panelCamara::OnRefrescarLista(wxCommandEvent& event) {
	//Enviar comando para obtener lista
	p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::CM_Lista);
}

void panelCamara::OnManageCam(wxCommandEvent& event) {
	//Abrir nueva frame para administrar la camara seleccionada en el combo box
	if (this->cam_Devices->GetStringSelection() != "") {
		this->pictureBox = new panelPictureBox(this, this->cam_Devices->GetStringSelection(), this->cam_Devices->GetSelection());
		this->pictureBox->sckCliente = this->sckCliente;
		this->pictureBox->Show(true);
	}

}
