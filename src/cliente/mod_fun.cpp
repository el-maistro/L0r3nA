#include "mod_fun.hpp"
#include "misc.hpp"

modFun::modFun() {
	this->hWinmmDLL = wrapLoadDLL("winmm.dll");

	if (this->hWinmmDLL) {
		this->WINMM.pMciSendStringA = (st_Winmm_Fun::LPMCISENDSTRINGA)wrapGetProcAddr(this->hWinmmDLL, "mciSendStringA");
	}
}

modFun::~modFun() {
	if (this->hWinmmDLL) {
		wrapFreeLibrary(this->hWinmmDLL);
	}
}

void modFun::m_SwapMouse(BOOL _swap) {
	SwapMouseButton(_swap);
}

void modFun::m_BlockInput(BOOL _block) {
	BlockInput(_block);
}

void modFun::m_Msg(const char* _msg, const char* _title, UINT _type) {
	MessageBox(NULL, static_cast<LPCSTR>(_msg), static_cast<LPCSTR>(_title), _type | MB_TOPMOST);
}

void modFun::m_CD(BOOL _open) {
	if (this->WINMM.pMciSendStringA) {
		this->WINMM.pMciSendStringA(_open ? "set cdaudio door open" : "set cdaudio door closed", NULL, NULL, NULL);
	}
}
