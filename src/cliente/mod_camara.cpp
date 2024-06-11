#include "mod_camara.hpp"

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

std::vector<char*> mod_Camera::ListNameCaptureDevices() {
    std::vector<char*> vcDevices;
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
                        vcDevices.push_back(cBuffer);
                    }
                    CoTaskMemFree(szFriendlyName);
                    ppDevices[i]->Release();
                }

                CoTaskMemFree(ppDevices);
            }
        }
        pAttributes->Release();
    }
    return vcDevices;
}

HRESULT mod_Camera::ConfigureSourceReader(IMFSourceReader* pReader)
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

    hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &this->width, &this->height);

done:

    SafeRelease(&pType);
    return hr;
}

HRESULT mod_Camera::ConfigureCapture(){
    HRESULT hr = S_OK;

    IMFMediaType* pType = NULL;

    hr = ConfigureSourceReader(this->m_pReader);

    if (SUCCEEDED(hr)){
        hr = this->m_pReader->GetCurrentMediaType(
            (DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
            &pType
        );
    }

    SafeRelease(&pType);
    return hr;
}

HRESULT mod_Camera::OpenMediaSource(IMFMediaSource* pSource){
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
            &this->m_pReader
        );
    }


    SafeRelease(&pAttributes);
    return hr;
}

HRESULT mod_Camera::Init(IMFActivate*& pDevice) {
    HRESULT hr = S_OK;
    hr = pDevice->ActivateObject(__uuidof(IMFMediaSource), (void**)&pSource);
    
    //Obtener link simbolico para administrar el dispositivo
    if (SUCCEEDED(hr)) {
        hr = pDevice->GetAllocatedString(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
            &this->m_pwszSymbolicLink,
            NULL);
    }

    if (SUCCEEDED(hr)){
        hr = OpenMediaSource(pSource);
    }

    if (SUCCEEDED(hr)){
        hr = ConfigureCapture();
    }

    return hr;
}

BYTE* mod_Camera::GetFrame() {
    //Liberar buffer retornado despues de haberse usado
    
    BYTE* cBufferOut = NULL;
    DWORD streamIndex, flags;
    LONGLONG llTimeStamp;
    IMFSample* pSample = NULL;
    HRESULT hr;
    //Hay que hacer un loop hasta que tome el frame
    while (1) {
        hr = this->m_pReader->ReadSample(
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
                cBufferOut = new BYTE[currentLength];
                memcpy(cBufferOut, pData, currentLength);

                //SaveBitmapToFile(cBuffern, width, height, 24, 0, L"captura.bmp", currentLength);

                pBuffer->Unlock();
            }
            pBuffer->Release();
        }
        pSample->Release();
    }

    return cBufferOut;
}