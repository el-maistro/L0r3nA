#ifndef __CAM_H
#define __CAM_H
#include "headers.hpp"
#include<cwchar>
#include<mfapi.h>
#include<mfidl.h>
#include<mfobjects.h>
#include<mfreadwrite.h>
#include<mferror.h>
#include<Wmcodecdsp.h>
#include<propvarutil.h>
#include<shlwapi.h>

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

struct CamRes {
    UINT32 width = 0;
    UINT32 height = 0;
};

class mod_Camera {
	public:
        mod_Camera() { 
            CoInitialize(nullptr);
            MFStartup(MF_VERSION);
        }
        ~mod_Camera() { 
            CoUninitialize();
            MFShutdown();
            CoTaskMemFree(m_pwszSymbolicLink);
        }

        struct CamRes Resolution() {
            struct CamRes tmp;
            tmp.height = this->height;
            tmp.width = this->width;
            return tmp;
        }

        HRESULT ListCaptureDevices(std::vector<IMFActivate*>& devices);
        std::vector<char*> ListNameCaptureDevices();

        HRESULT Init(IMFActivate*& pDevice);
        HRESULT OpenMediaSource(IMFMediaSource* pSource);
        HRESULT ConfigureCapture();
        HRESULT ConfigureSourceReader(IMFSourceReader* pReader);
        
        BYTE* GetFrame(int& iBytesOut);

	private:
        IMFSourceReader* m_pReader = NULL;
        IMFMediaSource* pSource = NULL;
        WCHAR* m_pwszSymbolicLink;
        UINT32 width = 0;
        UINT32 height = 0;
};

#endif
