#ifndef _FRAME_CLIENTE
#define _FRAME_CLIENTE
#include "headers.hpp"
#include "panel_reverse_shell.hpp"

#define WIN_WIDTH 1000
#define WIN_HEIGHT 500

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
        BNT_Shell,
        BTN_Limpiar_Log
    };
}

//Clase para las funciones a ejecutar con el cliente, admin archivos, procesos, etc...
class MyTreeCtrl : public wxTreeCtrl {
    public:
        //Eventos
        MyTreeCtrl(wxWindow* parent, const wxWindowID id,
            const wxPoint& pos, const wxSize& size, std::string _strID, std::string _strIP, SOCKET sck) :
            wxTreeCtrl(parent, id, pos, size) {
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
        std::string strClienteID = "";
        //std::string strIP = "";
        SOCKET sckCliente = INVALID_SOCKET;

        FrameCliente(std::string pstrID, SOCKET sckID, std::string strIP);
        FrameCliente(SOCKET _sckSocket, std::string _strID, std::string _strIP);

        void m_SetEstado(const wxString _str);

        void m_AddRemoteLog(const char* _buffer);

        MyTreeCtrl* m_tree;
        panelReverseShell* panelShell = nullptr;
    private:
        wxButton* btn_Test;
        wxTextCtrl* txtLog = nullptr;
        wxStaticText* lblestado = nullptr;
        
        //Eventos
        void OnClose(wxCloseEvent& event);
        void OnButton(wxCommandEvent& event);

        void OnClosePage(wxAuiNotebookEvent& event);

        wxDECLARE_EVENT_TABLE();

};
#endif
