#include "frame_remote_desktop.hpp"
#include "frame_client.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(frameRemoteDesktop, wxFrame)
	EVT_CLOSE(frameRemoteDesktop::Onclose)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Single, frameRemoteDesktop::OnSingle)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Lista, frameRemoteDesktop::OnObtenerLista)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Start, frameRemoteDesktop::OnStart)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Stop, frameRemoteDesktop::OnStop)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Save, frameRemoteDesktop::OnSave)
	EVT_TEXT(EnumRemoteDesktop::ID_CMB_Qoptions, frameRemoteDesktop::OnComboChange)
	EVT_CHECKBOX(EnumRemoteDesktop::ID_CHK_Control, frameRemoteDesktop::OnRemoteControl)
	EVT_CHECKBOX(EnumRemoteDesktop::ID_CHK_Vmouse, frameRemoteDesktop::OnCheckVmouse)
wxEND_EVENT_TABLE()

frameRemoteDesktop::frameRemoteDesktop(wxWindow* pParent) :
	wxFrame(pParent, EnumRemoteDesktop::ID_Main_Frame, "Escritorio Remoto", wxDefaultPosition, wxSize(900, 500)) {
	
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

	// - - - - - - - - - CONTROLES PRINCIPALES  - - - - - - - - - 
	wxButton* btn_Lista = new wxButton(this, EnumRemoteDesktop::ID_BTN_Lista, "Obtener Lista");
	wxButton* btn_Single = new wxButton(this, EnumRemoteDesktop::ID_BTN_Single, "Tomar Captura");
	wxButton* btn_Iniciar = new wxButton(this, EnumRemoteDesktop::ID_BTN_Start, "Iniciar");
	wxButton* btn_Detener = new wxButton(this, EnumRemoteDesktop::ID_BTN_Stop, "Detener");
	wxButton* btn_Guardar = new wxButton(this, EnumRemoteDesktop::ID_BTN_Save, "Guardar Captura");
	
	wxArrayString qOptions;
	qOptions.push_back(wxString("KK")); //8  kk
	qOptions.push_back(wxString("Baja")); //16 Baja
	qOptions.push_back(wxString("Media")); //24 Media
	qOptions.push_back(wxString("Mejor")); //32 Mejor


	this->chk_Control = new wxCheckBox(this, EnumRemoteDesktop::ID_CHK_Control, "Control Remoto (mouse y teclado)");
	wxCheckBox* chk_Vmouse = new wxCheckBox(this, EnumRemoteDesktop::ID_CHK_Vmouse, "Mostar mouse remoto");
	this->quality_options = new wxComboBox(this, EnumRemoteDesktop::ID_CMB_Qoptions, "Seleccionar calidad de imagen", wxDefaultPosition, wxDefaultSize, qOptions, wxCB_READONLY);
	this->combo_lista_monitores = new wxComboBox(this, EnumRemoteDesktop::ID_CMB_Monitores, "Lista de monitores", wxDefaultPosition, wxSize(200, wxDefaultSize.GetHeight()), wxArrayString(0, ' '), wxCB_READONLY);

	wxBoxSizer* sizer_controles = new wxBoxSizer(wxHORIZONTAL);

	sizer_controles->Add(btn_Lista);
	sizer_controles->Add(this->combo_lista_monitores);
	sizer_controles->Add(btn_Single);
	sizer_controles->Add(btn_Iniciar);
	sizer_controles->Add(btn_Detener);
	sizer_controles->Add(btn_Guardar);
	sizer_controles->Add(new wxStaticText(this, wxID_ANY, "Calidad:"));
	sizer_controles->Add(this->quality_options);
	sizer_controles->Add(chk_Control);
	sizer_controles->Add(chk_Vmouse);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	this->imageCtrl = new wxStaticBitmap(this, EnumRemoteDesktop::ID_Bitmap, wxBitmap(10,10));
	this->imageCtrl->SetScaleMode(wxStaticBitmap::ScaleMode::Scale_Fill);

	this->imageCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(frameRemoteDesktop::OnRemoteClick), NULL, this);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(sizer_controles, 0, wxEXPAND | wxALL, 2);
	main_sizer->Add(this->imageCtrl, 1, wxEXPAND | wxALL, 2);

	this->SetSizer(main_sizer);
	this->Layout();
	
}

void frameRemoteDesktop::OnSingle(wxCommandEvent&) {
	//Si ya esta live porque pedir una captura?
	if (!this->isLive) {
		int monitor_index = this->combo_lista_monitores->GetSelection();

		if (monitor_index != wxNOT_FOUND) {
			std::string strPaquete = this->strQuality;
			strPaquete.append(1, '|');
			strPaquete += std::to_string(monitor_index);
			p_Servidor->cChunkSend(this->sckCliente, strPaquete.c_str(), strPaquete.size(), 0, true, EnumComandos::RD_Single);
		}
		else {
			wxMessageBox("Monitor no seleccionado", "Error");
		}
	}
}

void frameRemoteDesktop::OnObtenerLista(wxCommandEvent&) {
	p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::RD_Lista);
}

void frameRemoteDesktop::OnStart(wxCommandEvent&) {
	int monitor_index = this->combo_lista_monitores->GetSelection();
	if (monitor_index != wxNOT_FOUND) {
		std::string strPaquete = "32|";
		strPaquete += std::to_string(monitor_index);
		p_Servidor->cChunkSend(this->sckCliente, strPaquete.c_str(), strPaquete.size(), 0, true, EnumComandos::RD_Start);
		this->isLive = true;
	}else {
		wxMessageBox("Monitor no seleccionado", "Error");
	}
}

void frameRemoteDesktop::OnStop(wxCommandEvent&) {
	p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::RD_Stop);
	this->isLive = false;
}

void frameRemoteDesktop::OnSave(wxCommandEvent&) {
	wxFileDialog dialog(this, "Guardar frame", wxEmptyString, "captura.jpg", wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_OK) {
		if (!this->imageCtrl->GetBitmap().SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
			wxMessageBox("No se pudo guardar " + dialog.GetPath(), "Error", wxID_OK);
		}
	}
}

void frameRemoteDesktop::OnComboChange(wxCommandEvent& event) {
	std::string strValue = this->quality_options->GetValue().ToStdString();
	
	if (strValue == "KK") {
		this->strQuality = "8";
	}else if (strValue == "Baja") {
		this->strQuality = "16";
	}else if (strValue == "Media") {
		this->strQuality = "24";
	}else if (strValue == "Mejor") {
		this->strQuality = "32";
	}else {
		//Esto no deberia de pasar pero por cualquier cosa
		this->strQuality = "32";
	}
	//si esta live actualizar, de lo contrario solo variable local
	if (this->isLive) {
		p_Servidor->cChunkSend(this->sckCliente, this->strQuality.c_str(), this->strQuality.size(), 0, false, EnumComandos::RD_Update_Q);
	}
	event.Skip();
}

void frameRemoteDesktop::OnDrawBuffer(const char*& cBuffer, int iBuffersize) {
	if (iBuffersize > 0) {
		wxMemoryInputStream imgStream(cBuffer, iBuffersize);
		wxImage img(imgStream, wxBITMAP_TYPE_JPEG);
		
		//wxIMAGE_QUALITY_FAST   Fast but same as wxIMAGE_QUALITY_NEAREST 
		//wxIMAGE_QUALITY_HIGH   best quality
		int x = this->GetSize().GetWidth()- 30;
		int y = this->GetSize().GetHeight() - 90;
		if (img.IsOk()) {
			img.Rescale(x, y, this->quality_options->GetValue() == "KK" ? wxIMAGE_QUALITY_NORMAL : wxIMAGE_QUALITY_HIGH);
			wxBitmap bmp_Obj(img);
			
			if (this->imageCtrl) {
				this->imageCtrl->SetBitmap(bmp_Obj);
				this->Refresh();
				this->Update();
			}
		}
	}
}

void frameRemoteDesktop::OnCheckVmouse(wxCommandEvent& event) {
	bool isChecked = event.IsChecked();
	p_Servidor->cChunkSend(this->sckCliente, (isChecked ? "1" : "0"), 1, 0, false, EnumComandos::RD_Update_Vmouse);
	event.Skip();
}

void frameRemoteDesktop::Onclose(wxCloseEvent&) {
	p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::RD_Stop);
	std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	Destroy();
}

void frameRemoteDesktop::OnRemoteControl(wxCommandEvent&) {
	this->isRemoteControl = this->chk_Control->IsChecked();
}

void frameRemoteDesktop::OnRemoteClick(wxMouseEvent& event) {
	//event.GetEventType();
	if (this->isRemoteControl && this->isLive) {
		int monitor_index = this->combo_lista_monitores->GetSelection();

		if (monitor_index != wxNOT_FOUND && monitor_index < this->vcMonitor.size()) {
			int localx = event.GetX();
			int localy = event.GetY();

			int localres_Width = this->imageCtrl->GetSize().GetWidth();
			int localres_Height = this->imageCtrl->GetSize().GetHeight();

			MonitorInfo monitor = this->GetCopy(monitor_index);
			int remote_res_Width = monitor.resWidth;
			int remote_res_Height = monitor.resHeight;

			int remote_x = localx * remote_res_Width / localres_Width;
			int remote_y = localy * remote_res_Height / localres_Height;

			//Enviar click
			std::string strComando = std::to_string(remote_x);
			strComando.append(1, CMD_DEL);
			strComando += std::to_string(remote_y);
			strComando.append(1, CMD_DEL);
			strComando += std::to_string(monitor_index);

			p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, true, EnumComandos::RD_Send_Click);

		}else {
			wxMessageBox("No hay un monitor seleccionado. whut?", "Remote_click");
		}
	}
	event.Skip();
}

void frameRemoteDesktop::LimpiarVector() {
	std::unique_lock<std::mutex> lock(this->mtx_vector);
	this->vcMonitor.clear();
}

void frameRemoteDesktop::AgregarMonitor(MonitorInfo& monitor) {
	std::unique_lock<std::mutex> lock(this->mtx_vector);
	this->vcMonitor.push_back(monitor);
}

MonitorInfo frameRemoteDesktop::GetCopy(int index) {
	std::unique_lock<std::mutex> lock(this->mtx_vector);
	return this->vcMonitor[index];
}
