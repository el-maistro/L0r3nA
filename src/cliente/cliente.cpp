#include "cliente.hpp"
#include "misc.hpp"

Cliente::Cliente() {
	WSACleanup();
}

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
		freeaddrinfo(sServer);
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
    std::cout<<strOS()<<std::endl;
}

void Cliente::iniPacket() {
    //Enviar SO

}

int Cliente::cSend(int& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock) {
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

int Cliente::cRecv(int& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock) {
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