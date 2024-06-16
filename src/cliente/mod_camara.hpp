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
    IMFSourceReader* sReader;
    IMFMediaSource* sSource;
    IMFActivate* sActivate;
    UINT32 width = 0;
    UINT32 height = 0;
};

//reader
//source
//bool activated
//bool islive

class mod_Camera {
	public:
        mod_Camera() { 
            CoInitialize(nullptr);
            MFStartup(MF_VERSION);
        }

        ~mod_Camera() { 
            int iCount = 0;
            for (; iCount<int(this->vcCamObjs.size()); iCount++) {
                this->vcCamObjs[iCount].sActivate->Release();

                if ( this->vcCamObjs[iCount].sReader) {
                    this->vcCamObjs[iCount].sReader->Release();
                }
                if (this->vcCamObjs[iCount].sSource) {
                    this->vcCamObjs[iCount].sSource->Release();
                }
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
        BYTE* GetFrame(int& iBytesOut, int pIndexDev);
        void LiveCam(int pIndexDev);
        void SpawnLive(int pIndexDev);
        void JoinLiveThread(int pIndexDev);

        //Conversion to JPEG
        int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
        BYTE* bmpHeader(LONG lWidth, LONG lHeight, WORD wBitsPerPixel, const unsigned long& padding_size, DWORD iBuffersize, unsigned int& iBuffsizeOut);
        BYTE* toJPEG(BYTE*& bmpBuffer, u_int uiBuffersize, u_int& uiOutBufferSize);
        
        
	private:
        std::thread thLive;
};

#endif
