#include "cliente.hpp"
#include "mod_reverse_proxy.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

ReverseProxy::ReverseProxy() {
	if (WSAStartup(MAKEWORD(2, 2), &this->wsa) != 0) {
		_DBG_("WSAStartup error", WSAGetLastError());
	}
}

void ReverseProxy::m_ProcesarDatosProxy(std::vector<char>& _vcdata, int _recibido) {
	if (_recibido < 4) {
		//Paquete muy pequenio
		return;
	}

	int data_size = _recibido - sizeof(int);
	int id_conexion = 0;
	
	memcpy(&id_conexion, _vcdata.data() + data_size, sizeof(int));
	
	_DBG_("ID Conexion: ", id_conexion);

	if (this->isSocksPrimerPaso(_vcdata, data_size)) {
		//Primer paso SOCKS
		__DBG_("[!]SOCKS Primer Paso");
		char cRespuesta[2] = { _vcdata[0], 0x00 };

		if (this->cSend(cCliente->sckSocket, cRespuesta, 2, id_conexion) == SOCKET_ERROR) {
			_DBG_("[Proxy]cSend error", WSAGetLastError());
		}
	}else if (this->isSocksSegundoPaso(_vcdata, data_size)) {
		//Paso final SOCKS
		__DBG_("[!]SOCKS Paso Final");
		char cHostType = _vcdata[3];
		std::vector<char> cHost;

		if (cHostType != 0x03) {
			//Parsear ipv4/ipv6
			const uint8_t* uAddr = reinterpret_cast<uint8_t*>(_vcdata.data());
			uAddr += 4;
			cHost = this->strParseIP(uAddr, cHostType);
		}else {
			size_t head_len = 4; // VER | CMD |  RSV  | ATYP
			size_t port_len = 2;
			size_t host_len = data_size - head_len - port_len - 1;
			cHost.resize(data_size - head_len - port_len);
			memcpy(cHost.data(), _vcdata.data() + 5, host_len);
		}

		u_short nPort = 0;
		int iPort = 0;
		size_t nPortOffset = data_size - 2;
		memcpy(&nPort, _vcdata.data() + nPortOffset, 2);
		iPort = static_cast<int>(ntohs(nPort));

		std::string strPort = std::to_string(iPort);

		const char* cPort = strPort.c_str();
		const char* cFinalHost = cHost.data();

		std::string strMessage = "Peticion recibida\n\tHost: ";
		strMessage += cHost.data();
		strMessage += " Puerto: ";
		strMessage += std::to_string(iPort);;

		__DBG_("\t" + strMessage);

		SOCKET sckPuntoFinal = this->m_sckConectar(cFinalHost, cPort);

		if (sckPuntoFinal != INVALID_SOCKET) {
			_vcdata[1] = 0x00;
			if (this->cSend(cCliente->sckSocket, _vcdata.data(), data_size, id_conexion) != SOCKET_ERROR) {
				this->addLocalSocket(id_conexion, sckPuntoFinal);

				//Iniciar thread para leer de punto final
				std::thread th(&ReverseProxy::th_Handle_Session, this, id_conexion);
				th.detach();
			}else {
				_DBG_("[Proxy]Conexion con punto final completo pero no se pudo enviar la respuesta al servidor", WSAGetLastError());
				closesocket(sckPuntoFinal);
			}
		}else {
			_vcdata[1] = 0x04;
			_DBG_("No se pudo conectar al punto final", WSAGetLastError());
			if (this->cSend(cCliente->sckSocket, _vcdata.data(), data_size, id_conexion) == SOCKET_ERROR) {
				_DBG_("[Proxy]No se pudo enviar la respuesta al servidor", WSAGetLastError());
			}
		}

	}else if (this->isHTTP(_vcdata, data_size)) {
		//Peticion HTTP/HTTPS
		HTTPRequest nRequest = this->parseHTTPrequest(_vcdata);
		if (nRequest.iTipoRequest == TipoRequestHTTP::HTTPS) {
			__DBG_("[!] HTTPS Request");

			//HTTPS
			SOCKET sckPuntoFinal = this->m_sckConectar(nRequest.strHost.c_str(), nRequest.strPort.c_str());
			std::string strResponse = "HTTP/1.1 200 Connection Established\r\n\r\n";
			if (sckPuntoFinal != INVALID_SOCKET) {
				if (this->cSend(cCliente->sckSocket, strResponse.c_str(), strResponse.size(), id_conexion) != SOCKET_ERROR) {
					this->addLocalSocket(id_conexion, sckPuntoFinal);
					std::thread th(&ReverseProxy::th_Handle_Session, this, id_conexion);
					th.detach();
				}else {
					_DBG_("[X] No se pudo enviar respuesta al servidor", WSAGetLastError());
				}
			}else {
				_DBG_("[X] No se pudo conectar con el punto final", WSAGetLastError());

				strResponse = "HTTP/1.1 500 Destination Not Found\r\n\r\n";
				if (this->cSend(cCliente->sckSocket, strResponse.c_str(), strResponse.size(), id_conexion) == SOCKET_ERROR) {
					_DBG_("[X] No se pudo enviar respuesta de error al servidor", WSAGetLastError());
				}
			}
		} else{
			//HTTP
			__DBG_("[!] HTTP Request");

			int offset = nRequest.iTipoRequest == TipoRequestHTTP::GET ? 4 : 5;

			size_t nSize = data_size;
			if (_vcdata[offset] != '/') {
				_vcdata.erase(_vcdata.begin() + offset, _vcdata.begin() + offset + 6 + nRequest.strHost.size() + nRequest.strPort.size());
				nSize = _vcdata.size() + nRequest.strPath.size();

				_vcdata.resize(nSize);

				_vcdata.insert(_vcdata.begin() + offset, nRequest.strPath.size(), ' ');

				memcpy(_vcdata.data() + offset, nRequest.strPath.c_str(), nRequest.strPath.size());
			}

			SOCKET sckPuntoFinal = this->m_sckConectar(nRequest.strHost.c_str(), nRequest.strPort.c_str());

			if (sckPuntoFinal != INVALID_SOCKET) {
				if (this->sendAllLocal(sckPuntoFinal, _vcdata.data(), nSize) != SOCKET_ERROR) {
					this->addLocalSocket(id_conexion, sckPuntoFinal);
					std::thread th(&ReverseProxy::th_Handle_Session, this, id_conexion);
					th.detach();
				}else {
					_DBG_("[X] Error reenviando peticion a punto final", WSAGetLastError());
				}
			}else {
				_DBG_("[X] No se pudo conectar con el punto final", WSAGetLastError());
			}
		}
	}else {
		//Datos para enviar a punto final
		SOCKET socket_punto_final = this->getLocalSocket(id_conexion);

		if (socket_punto_final != INVALID_SOCKET) {
			if (this->sendAllLocal(socket_punto_final, _vcdata.data(), data_size) == SOCKET_ERROR) {
				_DBG_("[X] Error reenviando datos al punto final", WSAGetLastError());
			}
		}else {
			__DBG_("[!] No se encontro un socket con el id " + std::to_string(id_conexion));
		}
	}
	return;
}

int ReverseProxy::sendAllLocal(SOCKET& _socket, const char* _cbuffer, int _buff_size) {
	int iEnviado = 0;
	int iRetrys = RETRY_COUNT;
	int iTotalEnviado = 0;

	while (iTotalEnviado < _buff_size) {
		iEnviado = send(_socket, _cbuffer + iTotalEnviado, _buff_size - iTotalEnviado, 0);
		if (iEnviado == 0) {
			break;
		}
		else if (iEnviado == SOCKET_ERROR) {
			int error_code = WSAGetLastError();
			if (error_code == WSAEWOULDBLOCK) {
				if (iRetrys-- > 0) {
					__DBG_("[sendAllLocal] Intento escritura...");
					std::this_thread::sleep_for(std::chrono::milliseconds(20));
					continue;
				}
			}
			else {
				return iTotalEnviado == 0 ? SOCKET_ERROR : iTotalEnviado;
			}
		}

		iTotalEnviado += iEnviado;
	}
	return iTotalEnviado;
}

std::vector<char> ReverseProxy::readAllLocal(SOCKET& _socket, int& _out_recibido) {
	_out_recibido = SOCKET_ERROR;
	std::vector<char> vcOut;
	int iChunk = 1024;
	int iTotalRecibido = 0;
	int iRetrys = RETRY_COUNT;
	int iRecibido = 0;
	while (1) {
		char cTempBuffer[1024];
		iRecibido = recv(_socket, cTempBuffer, iChunk, 0);
		if (iRecibido == 0) {
			break;
		}
		else if (iRecibido == SOCKET_ERROR) {
			int error_wsa = WSAGetLastError();
			if (error_wsa == WSAEWOULDBLOCK) {
				if (iRetrys-- > 0) {
					//DEBUG_MSG("[!] Intento lectura...");
					std::this_thread::sleep_for(std::chrono::milliseconds(10));
					continue;
				}
			}
			else {
				__DBG_("[!] readAllLocal ERR:" + std::to_string(error_wsa));
			}
			break;
		}

		iTotalRecibido += iRecibido;

		vcOut.resize(iTotalRecibido);

		memcpy(vcOut.data() + (iTotalRecibido - iRecibido), cTempBuffer, iRecibido);

		_out_recibido = iTotalRecibido;
	}

	return vcOut;
}

int ReverseProxy::cSend(SOCKET& _socket, const char* _cbuffer, size_t _buff_size, int _id_conexion) {
	size_t nSize = _buff_size + int(sizeof(int));
	std::vector<char> finalData(nSize);

	//   DATA | ID_CONEXION
	memcpy(finalData.data(), _cbuffer, _buff_size);
	memcpy(finalData.data() + _buff_size, &_id_conexion, sizeof(int));

	return cCliente->cChunkSend(_socket, finalData.data(), nSize, 0, false, nullptr, EnumComandos::PROXY_CMD);
}

SOCKET ReverseProxy::m_sckConectar(const char* _host, const char* _puerto) {
	struct addrinfo sAddress, * sP, * sServer;
	memset(&sAddress, 0, sizeof(sAddress));

	SOCKET temp_socket = INVALID_SOCKET;

	sAddress.ai_family = AF_UNSPEC;
	sAddress.ai_socktype = SOCK_STREAM;

	int iRes = getaddrinfo(_host, _puerto, &sAddress, &sServer);
	if (iRes != 0) {
		_DBG_("[X] getaddrinfo error", WSAGetLastError());
		return temp_socket;
	}

	for (sP = sServer; sP != nullptr; sP = sP->ai_next) {
		if ((temp_socket = socket(sP->ai_family, sP->ai_socktype, sP->ai_protocol)) == INVALID_SOCKET) {
			//socket error
			continue;
		}

		if (connect(temp_socket, sP->ai_addr, sP->ai_addrlen) == -1) {
			//No se pudo conectar
			_DBG_("[X] No se pudo conectar. Host: " + std::string(_host), WSAGetLastError());
			continue;
		}
		break;
	}

	if (sP == nullptr || temp_socket == INVALID_SOCKET) {
		freeaddrinfo(sServer);
		return temp_socket;
	}

	unsigned long int iBlock = 1;
	if (ioctlsocket(temp_socket, FIONBIO, &iBlock) != 0) {
		_DBG_("[X] No se pudo hacer non_block", WSAGetLastError());
	}

	freeaddrinfo(sServer);

	return temp_socket;
}

std::vector<char> ReverseProxy::strParseIP(const uint8_t* addr, uint8_t addr_type) {
	int addr_size = 0;
	int iFamily = 0;
	if (addr_type == 0x01) { //IPv4
		addr_size = INET_ADDRSTRLEN;
		iFamily = AF_INET;
	}
	else if (addr_type == 0x04) { //IPv6
		addr_size = INET6_ADDRSTRLEN;
		iFamily = AF_INET6;
	}
	std::vector<char> vc_ip(addr_size);
	inet_ntop(iFamily, addr, vc_ip.data(), addr_size);

	return vc_ip;
}

HTTPRequest ReverseProxy::parseHTTPrequest(const std::vector<char>& _vcdata) {
	HTTPRequest out_request;

	if (strncmp(_vcdata.data(), "GET", 3) == 0) {
		out_request.iTipoRequest = TipoRequestHTTP::GET;          //HTTP - GET
	}
	else if (strncmp(_vcdata.data(), "POST", 4) == 0) {
		out_request.iTipoRequest = TipoRequestHTTP::POST;         //HTTP - POST
	}
	else if (strncmp(_vcdata.data(), "CONNECT", 7) == 0) {
		out_request.iTipoRequest = TipoRequestHTTP::HTTPS;        //HTTPS
	}
	else {
		//Default ?
		out_request.iTipoRequest = TipoRequestHTTP::GET;
	}

	std::vector<char> in_data(_vcdata);
	in_data.push_back('\0');

	std::string strData(in_data.data());

	std::vector<std::string> vcSplit;

	switch (out_request.iTipoRequest) {
	case TipoRequestHTTP::GET:
	case TipoRequestHTTP::POST:
		vcSplit = strSplit(strData, " ", 3);
		if (vcSplit.size() == 3) {
			//Parsear URL - PUERTO - HOST
			std::vector<std::string> urlSplit = strSplit(vcSplit[1], "/", 1000); // suficiente? /path/to/url/super/long :v
			//  http: / / www.host.com / path ...
			if (urlSplit.size() >= 3) {

				//Parsear Host y Puerto
				size_t it = urlSplit[2].find(':');
				if (it != std::string::npos) {
					out_request.strPort = urlSplit[2].substr(it + 1, (urlSplit[2].size() - it) - 1);
					out_request.strHost = urlSplit[2].substr(0, it);
				}
				else {
					out_request.strPort = "80"; //Puerto 80 por defecto
					out_request.strHost = urlSplit[2];
				}

				//Parsear Path
				int offset = urlSplit[0].size() + urlSplit[2].size() + 3;
				out_request.strPath = "/" + vcSplit[1].substr(offset, vcSplit[1].size() - offset);

			}else {
				//No es http://somesite.com/path/file
				size_t host_offset = strData.find("Host: ");
				if (host_offset != std::string::npos) {
					size_t host_offset_end = strData.find("\r\n", host_offset);
					if (host_offset != std::string::npos) {
						out_request.strHost = strData.substr(host_offset + 6, host_offset_end - host_offset - 6);
						size_t port_offset = out_request.strHost.find(':');
						//El puerto es incluido en la cabecera Host
						if (port_offset != std::string::npos) {
							out_request.strPort = out_request.strHost.substr(port_offset + 1, out_request.strHost.size() - port_offset - 1);
						}
						else {
							out_request.strPort = "80";
						}
					}
				}
				else {
					__DBG_("[X] No se pudo parsear el host de la peticion");
					__DBG_("[DUMP]\n" + strData);
				}

				out_request.strPath = vcSplit[1];
			}
		}
		break;
	case TipoRequestHTTP::HTTPS:
		vcSplit = strSplit(strData, " ", 2);
		if (vcSplit.size() == 2) {
			size_t it = vcSplit[1].find(':');
			if (it != std::string::npos) {
				out_request.strHost = vcSplit[1].substr(0, it);
				out_request.strPort = vcSplit[1].substr(it + 1, (vcSplit[1].size() - it) - 1);
			}
			else {
				out_request.strHost = vcSplit[1];
				out_request.strPort = "443";
			}
			out_request.strPath = "";
		}
		break;
	default:
		//
		break;
	}

	return out_request;
}

void ReverseProxy::th_Handle_Session(int _id_conexion) {
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	SOCKET _socket_punto_final = this->getLocalSocket(_id_conexion);
	if (_socket_punto_final == INVALID_SOCKET) {
		__DBG_("[X] No existe un socket asociado con el ID " + std::to_string(_id_conexion));
		return;
	}

	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	fd_set fdMaster;
	FD_ZERO(&fdMaster);
	FD_SET(_socket_punto_final, &fdMaster);

	bool isRunning = true;
	__DBG_("[!] ENDPOINT Start");
	while (isRunning) {
		fd_set fdMaster_Copy = fdMaster;

		int iNumeroSockets = select(_socket_punto_final + 1, &fdMaster_Copy, nullptr, nullptr, &timeout);

		for (int index = 0; index < iNumeroSockets; index++) {
			SOCKET temp_socket = fdMaster_Copy.fd_array[index];

			if (temp_socket == _socket_punto_final) {
				//Datos del punto final. Reenviar a servidor junto con informacion de SOCKETS
				int iRecibido = 0;
				std::vector<char> vcData = this->readAllLocal(_socket_punto_final, iRecibido);
				if (iRecibido > 0) {
					if (this->cSend(cCliente->sckSocket, vcData.data(), iRecibido, _id_conexion) == SOCKET_ERROR) {
						_DBG_("[X] Error enviado respuesta del punto final", WSAGetLastError());
						FD_CLR(temp_socket, &fdMaster);
						closesocket(temp_socket);
						isRunning = false;
						break;
					}
				}else if (iRecibido == SOCKET_ERROR) {
					FD_CLR(_socket_punto_final, &fdMaster);
					closesocket(_socket_punto_final);
					isRunning = false;
					break;
				}
			}
		}
	}

	this->eraseLocalSocket(_id_conexion);

	return;
}

bool ReverseProxy::isSocksPrimerPaso(const std::vector<char>& _vcdata, int _recibido) {
	if (_recibido != 3) {
		return false;
	}

	if (_vcdata[0] == 0x5 || _vcdata[0] == 0x04) {      // SOCKS4 o SOCKS5
		if (_vcdata[1] == 0x01 && _vcdata[2] == 0x00) { // Sin Autenticacion
			return true;
		}
	}

	return false;
}

bool ReverseProxy::isSocksSegundoPaso(const std::vector<char>& _vcdata, int _recibido) {
	//  https://datatracker.ietf.org/doc/html/rfc1928
	/*  +----+-----+-------+------+----------+----------+
		|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
		+----+-----+-------+------+----------+----------+
		| 1  |  1  | X'00' |  1   | Variable |    2     |
		+----+-----+-------+------+----------+----------+*/
	if (_recibido < 4) {
		return false;
	}

	if (_vcdata[0] == 0x05 || _vcdata[0] == 0x04) {		                          //SOCKS4 o SOCKS5
		if (_vcdata[1] == 0x01                                                 // CMD
			&& _vcdata[2] == 0x00                                                 // RESERVADO
			&& (_vcdata[3] == 0x01 || _vcdata[3] == 0x03 || _vcdata[3] == 0x04)    // IPV4 IPV6 Domain Name
			) {
			return true;
		}
	}
	return false;
}

bool ReverseProxy::isHTTP(const std::vector<char>& _vcdata, int _recibido) {
	if (_recibido < 7) {
		return false;
	}

	//HTTP - GET
	if (strncmp(_vcdata.data(), "GET http", 8) == 0) {
		return true;
	}

	//HTTP - POST
	if (strncmp(_vcdata.data(), "POST http", 9) == 0) {
		return true;
	}

	//HTTPS
	if (strncmp(_vcdata.data(), "CONNECT", 7) == 0) {
		return true;
	}

	return false;
}

SOCKET ReverseProxy::getLocalSocket(int _id) {
	std::unique_lock<std::mutex> lock(this->mtx_MapSockets);
	auto it = this->map_sockets.find(_id);
	if (it != this->map_sockets.end()) {
		return it->second;
	}

	return INVALID_SOCKET;
}

void ReverseProxy::addLocalSocket(int _id, SOCKET _socket) {
	std::unique_lock<std::mutex> lock(this->mtx_MapSockets);
	this->map_sockets.insert({ _id, _socket });
}

bool ReverseProxy::eraseLocalSocket(int _id) {
	std::unique_lock<std::mutex> lock(this->mtx_MapSockets);
	if (this->map_sockets.erase(_id) == 1) {
		return true;
	}
	return false;
}

int ReverseProxy::getSocketID(SOCKET _socket) {
	std::unique_lock<std::mutex> lock(this->mtx_MapSockets);
	for (auto& it : this->map_sockets) {
		if (it.second == _socket) {
			return it.first;
		}
	}

	return -1;
}