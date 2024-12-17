#include "panel_reverse_proxy.hpp"

panelReverseProxy::panelReverseProxy(wxWindow* pParent, SOCKET sck) :
   wxPanel(pParent, wxID_ANY){
	this->sckSocket = sck;

	wxStaticText* label1 = new wxStaticText(this, wxID_ANY, "REVERSE_PROXY");
}
