#include "mod_reverse_proxy.hpp"
#include "server.hpp"
#include "misc.hpp"

extern Servidor* p_Servidor;

ReverseProxy::ReverseProxy(){
	if (WSAStartup(MAKEWORD(2, 2), &this->wsa) != 0) {
		ERROR_EW("WSAStartup error");
	}
}

bool ReverseProxy::m_InitSocket(SOCKET& _socket, int _puerto) {
	_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	if (_socket == INVALID_SOCKET) {
		ERROR_EW("Error creando el socket");
		return false;
	}

	int iReuseAddr = 1;
	if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iReuseAddr, sizeof(iReuseAddr)) < 0) {
		ERROR_EW("Error en setsockopt");
		return false;
	}

	unsigned long int iNonBlockMode = 1; //1 para activar 0 es por defecto (BLOCK_MODE)
	if (ioctlsocket(_socket, FIONBIO, &iNonBlockMode) != 0) {
		ERROR_EW("Error haciendo el socket non_block");
		return false;
	}

	struct sockaddr_in structServer;

	structServer.sin_family = AF_INET;
	structServer.sin_port = htons(_puerto);
	structServer.sin_addr.s_addr = INADDR_ANY;

	if (bind(_socket, (struct sockaddr*)&structServer, sizeof(struct sockaddr)) == -1) {
		ERROR_EW("Error en bind");
		return false;
	}

	if (listen(_socket, 10) == -1) {
		ERROR_EW("Error en listen");
		return false;
	}

	return true;
}

SOCKET ReverseProxy::m_Aceptar(SOCKET& _socket) {
	struct sockaddr_in structCliente;
	int struct_size = sizeof(struct sockaddr_in);
	SOCKET nSocket = accept(_socket, (struct sockaddr*)&structCliente, &struct_size);
	if (nSocket != INVALID_SOCKET) {
		unsigned long int iBlock = 1;
		if (ioctlsocket(nSocket, FIONBIO, &iBlock) != 0) {
			ERROR_EW("Error configurando el socket NON_BLOCK");
		}

		//int enable = 1;
		//if (setsockopt(nSocket, SOL_SOCKET, SO_KEEPALIVE, (const char*)&enable, sizeof(enable)) == SOCKET_ERROR) {
		//	ERROR_EW("[m_Aceptar] KEEP_ALIVE");
		//}
	}

	return nSocket;
}

void ReverseProxy::InitHandler(int _puerto, SOCKET _socket, ByteArray c_key) {
	this->enc_key = c_key;

	//_puerto = puerto por el cual se escuchara
	//_socket = socket del cliente que se esta usando de proxy
	this->addProxyRunning(_puerto, _socket);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	std::thread th(&ReverseProxy::EsperarConexiones, this, _puerto, _socket);
	th.detach();
}

void ReverseProxy::StopHandler(int _puerto) {
	std::unique_lock<std::mutex> lock(this->mtx_MapProxyRunning);
	auto& it = this->map_is_proxy_running.find(_puerto);
	if (it != this->map_is_proxy_running.end()) {
		it->second = false;
		this->map_is_proxy_running.erase(_puerto);
	}
}

void ReverseProxy::EsperarConexiones(int _puerto, SOCKET _socket) {
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	fd_set fdMaster;
	FD_ZERO(&fdMaster);

	SOCKET thLocalSocket = INVALID_SOCKET;
	if(!this->m_InitSocket(thLocalSocket, _puerto)){
		ERROR_EW("[EsperarConexiones] No se pudo configurar el socket para escuchar...");
		return;
	}

	FD_SET(thLocalSocket, &fdMaster);
	
	
	while (this->isProxyRunning(_puerto)) {

		fd_set fdMaster_copy = fdMaster;

		int iNumeroSocket = select(thLocalSocket + 1, &fdMaster_copy, nullptr, nullptr, &timeout);

		for (int index = 0; index < iNumeroSocket; index++) {
			SOCKET temp_socket = fdMaster_copy.fd_array[index];

			//Conexion local entrante (browser, etc)
			if (temp_socket == thLocalSocket) {
				SOCKET nSocketLocal = this->m_Aceptar(thLocalSocket);

				if (nSocketLocal != INVALID_SOCKET) {
					std::thread th(&ReverseProxy::th_Handle_Session, this, _socket, RandomID(), nSocketLocal);
					th.detach();
				}

			}
		}
	}

	closesocket(thLocalSocket);
	thLocalSocket = INVALID_SOCKET;

	DEBUG_MSG("[!] Apagando proxy en puerto " + std::to_string(_puerto));
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
			_out_recibido == SOCKET_ERROR ? iRecibido : _out_recibido;
			break;
		}
		else if (iRecibido == SOCKET_ERROR) {
			int error_wsa = WSAGetLastError();
			if (error_wsa == WSAEWOULDBLOCK) {
				if (iRetrys-- > 0) {
					//ERROR_EW("[!] WSAEWOULDBLOCK Intento lectura...");
					std::this_thread::sleep_for(std::chrono::milliseconds(20));
					continue;
				}
			}
			else {
				ERROR_EW("[!] readAllLocal ERR:" + std::to_string(error_wsa));
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

int ReverseProxy::sendAllLocal(SOCKET& _socket, const char* _cbuffer, int _buff_size, bool dbg) {
	int iEnviado = 0;
	int iRetrys = RETRY_COUNT;
	int iTotalEnviado = 0;

	if (dbg) {
		int _DEBUG_BREAK_DUMMY = 0;
		int _DEBUG_BREAK_DUMMY_2 = 0;
	}

	while (iTotalEnviado < _buff_size) {
		iEnviado = send(_socket, _cbuffer + iTotalEnviado, _buff_size - iTotalEnviado, 0);
		if (iEnviado == 0) {
			break;
		}
		else if (iEnviado == SOCKET_ERROR) {
			int error_code = WSAGetLastError();
			if (error_code == WSAEWOULDBLOCK) {
				if (iRetrys-- > 0) {
					ERROR_EW("[sendAllLocal] Intento escritura...");
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

int ReverseProxy::cSend(SOCKET& _socket, const char* _cbuffer, size_t _buff_size, int _id_conexion) {
	size_t nSize = _buff_size + int(sizeof(int));
	std::vector<char> finalData(nSize);

	//   DATA | ID_CONEXION
	memcpy(finalData.data(), _cbuffer, _buff_size);
	memcpy(finalData.data() + _buff_size, &_id_conexion, sizeof(int));

	return p_Servidor->cChunkSend(_socket, finalData.data(), nSize, 0, false, EnumComandos::PROXY_CMD, this->enc_key);
}

void ReverseProxy::procRespuestaProxy(int _recibido, const std::vector<char>& _vcdata) {
	if (_recibido > 4 /*sizeof(int) */ ) {
		//Obtener ID del paquete
		int _id_conexion = 0;
		int idatasize = _recibido - sizeof(int);
		memcpy(&_id_conexion, _vcdata.data() + idatasize, sizeof(int));
		
		SOCKET socket_local_remoto = this->getLocalSocket(_id_conexion);

		if (socket_local_remoto != INVALID_SOCKET) {

			if (this->isRespuestaSegundoPaso(_vcdata, _recibido)) {
				//std::thread th = std::thread(&ReverseProxy::th_Handle_Session, this, _proxy_remota, _id_conexion);
				//th.detach();
				DEBUG_MSG("[!] Conexion con punto final completa");
			}

			if (this->sendAllLocal(socket_local_remoto, _vcdata.data(), idatasize) == SOCKET_ERROR) {
				ERROR_EW("[X] Error enviado respuesta de proxy remota a cliente local. Bytes: " + std::to_string(idatasize));
			}

		}else {
			ERROR_EW("[X] No se pudo encontrar un socket con ID " + std::to_string(_id_conexion));
		}
	}
}

bool ReverseProxy::isRespuestaSegundoPaso(const std::vector<char>& _vcdata, int _recibido) {
	//  https://datatracker.ietf.org/doc/html/rfc1928
	/*  +----+-----+-------+------+----------+----------+
		|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
		+----+-----+-------+------+----------+----------+
		| 1  |  0  | X'00' |  1   | Variable |    2     |
		+----+-----+-------+------+----------+----------+*/
	if (_recibido < 4) {
		return false;
	}

	if (_vcdata[0] == 0x05 || _vcdata[0] == 0x04) {		                          //SOCKS4 o SOCKS5
		if (_vcdata[1] == 0x00                                                    // RESPUESTA
			&& _vcdata[2] == 0x00                                                 // RESERVADO
			&& (_vcdata[3] == 0x01 || _vcdata[3] == 0x03 || _vcdata[3] == 0x04)   // IPV4 IPV6 Domain Name
			) {
			return true;
		}
	}
	return false;
}

void ReverseProxy::th_Handle_Session(SOCKET _socket_proxy_remoto, int _id_conexion, SOCKET _socket_local) {
	//_socket_proxy_remoto = SOCKET del cliente
	// _id_conexion = ID de 5 digitos generado al llamar a esta funcion
	// _socket_local = SOCKET del cliente que se conecto al servidor local

	this->addLocalSocket(_id_conexion, _socket_local);
	DEBUG_MSG("[!] Conexion local aceptada " + std::to_string(_socket_local) + " ID: " + std::to_string(_id_conexion));

	bool isRunning = true;
	bool isHandShakeDone = false;
	struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	fd_set fdClienteMaster;
	FD_ZERO(&fdClienteMaster);
	FD_SET(_socket_local, &fdClienteMaster);

	while (isRunning) {
		fd_set fdMaster_copy = fdClienteMaster;

		int iNumeroSocket = select(_socket_local + 1, &fdMaster_copy, nullptr, nullptr, &timeout);

		for (int index = 0; index < iNumeroSocket; index++) {
			SOCKET temp_socket = fdMaster_copy.fd_array[index];
			
			if (temp_socket == _socket_local) {
				int iRecibido = 0;
				std::vector<char> vcData = this->readAllLocal(temp_socket, iRecibido);
				if (iRecibido > 0) {
					if (this->cSend(_socket_proxy_remoto, vcData.data(), iRecibido, _id_conexion) == SOCKET_ERROR) {
						ERROR_EW("[X] No se pudo enviar el paquete al proxy remoto")
						closesocket(temp_socket);
						FD_CLR(temp_socket, &fdClienteMaster);
						isRunning = false;
					}
				}else if (iRecibido == SOCKET_ERROR) {
					closesocket(temp_socket);
					FD_CLR(temp_socket, &fdClienteMaster);
					isRunning = false;
				}
			}
		}
	}

	if (FD_ISSET(_socket_local, &fdClienteMaster)) {
		FD_CLR(_socket_local, &fdClienteMaster);
	}
	this->eraseLocalSocket(_id_conexion);
}

std::string ReverseProxy::strTestBanner() {
	std::string strHTML = "<br><br><center><h1>Error :v</h1></center>";
	std::string strBanner = "HTTP/1.1 200 OK\r\n"\
		"Date: Sat, 10 Jan 2011 03:10:00 GMT\r\n"\
		"Server: Tanuki/1.0\r\n"\
		"Content-Length:";
	strBanner += std::to_string(strHTML.size());
	strBanner += "\r\nContent - type: text / plain\r\n\r\n";
	strBanner += strHTML;

	return strBanner;
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

bool ReverseProxy::isProxyRunning(int _puerto) {
	std::unique_lock<std::mutex> lock(this->mtx_MapProxyRunning);
	auto it = this->map_is_proxy_running.find(_puerto);
	if (it != this->map_is_proxy_running.end()) {
		return it->second;
	}
	return false;
}

void ReverseProxy::addProxyRunning(int _puerto, SOCKET _socket) {
	std::unique_lock<std::mutex> lock(this->mtx_MapProxyRunning);
	this->map_is_proxy_running.insert({ _puerto, _socket });
}