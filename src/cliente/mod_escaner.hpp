#ifndef __MOD_ESCANER_HPP__
#define __MOD_ESCANER_HPP__ 1

#include "headers.hpp"
#include<icmpapi.h>
#include<iphlpapi.h>

struct Host_Entry {
	std::string strip;
	std::string strmac;
	std::string strhostname;
};

class mod_Escaner {
	public:
		std::vector<Host_Entry> m_Escanear(const char* _host_base);
	private:
		std::mutex mtx_ips;
		std::vector<Host_Entry> vcIps;

		void m_AddEntry(const Host_Entry& _entry);
		void m_thPing(const char* _host_base, int _octect);
		std::string m_GetMac(const char* _host);
		std::string m_GetHostName(const char* _host);

};

#endif