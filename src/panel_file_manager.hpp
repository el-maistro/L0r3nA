#ifndef __PANEL_FILE_MANAGER
#define __PANEL_FILE_MANAGER

#include "headers.hpp"

class ListCtrlManager;

class panelFileManager: public wxPanel{
	public:
		ListCtrlManager* listManager = nullptr;
		
		void CrearLista();
		wxString RutaActual();
		void EnviarComando(std::string pComando);

		panelFileManager(wxWindow* pParent);

		//Eventos
		void OnToolBarClick(wxCommandEvent& event);
		
		//Variables
		int iMODE = -1;
		wxStaticText* p_RutaActual = nullptr;
		std::vector<wxString> c_RutaActual;

	private:
		wxToolBar* p_ToolBar = nullptr;
		std::string strID = "";

		wxDECLARE_EVENT_TABLE();

};

class ListCtrlManager : public wxListCtrl {
	public:
		ListCtrlManager(wxWindow* parent, const wxWindowID id, 
			            const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {}
	private:
		
		void OnActivated(wxListEvent& event);

		void ShowContextMenu(const wxPoint& pos, bool isFolder);
		void OnContextMenu(wxContextMenuEvent& event);

		//Eventos acciones menu contextual
		void OnCrearFolder(wxCommandEvent& event);
		void OnCrearArchivo(wxCommandEvent& event);
		void OnBorrarArchivo(wxCommandEvent& event);

		wxDECLARE_EVENT_TABLE();
};

#endif