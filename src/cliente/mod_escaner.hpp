#ifndef __MOD_ESCANER_HPP__
#define __MOD_ESCANER_HPP__ 1

#include "headers.hpp"
#include<iphlpapi.h>
#include<icmpapi.h>
#include<cmath>

struct Host_Entry {
	std::string strip;
	std::string strmac;
	std::string strhostname;
	std::string strports;
};

struct IP {
	std::string oct1;
	std::string oct2;
	std::string oct3;
	std::string oct4;
};

namespace TipoEscanerPuerto {
	enum Enum {
		SYN = 1,
		SCK
	};
}

class mod_Escaner {
	public:
		std::vector<Host_Entry> m_Escanear(const char* _cidr, bool _is_full_scan = false, bool _is_port_scan = false, int _scan_type = TipoEscanerPuerto::SCK);
	private:
		std::mutex mtx_ips;
		std::vector<Host_Entry> vcIps;

		std::vector<std::string> m_ParseCIDR(const char* _cidr);
		IP m_ParseIP(const char* _cidr);

		void m_EscanearPuertos(const char* _host, int _scan_type);

		std::vector<int> m_EscanearSYN(const char* _host);
		std::vector<int> m_EscanearSCK(const char* _host);


		void m_AddEntry(const Host_Entry& _entry);
		void m_thPing(const std::string _strip, bool _is_port_scan, int _scan_type);
		std::string m_GetMac(const char* _host);
		std::string m_GetHostName(const char* _host);

};

#endif