#ifndef __MOD_FUN
#define __MOD_FUN 1

#include "headers.hpp"

class modFun {
	public:
		void m_BlockInput(BOOL _block);
		void m_SwapMouse(BOOL _swap);
		void m_Msg(const char* _msg, const char* _title, UINT _type);
		void m_CD(BOOL _open);
};

#endif
