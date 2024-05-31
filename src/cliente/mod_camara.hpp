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

struct EncodingParameters
{
    GUID    subtype;
    UINT32  bitrate;
};

class mod_Camera {
	public:
        mod_Camera() { CoInitialize(nullptr);}
        ~mod_Camera() { CoUninitialize();}

        std::vector<std::string> ListCaptureDevices();
        HRESULT Init(IMFActivate*& pDevice);
        BYTE* GetFrame();

	private:

};


#endif