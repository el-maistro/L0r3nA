#include "Server.hpp"
#include "headers.hpp"

class MyApp : public wxApp{
    public:
        bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

class MyFrame : public wxFrame{
    public:
        MyFrame();
    private:
        Servidor *p_Servidor = new Servidor();
        wxPanel *m_Panel;
        wxMenu *menuFile, *menuHelp;


        void CrearLista(long flags, bool withText = true);
        void OnInitServidor(wxCommandEvent& event);
        void OnStopServidor(wxCommandEvent& event);
        void OnExit(wxCommandEvent& event);
        void OnAbout(wxCommandEvent& event);
};

enum{
    ID_OInit = 1,
    ID_OStop = 2
};

bool MyApp::OnInit(){
    MyFrame *frame = new MyFrame();
    frame->Show();
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
    
    this->m_Panel = new wxPanel(this, wxID_ANY);

    this->CrearLista(wxLC_REPORT | wxLC_SINGLE_SEL);

    wxBoxSizer* const sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(this->p_Servidor->m_listCtrl, wxSizerFlags(2).Expand().Border());
    
    //Build listview
    wxListItem itemCol;
    itemCol.SetText("ID");
    itemCol.SetImage(-1);
    this->p_Servidor->m_listCtrl->InsertColumn(0, itemCol);

    itemCol.SetText("IP");
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    this->p_Servidor->m_listCtrl->InsertColumn(1, itemCol);

    itemCol.SetText("OS");
    itemCol.SetAlign(wxLIST_FORMAT_RIGHT);
    this->p_Servidor->m_listCtrl->InsertColumn(2, itemCol);


    //Agregar como prueba
    /*this->m_listCtrl->InsertItem(0, wxString("1"));
    this->m_listCtrl->SetItem(0, 1, wxString("127.0.0.1"));

    this->m_listCtrl->InsertItem(1, wxString("2"));
    this->m_listCtrl->SetItem(1, 1, wxString("127.0.0.2"));
    
    this->m_listCtrl->InsertItem(2, wxString("3"));
    this->m_listCtrl->SetItem(2, 1, wxString("127.0.0.4"));*/
    

    this->m_Panel->SetSizer(sizer);


    SetClientSize(this->m_Panel->GetBestSize());
    
    CreateStatusBar();
    SetStatusText("IDLE");

    Bind(wxEVT_MENU, &MyFrame::OnInitServidor, this, ID_OInit);
    Bind(wxEVT_MENU, &MyFrame::OnStopServidor, this, ID_OStop);
    Bind(wxEVT_MENU, &MyFrame::OnAbout, this, wxID_ABOUT);
    Bind(wxEVT_MENU, &MyFrame::OnExit, this, wxID_EXIT);
}

void MyFrame::CrearLista(long flags, bool withText){
    this->p_Servidor->m_listCtrl = new MyListCtrl(this->m_Panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
    flags | wxBORDER_THEME | wxLC_EDIT_LABELS);
}

void MyFrame::OnExit(wxCommandEvent& event){
    this->p_Servidor->p_Escuchando = false;
    if(this->p_Servidor->m_listCtrl != nullptr){
        delete this->p_Servidor->m_listCtrl;
        this->p_Servidor->m_listCtrl = nullptr;
    }
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