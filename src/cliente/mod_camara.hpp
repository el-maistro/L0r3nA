#ifndef __CAM_H
#define __CAM_H
#include "headers.hpp"
#include <initguid.h> 
#include "mod_dynamic_load.hpp"

//// CLSID_CColorConvertDMO {98230571-0087-4204-b020-3282538e57d3}

DEFINE_GUID(CLSID_CColorConvertDMO,
    0x98230571, 0x0087, 0x4204, 0xb0, 0x20, 0x32, 0x82, 0x53, 0x8e, 0x57, 0xd3);

template <class T> void SafeRelease(T** ppT){
    if (*ppT){
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
        mod_Camera(st_GdiPlus& _gdiplus, st_Shlwapi& _shlwapi, st_Mfplat& _mfplat,
            st_Mf& _mf, st_Mfapi& _mfapi, st_Mfreadwrite& _mfreadwrite, st_Ole32& _ole32, st_Kernel32& _kernel32);
        ~mod_Camera();

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
        
        bool checkMod();
	private:
        std::thread thLive[MAX_CAMS];

        st_GdiPlus GDIPLUS;
        st_Kernel32 KERNEL32;
        st_Ole32 OLE32;
        st_Shlwapi SHLWAPI;
        st_Mfplat MFPLAT;
        st_Mf MF;
        st_Mfapi MFAPI;
        st_Mfreadwrite MFREADWRITE;

       
        bool isReady = false;
};

#endif
