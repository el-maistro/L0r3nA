#ifndef _FRAME_CLIENTE
#define _FRAME_CLIENTE
#include "headers.hpp"

class MyTreeCtrl : public wxTreeCtrl {
public:
    //Eventos
    MyTreeCtrl(wxWindow* parent, const wxWindowID id,
        const wxPoint& pos, const wxSize& size) : wxTreeCtrl(parent, id, pos, size) {};
    virtual ~MyTreeCtrl() {}

    void OnItemActivated(wxTreeEvent& event);

    //void CrearNotebook();
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

        //wxAuiNotebook* p_Notebook;
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

class panelReverseShell : public wxPanel {
    public:
        panelReverseShell(wxWindow* pParent);
        wxTextCtrl* txtConsole;
        void OnHook(wxKeyEvent& event);
        unsigned long int p_uliUltimo = 19;

    private:
        //Historial
        std::vector<wxString> vc_History;
        int iHistorialPos = 0;

        std::string strID = "";
        SOCKET sckCliente = INVALID_SOCKET;
};

class panelMicrophone : public wxPanel {
    public:
        panelMicrophone(wxWindow* pParent);

    private:

        wxComboBox* mic_devices = nullptr;

        std::string strID = "";
        void OnRefrescarDispositivos(wxCommandEvent& event);
        void OnEscuchar(wxCommandEvent& event);
        void OnDetener(wxCommandEvent& event);

        void EnviarComando(std::string pComando);
        
        wxDECLARE_EVENT_TABLE();
};
#endif