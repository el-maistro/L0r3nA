#include "cliente.hpp"
#include "mod_ventanas.hpp"
#include "misc.hpp"


BOOL CALLBACK EnumwindowsProc(HWND hwnd, LPARAM lParam) {
    mod_AdminVentanas* obj = (mod_AdminVentanas*)lParam;
    if (!obj) { return FALSE; }

    if (!obj->USER32.pGetWindowTextA || !obj->USER32.pGetForegroundWindow || !obj->USER32.pIsWindowVisible) {
        return FALSE;
    }

    if (obj->USER32.pIsWindowVisible(hwnd)) {
        char cBuffer[512];
        memset(cBuffer, 0, sizeof(cBuffer));
        int ret = obj->USER32.pGetWindowTextA(hwnd, (LPTSTR)&cBuffer, sizeof(cBuffer)-1);
        if (ret > 0) {
            VentanaInfo ventana;
            ventana.hwnd = hwnd;
            ventana.strTitle = cBuffer;
            
            ventana.active = (obj->USER32.pGetForegroundWindow() == hwnd);
            obj->vcVentanas.push_back(ventana);
        }
    }
    return TRUE;
}

mod_AdminVentanas::mod_AdminVentanas(HMODULE _user32DLL) {
    if (_user32DLL) {
        this->hUser32DLL = _user32DLL;
        this->USER32.pIsWindowVisible = (st_User32_WM::LPISWINDOWVISIBLE)wrapGetProcAddr(this->hUser32DLL, "IsWindowVisible");
        this->USER32.pGetWindowTextA = (st_User32_WM::LPGETWINDOWTEXTA)wrapGetProcAddr(this->hUser32DLL, "GetWindowTextA");
        this->USER32.pGetForegroundWindow = (st_User32_WM::LPGETFOREGROUNDWINDOW)wrapGetProcAddr(this->hUser32DLL, "GetForegroundWindow");
        this->USER32.pEnumWindows = (st_User32_WM::LPENUMWINDOWS)wrapGetProcAddr(this->hUser32DLL, "EnumWindows");
        this->USER32.pFindWindowA = (st_User32_WM::LPFINDWINDOWA)wrapGetProcAddr(this->hUser32DLL, "FindWindowA");
        this->USER32.pShowWindow = (st_User32_WM::LPSHOWWINDOW)wrapGetProcAddr(this->hUser32DLL, "ShowWindow");        
    }
}

std::vector<VentanaInfo> mod_AdminVentanas::m_ListaVentanas(){
    std::vector<VentanaInfo> vcOut;

    if (!this->USER32.pEnumWindows) {
        return vcOut;
    }

    this->vcVentanas.clear();
    if (this->USER32.pEnumWindows(EnumwindowsProc, (LPARAM)this)) {
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
    
    if (!this->USER32.pFindWindowA || !this->USER32.pShowWindow) {
        return;
    }

    int index = this->m_IndexOf(strTitle);
    HWND temp_hwnd = NULL;
    if (index == -1) {
        temp_hwnd = this->USER32.pFindWindowA(NULL, strTitle.c_str());
        if (!temp_hwnd) {
            return;
        }
    }else {
        temp_hwnd = this->vcVentanas[index].hwnd;
    }

    this->USER32.pShowWindow(temp_hwnd, iMessage);
}