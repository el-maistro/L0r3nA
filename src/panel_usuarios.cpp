#include "panel_usuarios.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelUsuarios, wxPanel)
	EVT_BUTTON(EnumPanelUsuarios::BTN_Refrescar, panelUsuarios::OnRefrescar)
wxEND_EVENT_TABLE()

panelUsuarios::panelUsuarios(wxWindow* pParent, SOCKET _sckSocket)
	: wxPanel(pParent, EnumIDS::ID_Panel_Info_Usuarios) {
	
	this->sckSocket = _sckSocket;

	wxButton* btn_refrescar = new wxButton(this, EnumPanelUsuarios::BTN_Refrescar, "Refrescar");
	this->CrearListCtrl();

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(btn_refrescar);
	main_sizer->Add(this->list_ctrl, 1, wxEXPAND | wxALL, 1);

	this->SetSizer(main_sizer);
}

void panelUsuarios::m_InsertarColumnas(std::vector<ColumnData2>& columns, wxListCtrl*& list_ctrl) {
	if (list_ctrl && columns.size() > 0) {
		wxListItem itemCol;
		for (int index = 0; index < columns.size(); index++) {
			itemCol.SetText(columns[index].strTitle.c_str());
			itemCol.SetWidth(columns[index].iSize);
			list_ctrl->InsertColumn(index, itemCol);
		}
	}
}

void panelUsuarios::m_InsertarDatos(const std::vector<std::vector<std::string>>& rows, wxListCtrl*& list_ctrl) {
	if (list_ctrl && rows.size() > 0) {
		int iCount = 0;
		for (auto& item : rows) {
			list_ctrl->InsertItem(iCount, item[0]);
			for (int index = 1; index < item.size(); index++) {
				list_ctrl->SetItem(iCount, index, item[index]);
			}
			iCount++;
		}
	}
}

void panelUsuarios::CrearListCtrl() {
	this->list_ctrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	
	std::vector<ColumnData2> columnas;
	columnas.push_back(ColumnData2{ "Nombre",			100 });
	columnas.push_back(ColumnData2{ "Comentario",	    150 });
	columnas.push_back(ColumnData2{ "Nombre Completo",   100 });
	columnas.push_back(ColumnData2{ "Antig Contraseña",	100 });
	columnas.push_back(ColumnData2{ "Directorio",		100 });
	columnas.push_back(ColumnData2{ "Ult Login",		    100 });
	columnas.push_back(ColumnData2{ "Ult Logout",	    100 });
	columnas.push_back(ColumnData2{ "# Int Login",		 60 });
	columnas.push_back(ColumnData2{ "# Logins",		     60 });
	columnas.push_back(ColumnData2{ "Servidor de Login", 100 });
	columnas.push_back(ColumnData2{ "Codigo Pais",		 60 });
	this->m_InsertarColumnas(columnas, this->list_ctrl);
}

void panelUsuarios::OnRefrescar(wxCommandEvent&) {
	p_Servidor->cChunkSend(this->sckSocket, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, true, EnumComandos::INF_Users);
}

void panelUsuarios::m_ProcesarDatos(const std::string& strBuffer) {
	std::vector<std::vector<std::string>> vcData;
	for (std::string& strRow : strSplit(strBuffer, this->strDelim1, 10000)) {
		std::vector<std::string> vcRow = strSplit(strRow, strDelim2, 11);
		vcData.push_back(vcRow);
	}
	this->m_InsertarDatos(vcData, this->list_ctrl);
}