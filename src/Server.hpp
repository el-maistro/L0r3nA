#include "headers.hpp"


struct Cliente{
    int _sckCliente;
    std::string _id;
    std::string _strIp;
    std::string _strSo;
};

struct ClientConInfo{
    int _sckSocket;
    std::string _strPuerto;
    std::string _strIp;
};

namespace LogType{
    const int LogMessage = 100;
    const int LogError   = 101;
    const int LogWarning = 102;
}

class MyLogClass{
    public:
        wxTextCtrl *p_txtCtrl;
        void LogThis(std::string strLog, int iType);
};


class MyListCtrl: public wxListCtrl{
    public:
        long m_updated;
        MyListCtrl(wxWindow *parent,
               const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
        : wxListCtrl(parent, id, pos, size, style){
            m_updated = -1;
        }

};

class Servidor{
    private:
        std::mutex p_mutex;
        SOCKET sckSocket = INVALID_SOCKET;
        std::vector<struct Cliente> vc_Clientes;
        u_int uiPuertoLocal = 0;
        WSADATA wsa;
        struct sockaddr_in structServer;

        int p_PingTime = 10000;
        
    public:
        Servidor();

        bool p_Escuchando = false;
        bool   m_Iniciar();
        ClientConInfo m_Aceptar();
        int iCount = 0;

        std::thread thListener;
        std::thread thPing;
    
        

        //Hilos
        void m_Handler();
        void m_Escucha();
        void m_Ping();

        //Manipular listview
        MyListCtrl *m_listCtrl;
        MyLogClass *m_txtLog;
        void m_InsertarCliente(struct Cliente& p_Cliente);
        void m_RemoverCliente(std::string p_ID);

        //Socket
        int cSend(int pSocket, const char* pBuffer, int pLen, int pFlags);
        int cRecv(int pSocket, char*& pBuffer, int pLen, int pFlags);

        //Mutex
        void m_Lock(){
            this->p_mutex.lock();
        }

        void m_Unlock(){
            this->p_mutex.unlock();
        }

        u_int m_lPuerto(){
            return uiPuertoLocal;
        }

        void m_sPuerto(int pNuevoPuerto){
            uiPuertoLocal = pNuevoPuerto;
        }

        ~Servidor(){
            if(sckSocket != INVALID_SOCKET){
                closesocket(sckSocket);
                sckSocket = INVALID_SOCKET;
            }
        }
};
