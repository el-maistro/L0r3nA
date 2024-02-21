#ifndef _FRAME_CLIENTE
#define _FRAME_CLIENTE
#include "headers.hpp"


class FrameCliente : public wxFrame {
    public:
        std::string strClienteID = "";
        std::vector<struct Cliente>::iterator p_Ite;
        FrameCliente(std::string pstrID);
    private:
        wxButton* btn_Test;

        //Eventos
        void OnTest(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);

        wxDECLARE_EVENT_TABLE();

};

class MyTreeCtrl : public wxTreeCtrl {
    public:
        //Eventos
        void OnItemActivated(wxTreeEvent& event);
        MyTreeCtrl(wxWindow* parent, const wxWindowID id,
            const wxPoint& pos, const wxSize& size) : wxTreeCtrl(parent, id, pos, size) {};
        virtual ~MyTreeCtrl() {}
    private:
        wxDECLARE_EVENT_TABLE();
};


#endif