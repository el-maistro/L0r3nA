#ifndef __MAIN_FRAME
#define __MAIN_FRAME
#include "headers.hpp"

class MyFrame : public wxFrame {
public:
    MyFrame();
private:
    wxPanel* m_RPanel, * m_LPanel, * m_BPanel;
    wxMenu* menuFile, * menuHelp;
    wxButton *btn_CryptDB, *btn_Builder;
    wxToggleButton* btn_toggle;

    wxMenuItem* iniciarMenu = new wxMenuItem(0, EnumIDS::ID_Iniciar_Servidor, "Iniciar Servidor");
    wxMenuItem* detenerMenu = new wxMenuItem(0, EnumIDS::ID_Detener_Servidor, "Detener Servidor");

    wxSize p_BotonS = wxSize(100, 30);

    //Eventos
    void OnLimpiar(wxCommandEvent& event);
    void OnToggle(wxCommandEvent& event);
    void OnCryptDB(wxCommandEvent& event);
    
    void CrearLista(long flags, bool withText = true);
    void CrearControlesPanelIzquierdo();
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnAbout(wxCommandEvent& event);


    wxDECLARE_EVENT_TABLE();
};

#endif
