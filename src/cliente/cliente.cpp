#include "cliente.hpp"
#include "misc.hpp"

Cliente::~Cliente() {
	this->CerrarConexion();
}

void Cliente::CerrarConexion() {
    if (this->sckSocket != INVALID_SOCKET) {
		closesocket(this->sckSocket);
		this->sckSocket = INVALID_SOCKET;
	}
}

bool Cliente::bConectar(const char* cIP, const char* cPuerto) {
	struct addrinfo sAddress, *sP, *sServer;
	memset(&sAddress, 0, sizeof(sAddress));

	sAddress.ai_family = AF_UNSPEC;
	sAddress.ai_socktype = SOCK_STREAM;

	int iRes = getaddrinfo(cIP, cPuerto, &sAddress, &sServer);
	if (iRes != 0) {
		error();
		return false;
	}

	for (sP = sServer; sP != nullptr; sP = sP->ai_next) {
		if ((this->sckSocket = socket(sP->ai_family, sP->ai_socktype, sP->ai_protocol)) == INVALID_SOCKET) {
			//socket error
			continue;
		}

		if (connect(this->sckSocket, sP->ai_addr, sP->ai_addrlen) == -1) {
			//No se pudo conectar
			error();
			continue;
		}
		//exito
		break;
	}

	if (sP == nullptr || this->sckSocket == INVALID_SOCKET) {
        freeaddrinfo(sServer);
        return false;
	}

    freeaddrinfo(sServer);
    
	return true;
}

void Cliente::MainLoop() {
    //Esperar por comandos
    char cBuffer[1024];
    memset(&cBuffer, 0, sizeof(cBuffer));
    while (this->isRunning) {
        //Limpiar el buffer
        if (cBuffer[0] != 0) {
            memset(&cBuffer, 0, sizeof(cBuffer));
        }

        //Espere el comando en modo block
#ifdef ___DEBUG_
        std::cout<<"Esperando comando\n";
#endif
        int iRecibido = this->cRecv(this->sckSocket, cBuffer, 1024, false);

        if (iRecibido <= 0) {
            //No se pudo recibir nada
            break;
        }
#ifdef ___DEBUG_
        std::cout << "[SERVIDOR] " << cBuffer << "\n";
#endif
        //Partir maximo 50 parametros
        this->ProcesarComando(strSplit(cBuffer, '~', 50));

    }
}

void Cliente::ProcesarComando(std::vector<std::string> strIn) {
    if (strIn.size() == 0) {
        //No hay comandos
        return;
    }
    if(strIn[0] == "PING"){
#ifdef ___DEBUG_
        std::cout << "Ping\n";
#endif
        //implementar un metodo para devolverle el pong con el numero de parametro
        this->cSend(this->sckSocket, "PONG", 4, 0);
    }
}


void Cliente::iniPacket() {
    //Enviar SO
    std::string strOut = strOS();
    int iB = cSend(this->sckSocket, strOut.c_str(), strOut.length(), 0, false);
#ifdef ___DEBUG_
    std::cout << "Enviados " << iB << " bytes - "<< strOut <<"\n";
#endif
}

int Cliente::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock) {
    //Implementar encriptacion de datos aqui
    // 1 non block
    // 0 block
    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }
        int iTemp = send(pSocket, pBuffer, pLen, pFlags);
        //Restaurar
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }

        return iTemp;
    }
    else {
        return send(pSocket, pBuffer, pLen, pFlags);
    }
}

int Cliente::cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock) {
    //Aqui el socket por defecto es block asi que si se pasa false es normal
    //Implementar desencriptacion una vez se reciban los datos
    // 1 non block
    // 0 block
    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }
        int iTemp = recv(pSocket, pBuffer, pLen, pFlags);
        //Restaurar
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }

        return iTemp;
    }
    else {
        return recv(pSocket, pBuffer, pLen, pFlags);
    }
}