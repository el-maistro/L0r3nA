#ifndef __PANEL_FILE_MANAGER
#define __PANEL_FILE_MANAGER

#include "headers.hpp"

class ListCtrlManager;

class panelFileManager: public wxPanel{
	public:
		ListCtrlManager* listManager = nullptr;
		
		void CrearLista();

		panelFileManager(wxWindow* pParent);

		//Eventos
		void OnToolBarClick(wxCommandEvent& event);
	private:
		wxToolBar* p_ToolBar = nullptr;

		wxDECLARE_EVENT_TABLE();

};

class ListCtrlManager : public wxListCtrl {
	public:
		ListCtrlManager(wxWindow* parent, const wxWindowID id, 
			            const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {}
	private:
		//wxDECLARE_EVENT_TABLE();
};

#endif