#include "mod_mic.hpp"

std::vector<std::string> Mod_Mic::ObtenerDispositivos() {
    std::vector<std::string> vcOut;
    for (int i = 0; i < waveInGetNumDevs(); i++) {
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

void Mod_Mic::Grabar_pacman() {
    constexpr int NUM_BUFFERS = 2;
    constexpr int SAMPLE_RATE = 44100;
    constexpr int NUM_CHANNELS = 2;
    constexpr int BITS_PER_SAMPLE = 16;
    constexpr int BUFFER_SIZE = SAMPLE_RATE * NUM_CHANNELS * BITS_PER_SAMPLE / 8 / 2; // Mitad de un segundo
    // Definir el formato de audio
    WAVEFORMATEX wfx = {};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = NUM_CHANNELS;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = BITS_PER_SAMPLE;
    wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8;
    wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;

    // Abrir el dispositivo de grabación
    
    //int iNumero = waveInGetNumDevs();
    /*

    HWAVEIN wi;
    if (waveInOpen(&wi, WAVE_MAPPER, &wfx, NULL, NULL, CALLBACK_NULL | WAVE_FORMAT_DIRECT) != MMSYSERR_NOERROR) {
        std::cerr << "Error al abrir el dispositivo de grabación" << std::endl;
        return;
    }

    // Crear buffers de audio
    char buffers[NUM_BUFFERS][BUFFER_SIZE];
    WAVEHDR headers[NUM_BUFFERS] = {};

    for (int i = 0; i < NUM_BUFFERS; ++i) {
        headers[i].lpData = buffers[i];
        headers[i].dwBufferLength = BUFFER_SIZE;
        waveInPrepareHeader(wi, &headers[i], sizeof(headers[i]));
        waveInAddBuffer(wi, &headers[i], sizeof(headers[i]));
    }

    // Iniciar grabación
    if (waveInStart(wi) != MMSYSERR_NOERROR) {
        std::cerr << "Error al iniciar la grabación" << std::endl;
        waveInClose(wi);
        return;
    }

    //Real time
    // 
    // Bucle principal de captura y envío de audio
    while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) {
        for (auto& h : headers) {
            if (h.dwFlags & WHDR_DONE) {
                // Enviar datos de audio al servidor
                //send(clientSocket, h.lpData, h.dwBufferLength, 0);

                // Preparar y reutilizar buffer
                h.dwFlags = 0;
                h.dwBytesRecorded = 0;
                waveInPrepareHeader(wi, &h, sizeof(h));
                waveInAddBuffer(wi, &h, sizeof(h));
            }
        }
    }

    // Detener y limpiar la grabación
    waveInStop(wi);
    for (auto& h : headers) {
        waveInUnprepareHeader(wi, &h, sizeof(h));
    }
    waveInClose(wi);
    */
}