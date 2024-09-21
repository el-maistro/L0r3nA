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
	qOptions.push_back(wxString("KK"));    //8  kk
	qOptions.push_back(wxString("Baja"));  //16 Baja
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

	this->ConectarEventos();
		
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

void frameRemoteDesktop::OnRemoteMouse_Click_Left(wxMouseEvent& event) {
	wxEventType evento = event.GetEventType();
	this->EnviarEvento(evento, event.GetX(), event.GetY());
	
	/*if (evento == wxEVT_LEFT_DOWN) {
		std::cout << "LEFT_DOWN\n";
	}else if (evento == wxEVT_LEFT_UP) {
		std::cout << "LEFT_UP\n";
	}
	*/
	event.Skip();
}

void frameRemoteDesktop::OnRemoteMouse_Click_Right(wxMouseEvent& event) {
	wxEventType evento = event.GetEventType();
	this->EnviarEvento(evento, event.GetX(), event.GetY());
	/*if (evento == wxEVT_RIGHT_DOWN) {
		std::cout << "RIGHT_DOWN\n";
	}
	else if (evento == wxEVT_RIGHT_UP) {
		std::cout << "RIGHT_UP\n";
	}*/
	event.Skip();
}

void frameRemoteDesktop::OnRemoteMouse_Click_Middle(wxMouseEvent& event) {
	wxEventType evento = event.GetEventType();
	this->EnviarEvento(evento, event.GetX(), event.GetY());
	/*if (evento == wxEVT_MIDDLE_DOWN) {
		std::cout << "MIDDLE_DOWN\n";
	}
	else if (evento == wxEVT_MIDDLE_UP) {
		std::cout << "MIDDLE_UP\n";
	}*/
	event.Skip();
}

void frameRemoteDesktop::OnRemoteMouse_Click_Double(wxMouseEvent& event) {
	wxEventType evento = event.GetEventType();
	this->EnviarEvento(evento, event.GetX(), event.GetY());
	/*if (evento == wxEVT_LEFT_DCLICK) {
		std::cout << "LEFT_DOUBLE\n";
	}else if (evento == wxEVT_RIGHT_DCLICK) {
		std::cout << "RIGHT_DOUBLE\n";
	}else if (evento == wxEVT_MIDDLE_DCLICK) {
		std::cout << "MIDDLE_DOUBLE\n";
	}*/

	event.Skip();
}

void frameRemoteDesktop::OnRemoteMouse_Wheel(wxMouseEvent& event) {
	this->EnviarEvento(event.GetEventType(), event.GetX(), event.GetY(), event.GetWheelRotation() < 0 ? true : false);

	//int position = event.GetWheelRotation();
	//std::cout << "WHEEL " << position << "\n";
	// < 0  scroll hacia abajo
	// > 0  scroll hacia arriba
}

void frameRemoteDesktop::EnviarEvento(wxEventType evento, int x, int y, bool isDown = false) {
	if (this->isRemoteControl && this->isLive) {
		int monitor_index = this->combo_lista_monitores->GetSelection();
		
		int mouse_action = 0;
		if (evento == wxEVT_LEFT_DOWN) {
			mouse_action = EnumRemoteMouse::_LEFT_DOWN;
		}else if (evento == wxEVT_LEFT_UP) {
			mouse_action = EnumRemoteMouse::_LEFT_UP;
		}else if (evento == wxEVT_RIGHT_DOWN) {
			mouse_action = EnumRemoteMouse::_RIGHT_DOWN;
		}else if (evento == wxEVT_RIGHT_UP) {
			mouse_action = EnumRemoteMouse::_RIGHT_UP;
		}else if (evento == wxEVT_MIDDLE_DOWN) {
			mouse_action = EnumRemoteMouse::_MIDDLE_DOWN;
		}else if (evento == wxEVT_RIGHT_UP) {
			mouse_action = EnumRemoteMouse::_MIDDLE_UP;
		}else if (evento == wxEVT_LEFT_DCLICK) {
			mouse_action = EnumRemoteMouse::_DOUBLE_LEFT;
		}else if (evento == wxEVT_RIGHT_DCLICK) {
			mouse_action = EnumRemoteMouse::_DOUBLE_RIGHT;
		}else if (evento == wxEVT_MIDDLE_DCLICK) {
			mouse_action = EnumRemoteMouse::_DOUBLE_MIDDLE;
		}else if (evento == wxEVT_MOUSEWHEEL) {
			mouse_action = (isDown ? EnumRemoteMouse::_WHEEL_DOWN : EnumRemoteMouse::_WHEEL_UP);
		}

		if (monitor_index != wxNOT_FOUND && monitor_index < this->vcMonitor.size()) {
			int localx = x;
			int localy = y;

			int localres_Width = this->imageCtrl->GetSize().GetWidth();
			int localres_Height = this->imageCtrl->GetSize().GetHeight();

			MonitorInfo monitor = this->GetCopy(monitor_index);
			int remote_res_Width = monitor.resWidth;
			int remote_res_Height = monitor.resHeight;

			int remote_x = localx * remote_res_Width / localres_Width;
			int remote_y = localy * remote_res_Height / localres_Height;

			//Armar paquete de evento
			//X | Y | monitor | action
			std::string strComando = std::to_string(remote_x);
			strComando.append(1, CMD_DEL);
			strComando += std::to_string(remote_y);
			strComando.append(1, CMD_DEL);
			strComando += std::to_string(monitor_index);
			strComando.append(1, CMD_DEL);
			strComando += std::to_string(mouse_action);
			
			p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, true, EnumComandos::RD_Send_Click);

		}else {
			//Esto no deberia de pasar
			wxMessageBox("No hay un monitor seleccionado. whut?", "Remote_click");
		}
	}
}

void frameRemoteDesktop::ConectarEventos() {
	//Click izquierdo 
	this->imageCtrl->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Left), NULL, this);
	this->imageCtrl->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Left), NULL, this);

	//Click derecho
	this->imageCtrl->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Right), NULL, this);
	this->imageCtrl->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Right), NULL, this);

	//Click boton central
	this->imageCtrl->Connect(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Middle), NULL, this);
	this->imageCtrl->Connect(wxEVT_MIDDLE_UP, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Middle), NULL, this);

	//Double click
	this->imageCtrl->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Double), NULL, this);
	this->imageCtrl->Connect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Double), NULL, this);
	this->imageCtrl->Connect(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Click_Double), NULL, this);
	
	//wheel
	this->imageCtrl->Connect(wxEVT_MOUSEWHEEL, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse_Wheel), NULL, this);
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
