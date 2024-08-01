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
		ID_CHK_Control
	};
}

class frameRemoteDesktop : public wxFrame {
	public:
		frameRemoteDesktop(wxWindow* pParent);
		wxStaticBitmap* imageCtrl = nullptr;

		void OnDrawBuffer(const char* cBuffer, int iBuffersize);
	private:
		SOCKET sckCliente = INVALID_SOCKET;

		void OnSingle(wxCommandEvent&);
		void OnStart(wxCommandEvent&);
		void OnStop(wxCommandEvent&);
		void Onclose(wxCloseEvent&);

		wxDECLARE_EVENT_TABLE();
};

#endif