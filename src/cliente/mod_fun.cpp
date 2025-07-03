#include "mod_fun.hpp"
#include "misc.hpp"

modFun::modFun(HMODULE _hUser32) {
	this->hWinmmDLL = wrapLoadDLL("winmm.dll");
	this->hUser32DLL = _hUser32;

	if (this->hWinmmDLL) {
		this->WINMM.pMciSendStringA = (st_Winmm_Fun::LPMCISENDSTRINGA)wrapGetProcAddr(this->hWinmmDLL, "mciSendStringA");
	}

	if (this->hUser32DLL) {
		this->USER32.pSwapMouseButton = (st_User32_Fun::LPSWAPMOUSEBUTTON)wrapGetProcAddr(this->hUser32DLL, "SwapMouseButton");
		this->USER32.pBlockInput = (st_User32_Fun::LPBLOCKINPUT)wrapGetProcAddr(this->hUser32DLL, "BlockInput");
		this->USER32.pMessageBoxA = (st_User32_Fun::LPMESSAGEBOX)wrapGetProcAddr(this->hUser32DLL, "MessageBoxA");
	}
}

modFun::~modFun() {
	if (this->hWinmmDLL) {
		wrapFreeLibrary(this->hWinmmDLL);
	}
	this->hUser32DLL = nullptr;
}

void modFun::m_SwapMouse(BOOL _swap) {
	if (this->USER32.pSwapMouseButton) {
		this->USER32.pSwapMouseButton(_swap);
	}
}

void modFun::m_BlockInput(BOOL _block) {
	if (this->USER32.pBlockInput) {
		this->USER32.pBlockInput(_block);
	}
}

void modFun::m_Msg(const char* _msg, const char* _title, UINT _type) {
	if (this->USER32.pMessageBoxA) {
		this->USER32.pMessageBoxA(NULL, static_cast<LPCSTR>(_msg), static_cast<LPCSTR>(_title), _type | MB_TOPMOST);
	}
}

void modFun::m_CD(BOOL _open) {
	if (this->WINMM.pMciSendStringA) {
		this->WINMM.pMciSendStringA(_open ? "set cdaudio door open" : "set cdaudio door closed", NULL, NULL, NULL);
	}
}
