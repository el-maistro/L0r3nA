#include "panel_file_manager.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

panelFileManager::panelFileManager(wxWindow* pParent) :
	wxPanel(pParent, EnumIDS::ID_Panel_FM) {
	this->SetBackgroundColour(wxColor(200, 200, 200));
	
	//Crear barra lateral dentro del panel con accesos rapidos como
	//dispositivos conectados, desktop, downloads, etc...
	this->p_ToolBar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_VERTICAL | wxTB_LEFT);
	wxImage::AddHandler(new wxPNGHandler);

	wxBitmap openBitmap(wxT("C:\\copy.png"), wxBITMAP_TYPE_PNG);
	wxBitmap desktopBitmap(wxT("C:\\desktop.png"), wxBITMAP_TYPE_PNG);
	wxBitmap downloadBitmap(wxT("C:\\download.png"), wxBITMAP_TYPE_PNG);


	this->p_ToolBar->AddTool(wxID_OPEN, wxT("Open"), openBitmap, "Test");
	this->p_ToolBar->AddSeparator(); // Separador entre grupos de botones
	this->p_ToolBar->AddTool(wxID_SAVE, wxT("Escritorio"), desktopBitmap, "Escritorio");
	this->p_ToolBar->AddSeparator(); // Separador entre grupos de botones
	this->p_ToolBar->AddTool(wxID_EXIT, wxT("Descargas"), downloadBitmap, "Descargas");
	this->p_ToolBar->Realize();

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(this->p_ToolBar, 0, wxEXPAND);
	sizer->Add(new wxStaticText(this, wxID_ANY, "Testing"), 0, wxEXPAND);
	SetSizer(sizer);

}