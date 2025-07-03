#ifndef __MOD_FUN
#define __MOD_FUN 1

#include "headers.hpp"

struct st_Winmm_Fun {
	//mciSendStringA
	typedef MCIERROR(WINAPI* LPMCISENDSTRINGA)(LPCTSTR, LPTSTR, UINT, HANDLE);
	LPMCISENDSTRINGA pMciSendStringA = nullptr;
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

class modFun {
	public:
		void m_BlockInput(BOOL _block);
		void m_SwapMouse(BOOL _swap);
		void m_Msg(const char* _msg, const char* _title, UINT _type);
		void m_CD(BOOL _open);

		HMODULE hWinmmDLL = NULL;
		HMODULE hUser32DLL = NULL;

		st_Winmm_Fun WINMM;
		st_User32_Fun USER32;

		modFun(HMODULE _hUser32);
		~modFun();
};

#endif
