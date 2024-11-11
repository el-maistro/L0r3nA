#ifndef __PNL_USERS
#define __PNL_USERS 1

#include "headers.hpp"

namespace EnumPanelUsuarios {
	enum Enum {
		BTN_Refrescar = 30000
	};
}

struct ColumnData2 {
	std::string strTitle;
	int iSize;
};

class panelUsuarios : public wxPanel {
	public:
		panelUsuarios(wxWindow* pParent, SOCKET sckSocket);

		void m_ProcesarDatos(const std::string& strBuffer);

	private:
		SOCKET sckSocket = INVALID_SOCKET;
		wxListCtrl* list_ctrl = nullptr;

		void OnRefrescar(wxCommandEvent&);

		void m_InsertarColumnas(std::vector<ColumnData2>& columns, wxListCtrl*& list_ctrl);
		void m_InsertarDatos(const std::vector<std::vector<std::string>>& rows, wxListCtrl*& list_ctrl);

		void CrearListCtrl();

		std::string strDelim1 = ":[<>]:";
		std::string strDelim2 = ":[<->]:";

		wxDECLARE_EVENT_TABLE();
};

#endif
