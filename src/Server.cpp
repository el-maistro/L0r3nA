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
    this->p_txtCtrl->AppendText(strLine);

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
    const auto Listener = [this](){
        this->m_Escucha();
        this->thListener.detach();
    };   

    const auto Ping = [this](){
        this->m_Ping();
        this->thPing.detach();
    };
    this->thListener = std::thread{Listener};
    this->thPing = std::thread(Ping);
}

void Servidor::m_Ping(){
    this->m_Lock();
    this->m_txtLog->LogThis("Thread PING iniciada", LogType::LogMessage);
    this->m_Unlock();
    while(this->p_Escuchando){
        //Ping cada 10 segundos
        int iCount = 0;
        for(std::vector<struct Cliente>::iterator it = this->vc_Clientes.begin() ; it != this->vc_Clientes.end();){
            int iBytes = send(it->_sckCliente, "PING", 4, 0);
            if(iBytes <= 0){
                //No se pudo enviar
                this->m_RemoverCliente(it->_id);
                std::string strTmp = "Cliente " + it->_strIp + "-" + it->_id + " desconectado";
                this->m_txtLog->LogThis(strTmp, LogType::LogMessage);
                it = this->vc_Clientes.erase(it);
                this->m_Lock();
                this->iCount--;
                this->m_Unlock();
            } else {
                ++it;
            }
        }
        Sleep(this->p_PingTime);
    }
    this->m_txtLog->LogThis("Thread PING terminada", LogType::LogMessage);
}

void Servidor::m_Escucha(){
    this->m_Lock();
    this->m_txtLog->LogThis("Thread LISTENER iniciada", LogType::LogMessage);
    this->m_Unlock();
    while(this->p_Escuchando){
        Sleep(100); //uso de CPU
        struct ClientConInfo sckNuevoCliente = this->m_Aceptar();
        if(sckNuevoCliente._sckSocket != INVALID_SOCKET){
            
            std::string strTmp = sckNuevoCliente._strIp;
            strTmp.append(1, ':');
            strTmp.append(sckNuevoCliente._strPuerto);
            
            struct Cliente structNuevoCliente = {sckNuevoCliente._sckSocket, RandomID(7), strTmp, "Windows"};
            this->vc_Clientes.push_back(structNuevoCliente);
            this->m_Lock();
            this->m_InsertarCliente(structNuevoCliente);
            this->iCount++;
            this->m_Unlock();
        }
    }
    this->m_txtLog->LogThis("Thread LISTENER terminada", LogType::LogMessage);
}

void Servidor::m_InsertarCliente(struct Cliente& p_Cliente){
    m_listCtrl->InsertItem(this->iCount, wxString(p_Cliente._id));
    m_listCtrl->SetItem(this->iCount, 1, wxString(p_Cliente._strIp));
    m_listCtrl->SetItem(this->iCount, 2, wxString(p_Cliente._strSo));
}

void Servidor::m_RemoverCliente(std::string p_ID){
    long lFound = this->m_listCtrl->FindItem(0, wxString(p_ID));
    if(lFound != wxNOT_FOUND){
        this->m_listCtrl->DeleteItem(lFound);
    } else{
        error();
    }
}