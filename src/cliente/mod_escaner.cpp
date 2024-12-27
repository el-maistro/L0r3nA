#include "mod_escaner.hpp"

std::vector<Host_Entry> mod_Escaner::m_Escanear(const char* _host_base) {
    std::thread th[255];
    for (int i = 1; i <= 255; i++) {
        th[i - 1] = std::thread(&mod_Escaner::m_thPing, this, _host_base, i);
    }

    for (int i = 0; i < 255; i++) {
        if (th[i].joinable()) {
            th[i].join();
        }
    }

    return this->vcIps;
}

void mod_Escaner::m_AddEntry(const Host_Entry& _entry) {
	std::unique_lock<std::mutex> lock(this->mtx_ips);
	this->vcIps.push_back(_entry);
}

void mod_Escaner::m_thPing(const char* _host_base, int _octect) {
	std::string strHost = _host_base;
	strHost += std::to_string(_octect);

	const char* _host = strHost.c_str();

    HANDLE hIcmpFile;
    unsigned long ipaddr = inet_addr(_host);
    DWORD dwRetVal = 0;
    char SendData[32] = "ABCDEFGHIJL";
    LPVOID ReplyBuffer = 0;
    DWORD ReplySize = 0;

    hIcmpFile = IcmpCreateFile();
    if (hIcmpFile == INVALID_HANDLE_VALUE) {
        __DBG_("No se pudo crear el handle");
        return;
    }

    ReplySize = sizeof(ICMP_ECHO_REPLY) + sizeof(SendData);
    ReplyBuffer = (VOID*)malloc(ReplySize);
    if (ReplyBuffer == NULL) {
        __DBG_("No se pudo reservar el buffer");
        return;
    }

    dwRetVal = IcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 500);

    if (dwRetVal != 0) {

        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        const char* phost = inet_ntoa(ReplyAddr);
        Host_Entry nEntry = { phost, this->m_GetMac(phost), this->m_GetHostName(phost) };
        this->m_AddEntry(nEntry);
    }

    if (ReplyBuffer != NULL) {
        free(ReplyBuffer);
        ReplyBuffer = NULL;
    }
}

std::string mod_Escaner::m_GetMac(const char* _host) {
    ULONG macAddr[2] = { 0 };
    ULONG macAddrLen = 6;
    struct sockaddr_in dest;

    dest.sin_family = AF_INET;
    inet_pton(AF_INET, _host, &dest.sin_addr);

    if (SendARP(dest.sin_addr.s_addr, 0, macAddr, &macAddrLen) == NO_ERROR) {
        unsigned char* mac = (unsigned char*)macAddr;
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        return std::string(macStr);
    }else {
        return "No se encontro MAC";
    }
}

std::string mod_Escaner::m_GetHostName(const char* _host) {
    DWORD dwRetval;

    struct sockaddr_in saGNI;
    char hostname[NI_MAXHOST];

    saGNI.sin_family = AF_INET;
    inet_pton(AF_INET, _host, &saGNI.sin_addr);

    dwRetval = getnameinfo((struct sockaddr*)&saGNI,
        sizeof(struct sockaddr),
        hostname,
        NI_MAXHOST, NULL, 0, NI_NAMEREQD);

    if (dwRetval == 0) {
        return std::string(hostname);
    }
    else {
        return "Desconocido";
    }
}