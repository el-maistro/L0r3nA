#include "Server.hpp"
#include "misc.hpp"



bool Servidor::m_Iniciar(){
    WSACleanup();
    if(WSAStartup(MAKEWORD(2,2), &wsa) != 0){
        return false;
    }

    this->sckSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(!this->sckSocket){
        return false;
    }

    int iTemp = 1;
    if(setsockopt(this->sckSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iTemp, sizeof(int)) < 0){
        return false;
    }

    unsigned long int iBlock = 1;
    if(ioctlsocket(this->sckSocket, FIONBIO, &iBlock) != 0){
        error();
        return false;
    }

    this->structServer.sin_family        =                    AF_INET;
    this->structServer.sin_port          = htons(this->uiPuertoLocal);
    this->structServer.sin_addr.s_addr =                 INADDR_ANY;

    if(bind(this->sckSocket, (struct sockaddr *)&structServer, sizeof(struct sockaddr)) == -1){
        return false;
    }

    if(listen(this->sckSocket, 10) == -1){
        return false;
    }

    return true;
}

SOCKET Servidor::m_Aceptar(){
    struct sockaddr_in structCliente;
    int iTempC = sizeof(struct sockaddr_in);
    SOCKET tmpSck = accept(this->sckSocket, (struct sockaddr *)&structCliente, &iTempC) ;
    return tmpSck;
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
    while(this->p_Escuchando){
        //Ping cada 10 segundos
        int iCount = 0;
        for(std::vector<struct Cliente>::iterator it = this->vc_Clientes.begin() ; it != this->vc_Clientes.end();){
            int iBytes = send(it->_sckCliente, "PING", 4, 0);
            if(iBytes <= 0){
                //No se pudo enviar
                this->m_RemoverCliente(it->_id);
                std::cout<<it->_id<<" funado\n";
                it = this->vc_Clientes.erase(it);
                this->m_Lock();
                this->iCount--;
                this->m_Unlock();
            } else {
                ++it;
            }
        }
        Sleep(10000);
    }
    std::cout<<"FUNADO PING\n";
}

void Servidor::m_Escucha(){
    //Mientras sea true, esperar por conexiones
    while(this->p_Escuchando){
        //std::cout<<"Waiting...\n";
        Sleep(100);
        //Esperar conexion de nuevo cliente
        SOCKET sckNuevoCliente = this->m_Aceptar();
        if(sckNuevoCliente != INVALID_SOCKET){
            struct Cliente structNuevoCliente = {sckNuevoCliente, RandomID(10), "127.0.0.1", "Windows"};
            this->vc_Clientes.push_back(structNuevoCliente);
            this->m_InsertarCliente(structNuevoCliente);
            this->m_Lock();
            this->iCount++;
            this->m_Unlock();
        }
    }
    std::cout<<"FUNADO LISTENER\n";
}

void Servidor::m_InsertarCliente(struct Cliente& p_Cliente){
    this->m_Lock();
    m_listCtrl->InsertItem(this->iCount, wxString(p_Cliente._id));
    m_listCtrl->SetItem(this->iCount, 1, wxString(p_Cliente._strIp));
    m_listCtrl->SetItem(this->iCount, 2, wxString(p_Cliente._strSo));
    this->m_Unlock();
}

void Servidor::m_RemoverCliente(std::string p_ID){
    long lFound = this->m_listCtrl->FindItem(0, wxString(p_ID));
    if(lFound != wxNOT_FOUND){
        this->m_listCtrl->DeleteItem(lFound);
    } else{
        error();
    }
}