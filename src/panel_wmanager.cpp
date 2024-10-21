#include "panel_wmanager.hpp"

panelWManager::panelWManager(wxWindow* pParent, SOCKET sckCliente)
	: wxPanel(pParent, EnumIDS::ID_Panel_WM){
	this->sckCliente = sckCliente;

}