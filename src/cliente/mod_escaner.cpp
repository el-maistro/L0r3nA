#include "mod_escaner.hpp"

mod_Escaner::mod_Escaner(st_Iphl& _iphlapi, st_Ws2_32& _ws2_32) {
    this->IPHLAPI = _iphlapi;
    this->WS32 = _ws2_32;

    if (this->WS32.pWsaStartup) {
        WSADATA wsa;
        if (this->WS32.pWsaStartup(MAKEWORD(2, 2), &wsa) != 0) {
            error();
        }
        this->isReady = true;
    }else {
        __DBG_("[X]mod_Escaner no se cargaron las funciones");
        this->isReady = false;
    }

    
}

bool mod_Escaner::checkMod() {
    return this->isReady;
}

std::vector<Host_Entry> mod_Escaner::m_Escanear(const char* _cidr, bool _is_full_scan, bool _is_port_scan, int _scan_type, int _puerto_inicio, int _puerto_fin) {
    this->vcIps.clear();

    //Parse CIDR y escanear vector con ips
    std::vector<std::string> valid_ips = this->m_ParseCIDR(_cidr);

    std::vector<std::unique_ptr<std::thread>> th;

    for (int i = 0; i < valid_ips.size(); i++) {
        if (_is_full_scan) {
            th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_thPing, this, valid_ips[i], true, _scan_type, _puerto_inicio, _puerto_fin));
        } else if (_is_port_scan) {
            th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_EscanearPuertos, this, valid_ips[i].c_str(), _scan_type, _puerto_inicio, _puerto_fin));
        }else {
            th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_thPing, this, valid_ips[i], false, 0, 0, 0));
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
            int numero_hosts = static_cast<int>(pow(2, 32 - prefijo));
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

void mod_Escaner::m_EscanearPuertos(const char* _host, int _scan_type, int _puerto_inicio, int _puerto_fin) {
    std::vector<int> puertos;
    if (_scan_type == TipoEscanerPuerto::SYN) {
        puertos = this->m_EscanearSYN(_host, _puerto_inicio, _puerto_fin);
    }else { 
        puertos = this->m_EscanearSCK(_host, _puerto_inicio, _puerto_fin);
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

std::vector<int> mod_Escaner::m_EscanearSYN(const char* _host, int _puerto_inicio, int _puerto_fin) {
    //Requiere Admin
    //Thanks https://marcocetica.com/posts/socket_tutorial_part3/

    this->vcPorts.clear();

    //struct in_addr server_ip;
    //const char* ip_addr = "127.0.0.1"; //Local ip address

    //server_ip.s_addr = inet_addr(_host);

    //SOCKET socket_fd = socket(AF_UNSPEC, SOCK_RAW, IPPROTO_RAW);
    //if (socket_fd == INVALID_SOCKET) {
    //    _DBG_("[SYN] Error creando el socket", WSAGetLastError());
    //    return this->vcPorts;
    //}

    ////Prepara TCP/IP Header
    //char datagrama[DATAGRAM_BUF_SIZE];
    //struct iphdr* ip_head = (struct iphdr*)datagrama;
    //struct tcphdr* tcp_head = (struct tcphdr*)(datagrama + sizeof(struct iphdr));

    //this->m_SetDatagram(datagrama, server_ip, ip_addr, ip_head, tcp_head);

    //int flag = 1;
    //if (setsockopt(socket_fd, IPPROTO_IP, IP_HDRINCL, (char*)&flag, sizeof(flag)) == SOCKET_ERROR) {
    //    _DBG_("Error configurando IP_HDRINCL", WSAGetLastError());
    //    closesocket(socket_fd);
    //    return this->vcPorts;
    //}

    //std::vector<std::unique_ptr<std::thread>> th;
    //for (size_t puerto = _puerto_inicio; puerto <= _puerto_fin; puerto++) {
    //    //Llamar funcion para escanear puerto
    //    th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_thCheckPortSYN, this, socket_fd, datagrama, server_ip, ip_addr, tcp_head, puerto));
    //}

    //for (auto& it : th) {
    //    if (it.get()->joinable()) {
    //        it.get()->join();

    //    }
    //}
    return this->vcPorts;
}

std::vector<int> mod_Escaner::m_EscanearSCK(const char* _host, int _puerto_inicio, int _puerto_fin) {
    __DBG_("Escaneando...");
    this->vcPorts.clear();
    
    std::vector<std::unique_ptr<std::thread>> th;

    for (int i = _puerto_inicio; i <= _puerto_fin; i++) {
        th.push_back(std::make_unique<std::thread>(&mod_Escaner::m_thCheckPortSCK, this, _host, i));
    }

    for (auto& it : th) {
        if (it.get()->joinable()) {
            it.get()->join();
        }
    }

    sort(this->vcPorts.begin(), this->vcPorts.end());

    return this->vcPorts;
}

void mod_Escaner::m_thCheckPortSCK(const char* _host, int _port) {
    if (!this->WS32.pGetAddrInfo ||
        !this->WS32.pSocket ||
        !this->WS32.pConnect ||
        !this->WS32.pCloseSocket ||
        !this->WS32.pFreeAddrInfo ||
        !this->WS32.pIoctlSocket ||
        !this->WS32.pSelect ||
        !this->WS32.pWSAGetLastError ||
        !this->WS32.p__WSAFDIsSet) {
        __DBG_("[X]m_thCheckPortSCK: No se cargaron las funciones");
        return;
    }
    struct addrinfo sAddress, * sP, * sServer;
    m_memset(&sAddress, 0, sizeof(sAddress));

    SOCKET temp_socket = INVALID_SOCKET;

    sAddress.ai_family = AF_UNSPEC;
    sAddress.ai_socktype = SOCK_STREAM;

    std::string strport = std::to_string(_port);
    const char* _cport = strport.c_str();
    
    int iRes = this->WS32.pGetAddrInfo(_host, _cport, &sAddress, &sServer);
    if (iRes != 0) {
        _DBG_("[X] getaddrinfo error", iRes);
        _DBG_("HOST_LEN: ", strlen(_host));
        if (_port == 110) {
            int dbgg = 12;
            int sad = 22;

        }
        __DBG_("HOST: " + std::string(_host) + " PORT:" + std::string(_cport));
        return;
    }

    for (sP = sServer; sP != nullptr; sP = sP->ai_next) {
        if ((temp_socket = this->WS32.pSocket(sP->ai_family, sP->ai_socktype, sP->ai_protocol)) == INVALID_SOCKET) {
            //socket error
            continue;
        }
        unsigned long int iBlock = 1;
        if (this->WS32.pIoctlSocket(temp_socket, FIONBIO, &iBlock) != 0) {
            _DBG_("[X] No se pudo hacer non_block", this->WS32.pWSAGetLastError());
        }


        if (this->WS32.pConnect(temp_socket, sP->ai_addr, static_cast<int>(sP->ai_addrlen)) == -1) {
            //No se pudo conectar
            //_DBG_("[X] No se pudo conectar. Puerto: " + std::string(_cport), WSAGetLastError());
            TIMEVAL Timeout;
            Timeout.tv_sec = 1;
            Timeout.tv_usec = 0;
            iBlock = 0;
            if (this->WS32.pIoctlSocket(temp_socket, FIONBIO, &iBlock) != 0) {
                _DBG_("[X] No se pudo hacer block", this->WS32.pWSAGetLastError());
            }
            fd_set Write, Err;
            FD_ZERO(&Write);
            FD_ZERO(&Err);
            FD_SET(temp_socket, &Write);
            FD_SET(temp_socket, &Err);

            this->WS32.pSelect(0, NULL, &Write, &Err, &Timeout);
            //if (!FD_ISSET(temp_socket, &Write)){
            if (!this->WS32.p__WSAFDIsSet(temp_socket, &Write)){
                continue;
            }
        }
        _DBG_("[!] " + std::string(_host) + " OPEN:", _port);
        this->WS32.pCloseSocket(temp_socket);
        break;
    }

    this->WS32.pFreeAddrInfo(sServer);

    if (sP == nullptr) {

        return;
    }
    
    this->m_AddEntryPort(_port);
}

void mod_Escaner::m_AddEntryPort(int _port) {
    std::unique_lock<std::mutex> lock(this->mtx_ports);
    this->vcPorts.push_back(_port);
}

void mod_Escaner::m_AddEntry(const Host_Entry& _entry) {
	std::unique_lock<std::mutex> lock(this->mtx_ips);
	this->vcIps.push_back(_entry);
}

void mod_Escaner::m_thPing(const std::string _strip, bool _is_port_scan, int _scan_type, int _puerto_inicio, int _puerto_fin) {
    if (!this->IPHLAPI.pIcmpCreateFile || !this->IPHLAPI.pIcmpSendEcho || !this->WS32.pInetntoA || !this->WS32.pInet_addr) {
        __DBG_("[NET][X] m_thPing no se cargaron las funciones");
        return;
    }
    const char* _host = _strip.c_str();

    HANDLE hIcmpFile;
    unsigned long ipaddr = this->WS32.pInet_addr(_host);
    DWORD dwRetVal = 0;
    char SendData[32] = "ABCDEFGHIJL";
    LPVOID ReplyBuffer = 0;
    DWORD ReplySize = 0;

    hIcmpFile = this->IPHLAPI.pIcmpCreateFile();
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

    dwRetVal = this->IPHLAPI.pIcmpSendEcho(hIcmpFile, ipaddr, SendData, sizeof(SendData), NULL, ReplyBuffer, ReplySize, 500);

    if (dwRetVal != 0) {

        PICMP_ECHO_REPLY pEchoReply = (PICMP_ECHO_REPLY)ReplyBuffer;
        struct in_addr ReplyAddr;
        ReplyAddr.S_un.S_addr = pEchoReply->Address;
        const char* phost = this->WS32.pInetntoA(ReplyAddr);
        Host_Entry nEntry = { phost, this->m_GetMac(phost), this->m_GetHostName(phost) };

        if (_is_port_scan) {
            std::vector<int> puertos;
            if (_scan_type == TipoEscanerPuerto::SYN) {
                puertos = this->m_EscanearSYN(_host, _puerto_inicio, _puerto_fin);
            }else {
                puertos = this->m_EscanearSCK(_host, _puerto_inicio, _puerto_fin);
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
    if (!this->IPHLAPI.pSendARP || !this->WS32.pInetptoN) {
        __DBG_("[NET][X] m_GetMac no se cargo la funcion");
        return "ERR";
    }
    ULONG macAddr[2] = { 0 };
    ULONG macAddrLen = 6;
    struct sockaddr_in dest;

    dest.sin_family = AF_INET;
    this->WS32.pInetptoN(AF_INET, _host, &dest.sin_addr);

    if (this->IPHLAPI.pSendARP(dest.sin_addr.s_addr, 0, macAddr, &macAddrLen) == NO_ERROR) {
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
    if (!this->WS32.pInetptoN || !this->WS32.pGetNameInfo) {
        __DBG_("[X] m_GetHostName no se cargaron las funciones");
        return "ERR";
    }
    DWORD dwRetval;

    struct sockaddr_in saGNI;
    char hostname[NI_MAXHOST];

    saGNI.sin_family = AF_INET;
    this->WS32.pInetptoN(AF_INET, _host, &saGNI.sin_addr);

    dwRetval = this->WS32.pGetNameInfo((struct sockaddr*)&saGNI,
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

//SYN
void mod_Escaner::m_SetDatagram(char* datagram, in_addr server_ip, const char* client_ip, iphdr* ip_head, tcphdr* tcp_head) {
    m_memset(datagram, 0, DATAGRAM_BUF_SIZE);

    ////Cabecera IP
    //ip_head->ihl = 5; //HELEN
    //ip_head->version = 4;
    //ip_head->tos = 0; //type of service
    //ip_head->tot_len = (sizeof(iphdr) + sizeof(tcphdr));
    //ip_head->id = htons(36521);
    //ip_head->frag_off = htons(16384);
    //ip_head->ttl = 64;
    //ip_head->protocol = IPPROTO_TCP;
    //ip_head->saddr = inet_addr(client_ip);
    //ip_head->daddr = server_ip.s_addr;
    //ip_head->check = this->checksum((unsigned short*)datagram, ip_head->tot_len >> 1);

    ////Cabecera TCP
    //tcp_head->source = htons(46300);
    //tcp_head->dest = htons(80);
    //tcp_head->seq = htonl(1105024978);
    //tcp_head->ack_seq = 0;
    //tcp_head->doff = (sizeof(tcphdr) / 4);
    //tcp_head->fin = 0;
    //tcp_head->syn = 1;
    //tcp_head->rst = 0;
    //tcp_head->psh = 0;
    //tcp_head->ack = 0;
    //tcp_head->urg = 0;
    //tcp_head->window = htons(14600);
    //tcp_head->check = 0;
    //tcp_head->urg_ptr = 0;
}

unsigned short mod_Escaner::checksum(void* buffer, int len) {
    unsigned short* buf = (unsigned short*)buffer;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char*)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = (unsigned short)~sum;
    return result;
}

void mod_Escaner::m_thCheckPortSYN(SOCKET sock_fd, char* datagram, in_addr server_ip, const char* client_ip, tcphdr* tcp_head, u_int target_port) {
    //Enviar paquete y leer respuesta

    //pseudo_header psh;
    //sockaddr_in ip_dest;
    //target_header target;

    //target.target_ip = server_ip;
    //target.target_port = target_port;

    //
    //ip_dest.sin_family = AF_INET;
    //ip_dest.sin_addr.s_addr = server_ip.s_addr;

    ////Cabecera TCP
    //tcp_head->dest = htons(target_port);
    //tcp_head->check = 0;

    ////Pseaudo header for checksum
    //psh.source_addr = inet_addr(client_ip);
    //psh.dest_addr = ip_dest.sin_addr.s_addr;
    //psh.plc = 0;
    //psh.prt = IPPROTO_TCP;
    //psh.tcp_len = htons(sizeof(tcphdr));

    //memcpy(&psh.tcp, tcp_head, sizeof(tcphdr));
    //tcp_head->check = this->checksum((unsigned short*)&psh, sizeof(pseudo_header));

    //if (sendto(sock_fd, datagram, sizeof(iphdr) + sizeof(tcphdr), 0, (sockaddr*)&ip_dest, sizeof(ip_dest)) < 0) {
    //    _DBG_("[X] sendto error:", WSAGetLastError());
    //    return;
    //}

    ////LEER RESPUESTA
    //SOCKET sock_raw;
    //int saddr_size, data_size;

    //struct sockaddr saddr;
    ////std::vector<char> buf(BUF_SIZE);
    //std::vector<char> buf(BUF_SIZE);

    //sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    //if (sock_raw == INVALID_SOCKET) {
    //    _DBG_("Error creando el socket raw", WSAGetLastError());
    //    return;
    //}

    //saddr_size = sizeof(saddr);

    //data_size = recvfrom(sock_raw, buf.data(), BUF_SIZE, 0, (struct sockaddr*)&saddr, (socklen_t*)&saddr_size);
    //if (data_size < 0) {
    //    _DBG_("Error recibiendo paquetes", WSAGetLastError());
    //    return;
    //}
    //__DBG_("DATA...");

    //iphdr* ip_head = (struct iphdr*)buf.data();
    //sockaddr_in source;
    //
    //unsigned short ip_head_len = ip_head->ihl * 4;
    //tcphdr* tcp_head2 = (tcphdr*)(buf.data() + ip_head_len);
    //
    //memset(&source, 0, sizeof(source));
    //source.sin_addr.s_addr = ip_head->saddr;

    //if (ip_head->protocol == IPPROTO_TCP) {
    //    if (tcp_head2->syn == 1 && tcp_head2->ack == 1 && source.sin_addr.s_addr == server_ip.s_addr) {
    //        //Puerto abiero
    //        this->m_AddEntryPort(target_port);
    //        _DBG_("PUERTO ABIERTO: ", target_port);
    //    }
    //}
    return;
}