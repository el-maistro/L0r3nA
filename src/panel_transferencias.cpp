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
    this->dataView = new wxDataViewListCtrl(this, EnumIDS::ID_Panel_Transferencias_List, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_VERT_RULES);

    this->dataView->AppendTextColumn("Nombre de archivo")->SetWidth(200);
    this->dataView->AppendTextColumn("Estado");
    this->dataView->AppendTextColumn("-")->SetWidth(30);
    this->dataView->AppendProgressColumn("Progreso")->SetMinWidth(FromDIP(100));

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(this->dataView, 1, wxALL | wxEXPAND, 1);

    this->SetSizer(sizer);

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
    if (this->dataView) {
        double dPorcentaje = 0;
        int iPorcentajeEntero = 0;
        try {
            double dDescargado = static_cast<double>(transferencia.uDescargado);
            double duTamano = static_cast<double>(transferencia.uTamano);
            dPorcentaje = (dDescargado / duTamano) * 100;
        }
        catch (const std::exception& e) {
            DEBUG_MSG(e.what());
        }
        std::istringstream strnum(std::to_string(dPorcentaje));
        std::string strPor = "";
        strnum >> std::setw(5) >> strPor;
        strPor.append(1, '%');

        iPorcentajeEntero = static_cast<int>(dPorcentaje);

        int index_pos = this->m_IndexOf(wxString(transferencia.strNombre));
        if (index_pos != wxNOT_FOUND) {
            //Ya existe solo actualizar si no ha finalizado
            if (!transferencia.isDone) {
                this->dataView->SetValue(dPorcentaje >= 100 ? "TRANSFERIDO" : (transferencia.isUpload ? "SUBIENDO" : "DESCARGANDO"), index_pos, 1);
                this->dataView->SetValue(strPor, index_pos, 2);
                this->dataView->SetValue(iPorcentajeEntero, index_pos, 3);
            }
        }else {
            //No se ha agregado el item
            wxVector<wxVariant> data;
            data.push_back(transferencia.strNombre);
            data.push_back(dPorcentaje >= 100 ? "TRANSFERIDO" : (transferencia.isUpload ? "SUBIENDO" : "DESCARGANDO"));
            data.push_back(strPor);
            data.push_back(iPorcentajeEntero);
            this->dataView->AppendItem(data);
        }
    }
}

int panelTransferencias::m_IndexOf(const wxString& strID) {
    if (this->dataView) {
        int iCount = this->dataView->GetItemCount();
        if (iCount > 0) {
            for (int iIndex = 0; iIndex < iCount; iIndex++) {
                if (this->dataView->GetTextValue(iIndex, 0) == strID) {
                    return iIndex;
                }
            }
        }
    }
    return wxNOT_FOUND;
}