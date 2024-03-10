#ifndef __PANEL_FILE_MANAGER
#define __PANEL_FILE_MANAGER

#include "headers.hpp"

class panelFileManager: public wxPanel{
	private:
		wxToolBar* p_ToolBar = nullptr;
	public:
		panelFileManager(wxWindow* pParent);

};

#endif