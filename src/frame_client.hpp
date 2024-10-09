#ifndef _FRAME_CLIENTE
#define _FRAME_CLIENTE
#include "headers.hpp"

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

        MyTreeCtrl* m_tree;

    private:
        wxButton* btn_Test;
        
        //Eventos
        void OnClose(wxCloseEvent& event);

        void OnClosePage(wxAuiNotebookEvent& event);

        wxDECLARE_EVENT_TABLE();

};
#endif