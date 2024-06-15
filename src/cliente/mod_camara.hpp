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
            int iCount = 0;
            for (; iCount<int(this->vcCams.size()); iCount++) {
                this->vcCams[iCount]->Release();
            }

            for (iCount = 0; iCount<int(this->vc_pSource.size()); iCount++) {
                if (this->vc_pSource[iCount]) {
                    this->vc_pSource[iCount]->Release();
                }
            }

            for (iCount = 0; iCount<int(this->vc_pReader.size()); iCount++) {
                if (this->vc_pReader[iCount]) {
                    this->vc_pReader[iCount]->Release();
                }
            }

            CoUninitialize();
            MFShutdown();
            //CoTaskMemFree(m_pwszSymbolicLink);
        }

        struct CamRes Resolution() {
            struct CamRes tmp;
            tmp.height = this->height;
            tmp.width = this->width;
            return tmp;
        }

        HRESULT ListCaptureDevices(std::vector<IMFActivate*>& devices);
        std::vector<char*> ListNameCaptureDevices();
        std::vector<IMFActivate*> vcCams;
        std::vector<bool> vcActivated;

        HRESULT Init(IMFActivate*& pDevice, int pIndexDev);
        HRESULT OpenMediaSource(IMFMediaSource*& pSource, int pIndexDev);
        HRESULT ConfigureCapture(IMFSourceReader*& pReader);
        HRESULT ConfigureSourceReader(IMFSourceReader*& pReader);

        //Conversion to JPEG
        int GetEncoderClsid(const WCHAR* format, CLSID* pClsid);
        BYTE* bmpHeader(LONG lWidth, LONG lHeight, WORD wBitsPerPixel, const unsigned long& padding_size, DWORD iBuffersize, unsigned int& iBuffsizeOut);
        BYTE* toJPEG(BYTE*& bmpBuffer, u_int uiBuffersize, u_int& uiOutBufferSize);
        BYTE* GetFrame(int& iBytesOut, int pIndexDev);

        
	private:
        std::vector<IMFSourceReader*> vc_pReader;
        std::vector<IMFMediaSource*>  vc_pSource;

        IMFSourceReader* m_pReader = NULL;
        IMFMediaSource* pSource = NULL;
        //WCHAR* m_pwszSymbolicLink;
        UINT32 width = 0;
        UINT32 height = 0;
};

#endif
