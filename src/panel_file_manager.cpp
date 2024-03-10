#include "panel_file_manager.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

panelFileManager::panelFileManager(wxWindow* pParent) :
	wxPanel(pParent, EnumIDS::ID_Panel_FM, wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE) {
	this->SetBackgroundColour(wxColor(200, 200, 200));
	
	//Crear barra lateral dentro del panel con accesos rapidos como
	//dispositivos conectados, desktop, downloads, etc...
	
}