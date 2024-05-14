#ifndef __KEYLOG_
#define __KEYLOG_

#include "headers.hpp"

class panelKeylogger : public wxPanel {
	public:
		panelKeylogger(wxWindow* pParent);
		wxToggleButton* btn_Iniciar = nullptr;
	private:

		void OnToggle(wxCommandEvent& event); 

		wxDECLARE_EVENT_TABLE();
};

#endif