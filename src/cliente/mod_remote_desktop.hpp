#ifndef __MOD_REMOTE_DESKTOP
#define __MOD_REMOTE_DESKTOP

#include "headers.hpp"

class mod_RemoteDesktop {
	private:
		mod_RemoteDesktop();
		~mod_RemoteDesktop();

		std::vector<BYTE> getFrameBytes();

	public:

};

#endif