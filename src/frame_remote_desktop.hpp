#ifndef __REMOTE_DESKTOP
#define __REMOTE_DESKTOP

#include "headers.hpp"
#include <wx/mstream.h>

struct MonitorInfo {
	int resWidth;
	int resHeight;
};

namespace EnumRemoteDesktop {
	enum Enum {
		ID_Main_Frame = 11000,
		ID_Bitmap,
		ID_BTN_Lista,
		ID_BTN_Single,
		ID_BTN_Start,
		ID_BTN_Stop,
		ID_BTN_Save,
		ID_CHK_Control,
		ID_CHK_Vmouse,
		ID_CMB_Qoptions,
		ID_CMB_Monitores
	};
}

class frameRemoteDesktop : public wxFrame {
	public:
		frameRemoteDesktop(wxWindow* pParent);

		wxStaticBitmap* imageCtrl		  = nullptr;
		wxComboBox* quality_options       = nullptr;
		wxComboBox* combo_lista_monitores = nullptr;
		wxCheckBox* chk_Control			  = nullptr;

		void AgregarMonitor(MonitorInfo& monitor) {
			vcMonitor.push_back(monitor);
		}

		void OnDrawBuffer(const char*& cBuffer, int iBuffersize);
	private:
		bool isRemoteControl = false;
		SOCKET sckCliente = INVALID_SOCKET;

		std::vector<MonitorInfo> vcMonitor;        //Aloja resoluciones de cada monitor

		void OnRemoteControl(wxCommandEvent&);   //Habilitar control remoto (toggle)
		void OnRemoteClick(wxMouseEvent&);		 //Enviar click remoto
		void OnCheckVmouse(wxCommandEvent&);     //Mostrar mouse remoto
		void OnComboChange(wxCommandEvent&);	 //Cambiar calidad de imagen
		void OnObtenerLista(wxCommandEvent&);    //Obtener lista de monitores
		void OnSingle(wxCommandEvent&);			 //Tomar captura sencilla
		void OnStart(wxCommandEvent&);			 //Iniciar live
		void OnStop(wxCommandEvent&);			 //Detener live
		void OnSave(wxCommandEvent&);			 //Guardar captura
		void Onclose(wxCloseEvent&);

		wxDECLARE_EVENT_TABLE();
};

#endif