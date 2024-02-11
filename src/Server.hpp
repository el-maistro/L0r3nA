#include "headers.hpp"


struct Cliente{
    int _sckCliente;
    time_t _ttUltimaVez;
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
        std::mutex log_mutex;

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
        std::mutex ping_mutex;
        std::mutex count_mutex;

        SOCKET sckSocket = INVALID_SOCKET;
        std::vector<struct Cliente> vc_Clientes;
        u_int uiPuertoLocal = 0;
        WSADATA wsa;
        struct sockaddr_in structServer;

        int p_PingTime = 1000 * 60; //60 segundos
        
    public:
        Servidor();

        std::mutex p_mutex;

        bool p_Escuchando = false;
        bool   m_Iniciar();
        ClientConInfo m_Aceptar();
        int iCount = 0;

        std::thread thListener;
        std::thread thPing;
      

        //Hilos
        void m_Handler();
        void m_StopHandler();
        void m_JoinThreads();
        void m_Escucha();
        void m_Ping();

        //Manipular listview
        MyListCtrl *m_listCtrl;
        MyLogClass *m_txtLog;
        void m_InsertarCliente(struct Cliente& p_Cliente);
        void m_RemoverCliente(std::string p_ID);

        //Socket wraps
        int cSend(int& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false);
        int cRecv(int& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock = false);

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

            if (m_txtLog != nullptr) {
                delete m_txtLog;
                m_txtLog = nullptr;
            }
       
        }
};
