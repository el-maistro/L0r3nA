#include "frame_remote_desktop.hpp"

wxBEGIN_EVENT_TABLE(frameRemoteDesktop, wxFrame)
	EVT_CLOSE(frameRemoteDesktop::Onclose)
wxEND_EVENT_TABLE()

frameRemoteDesktop::frameRemoteDesktop(wxWindow* pParent) :
	wxFrame(pParent, wxID_ANY, "Escritorio Remoto", wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE) {
	//Crear controles en la parte superior
	//  [Iniciar] [Detener]  [Guardar Captura]  [] CheckBox para controlar mouse y teclado
	//Crear picture box
}

void frameRemoteDesktop::Onclose(wxCloseEvent&) {
	Destroy();
}