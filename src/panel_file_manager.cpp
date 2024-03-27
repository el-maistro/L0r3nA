#include "panel_file_manager.hpp"
#include "frame_client.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(panelFileManager, wxPanel)
	EVT_MENU(wxID_ANY, panelFileManager::OnToolBarClick)
wxEND_EVENT_TABLE()


panelFileManager::panelFileManager(wxWindow* pParent) :
	wxPanel(pParent, EnumIDS::ID_Panel_FM) {
	this->SetBackgroundColour(wxColor(200, 200, 200));
	
	
	wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
	if (wxTree) {
		wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
		if (panel_cliente) {
			FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
			if (frame_cliente) {
				this->strID = frame_cliente->strClienteID;
			}
		}
	}

	this->p_ToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_LEFT);
	wxImage::AddHandler(new wxPNGHandler);

	wxBitmap pcBitmap(wxT(".\\imgs\\computer.png"), wxBITMAP_TYPE_PNG);
	wxBitmap desktopBitmap(wxT(".\\imgs\\desktop.png"), wxBITMAP_TYPE_PNG);
	wxBitmap downloadBitmap(wxT(".\\imgs\\download.png"), wxBITMAP_TYPE_PNG);


	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Equipo, wxT("Equipo"), pcBitmap, "Equipo");
	this->p_ToolBar->AddSeparator(); // Separador entre grupos de botones
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Escritorio, wxT("Escritorio"), desktopBitmap, "Escritorio");
	this->p_ToolBar->AddSeparator(); // Separador entre grupos de botones
	this->p_ToolBar->AddTool(EnumIDS::ID_Panel_FM_Descargas, wxT("Descargas"), downloadBitmap, "Descargas");
	this->p_ToolBar->Realize();

	this->CrearLista();

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(this->p_ToolBar, 0, wxEXPAND);
	sizer->Add(this->listManager, 1, wxEXPAND, 1);
	//sizer->Add(new wxStaticText(this, wxID_ANY, "Testing"), 0, wxEXPAND);
	SetSizer(sizer);

}

void panelFileManager::OnToolBarClick(wxCommandEvent& event) {
	wxListItem itemCol;
	std::string strComando = "";
	switch (event.GetId()) {
		case EnumIDS::ID_Panel_FM_Equipo:
			this->listManager->ClearAll();
			
			itemCol.SetText("-");
			itemCol.SetWidth(20);
			itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
			this->listManager->InsertColumn(0, itemCol);

			itemCol.SetText("Nombre");
			itemCol.SetAlign(wxLIST_FORMAT_LEFT);
			itemCol.SetWidth(150);
			this->listManager->InsertColumn(1, itemCol);

			itemCol.SetText("Tipo");
			itemCol.SetWidth(100);
			this->listManager->InsertColumn(2, itemCol);

			itemCol.SetText("Capacidad");
			itemCol.SetWidth(100);
			this->listManager->InsertColumn(3, itemCol);

			//ENVIAR COMANDO OBTENER DRIVES
			strComando = std::to_string(EnumComandos::FM_Discos);
			strComando.append(1, '~');
			this->EnviarComando(strComando);
			break;
		case EnumIDS::ID_Panel_FM_Descargas:
			this->listManager->ClearAll();
			itemCol.SetText("-");
			itemCol.SetWidth(20);
			itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
			this->listManager->InsertColumn(0, itemCol);

			itemCol.SetText("Nombre");
			itemCol.SetWidth(150);
			itemCol.SetAlign(wxLIST_FORMAT_LEFT);
			this->listManager->InsertColumn(1, itemCol);

			itemCol.SetText("Tamaño");
			itemCol.SetWidth(100);
			this->listManager->InsertColumn(2, itemCol);
			//ENVIAR COMANDO OBTENER FOLDER DESCARGAS
			break;
	}
}

void panelFileManager::CrearLista() {
	this->listManager = new ListCtrlManager(this, EnumIDS::ID_Panel_FM_List, wxDefaultPosition, wxSize(600, 300), wxBORDER_THEME | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);

}

void panelFileManager::EnviarComando(std::string pComando) {
	std::vector<struct Cliente> vc_copy;
	std::unique_lock<std::mutex> lock(vector_mutex);
	vc_copy = p_Servidor->vc_Clientes;
	lock.unlock();

	for (auto vcCli : vc_copy) {
		if (vcCli._id == this->strID) {
			int iEnviado = p_Servidor->cSend(vcCli._sckCliente, pComando.c_str(), pComando.size() + 1, 0, false);
			break;
		}
	}
}