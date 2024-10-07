#include "panel_transferencias.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;

//wxBEGIN_EVENT_TABLE(panelTransferencias, wxPanel)
    //EVT_CLOSE(panelTransferencias::OnClose)
//wxEND_EVENT_TABLE()

panelTransferencias::~panelTransferencias() {
    this->SetActive(false);
    this->JoinThread();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
}

panelTransferencias::panelTransferencias(wxWindow* pParent, std::string strID) :
	wxPanel(pParent, EnumIDS::ID_Panel_Transfer) {
    this->strClienteID = strID;
    this->listView = DBG_NEW wxListCtrl(this, EnumIDS::ID_Panel_Transferencias_List, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 450), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);

    wxListItem itemCol;
    
    itemCol.SetText("Nombre de archivo");
    itemCol.SetWidth(200);
    this->listView->InsertColumn(0, itemCol);

    itemCol.SetText("Estado");
    itemCol.SetWidth(120);
    this->listView->InsertColumn(1, itemCol);

    itemCol.SetText("Progreso");
    itemCol.SetWidth(140);
    this->listView->InsertColumn(2, itemCol);

    this->SetActive(true);
    this->SpawnThread();
}

bool panelTransferencias::isActive() {
    std::unique_lock<std::mutex> lock(this->mtx_global);
    return this->_isActive;
}

void panelTransferencias::SetActive(bool status) {
    std::unique_lock<std::mutex> lock(this->mtx_global);
    this->_isActive = status;
}  

void panelTransferencias::SpawnThread() {
    this->th_monitor = std::thread(&panelTransferencias::thMonitor, this);
}

void panelTransferencias::JoinThread() {
    if (this->th_monitor.joinable()) {
        this->th_monitor.join();
    }
}

void panelTransferencias::thMonitor() {
    while (true) {
        if (!this->isActive()) {
            break;
        }
        //iterar sobre el vector del objeto cliente para leer entradas
        //Leer lista actual y crear actualizar listview
        int iIndex = p_Servidor->IndexOf(this->strClienteID);
        if (iIndex != -1) {
            int vSize = p_Servidor->vc_Clientes[iIndex]->Transfers_Size();
            for (int iT = 0; iT < vSize; iT++) {
                this->m_InsertarTransfer(p_Servidor->vc_Clientes[iIndex]->Transfer_Get(iT));
            }
        }else {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

}

void panelTransferencias::m_InsertarTransfer(const TransferStatus& transferencia){
    std::unique_lock<std::mutex> lock(this->mtx_global);
    DEBUG_MSG("Insertando " + transferencia.strNombre);
    if (this->listView) {
        int iRowCount = this->listView->GetItemCount();

        //Porcentaje de transferencia
        double dPercentage = (transferencia.uDescargado > 0 && transferencia.uTamano > 0) ? (transferencia.uDescargado / transferencia.uTamano) * 100 : 0;
        wxString strPor = std::to_string(dPercentage);
        strPor.append(1, '%');

        long lFound = this->listView->FindItem(0, wxString(transferencia.strNombre));
        if (lFound != wxNOT_FOUND) {
            //Ya existe, solo actualizar
            this->listView->SetItem(lFound, 1, wxString(transferencia.uDescargado >= transferencia.uTamano ? "TRANSFERIDO" : (transferencia.isUpload ? "SUBIENDO" : "DESCARGANDO")));
            this->listView->SetItem(lFound, 2, wxString(strPor));
            DEBUG_MSG("Ya existe " + transferencia.strNombre);
        }
        else {
            //No se ha agregado el item
            this->listView->InsertItem(iRowCount, wxString(transferencia.strNombre));
            this->listView->SetItem(iRowCount, 1, wxString(transferencia.uDescargado >= transferencia.uTamano ? "TRANSFERIDO" : (transferencia.isUpload ? "SUBIENDO" : "DESCARGANDO")));
            this->listView->SetItem(iRowCount, 2, wxString(strPor));
            DEBUG_MSG("No existe " + transferencia.strNombre);
        }
    }
}
