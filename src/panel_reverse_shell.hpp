#ifndef ___REVERSE
#define ___REVERSE

#include "headers.hpp"

class panelReverseShell : public wxPanel {
public:
    panelReverseShell(wxWindow* pParent);
    wxTextCtrl* txtConsole;
    void OnHook(wxKeyEvent& event);
    unsigned long int p_uliUltimo = 19;

private:
    //Historial
    std::vector<wxString> vc_History;
    int iHistorialPos = 0;

    std::string strID = "";
    SOCKET sckCliente = INVALID_SOCKET;
};

#endif
