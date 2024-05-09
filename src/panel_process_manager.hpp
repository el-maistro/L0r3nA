#ifndef _PROC_MANAGER
#define _PROC_MANAGER
#include "headers.hpp"

class ListCtrlManager2 : public wxListCtrl {
	public:

		ListCtrlManager2(wxWindow* parent, const wxWindowID id,
			const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {}

		SOCKET sckCliente = INVALID_SOCKET;

		void AgregarData(std::string strBuffer, std::string _strPID);
	private:

		void OnRefrescar(wxCommandEvent& event);
		void OnTerminarPID(wxCommandEvent& event);


		void ShowContextMenu(const wxPoint& pos, bool isEmpty);
		void OnContextMenu(wxContextMenuEvent& event);


		wxDECLARE_EVENT_TABLE();
};


class panelProcessManager : public wxPanel {
	public:
		panelProcessManager(wxWindow* pParent);

		void CrearListview();

		ListCtrlManager2* listManager = nullptr;
		SOCKET sckCliente = INVALID_SOCKET;

	private:
		
};

#endif