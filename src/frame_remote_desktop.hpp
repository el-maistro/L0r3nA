#ifndef __REMOTE_DESKTOP
#define __REMOTE_DESKTOP

#include "headers.hpp"
#include <wx/mstream.h>

namespace EnumRemoteDesktop {
	enum Enum {
		ID_Main_Frame = 11000,
		ID_Bitmap,
		ID_BTN_Single,
		ID_BTN_Start,
		ID_BTN_Stop,
		ID_BTN_Save,
		ID_CHK_Control,
		ID_CHK_Vmouse,
		ID_CMB_Qoptions
	};
}

class frameRemoteDesktop : public wxFrame {
	public:
		frameRemoteDesktop(wxWindow* pParent);
		wxStaticBitmap* imageCtrl = nullptr;
		wxComboBox* quality_options = nullptr;

		void OnDrawBuffer(const char*& cBuffer, int iBuffersize);
	private:
		SOCKET sckCliente = INVALID_SOCKET;

		void OnCheckVmouse(wxCommandEvent&); //Mostrar mouse remoto
		void OnComboChange(wxCommandEvent&); //Cambiar calidad de imagen
		void OnSingle(wxCommandEvent&);      //Tomar captura sencilla
		void OnStart(wxCommandEvent&);       //Iniciar live
		void OnStop(wxCommandEvent&);        //Detener live
		void OnSave(wxCommandEvent&);        //Guardar captura
		void Onclose(wxCloseEvent&);

		wxDECLARE_EVENT_TABLE();
};

#endif