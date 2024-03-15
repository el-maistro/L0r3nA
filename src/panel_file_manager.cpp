#include "panel_file_manager.hpp"
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
	
	//Crear barra lateral dentro del panel con accesos rapidos como
	//dispositivos conectados, desktop, downloads, etc...
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
	switch (event.GetId()) {
		case EnumIDS::ID_Panel_FM_Equipo:
			this->listManager->ClearAll();
			
			itemCol.SetText("-");
			itemCol.SetWidth(20);
			itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
			this->listManager->InsertColumn(0, itemCol);

			itemCol.SetText("Capacidad");
			itemCol.SetAlign(wxLIST_FORMAT_LEFT);
			itemCol.SetWidth(100);
			this->listManager->InsertColumn(1, itemCol);

			itemCol.SetText("Libre");
			itemCol.SetWidth(50);
			this->listManager->InsertColumn(2, itemCol);
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
			break;
	}
}

void panelFileManager::CrearLista() {
	this->listManager = new ListCtrlManager(this, wxID_ANY, wxDefaultPosition, wxSize(600, 300), wxBORDER_THEME | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);
	
	/*p_Servidor->m_listCtrl = new MyListCtrl(this->m_RPanel, wxID_ANY, wxDefaultPosition, wxSize(600, 300), flags | wxBORDER_THEME);
   
    wxListItem itemCol;
    itemCol.SetText("ID");
    itemCol.SetWidth(100);
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    p_Servidor->m_listCtrl->InsertColumn(0, itemCol);

    itemCol.SetText("USUARIO");
    itemCol.SetWidth(160);
    p_Servidor->m_listCtrl->InsertColumn(1, itemCol);

    itemCol.SetText("IP");
    itemCol.SetWidth(120);
    p_Servidor->m_listCtrl->InsertColumn(2, itemCol);

    itemCol.SetText("SO");
    itemCol.SetWidth(140);
    p_Servidor->m_listCtrl->InsertColumn(3, itemCol);

    itemCol.SetText("CPU");
    itemCol.SetWidth(200);
    p_Servidor->m_listCtrl->InsertColumn(4, itemCol);*/
}