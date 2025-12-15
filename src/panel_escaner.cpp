#include "panel_escaner.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

//Custom Event
wxDEFINE_EVENT(ADD_SCAN_OUTPUT, wxCommandEvent);

wxBEGIN_EVENT_TABLE(panelEscaner, wxFrame)
	EVT_BUTTON(wxID_ANY, panelEscaner::OnScan)
wxEND_EVENT_TABLE()


panelEscaner::panelEscaner(wxWindow* pParent, SOCKET _sck, std::string _strID)
	: wxFrame(pParent, EnumEscanerIDS::Main_Window, "Escaner de red") {
	this->sckSocket = _sck;
	this->SetName(_strID + "-NET");
	this->SetTitle("[" + _strID + "] Escaner de red");

	
	//////  Host(s) y prefijo    /////////
	wxPanel* pnlHost = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* boxHost = new wxStaticBoxSizer(wxVERTICAL, pnlHost, "Host y tipo de escaner");
	
	this->txtHostBase = new wxTextCtrl(pnlHost, wxID_ANY);

	wxArrayString arrSubnets;
	for (int i = 1; i <= 24; i++) {
		arrSubnets.push_back("/" + std::to_string(i));
	}
	arrSubnets.push_back(" ");

	this->cmb_Subnet = new wxComboBox(pnlHost, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, arrSubnets, wxCB_READONLY);

	boxHost->Add(new wxStaticText(pnlHost, wxID_ANY, "Host(s):"), 0, wxALL | wxEXPAND);
	boxHost->Add(this->txtHostBase, 0, wxALL | wxEXPAND);
	boxHost->Add(new wxStaticText(pnlHost, wxID_ANY, "Prefijo:"), 0, wxALL | wxEXPAND);
	boxHost->Add(this->cmb_Subnet, 0, wxALL | wxEXPAND);

	arrSubnets.Clear();
	arrSubnets.push_back("Ping");
	//arrSubnets.push_back("Escaner de puertos (SYN)");
	arrSubnets.push_back("Escaner de puertos (SCK)");
	//arrSubnets.push_back("Full (SYN)");
	arrSubnets.push_back("Full (SCK)");

	this->cmb_Tipo = new wxComboBox(pnlHost, wxID_ANY, "Ping", wxDefaultPosition, wxDefaultSize, arrSubnets, wxCB_READONLY);

	boxHost->Add(new wxStaticText(pnlHost, wxID_ANY, "Tipo escaneo:"), 0, wxALL | wxEXPAND);
	boxHost->Add(this->cmb_Tipo, 0, wxALL | wxEXPAND);
	
	pnlHost->SetSizer(boxHost);
	////////////////////////////////////////

	/////// Rango de puertos ///////////////
	wxPanel* pnlPuertos = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxStaticBoxSizer* boxPuertos = new wxStaticBoxSizer(wxVERTICAL, pnlPuertos, "Rango de puertos");

	this->txtPortFrom = new wxTextCtrl(pnlPuertos, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 25));
	this->txtPortTo = new wxTextCtrl(pnlPuertos, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(50, 25));
	
	boxPuertos->Add(new wxStaticText(pnlPuertos, wxID_ANY, "De:", wxDefaultPosition, wxDefaultSize), 0, wxALL | wxEXPAND);
	boxPuertos->Add(this->txtPortFrom, 0, wxALL | wxEXPAND);
	boxPuertos->Add(new wxStaticText(pnlPuertos, wxID_ANY, "Hasta:", wxDefaultPosition, wxDefaultSize), 0, wxALL | wxEXPAND);
	boxPuertos->Add(this->txtPortTo, 0, wxALL | wxEXPAND);

	pnlPuertos->SetSizer(boxPuertos);

	////////////////////////////////////////

	wxButton* btnScan = new wxButton(this, EnumEscanerIDS::BTN_Scan, "Iniciar");

	wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);

	top_sizer->Add(pnlHost, 0, wxALL | wxEXPAND);
	top_sizer->Add(pnlPuertos, 0, wxALL | wxEXPAND);
	
	this->CrearListView();

	wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
	
	main_sizer->Add(top_sizer, 0, wxALIGN_CENTER);
	main_sizer->Add(btnScan, 0, wxALIGN_CENTER);
	main_sizer->Add(this->list_ctrl, 1, wxALL|wxEXPAND);

	this->Connect(EnumEscanerIDS::ListCtrl, wxEVT_LIST_ITEM_ACTIVATED, wxListEventHandler(panelEscaner::OnMostrarPuertos));
	this->Bind(ADD_SCAN_OUTPUT, &panelEscaner::OnAgregarDatos, this);

	this->SetSizerAndFit(main_sizer);

	ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void panelEscaner::CrearListView() {
   this->list_ctrl = new wxListCtrl(this, EnumEscanerIDS::ListCtrl, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);

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

	//Crear spinner
	this->m_indicator = new wxActivityIndicator(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	this->m_indicator->Show(false);

}

void panelEscaner::AddData(const char* _buffer) {
	wxString strData(_buffer);

	wxCommandEvent evento(ADD_SCAN_OUTPUT, GetId());

	evento.SetString(_buffer);

	wxPostEvent(this, evento);
}

void panelEscaner::OnAgregarDatos(wxCommandEvent& event) {
	// IP | MAC | HOST <||>
	int iCount = 0;
	for (const std::string& host_entry : strSplit(event.GetString().ToStdString(), "<||>", 100000)) {
		std::vector<std::string> host = strSplit(host_entry, '|', 4);
		size_t dsize = host.size();
		if (dsize >= 3 && this->list_ctrl) {
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
	this->OcultarCarga();
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
	
	int iSent = p_Servidor->cChunkSend(this->sckSocket, strHostBase.ToStdString().c_str(), strHostBase.size(), 0, false, iComando);
	if (iSent > 0) {
		this->MostrarCarga();
	} // else error enviando el comando
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

void panelEscaner::MostrarCarga() {
	std::unique_lock<std::mutex> lock(this->mtx_carga);
	wxSize this_size = this->GetParent()->GetSize();
	wxSize loader_size = this->m_indicator->GetSize();
	wxPoint pos((this_size.GetWidth() / 2) - (loader_size.GetWidth() / 2), (this_size.GetHeight() / 2) - (loader_size.GetHeight() /2));
	
	this->m_indicator->SetPosition(pos);
	this->m_indicator->Show(true);
	this->m_indicator->Start();
	this->list_ctrl->Enable(false);
}

void panelEscaner::OcultarCarga() {
	std::unique_lock<std::mutex> lock(this->mtx_carga);

	this->m_indicator->Stop();
	this->m_indicator->Show(false);
	this->list_ctrl->Enable(true);
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