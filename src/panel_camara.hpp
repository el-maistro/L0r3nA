#ifndef __CAM_
#define __CAM_

#include "headers.hpp"

namespace EnumCamMenu {
	enum Enum {
		ID_SingleShot = 10000,
		ID_StartLive,
		ID_Close,
		ID_Combo_Devices,
		ID_Refrescar_Lista,
		ID_Spawn_Frame
	};
}

class panelPictureBox : public wxFrame {
	public:
		panelPictureBox(wxWindow* parent, wxString cTitle, int iCamIndex);
		~panelPictureBox();

		char* cPictureBuffer = nullptr;
		bool isCalled = false;
		u_int uiWidth = 0;
		u_int uiHeight = 0;
		SOCKET sckCliente = INVALID_SOCKET;

		void OnPaint(wxPaintEvent& event);
		
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
		wxDECLARE_EVENT_TABLE();

};

#endif