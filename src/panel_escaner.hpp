#ifndef __PNL_ESCANER
#define __PNL_ESCANER 1

#include "headers.hpp"

namespace EnumEscanerIDS {
	enum Enum {
		Main_Window = 100,
		BTN_Scan
	};
}

class panelEscaner : public wxPanel {
	public:
		panelEscaner(wxWindow* pParent, SOCKET _sck);

		void AddData(const char* _buffer);
	private:
		wxTextCtrl* txtHostBase = nullptr;
		wxButton* btnScan = nullptr;
		wxListCtrl* list_ctrl = nullptr;

		void CrearListView();
		void OnScan(wxCommandEvent& event);

		SOCKET sckSocket = INVALID_SOCKET;

		wxDECLARE_EVENT_TABLE();

};

#endif
