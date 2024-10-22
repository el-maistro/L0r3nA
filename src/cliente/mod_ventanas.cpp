#include "mod_ventanas.hpp"

BOOL CALLBACK EnumwindowsProc(HWND hwnd, LPARAM lParam) {
    mod_AdminVentanas* obj = (mod_AdminVentanas*)lParam;
    if (!obj) { return FALSE; }

    if (::IsWindowVisible(hwnd)) {
        char cBuffer[512];
        memset(cBuffer, 0, sizeof(cBuffer));
        int ret = GetWindowText(hwnd, (LPTSTR)&cBuffer, sizeof(cBuffer)-1);
        if (ret > 0) {
            VentanaInfo ventana;
            ventana.hwnd = hwnd;
            ventana.strTitle = cBuffer;
            
            ventana.active = (::GetForegroundWindow() == hwnd);
            obj->vcVentanas.push_back(ventana);
        }
    }
    return TRUE;
}

std::vector<VentanaInfo> mod_AdminVentanas::m_ListaVentanas(){
    std::vector<VentanaInfo> vcOut;
    this->vcVentanas.clear();
    if (::EnumWindows(EnumwindowsProc, (LPARAM)this)) {
        vcOut = this->vcVentanas;
    }
    return vcOut;
}

int mod_AdminVentanas::m_IndexOf(const std::string strTitle) {
    int index = -1;
    for (int i = 0; i < this->vcVentanas.size(); i++) {
        if (this->vcVentanas[i].strTitle == strTitle) {
            index = i;
            break;
        }
    }
    return index;
}

void mod_AdminVentanas::m_WindowMSG(const std::string strTitle, int iMessage) {
    __DBG_("WM: " + strTitle);
    __DBG_(iMessage);

    int index = this->m_IndexOf(strTitle);
    HWND temp_hwnd = NULL;
    if (index == -1) {
        temp_hwnd = FindWindow(NULL, strTitle.c_str());
        if (!temp_hwnd) {
            return;
        }
    }else {
        temp_hwnd = this->vcVentanas[index].hwnd;
    }

    ::ShowWindow(temp_hwnd, iMessage);
}