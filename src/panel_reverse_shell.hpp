#ifndef ___REVERSE
#define ___REVERSE

#include "headers.hpp"

namespace EnumReverseShell {
    enum Enum {
        BTN_Start = 100,
        BTN_Stop
    };
}

class panelReverseShell : public wxPanel {
    public:
        panelReverseShell(wxWindow* pParent, SOCKET sck);
        
        ~panelReverseShell() {
            DEBUG_MSG("Destruyendo panelReverseShell");
        }
        void OnHook(wxKeyEvent& event);
        unsigned long int p_uliUltimo = 19;

        void EscribirSalida(const char*& cBuffer);

    private:
        //Historial
        std::vector<wxString> vc_History;
        int iHistorialPos = 0;

        std::string strID = "";
        SOCKET sckCliente = INVALID_SOCKET;

        wxTextCtrl* txtConsole = nullptr;

        void OnButton(wxCommandEvent& event);

        wxDECLARE_EVENT_TABLE();
};

#endif
