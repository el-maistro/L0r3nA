#ifndef ___REVERSE
#define ___REVERSE

#include "headers.hpp"

namespace EnumReverseShell {
    enum Enum {
        BTN_Start = 100,
        BTN_Stop
    };
}

class panelReverseShell : public wxFrame {
    public:
        panelReverseShell(wxWindow* pParent, SOCKET sck, std::string _strID, ByteArray c_key);
        
        ~panelReverseShell() {
            DEBUG_MSG("Destruyendo panelReverseShell");
        }

        void OnHook(wxKeyEvent& event);
        void OnAgregarTexto(wxCommandEvent& event);

        unsigned long int p_uliUltimo = 19;

        void EscribirSalida(const char* cBuffer);
        
        SOCKET sckCliente = INVALID_SOCKET;

        wxTextCtrl* txtConsole = nullptr;

    private:
        wxTextCtrl* txtShellPath = nullptr;
        ByteArray enc_key;

        //Historial
        std::vector<wxString> vc_History;
        int iHistorialPos = 0;

        std::string strID = "";
        
        void OnButton(wxCommandEvent& event);

        wxDECLARE_EVENT_TABLE();
};

#endif
