#include "mod_fun.hpp"
#include "misc.hpp"

modFun::modFun(st_User32_Fun& _user32, st_Winmm& _winmm) {
	this->USER32 = _user32;
	this->WINMM = _winmm;
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
