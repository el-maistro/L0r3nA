#include "Server.hpp"
#include "headers.hpp"

class MyApp : public wxApp{
    public:
        bool OnInit() override;
};

enum{
    ID_OInit = 1,
    ID_OStop = 2
};

class MyFrame : public wxFrame{
    public:
        MyFrame();
    private:
        Servidor *p_Servidor = new Servidor();
        wxPanel *m_RPanel;
        wxPanel *m_LPanel;
        wxPanel *m_BPanel;
        wxMenu *menuFile, *menuHelp;


        void CrearLista(long flags, bool withText = true);
        void OnInitServidor(wxCommandEvent& event);
        void OnStopServidor(wxCommandEvent& event);
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);

        wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(ID_OInit, MyFrame::OnInitServidor)
    EVT_MENU(ID_OStop, MyFrame::OnStopServidor)
    EVT_MENU(wxID_ABOUT, MyFrame::OnAbout)
    EVT_MENU(wxID_EXIT, MyFrame::OnExit)
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
    this->menuFile = new wxMenu;
    this->menuFile->Append(ID_OInit, "&Iniciar...\tCtrl+H", "Poner servidor a la escucha");
    this->menuFile->Append(ID_OStop, "&Detener...\tCtrl+K", "Detener servidor");
    this->menuFile->Enable(ID_OStop, false);
    this->menuFile->AppendSeparator();
    this->menuFile->Append(wxID_EXIT);

    this->menuHelp = new wxMenu;
    this->menuHelp->Append(wxID_ABOUT);

    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(this->menuFile, "&Servidor");
    menuBar->Append(this->menuHelp, "&Ashuda");

    SetMenuBar(menuBar);

    this->p_Servidor->m_listCtrl = nullptr;

    
    this->m_RPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(600, 300));
    
    this->m_LPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(200, 600));
    this->m_LPanel->SetBackgroundColour(wxColor(255,0,0));

    this->m_BPanel = new wxPanel(this, wxID_ANY);
    this->m_BPanel->SetBackgroundColour(wxColor(0,255,0));

        
    this->CrearLista(wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(this->m_LPanel, 0, wxALL, 2);
    sizer->Add(this->m_RPanel, 1, wxEXPAND | wxALL, 2);

    this->SetSizerAndFit(sizer);

    /*wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(this->m_RPanel, 1, wxEXPAND, 10);
    sizer->Add(this->m_RPanel, 1, wxEXPAND, 10)
    //sizer->Add(this->p_Servidor->m_listCtrl, wxSizerFlags(2).Expand().Border());*/
    

    SetClientSize(800,600);
    
    CreateStatusBar();
    SetStatusText("IDLE");
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

void MyFrame::OnInitServidor(wxCommandEvent& event){
    if(this->p_Servidor->m_Iniciar()){
        this->menuFile->Enable(ID_OInit, false);
        this->menuFile->Enable(ID_OStop, true);
        this->p_Servidor->m_Handler();
    } else {
        error();
    }
}

void MyFrame::OnStopServidor(wxCommandEvent& event){
    //Bloquear acceso a la variable 
    this->p_Servidor->m_Lock();
    this->p_Servidor->p_Escuchando = false;
    this->p_Servidor->m_Unlock();

    this->menuFile->Enable(ID_OInit, true);
    this->menuFile->Enable(ID_OStop, false);
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