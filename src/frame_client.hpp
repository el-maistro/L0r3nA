#ifndef _FRAME_CLIENTE
#define _FRAME_CLIENTE
#include "headers.hpp"

//Clase para las funciones a ejecutar con el cliente, admin archivos, procesos, etc...
class MyTreeCtrl : public wxTreeCtrl {
    public:
        //Eventos
        MyTreeCtrl(wxWindow* parent, const wxWindowID id,
            const wxPoint& pos, const wxSize& size) : wxTreeCtrl(parent, id, pos, size) {};
        virtual ~MyTreeCtrl() {}

        void OnItemActivated(wxTreeEvent& event);

        wxAuiNotebook* p_Notebook;
    private:
        wxDECLARE_EVENT_TABLE();
};

class FrameCliente : public wxFrame {
    public:
        std::string strClienteID = "";
        std::string strIP = "";
        SOCKET sckCliente = INVALID_SOCKET;

        FrameCliente(std::string pstrID, SOCKET sckID, std::string strIP);

        MyTreeCtrl* m_tree;

    private:
        wxButton* btn_Test;
        
        //Eventos
        void OnTest(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);

        void OnClosePage(wxAuiNotebookEvent& event);

        void MonitorTransferencias();

        wxDECLARE_EVENT_TABLE();

};

class panelTest : public wxPanel {
    public:
        panelTest(wxWindow* pParent);
        wxStaticText* lblOutputTest;
    private:
        
};

class panelTransferencias : public wxPanel {
    public:
        panelTransferencias(wxWindow* pParent);
    private:

};

class panelMicrophone : public wxPanel {
    public:
        panelMicrophone(wxWindow* pParent);

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