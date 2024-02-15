#include "Server.hpp"
#include "misc.hpp"


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

Servidor::Servidor(){
    this->uiPuertoLocal = 30000;

    //clase para logear
    this->m_txtLog = new MyLogClass();
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

void Servidor::m_CerrarConexion(int& pSocket) {
    if (pSocket != -1) {
        closesocket(pSocket);
        pSocket = -1;
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
        
        std::unique_lock<std::mutex> lock2(this->vector_mutex);
        for(auto it = this->vc_Clientes.begin(); it != this->vc_Clientes.end();){
            //Si le mande un ping hace 10 segundos o menos continuar con el otro
            if ((time(0) - it->_ttUltimaVez) <= PING_TIME) {
                ++it;
                Sleep(100);
                continue;
            }

            int iBytes = this->cSend(it->_sckCliente, "PING~1", 6, 0, false);
            if(iBytes <= 0){
                //No se pudo enviar
                std::string strTmp = "Cliente " + it->_strIp + "-" + it->_id + " desconectado";
                this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

                this->m_RemoverClienteLista(it->_id);
                it = this->vc_Clientes.erase(it);
                
                std::unique_lock<std::mutex> lock3(this->count_mutex);
                this->iCount--;
            } else {
                //Leer pong
                char cBuff[5];
                int iRecv = this->cRecv(it->_sckCliente, cBuff, 4, 0, true);
                cBuff[4] = '\0';
                if (strncmp(cBuff, "PONG", 4) != 0) {
                    //Error recibiendo el pong, desconectar cliente
                    this->m_CerrarConexion(it->_sckCliente);
                    it->_ttUltimaVez = PING_TIME;
                }else {
                    it->_ttUltimaVez = time(0);
                }
                ++it;
            }
        }
        lock2.unlock(); //desbloquear vector
    }
}

void Servidor::m_Escucha(){
    this->m_txtLog->LogThis("Thread LISTENER iniciada", LogType::LogMessage);
    while(this->p_Escuchando){
        Sleep(100); //uso de CPU
        struct ClientConInfo sckNuevoCliente = this->m_Aceptar();
        if(sckNuevoCliente._sckSocket != INVALID_SOCKET){
            //Socket nuevo hereda non_block del mainsocket
            
            std::string strTmp = sckNuevoCliente._strIp;
            strTmp.append(1, ':');
            strTmp.append(sckNuevoCliente._strPuerto);

            //Leer version y SO
            char cBuff[1024];
            memset(&cBuff, 0, 1024);
            int iR = this->cRecv(sckNuevoCliente._sckSocket, cBuff, 1024, 0, true);

            struct Cliente structNuevoCliente = { sckNuevoCliente._sckSocket, time(0), RandomID(7), strTmp, cBuff};
            
            std::unique_lock<std::mutex> lock(this->vector_mutex);
            this->vc_Clientes.push_back(structNuevoCliente);
            lock.unlock();

            std::unique_lock<std::mutex> lock2(this->count_mutex);
            this->m_InsertarCliente(structNuevoCliente);
            this->iCount++;
        }
    }
    std::cout << "DONE Listen\n";
}

void Servidor::m_InsertarCliente(struct Cliente& p_Cliente){
    m_listCtrl->InsertItem(this->iCount, wxString(p_Cliente._id));
    m_listCtrl->SetItem(this->iCount, 1, wxString(p_Cliente._strIp));
    m_listCtrl->SetItem(this->iCount, 2, wxString(p_Cliente._strSo));
}

void Servidor::m_RemoverClienteLista(std::string p_ID){
    long lFound = this->m_listCtrl->FindItem(0, wxString(p_ID));
    if(lFound != wxNOT_FOUND){
        this->m_listCtrl->DeleteItem(lFound);
    } else{
        error();
    }
}

int Servidor::cSend(int& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock) {
    //Implementar encriptacion de datos aqui
    // 1 non block
    // 0 block
    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }
        int iTemp = send(pSocket, pBuffer, pLen, pFlags);
        //Restaurar
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }

        return iTemp;
    }
    else {
        return send(pSocket, pBuffer, pLen, pFlags);
    } 
}

int Servidor::cRecv(int& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock) {
    //Implementar desencriptacion una vez se reciban los datos
    // 1 non block
    // 0 block
    if (isBlock) {
        //Hacer el socket block
        unsigned long int iBlock = 0;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }
        int iTemp = recv(pSocket, pBuffer, pLen, pFlags);
        //Restaurar
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            this->m_txtLog->LogThis("Error configurando el socket NON_BLOCK", LogType::LogError);
        }

        return iTemp;
    }
    else {
        return recv(pSocket, pBuffer, pLen, pFlags);
    }
}