//#ifdef ___TESTING_NEW_UI

#ifndef _FRAME_CLIENTE
#define _FRAME_CLIENTE

#include "server.hpp"
#include "headers.hpp"
#include "panel_reverse_shell.hpp"
#include "panel_transferencias.hpp"
#include "panel_escaner.hpp"

#define WIN_WIDTH 960
#define WIN_HEIGHT 380

namespace EnumFrameIDS {
    enum Enum {
        BTN_Keylogger = 100,
        BTN_Camara,
        BTN_Adm_Ventanas,
        BTN_Adm_Procesos,
        BTN_Informacion,
        BTN_Escaner,
        BTN_Remote_Desktop,
        BTN_Fun,
        BTN_Proxy_Iniciar,
        BTN_Proxy_Detener,
        BTN_Shell,
        BTN_Transferencias,
        BTN_Limpiar_Log
    };
}

//Clase para las funciones a ejecutar con el cliente, admin archivos, procesos, etc...
class MyTreeCtrl : public wxTreeCtrl {
    public:
        //Eventos
        MyTreeCtrl(wxWindow* parent, const wxWindowID id,
            const wxPoint& pos, const wxSize& size, std::string _strID, std::string _strIP, SOCKET sck) :
            wxTreeCtrl(parent, id, pos, size), p_Notebook(nullptr) { // Inicializar p_Notebook
            strClienteID = _strID;
            strClienteIP = _strIP; 
            sckCliente = sck;
        }

        virtual ~MyTreeCtrl() {}

        void OnItemActivated(wxTreeEvent& event);

        wxAuiNotebook* p_Notebook;
    private:
        std::string strClienteID;
        std::string strClienteIP;
        SOCKET sckCliente = INVALID_SOCKET;
        wxDECLARE_EVENT_TABLE();
};

class FrameCliente : public wxFrame {
    public:
        /*std::string strClienteID = "";
        SOCKET sckCliente = INVALID_SOCKET;

        FrameCliente(SOCKET _sckSocket, std::string _strID, struct Cliente& _cliente);

        void m_AddRemoteLog(const char* _buffer);

        MyTreeCtrl* m_tree = nullptr;
        panelReverseShell* panelShell = nullptr;
        panelTransferencias* panelTransfers = nullptr;
        panelEscaner* panelScaner = nullptr;*/
    private:
        //wxButton* btn_Test = nullptr;
        //wxTextCtrl* txtLog = nullptr;
        //wxComboBox* cmdIPS = nullptr;


        //void ChangeMyChildButtons(wxWindow* parent);

        ////Eventos
        //void OnClose(wxCloseEvent& event);
        //void OnButton(wxCommandEvent& event);

        ////Eventos de disenio
        //void OnMouseHover(wxMouseEvent& event);
        //void OnMouseLeave(wxMouseEvent& event);


        //void OnClosePage(wxAuiNotebookEvent& event);

        //wxDECLARE_EVENT_TABLE();

};
#endif

//#endif