#include "panel_escaner.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelEscaner, wxPanel)
	EVT_BUTTON(EnumEscanerIDS::BTN_Scan, panelEscaner::OnScan)
wxEND_EVENT_TABLE()


panelEscaner::panelEscaner(wxWindow* pParent, SOCKET _sck)
	: wxPanel(pParent, EnumEscanerIDS::Main_Window) {
	this->sckSocket = _sck;

	wxStaticText* lblTest = new wxStaticText(this, wxID_ANY, "Host(s):");
	this->txtHostBase = new wxTextCtrl(this, wxID_ANY);
	this->btnScan = new wxButton(this, EnumEscanerIDS::BTN_Scan, "Escanear");

	wxBoxSizer* controles_up = new wxBoxSizer(wxHORIZONTAL);
	controles_up->Add(lblTest);
	controles_up->Add(this->txtHostBase);
	controles_up->Add(this->btnScan);

	this->CrearListView();

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(controles_up);
	main_sizer->Add(this->list_ctrl);

	this->SetSizer(main_sizer);
}

void panelEscaner::CrearListView() {
	this->list_ctrl = new wxListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH * 3, FRAME_CLIENT_SIZE_WIDTH * 3), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);

	wxListItem itemCol;

	itemCol.SetText("IP");
	itemCol.SetWidth(150);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->list_ctrl->InsertColumn(0, itemCol);

	itemCol.SetText("MAC");
	itemCol.SetWidth(150);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->list_ctrl->InsertColumn(1, itemCol);

	itemCol.SetText("HostName");
	itemCol.SetWidth(200);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->list_ctrl->InsertColumn(2, itemCol);

}

void panelEscaner::AddData(const char* _buffer) {
	// IP | MAC | HOST <||>
	int iCount = 0;
	for (const std::string& host_entry : strSplit(std::string(_buffer), "<||>", 100000)) {
		std::vector<std::string> host = strSplit(host_entry, '|', 3);
		if (host.size() == 3) {
			this->list_ctrl->InsertItem(iCount, host[0]);
			this->list_ctrl->SetItem(iCount, 1, host[1]);
			this->list_ctrl->SetItem(iCount, 2, host[2]);

			iCount++;
		}
	}
}

void panelEscaner::OnScan(wxCommandEvent&) {
	this->list_ctrl->DeleteAllItems();

	wxString strHostBase = this->txtHostBase->GetValue();
	
	p_Servidor->cChunkSend(this->sckSocket, strHostBase.ToStdString().c_str(), strHostBase.size(), 0, false, EnumComandos::Net_Scan);
}
