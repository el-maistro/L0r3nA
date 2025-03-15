#ifndef __MAIN_FRAME
#define __MAIN_FRAME
#include "headers.hpp"

class MyFrame : public wxFrame {
public:
    MyFrame();
private:
    wxPanel *m_RPanel = nullptr, *m_LPanel = nullptr, *m_BPanel = nullptr;
    wxMenu *menuFile = nullptr, *menuHelp = nullptr;
    wxButton *btn_CryptDB = nullptr, *btn_Builder = nullptr;
    wxToggleButton *btn_toggle = nullptr;
    wxStaticText *lblEstado = nullptr;

    wxMenuItem* iniciarMenu = new wxMenuItem(0, EnumIDS::ID_Iniciar_Servidor, "Iniciar Servidor");
    wxMenuItem* detenerMenu = new wxMenuItem(0, EnumIDS::ID_Detener_Servidor, "Detener Servidor");

    wxSize p_BotonS = wxSize(100, 30);

    //Eventos
    void OnLimpiar(wxCommandEvent& event);
    void OnToggle(wxCommandEvent& event);
    void OnCryptDB(wxCommandEvent& event);

    void SetEstado(const char* _cestado);
    
    void CrearLista(long flags, bool withText = true);
    void CrearControlesPanelIzquierdo();
    void OnExit(wxCommandEvent& event);
    void OnClose(wxCloseEvent& event);
    void OnAbout(wxCommandEvent& event);


    wxDECLARE_EVENT_TABLE();
};

#endif
