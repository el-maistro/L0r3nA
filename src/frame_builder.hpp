#ifndef __FRAME_BUILDER
#define __FRAME_BUILDER

#include "headers.hpp"

namespace EnumBuilderIDS {
	enum Enum {
		BTN_Generar = 10
	};
}

class FrameBuilder : public wxFrame {
public:
	FrameBuilder(wxWindow*);
private:
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

	wxButton* btnGenerar = nullptr;

	void OnGenerarCliente(wxCommandEvent& event);
	
	wxDECLARE_EVENT_TABLE();
};

#endif
