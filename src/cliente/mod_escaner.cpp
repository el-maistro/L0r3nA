#include "mod_escaner.hpp"

std::vector<Host_Entry> mod_Escaner::m_Escanear(const char* _cidr, bool _is_full_scan, bool _is_port_scan, int _scan_type) {
    this->vcIps.clear();

    //Parse CIDR y escanear vector con ips
    std::vector<std::string> valid_ips = this->m_ParseCIDR(_cidr);

    std::vector<std::unique_ptr<std::thread>> th;

    for (int i = 0; i < valid_ips.size(); i++) {
        if (_is_full_scan) {
            th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_thPing, this, valid_ips[i], true, _scan_type));
        } else if (_is_port_scan) {
            th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_EscanearPuertos, this, valid_ips[i].c_str(), _scan_type));
        }else {
            th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_thPing, this, valid_ips[i], false, 0));
        }
    }

    for (auto& it : th) {
        if (it.get()->joinable()) {
            it.get()->join();
        }
    }

    _DBG_("Resultado:", this->vcIps.size());

    return this->vcIps;
}

std::vector<std::string> mod_Escaner::m_ParseCIDR(const char* _cidr) {
    std::vector<std::string> vc_ips;
    std::string str(_cidr);
    size_t pos = str.find('/');
    if (pos != std::string::npos) {
        int prefijo = atoi(str.substr(pos + 1, str.size() - pos - 1).c_str());
        if (prefijo > 0 && prefijo <= 24) {
            _DBG_("[SCAN] Prefijo: ", prefijo);
            int octeto = 0;
            int numero_octecto = 0;
            if (prefijo <= 8) {
                octeto = 8;
                numero_octecto = 1;
            }
            else if (prefijo <= 16) {
                octeto = 16;
                numero_octecto = 2;
            }
            else if (prefijo <= 24) {
                octeto = 24;
                numero_octecto = 3;
            }

            int bits_usables_en_octeto = octeto - prefijo;
            int temp_bits = bits_usables_en_octeto;
            int top_subnet_octecto = 1;
            while (temp_bits-- > 0) {
                top_subnet_octecto += top_subnet_octecto;
            }
            top_subnet_octecto--;

            //Numero hosts = 2 ^ (32 - prefijo)
            int numero_hosts = pow(2, 32 - prefijo);
            int subnets = numero_hosts / 256;

            __DBG_("[RESUMEN SCAN]\n");
            _DBG_("Hosts: " ,numero_hosts);
            _DBG_("Subnets: ", subnets);
            _DBG_("Octeto: ", numero_octecto);
            _DBG_("Bits usables en octeto: ", bits_usables_en_octeto);
            _DBG_("Top octecto: ", top_subnet_octecto);
            __DBG_("=========================");

            IP nip = this->m_ParseIP(_cidr);

            for (int i = 0; i <= top_subnet_octecto; i++) {
                if (numero_octecto == 1) {
                    nip.oct1 = std::to_string(i);
                    for (int _oct2 = 0; _oct2 <= 255; _oct2++) {
                        for (int _oct3 = 1; _oct3 <= 255; _oct3++) {
                            for (int _oct4 = 1; _oct4 <= 255; _oct4++) {
                                nip.oct2 = std::to_string(_oct2);
                                nip.oct3 = std::to_string(_oct3);
                                nip.oct4 = std::to_string(_oct4);
                                std::string strip = nip.oct1 + "." + nip.oct2 + "." + nip.oct3 + "." + nip.oct4;
                                vc_ips.push_back(strip);
                            }
                        }
                    }
                }else if (numero_octecto == 2) {
                    nip.oct2 = std::to_string(i);
                    for (int _oct3 = 1; _oct3 <= 255; _oct3++) {
                        for (int _oct4 = 1; _oct4 <= 255; _oct4++) {
                            nip.oct3 = std::to_string(_oct3);
                            nip.oct4 = std::to_string(_oct4);
                            std::string strip = nip.oct1 + "." + nip.oct2 + "." + nip.oct3 + "." + nip.oct4;
                            vc_ips.push_back(strip);
                        }
                    }
                }else if (numero_octecto == 3) {
                    nip.oct3 = std::to_string(i);
                    for (int _oct4 = 1; _oct4 <= 255; _oct4++) {
                        nip.oct4 = std::to_string(_oct4);
                        std::string strip = nip.oct1 + "." + nip.oct2 + "." + nip.oct3 + "." + nip.oct4;
                        vc_ips.push_back(strip);
                    }
                }


            }

            __DBG_("[SCAN] Vector IPS size: " + std::to_string(vc_ips.size()));
        }else {
            __DBG_("[SCAN] Prefijo no permitido: " + std::to_string(prefijo));
        }
    }else {
        //Direccion unica
        vc_ips.push_back(std::string(_cidr));
    }

    return vc_ips;
}

IP mod_Escaner::m_ParseIP(const char* _cidr) {
    IP nout = { "127", "0", "0", "1" };
    std::vector<std::string> vc;
    std::string str(_cidr);
    size_t prev = 0;
    size_t pos = std::string::npos;
    while ((pos = str.find('.', prev)) != std::string::npos) {
        vc.push_back(str.substr(prev, (pos - prev)));
        prev = pos + 1;
    }
    if (prev < str.size()) {
        pos = str.find('/', prev);
        if (pos != std::string::npos) {
            vc.push_back(str.substr(prev, (pos - prev)));
        }
        else {
            vc.push_back(str.substr(prev, str.size() - prev));
        }
    }

    if (vc.size() == 4) {
        return { vc[0], vc[1], vc[2], vc[3] };
    }

    return nout;
}

void mod_Escaner::m_EscanearPuertos(const char* _host, int _scan_type) {
    std::vector<int> puertos;
    if (_scan_type == TipoEscanerPuerto::SYN) {
        puertos = this->m_EscanearSYN(_host);
    }else { 
        puertos = this->m_EscanearSCK(_host);
    }

    if (puertos.size() > 0) {
        std::string strport = "";
        for (int& puerto : puertos) {
            strport += std::to_string(puerto);
            strport += ",";
        }
        strport.pop_back();

        Host_Entry nEntry = {std::string(_host), "-", "-", strport };
        this->m_AddEntry(nEntry);
    }
}

std::vector<int> mod_Escaner::m_EscanearSYN(const char* _host) {
    std::vector<int> vcout;
    //Dummy loop para probar

    for (int i = 1; i <= 100; i += 2) {
        vcout.push_back(i);
    }

    return vcout;
}

std::vector<int> mod_Escaner::m_EscanearSCK(const char* _host) {
    std::vector<int> vcout;

    for (int i = 100; i <= 200; i += 2) {
        vcout.push_back(i);
    }

    return vcout;
}

void mod_Escaner::m_AddEntry(const Host_Entry& _entry) {
	std::unique_lock<std::mutex> lock(this->mtx_ips);
	this->vcIps.push_back(_entry);
}

void mod_Escaner::m_thPing(const std::string _strip, bool _is_port_scan, int _scan_type) {
	const char* _host = _strip.c_str();

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

        if (_is_port_scan) {
            std::vector<int> puertos;
            if (_scan_type == TipoEscanerPuerto::SYN) {
                puertos = this->m_EscanearSYN(_host);
            }else {
                puertos = this->m_EscanearSCK(_host);
            }

            if (puertos.size() > 0) {
                std::string strport = "";
                for (int& puerto : puertos) {
                    strport += std::to_string(puerto);
                    strport += ",";
                }
                strport.pop_back();

                nEntry.strports = strport;
            }
        }
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