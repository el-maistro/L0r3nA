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

frameRemoteDesktop::frameRemoteDesktop(wxWindow* pParent, SOCKET sck, std::string strID) :
	wxFrame(pParent, EnumRemoteDesktop::ID_Main_Frame, "Escritorio Remoto", wxDefaultPosition, wxSize(900, 500)) {
	
	this->sckCliente = sck;
	this->SetTitle("[" + strID.substr(0, strID.find('/', 0)) + "] Escritorio Remoto");

	this->InitGDI();
	
	// - - - - - - - - - CONTROLES PRINCIPALES  - - - - - - - - - 
	wxButton* btn_Lista = new wxButton(this, EnumRemoteDesktop::ID_BTN_Lista, "Obtener Lista");
	this->combo_lista_monitores = new wxComboBox(this, EnumRemoteDesktop::ID_CMB_Monitores, "Lista de monitores", wxDefaultPosition, wxSize(200, wxDefaultSize.GetHeight()), wxArrayString(0, ' '), wxCB_READONLY);

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(btn_Lista, 0);
	topSizer->Add(this->combo_lista_monitores, 1, wxALL | wxEXPAND);


	wxButton* btn_Single = new wxButton(this, EnumRemoteDesktop::ID_BTN_Single, "Tomar Captura");
	wxButton* btn_Iniciar = new wxButton(this, EnumRemoteDesktop::ID_BTN_Start, "Iniciar");
	wxButton* btn_Detener = new wxButton(this, EnumRemoteDesktop::ID_BTN_Stop, "Detener");
	wxButton* btn_Guardar = new wxButton(this, EnumRemoteDesktop::ID_BTN_Save, "Guardar Captura");
	
	wxBoxSizer* middleSizer = new wxBoxSizer(wxHORIZONTAL);
	middleSizer->Add(btn_Single, 1, wxALL | wxEXPAND);
	middleSizer->Add(btn_Iniciar, 1, wxALL | wxEXPAND);
	middleSizer->Add(btn_Detener, 1, wxALL | wxEXPAND);
	middleSizer->Add(btn_Guardar, 1, wxALL | wxEXPAND);


	wxArrayString qOptions;
	qOptions.push_back(wxString("KK"));    //8  kk
	qOptions.push_back(wxString("Baja"));  //16 Baja
	qOptions.push_back(wxString("Media")); //24 Media
	qOptions.push_back(wxString("Mejor")); //32 Mejor

	this->chk_Control = new wxCheckBox(this, EnumRemoteDesktop::ID_CHK_Control, "Control Remoto");
	wxCheckBox* chk_Vmouse = new wxCheckBox(this, EnumRemoteDesktop::ID_CHK_Vmouse, "Mostar mouse");
	this->quality_options = new wxComboBox(this, EnumRemoteDesktop::ID_CMB_Qoptions, "Calidad de imagen", wxDefaultPosition, wxDefaultSize, qOptions, wxCB_READONLY);

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxHORIZONTAL);
	bottomSizer->Add(this->chk_Control, 0);
	bottomSizer->Add(chk_Vmouse, 0);
	bottomSizer->Add(new wxStaticText(this, wxID_ANY, "Calidad:", wxDefaultPosition, wxDefaultSize), 0);
	bottomSizer->Add(this->quality_options, 0);

	this->pnl_main = new MyPanel(this);

	this->ConectarEventos();
		
	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(topSizer, 0, wxALIGN_CENTER_HORIZONTAL);
	main_sizer->Add(middleSizer, 0, wxALIGN_CENTER_HORIZONTAL);
	main_sizer->Add(this->pnl_main, 1, wxALL | wxEXPAND);
	main_sizer->Add(bottomSizer, 0, wxALIGN_CENTER_HORIZONTAL);

	this->SetSizer(main_sizer);
	this->Layout();

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
	
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
	time_t temp = time(0);
	struct tm* timeptr = localtime(&temp);

	std::string strLine = this->GetTitle();

	strLine += "[" + std::to_string(timeptr->tm_hour);
	strLine += "-" + std::to_string(timeptr->tm_min) + "-";
	strLine += std::to_string(timeptr->tm_sec) + "] captura.jpg";

	wxFileDialog dialog(this, "Guardar frame", wxEmptyString, strLine, wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_OK) {
		wxSize panelSize = this->pnl_main->GetSize();

		// Create a bitmap with the same size as the panel
		wxBitmap bitmap(panelSize.GetWidth(), panelSize.GetHeight());

		// Create a memory device context to draw on the bitmap
		wxMemoryDC memDC;
		memDC.SelectObject(bitmap);

		// Create a client device context to get the panel's content
		wxClientDC clientDC(this->pnl_main);

		// Blit (copy) the panel's content to the memory device context
		memDC.Blit(0, 0, panelSize.GetWidth(), panelSize.GetHeight(), &clientDC, 0, 0);

		// Deselect the bitmap from the memory device context
		memDC.SelectObject(wxNullBitmap);

		if (!bitmap.SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
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
	//wxIMAGE_QUALITY_FAST   Fast but same as wxIMAGE_QUALITY_NEAREST 
	//wxIMAGE_QUALITY_HIGH   best quality
	if (iBuffersize > 0) {
		wxLogNull noerrormessages;
		wxMemoryInputStream imgStream(cBuffer, iBuffersize);
		this->oldBitmap = std::make_shared<wxImage>(imgStream, wxBITMAP_TYPE_JPEG);
		this->DrawImage(this->oldBitmap);
	}
}

void frameRemoteDesktop::ProcesaPixelData(const char*& cBuffer, int iBuffersize) {
	if (iBuffersize % sizeof(Pixel_Data) != 0) {
		DEBUG_MSG("Error en transferencia de pixeles");
		return;
	}

	if (this->BitmapUpdate(this->DeserializePixels(cBuffer, iBuffersize))) {
		//wxImage nBitmap = this->GDIPlusBitmapToWxImage(this->oldBitmap);
		this->DrawImage(this->oldBitmap);

	}else {
		DEBUG_MSG("No se pudo actualizar el bitmap existente");
	}
}

void frameRemoteDesktop::ProcesarLista(const char*& pBuffer) {
	std::vector<std::string> vcMonitores = strSplit(std::string(pBuffer), '|', 20);
	if (vcMonitores.size() > 0) {
		this->LimpiarVector();
		for (const std::string& strMonitor : vcMonitores) {
			//nombre | width | height
			std::vector<std::string> vcInfo = strSplit(strMonitor, CMD_DEL, 3);
			if (vcInfo.size() == 3) {
				MonitorInfo new_monitor;
				new_monitor.resWidth = atoi(vcInfo[1].c_str());
				new_monitor.resHeight = atoi(vcInfo[2].c_str());
				
				this->AgregarMonitor(new_monitor);
				
				wxString strEntry = vcInfo[0];
				strEntry.append(1, ' ');
				strEntry += vcInfo[1];
				strEntry.append(1, 'x');
				strEntry += vcInfo[2];

				this->combo_lista_monitores->Clear();
				combo_lista_monitores->Append(strEntry);
			}
			else {
				DEBUG_MSG("No se pudo parsear la info del monitor " + strMonitor);
			}
		}
	}else {
		DEBUG_MSG("No se pudo parsear la info de los monitores");
		DEBUG_MSG(pBuffer);
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

void frameRemoteDesktop::OnRemoteMouse(wxMouseEvent& event) {
	wxEventType evento = event.GetEventType();
	this->pnl_main->SetFocus();
	if (evento == wxEVT_MOUSEWHEEL) {
		this->EnviarEventoMouse(evento, event.GetX(), event.GetY(), event.GetWheelRotation() < 0 ? true : false);
	}else {
		this->EnviarEventoMouse(evento, event.GetX(), event.GetY());
	}
	event.Skip();
}

void frameRemoteDesktop::OnRemoteKey(wxKeyEvent& event) {
	this->EnviarEventoTeclado(event.GetEventType(), event.GetRawKeyCode());
	event.Skip();
}

void frameRemoteDesktop::EnviarEventoMouse(wxEventType evento, int x, int y, bool isDown) {
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

			int localres_Width = this->pnl_main->GetSize().GetWidth();
			int localres_Height = this->pnl_main->GetSize().GetHeight();

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

void frameRemoteDesktop::EnviarEventoTeclado(wxEventType evento, u_int key) {
	if (this->isRemoteControl && this->isLive) {
		bool isDown = false;
		if (evento == wxEVT_KEY_DOWN) {
			isDown = true;
		}
		std::string strComando = std::to_string(key);
		strComando.append(1, CMD_DEL);
		strComando.append(1, isDown ? '0' : '1');
		
		p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, true, EnumComandos::RD_Send_Teclado);
	}
}

void frameRemoteDesktop::ConectarEventos() {
	//Mouse
	//Click izquierdo 
	this->pnl_main->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	this->pnl_main->Connect(wxEVT_LEFT_UP, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	//Click derecho
	this->pnl_main->Connect(wxEVT_RIGHT_DOWN, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	this->pnl_main->Connect(wxEVT_RIGHT_UP, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	//Click boton central
	this->pnl_main->Connect(wxEVT_MIDDLE_DOWN, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	this->pnl_main->Connect(wxEVT_MIDDLE_UP, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	//Double click
	this->pnl_main->Connect(wxEVT_LEFT_DCLICK, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	this->pnl_main->Connect(wxEVT_RIGHT_DCLICK, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	this->pnl_main->Connect(wxEVT_MIDDLE_DCLICK, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);
	//wheel
	this->pnl_main->Connect(wxEVT_MOUSEWHEEL, wxMouseEventHandler(frameRemoteDesktop::OnRemoteMouse), NULL, this);

	//Teclado 
	this->pnl_main->Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(frameRemoteDesktop::OnRemoteKey), NULL, this);
	this->pnl_main->Connect(wxEVT_KEY_UP, wxKeyEventHandler(frameRemoteDesktop::OnRemoteKey), NULL, this);

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

void frameRemoteDesktop::DrawImage(std::shared_ptr<wxImage>& _img) {
	int x = this->GetSize().GetWidth() - 30;
	int y = this->GetSize().GetHeight() - 90;
	if (_img.get()->IsOk() && _img.get()->GetType() != wxBitmapType::wxBITMAP_TYPE_INVALID) {
		wxImage tmp_img(*_img.get());
		if (tmp_img.IsOk()) {
			tmp_img.Rescale(x, y, this->quality_options->GetValue() == "KK" ? wxIMAGE_QUALITY_NORMAL : wxIMAGE_QUALITY_HIGH);
			wxBitmap bmp_Obj(tmp_img);

			if (bmp_Obj.IsOk()) {
				this->pnl_main->SetBitmap(bmp_Obj);
				/*try {
					if (this->imageCtrl) {
						this->imageCtrl->SetBitmap(bmp_Obj);
						this->imageCtrl->Refresh();
					}
				}
				catch (const std::exception& e) {
					DEBUG_MSG("exception:");
					DEBUG_MSG(e.what());
				}
				catch (...) {
					throw;
				}*/
			}else {
				DEBUG_MSG("La imagen es invalida");
			}
		}
	}else {
		DEBUG_MSG("Bitmap invalido");
	}
}

std::vector<Pixel_Data> frameRemoteDesktop::DeserializePixels(const char*& cBuffer, int iBufferSize) {
	std::vector<Pixel_Data> vcOut;
	for(int iPos = 0; iPos < iBufferSize; iPos+=sizeof(Pixel_Data)){
		Pixel_Data nPixel;
		memcpy(&nPixel, cBuffer + iPos, sizeof(Pixel_Data));
		if (nPixel.data.R <= 255 && nPixel.data.G <= 255 && nPixel.data.B <= 255) {
			vcOut.push_back(nPixel);
		}
	}
	return vcOut;
}

bool frameRemoteDesktop::BitmapUpdate(std::vector<Pixel_Data>& vcin) {
	if (this->oldBitmap) {
		std::cout << "PIXELS: " << vcin.size() << "\n";
		//DEBUG_MSG(vcin.size());
		UINT width = this->oldBitmap->GetWidth();
		UINT height = this->oldBitmap->GetHeight();
		if (width == 0 || height == 0) {
			DEBUG_MSG("dimensiones invalidas");
			return false;
		}

		for (Pixel_Data& pixel : vcin) {
			this->oldBitmap->SetRGB(pixel.x, pixel.y, pixel.data.R, pixel.data.G, pixel.data.B);
		}

		return true;
	}else {
		DEBUG_MSG("Bitmap viejo es invalido. Detener y volver a iniciar");
		return false;
	}
}

wxImage frameRemoteDesktop::GDIPlusBitmapToWxImage(Gdiplus::Bitmap*& gdiBitmap) {
	if (!gdiBitmap) {
		DEBUG_MSG("El objeto Gdibitmap es invalido");
		return wxImage();
	}

	int width = gdiBitmap->GetWidth();
	int height = gdiBitmap->GetHeight();

	// Acceder a los datos del bitmap de GDI+
	Gdiplus::BitmapData bitmapData;
	Gdiplus::Rect rect(0, 0, width, height);
	if (gdiBitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat24bppRGB, &bitmapData) == Gdiplus::Status::Ok) {

		wxImage wxImg(width, height);
		wxImg.SetType(wxBitmapType::wxBITMAP_TYPE_JPEG);

		// Copiar los datos de GDI+ a wxImage
		BYTE* srcPixels = static_cast<BYTE*>(bitmapData.Scan0);
		BYTE* dstPixels = wxImg.GetData(); // wxImage solo maneja RGB (no alpha)

		for (int y = 0; y < height; ++y) {
			for (int x = 0; x < width; ++x) {
				int srcIndex = y * bitmapData.Stride + x * 3;
				dstPixels[srcIndex    ] = srcPixels[srcIndex    ]; // R
				dstPixels[srcIndex + 1] = srcPixels[srcIndex + 1]; // G
				dstPixels[srcIndex + 2] = srcPixels[srcIndex + 2]; // B
			}
		}

		// Desbloquear los bits de GDI+
		gdiBitmap->UnlockBits(&bitmapData);
		return wxImg;
	}

	
	// Convertir wxImage a wxBitmap
	return wxImage();
}

void frameRemoteDesktop::InitGDI() {
	Gdiplus::GdiplusStartup(&this->gdiplusToken, &this->gdiplusStartupInput, NULL);
}

void frameRemoteDesktop::StopGDI() {
	Gdiplus::GdiplusShutdown(this->gdiplusToken);
}