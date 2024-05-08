#ifndef _PROC_MANAGER
#define _PROC_MANAGER
#include "headers.hpp"

class ListCtrlManager2 : public wxListCtrl {
	public:

		ListCtrlManager2(wxWindow* parent, const wxWindowID id,
			const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {}

		SOCKET sckCliente = INVALID_SOCKET;
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

		SOCKET sckCliente = INVALID_SOCKET;

	private:
		ListCtrlManager2* listManager = nullptr;
};

#endif