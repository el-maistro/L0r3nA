#ifndef __MOD_FUN
#define __MOD_FUN 1

#include "headers.hpp"
#include "mod_dynamic_load.hpp"

class modFun {
	public:
		void m_BlockInput(BOOL _block);
		void m_SwapMouse(BOOL _swap);
		void m_Msg(const char* _msg, const char* _title, UINT _type);
		void m_CD(BOOL _open);

		modFun(st_User32_Fun& _user32, st_Winmm& _winmm);

		st_User32_Fun USER32;
		st_Winmm WINMM;
};

#endif
