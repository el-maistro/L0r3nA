#ifndef __MOD_FUN
#define __MOD_FUN 1

#include "headers.hpp"

struct st_Winmm_Fun {
	//mciSendStringA
	typedef MCIERROR(WINAPI* LPMCISENDSTRINGA)(LPCTSTR, LPTSTR, UINT, HANDLE);
	LPMCISENDSTRINGA pMciSendStringA = nullptr;
};

//SwapMouseButton  user32


class modFun {
	public:
		void m_BlockInput(BOOL _block);
		void m_SwapMouse(BOOL _swap);
		void m_Msg(const char* _msg, const char* _title, UINT _type);
		void m_CD(BOOL _open);

		HMODULE hWinmmDLL = NULL;
		HMODULE hUser32DLL = NULL;

		st_Winmm_Fun WINMM;

		modFun();
		~modFun();
};

#endif
