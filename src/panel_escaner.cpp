#include "panel_escaner.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelEscaner, wxPanel)
	EVT_BUTTON(wxID_ANY, panelEscaner::OnScan)
wxEND_EVENT_TABLE()


panelEscaner::panelEscaner(wxWindow* pParent, SOCKET _sck)
	: wxPanel(pParent, EnumEscanerIDS::Main_Window) {
	this->sckSocket = _sck;

	wxStaticText* lblHost = new wxStaticText(this, wxID_ANY, "Host(s):");
	wxStaticText* lblEscaner = new wxStaticText(this, wxID_ANY, "Tipo escaneo:");
	
	this->txtHostBase = new wxTextCtrl(this, wxID_ANY);
	this->txtPortFrom = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 25));
	this->txtPortTo = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 25));


	wxArrayString arrSubnets;
	for (int i = 1; i <= 24; i++) {
		arrSubnets.push_back("/" + std::to_string(i));
	}
	arrSubnets.push_back(" ");

	this->cmb_Subnet = new wxComboBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, arrSubnets, wxCB_READONLY);

	arrSubnets.Clear();

	arrSubnets.push_back("Ping");
	//arrSubnets.push_back("Escaner de puertos (SYN)");
	arrSubnets.push_back("Escaner de puertos (SCK)");
	//arrSubnets.push_back("Full (SYN)");
	arrSubnets.push_back("Full (SCK)");

	this->cmb_Tipo = new wxComboBox(this, wxID_ANY, "Ping", wxDefaultPosition, wxDefaultSize, arrSubnets, wxCB_READONLY);

	wxButton* btnScan = new wxButton(this, EnumEscanerIDS::BTN_Scan, ">>>");


	wxBoxSizer* controles_up = new wxBoxSizer(wxHORIZONTAL);
	controles_up->Add(lblHost);
	controles_up->Add(this->txtHostBase);
	controles_up->Add(this->cmb_Subnet);
	controles_up->Add(lblEscaner);
	controles_up->Add(this->cmb_Tipo);
	controles_up->AddSpacer(10);
	controles_up->Add(new wxStaticText(this, wxID_ANY, "Rango puertos:"));
	controles_up->Add(this->txtPortFrom);
	controles_up->Add(new wxStaticText(this, wxID_ANY, "-"));
	controles_up->Add(this->txtPortTo);
	controles_up->Add(btnScan);

	this->CrearListView();

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

	main_sizer->Add(controles_up);
	main_sizer->Add(this->list_ctrl, 1, wxALL|wxEXPAND);

	this->Connect(EnumEscanerIDS::ListCtrl, wxEVT_LIST_ITEM_ACTIVATED, wxListEventHandler(panelEscaner::OnMostrarPuertos));

	this->SetSizer(main_sizer);
}

void panelEscaner::CrearListView() {
   this->list_ctrl = new wxListCtrl(this, EnumEscanerIDS::ListCtrl, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH * 3, FRAME_CLIENT_SIZE_WIDTH * 3), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);

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

	itemCol.SetText("Puertos");
	itemCol.SetWidth(200);
	itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
	this->list_ctrl->InsertColumn(3, itemCol);

}

void panelEscaner::AddData(const char* _buffer) {
	// IP | MAC | HOST <||>
	int iCount = 0;
	for (const std::string& host_entry : strSplit(std::string(_buffer), "<||>", 100000)) {
		std::vector<std::string> host = strSplit(host_entry, '|', 4);
		size_t dsize = host.size();
		if (dsize >= 3) { 
			this->list_ctrl->InsertItem(iCount, host[0]);
			this->list_ctrl->SetItem(iCount, 1, host[1]);
			this->list_ctrl->SetItem(iCount, 2, host[2]);

			//Contiene informacion de puertos
			if (dsize == 4) {
				this->list_ctrl->SetItem(iCount, 3, host[3]);
			}

			iCount++;
		}
	}
}

void panelEscaner::OnScan(wxCommandEvent& event) {
	this->list_ctrl->DeleteAllItems();

	int iComando = 0;

	wxString strFrom = this->txtPortFrom->GetValue() != "" ? this->txtPortFrom->GetValue() : "1";
	wxString strTo = this->txtPortTo->GetValue() != "" ? this->txtPortTo->GetValue() : "65535";

	wxString strHostBase = this->txtHostBase->GetValue();
	strHostBase += this->cmb_Subnet->GetValue() + "|";
	strHostBase += strFrom + "|" + strTo;

	int iTipoEscaner = this->cmb_Tipo->GetSelection();

	switch (iTipoEscaner) {
		case 0:
			iComando = EnumComandos::Net_Scan;
			break;
		case 1:
			iComando = EnumComandos::Net_Scan_Syn;
			break;
		case 2:
			iComando = EnumComandos::Net_Scan_Sck;
			break;
		case 3:
			iComando = EnumComandos::Net_Scan_Full_Syn;
			break;
		case 4:
			iComando = EnumComandos::Net_Scan_Full_Sck;
			break;
		default:
			iComando = EnumComandos::Net_Scan;
			break;
	}
	
	p_Servidor->cChunkSend(this->sckSocket, strHostBase.ToStdString().c_str(), strHostBase.size(), 0, false, iComando);
}

void panelEscaner::OnMostrarPuertos(wxListEvent& event) {
	long item = this->list_ctrl->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if (item != wxNOT_FOUND) {
		wxString strtitle = this->list_ctrl->GetItemText(item, 0);
		wxString ports = this->list_ctrl->GetItemText(item, 3);
		framePorts* nframe = new framePorts(this, ports, strtitle);
		nframe->Show(true);
	}
	
}

framePorts::framePorts(wxWindow* pParent, wxString _ports, wxString _title)
	: wxFrame(pParent, wxID_ANY, _title, wxDefaultPosition, wxSize(280, 250)) {
	wxArrayString puertos;
	for (std::string& numero : strSplit(_ports.ToStdString(), ',', 65535)) {
		puertos.Add(numero);
	}
	wxListBox* lista = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, puertos);

	wxBoxSizer* main_sizer = new wxBoxSizer(wxHORIZONTAL);

	main_sizer->Add(lista, 1, wxALL | wxEXPAND);

	this->SetSizer(main_sizer);
}