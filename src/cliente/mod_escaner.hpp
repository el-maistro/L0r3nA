#ifndef __MOD_ESCANER_HPP__
#define __MOD_ESCANER_HPP__ 1

#include "headers.hpp"
#include<iphlpapi.h> //SendARP
#include<icmpapi.h>
#include<mstcpip.h>
#include<cmath>


#define DATAGRAM_BUF_SIZE 4096
#define BUF_SIZE 65536
#define MAX_PORTBUF_SIZE 1024

//Cabecera IP
struct iphdr {
	unsigned char  ihl;
	unsigned char  version;
	unsigned char  tos;
	unsigned short tot_len;
	unsigned short id;
	unsigned short frag_off;
	unsigned char  ttl;
	unsigned char  protocol;
	unsigned short check;
	unsigned int   saddr;
	unsigned int   daddr;
};

//Cabecera TCP
struct tcphdr {
	unsigned short source;
	unsigned short dest;
	unsigned int   seq;
	unsigned int   ack_seq;
	unsigned char  res1;
	unsigned char  doff;
	unsigned char  fin;
	unsigned char  syn;
	unsigned char  rst;
	unsigned char  psh;
	unsigned char  ack;
	unsigned char  urg;
	unsigned char  res2;
	unsigned short window;
	unsigned short check;
	unsigned short urg_ptr;
};

// Needed for checksum computation
struct pseudo_header {
	unsigned int source_addr;
	unsigned int dest_addr;
	unsigned char plc;
	unsigned char prt;
	unsigned short tcp_len;
	struct tcphdr tcp;
};

struct target_header {
	struct in_addr target_ip;
	unsigned int target_port;
};

struct datagram_header {
	char datagram[DATAGRAM_BUF_SIZE];
	struct iphdr* ip_head;
	struct tcphdr* tcp_head;
};

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
		mod_Escaner();

		std::vector<Host_Entry> m_Escanear(const char* _cidr, bool _is_full_scan = false, bool _is_port_scan = false, int _scan_type = TipoEscanerPuerto::SCK, int _puerto_inicio = 0, int _puerto_fin = 0);
	private:
		std::mutex mtx_ips;
		std::mutex mtx_ports;
		std::vector<Host_Entry> vcIps;
		std::vector<int> vcPorts;

		std::vector<std::string> m_ParseCIDR(const char* _cidr);
		IP m_ParseIP(const char* _cidr);

		void m_EscanearPuertos(const char* _host, int _scan_type, int _puerto_inicio, int _puerto_fin);

		std::vector<int> m_EscanearSYN(const char* _host, int _puerto_inicio, int _puerto_fin);
		std::vector<int> m_EscanearSCK(const char* _host, int _puerto_inicio, int _puerto_fin);

		void m_thCheckPortSCK(const char* _host, int _port);

		void m_AddEntryPort(int _port);
		void m_AddEntry(const Host_Entry& _entry);
		void m_thPing(const std::string _strip, bool _is_port_scan, int _scan_type, int _puerto_inicio, int _puerto_fin);
		std::string m_GetMac(const char* _host);
		std::string m_GetHostName(const char* _host);

		//SYN
		void m_SetDatagram(char* datagram, in_addr server_ip, const char* client_ip, iphdr* ip_head, tcphdr* tcp_head);
		unsigned short checksum(void* buffer, int len);
		void m_thCheckPortSYN(SOCKET sock_fd, char* datagram, in_addr server_ip, const char* client_ip, tcphdr* tcp_head, u_int target_port);
};

#endif