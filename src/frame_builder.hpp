#ifndef __FRAME_BUILDER
#define __FRAME_BUILDER

#include "server.hpp"
#include "headers.hpp"
#include <wx/process.h>

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
	wxTimer m_timer;
private:
	MyProcess* nProcess = nullptr;
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

	void OnTimer(wxTimerEvent& event);

	wxDECLARE_EVENT_TABLE();
};

class MyProcess : public wxProcess{
	public:
		MyProcess(FrameBuilder* parent)
			: wxProcess(wxEXEC_ASYNC), m_parent(parent)
		{
			Redirect();
		}

		void OnTerminate(int pid, int status) override{
			wxMessageBox("Done omar");
			m_parent->m_timer.Stop();
		}

private:
	FrameBuilder* m_parent;
};



#endif
