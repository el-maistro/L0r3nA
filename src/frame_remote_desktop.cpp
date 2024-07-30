#include "frame_remote_desktop.hpp"
#include "frame_client.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(frameRemoteDesktop, wxFrame)
	EVT_CLOSE(frameRemoteDesktop::Onclose)
wxEND_EVENT_TABLE()

frameRemoteDesktop::frameRemoteDesktop(wxWindow* pParent) :
	wxFrame(pParent, wxID_ANY, "Escritorio Remoto", wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE) {
	//Crear controles en la parte superior
	//  [Iniciar] [Detener]  [Guardar Captura]  [] CheckBox para controlar mouse y teclado
	//  Resolution Fastest | Low | Medium | High
	//Crear picture box
	wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
	if (wxTree) {
		wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
		if (panel_cliente) {
			FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
			if (frame_cliente) {
				this->sckCliente = frame_cliente->sckCliente;
			}
		}
	}
	std::string strComando = std::to_string(EnumComandos::RD_Single);
	strComando.append(1, CMD_DEL);
	strComando += "24"; //quality

	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
}

void frameRemoteDesktop::Onclose(wxCloseEvent&) {
	Destroy();
}