#ifndef __CAM_H
#define __CAM_H
#include "headers.hpp"
#include<cwchar>
#include<mfapi.h>
#include<mfidl.h>
#include<mfobjects.h>
#include<mfreadwrite.h>
#include<mferror.h>
#include<mftransform.h>
#include<Wmcodecdsp.h>
#include<propvarutil.h>
#include<shlwapi.h>
#include<gdiplus.h>

template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

struct camOBJ {
    bool isActivated;
    bool isLive;
    bool isConvert;
    IMFSourceReader* sReader;
    IMFMediaSource* sSource;
    IMFActivate* sActivate;
    UINT32 width = 0;
    UINT32 height = 0;
    UINT32 numerator = 0;
    UINT32 denominator = 0;
    GUID mediaType = { 0 };
    
    void ReleaseCam() {
        isActivated = isLive = isConvert = false;
        if (sActivate) {
            sActivate->ShutdownObject();
        }
        SafeRelease(&sReader);
        SafeRelease(&sSource);    
    }
};

class mod_Camera {
	public:
        mod_Camera() { 
            CoInitialize(nullptr);
            MFStartup(MF_VERSION);
        }

        ~mod_Camera() { 
            for (int iThNum = 0; iThNum < MAX_CAMS; iThNum++) {
                if (this->thLive[iThNum].joinable()) {
                    this->thLive[iThNum].join();
                }
            }
            for (int iCount = 0; iCount<int(this->vcCamObjs.size()); iCount++) {
                this->vcCamObjs[iCount].ReleaseCam();
            }

            CoUninitialize();
            MFShutdown();
        }

        HRESULT ListCaptureDevices(std::vector<IMFActivate*>& devices);
        std::vector<std::string> ListNameCaptureDevices();
        std::vector<struct camOBJ> vcCamObjs;

        HRESULT Init(IMFActivate*& pDevice, int pIndexDev);
        HRESULT OpenMediaSource(IMFMediaSource*& pSource, IMFSourceReader*& pReader);
        HRESULT ConfigureCapture(IMFSourceReader*& pReader, int pIndexDev);
        HRESULT ConfigureSourceReader(IMFSourceReader*& pReader, int pIndexDev);
        std::vector<BYTE> GetFrame(int pIndexDev);
        void LiveCam(int pIndexDev);
        void SpawnLive(int pIndexDev);
        void JoinLiveThread(int pIndexDev);

        //Conversion to JPEG
        int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
        std::vector<BYTE> bmpHeader(LONG lWidth, LONG lHeight, WORD wBitsPerPixel, const unsigned long& padding_size, DWORD iBuffersize);
        std::vector<BYTE> toJPEG(const BYTE* bmpBuffer, u_int uiBuffersize);
        
        
	private:
        std::thread thLive[MAX_CAMS];
};

#endif
