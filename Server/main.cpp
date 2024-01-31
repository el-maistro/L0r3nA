#include "Server.hpp"
#include "headers.hpp"

class MyApp : public wxApp{
    public:
        bool OnInit() override;
};

namespace EnumIDS{
    const int ID_Escuchar = 100;
    const int ID_Detener =  101;
};

class MyFrame : public wxFrame{
    public:
        MyFrame();
    private:
        Servidor *p_Servidor;
        wxPanel *m_RPanel, *m_LPanel, *m_BPanel;
        wxButton *btn_Escuchar, *btn_Detener, *btn_Salir;
        wxTextCtrl *txt_Log;
        wxMenu *menuFile, *menuHelp;
        
        wxSize p_BotonS = wxSize(100, 30);

        //Eventos
        void OnClickEscuchar(wxCommandEvent& event);
        void OnclickDetener(wxCommandEvent& event);

        void CrearLista(long flags, bool withText = true);
        void CrearControlesPanelIzquierdo();
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);

        wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_BUTTON(EnumIDS::ID_Escuchar, MyFrame::OnClickEscuchar)
    EVT_BUTTON(EnumIDS::ID_Detener, MyFrame::OnclickDetener)
    EVT_BUTTON(wxID_EXIT, MyFrame::OnExit)
wxEND_EVENT_TABLE()


wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit(){
    MyFrame *frame = new MyFrame();
    frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Lorena")
{

    this->p_Servidor = new Servidor();
    this->p_Servidor->m_listCtrl = nullptr;

    
    this->m_RPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(600, 450));
    
    this->m_LPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(110, 600));
    this->m_LPanel->SetBackgroundColour(wxColor(255,0,0)); // REMOVE AT THE END

    this->m_BPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 150));
    this->m_BPanel->SetBackgroundColour(wxColor(0,255,0)); // REMOVE AT THE END

    
    //Crear lista
    this->CrearLista(wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);
    wxBoxSizer *sizerlist = new wxBoxSizer(wxHORIZONTAL);
    sizerlist->Add(this->p_Servidor->m_listCtrl, 1, wxALL | wxEXPAND, 1);
    this->m_RPanel->SetSizer(sizerlist);

    
    //Crear controles panel izquierdo
    this->CrearControlesPanelIzquierdo();

    //Crear txt para log
    this->p_Servidor->m_txtLog->p_txtCtrl = new wxTextCtrl(this->m_BPanel, wxID_ANY, ":v\n", wxDefaultPosition, wxSize(100,150), wxTE_MULTILINE | wxTE_READONLY);
    wxBoxSizer *sizertxt = new wxBoxSizer(wxHORIZONTAL);
    sizertxt->Add(this->p_Servidor->m_txtLog->p_txtCtrl, 1, wxALL | wxEXPAND, 1);
    this->m_BPanel->SetSizer(sizertxt);

    this->p_Servidor->m_txtLog->LogThis("Lista creada", LogType::LogMessage);
    this->p_Servidor->m_txtLog->LogThis("Lista creada", LogType::LogError);
    this->p_Servidor->m_txtLog->LogThis("Lista creada", LogType::LogWarning);

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(this->m_LPanel, 0, wxALL, 2);

    wxBoxSizer *sizer2 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(this->m_RPanel, 1, wxEXPAND | wxALL, 2);
    sizer2->Add(this->m_BPanel, 0, wxEXPAND | wxALL, 2);

    sizer->Add(sizer2, 1, wxALL, 1);

    this->SetSizerAndFit(sizer);

    SetClientSize(800,600);
    
    CreateStatusBar();
    SetStatusText("IDLE");
}

void MyFrame::CrearControlesPanelIzquierdo(){
    
    this->btn_Escuchar = new wxButton(this->m_LPanel, EnumIDS::ID_Escuchar, "Iniciar Servidor", wxDefaultPosition, this->p_BotonS);
    this->btn_Detener = new wxButton(this->m_LPanel, EnumIDS::ID_Detener, "Detener Servidor", wxDefaultPosition, this->p_BotonS);
    this->btn_Salir = new wxButton(this->m_LPanel, wxID_EXIT, "Salir", wxDefaultPosition, this->p_BotonS);
    

    wxBoxSizer *m_paneSizer = new wxBoxSizer(wxVERTICAL);
    m_paneSizer->AddSpacer(20);    
    m_paneSizer->Add(this->btn_Escuchar, 0, wxALIGN_CENTER | wxALL, 3);
    m_paneSizer->Add(this->btn_Detener,0, wxALIGN_CENTER | wxALL, 3);
    m_paneSizer->Add(this->btn_Salir,0, wxALIGN_CENTER | wxALL, 3);

    this->btn_Detener->Enable(false);

    this->m_LPanel->SetSizerAndFit(m_paneSizer);

}

void MyFrame::CrearLista(long flags, bool withText){
    this->p_Servidor->m_listCtrl = new MyListCtrl(this->m_RPanel, wxID_ANY, wxDefaultPosition, wxSize(600, 300), 
    flags | wxBORDER_THEME);
    wxListItem itemCol;
    itemCol.SetText("ID");
    itemCol.SetWidth(60);
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    this->p_Servidor->m_listCtrl->InsertColumn(0, itemCol);

    itemCol.SetText("IP");
    itemCol.SetWidth(120);
    this->p_Servidor->m_listCtrl->InsertColumn(1, itemCol);

    itemCol.SetText("SO");
    itemCol.SetWidth(140);
    this->p_Servidor->m_listCtrl->InsertColumn(2, itemCol);
}

void MyFrame::OnExit(wxCommandEvent& event){
    this->p_Servidor->p_Escuchando = false;
    Sleep(2000);
    Close(true);
}

void MyFrame::OnClickEscuchar(wxCommandEvent& event){
    if(this->p_Servidor->m_Iniciar()){
        this->p_Servidor->m_Handler();
        this->btn_Detener->Enable(true);
        this->btn_Escuchar->Enable(false);
    } else {
        error();
    }
}

void MyFrame::OnclickDetener(wxCommandEvent& event){
    //Bloquear acceso a la variable 
    this->p_Servidor->m_Lock();
    this->p_Servidor->p_Escuchando = false;
    this->p_Servidor->m_Unlock();

    this->btn_Detener->Enable(false);
    this->btn_Escuchar->Enable(true);
}

void MyFrame::OnAbout(wxCommandEvent& event){
    long lFound = this->p_Servidor->m_listCtrl->FindItem(0, wxString("2"));
    if(lFound != wxNOT_FOUND){
        this->p_Servidor->m_listCtrl->DeleteItem(lFound);
    }
}


/*

g++ `wx-config --cppflags` wx.cpp -lwx_baseu-3.2 -lwx_baseu_net-3.2 -lwx_baseu_xml-3.2 -lwx_mswu_adv-3.2 -lwx_mswu_aui-3.2 -lwx_mswu_core-3.2 -lwx_mswu_gl-3.2 -lwx_mswu_html-3.2 -lwx_mswu_media-3.2 -lwx_mswu_propgrid-3.2 -lwx_mswu_qa-3.2 -lwx_mswu_ribbon-3.2 -lwx_mswu_richtext-3.2 -lwx_mswu_stc-3.2 -lwx_mswu_webview-3.2 -lwx_mswu_xrc-3.2

*/