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

namespace EnumRemoteMouse {
	enum Enum {
		_LEFT_DOWN = 1,
		_LEFT_UP,
		_RIGHT_DOWN,
		_RIGHT_UP,
		_MIDDLE_DOWN,
		_MIDDLE_UP,
		_DOUBLE_LEFT,
		_DOUBLE_RIGHT,
		_DOUBLE_MIDDLE,
		_WHEEL_DOWN,
		_WHEEL_UP
	};
}

class frameRemoteDesktop : public wxFrame {
	public:
		frameRemoteDesktop(wxWindow* pParent);

		wxStaticBitmap* imageCtrl		  = nullptr;
		wxComboBox* quality_options       = nullptr;
		wxComboBox* combo_lista_monitores = nullptr;
		wxCheckBox* chk_Control			  = nullptr;

		//Manipular el vector seguramente
		void LimpiarVector();
		void AgregarMonitor(MonitorInfo& monitor);
		MonitorInfo GetCopy(int index);

		void OnDrawBuffer(const char*& cBuffer, int iBuffersize);
	private:
		std::mutex mtx_vector;
		bool isRemoteControl   =     	  false;
		bool isLive            =          false;
		SOCKET sckCliente      = INVALID_SOCKET;
		std::string strQuality =           "32";             //Calidad por defecto de la imagen

		std::vector<MonitorInfo> vcMonitor;                  //Aloja resoluciones de cada monitor

		void OnRemoteControl(wxCommandEvent&);               //Habilitar control remoto (toggle)
		
		//Control de Mouse
		void OnRemoteMouse_Click_Left(wxMouseEvent&);		 //Click izquierdo
		void OnRemoteMouse_Click_Right(wxMouseEvent&);	     //Click derecho
		void OnRemoteMouse_Click_Middle(wxMouseEvent&);      //Click boton central
		void OnRemoteMouse_Click_Double(wxMouseEvent&);      //Doble click
		void OnRemoteMouse_Wheel(wxMouseEvent&);             //Scroll

		void OnCheckVmouse(wxCommandEvent&);                 //Mostrar mouse remoto
		void OnComboChange(wxCommandEvent&);	             //Cambiar calidad de imagen
		void OnObtenerLista(wxCommandEvent&);                //Obtener lista de monitores
		void OnSingle(wxCommandEvent&);			             //Tomar captura sencilla
		void OnStart(wxCommandEvent&);			             //Iniciar live
		void OnStop(wxCommandEvent&);			             //Detener live
		void OnSave(wxCommandEvent&);			             //Guardar captura
		void Onclose(wxCloseEvent&);

		void EnviarEvento(wxEventType evento, int x, int y, bool isDown = false); //Enviar evento de mouse
		void ConectarEventos();

		wxDECLARE_EVENT_TABLE();
};

#endif