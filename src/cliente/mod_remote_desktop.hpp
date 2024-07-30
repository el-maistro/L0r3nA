#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"
#include<shlwapi.h>
#include<gdiplus.h>

class mod_RemoteDesktop {
	private:
		
		
	public:
		mod_RemoteDesktop();
		~mod_RemoteDesktop();
		std::vector<BYTE> getFrameBytes(ULONG quality);
};

#endif