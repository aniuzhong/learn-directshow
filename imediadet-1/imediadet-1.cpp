// imediadet-1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <dshow.h>

#include <iostream>

extern "C" const CLSID CLSID_MediaDet;
extern "C" const IID IID_IMediaDet;
struct ISampleGrabber;

struct __declspec(uuid("65BD0710-24D2-4ff7-9324-ED2E5D3ABAFA")) __declspec(novtable) IMediaDet : public IUnknown
{
public:
    virtual  HRESULT __stdcall get_Filter(IUnknown **pVal) = 0;
    virtual  HRESULT __stdcall put_Filter(IUnknown *newVal) = 0;
    virtual  HRESULT __stdcall get_OutputStreams(long *pVal) = 0;
    virtual  HRESULT __stdcall get_CurrentStream(long *pVal) = 0;
    virtual  HRESULT __stdcall put_CurrentStream(long newVal) = 0;
    virtual  HRESULT __stdcall get_StreamType(GUID *pVal) = 0;
    virtual  HRESULT __stdcall get_StreamTypeB(BSTR *pVal) = 0;
    virtual  HRESULT __stdcall get_StreamLength(double *pVal) = 0;
    virtual  HRESULT __stdcall get_Filename(BSTR *pVal) = 0;
    virtual  HRESULT __stdcall put_Filename(BSTR newVal) = 0;
    virtual  HRESULT __stdcall GetBitmapBits(double StreamTime, long *pBufferSize, char *pBuffer, long Width, long Height) = 0;
    virtual  HRESULT __stdcall WriteBitmapBits(double StreamTime, long Width, long Height, BSTR Filename) = 0;
    virtual  HRESULT __stdcall get_StreamMediaType(AM_MEDIA_TYPE *pVal) = 0;
    virtual  HRESULT __stdcall GetSampleGrabber(ISampleGrabber **ppVal) = 0;
    virtual  HRESULT __stdcall get_FrameRate(double *pVal) = 0;
    virtual  HRESULT __stdcall EnterBitmapGrabMode(double SeekTime) = 0;

protected:
    ~IMediaDet() {}
};

extern "C" const IID IID_ISampleGrabberCB;

struct __declspec(uuid("0579154A-2B53-4994-B0D0-E773148EFF85")) __declspec(novtable) ISampleGrabberCB : public IUnknown
{
public:
    virtual HRESULT __stdcall SampleCB(double SampleTime, IMediaSample *pSample) = 0;
    virtual HRESULT __stdcall BufferCB(double SampleTime, BYTE *pBuffer, long BufferLen) = 0;

protected:
    ~ISampleGrabberCB() {}
};

extern "C" const IID IID_ISampleGrabber;
struct __declspec(uuid("6B652FFF-11FE-4fce-92AD-0266B5D7C78F")) __declspec(novtable) ISampleGrabber : public IUnknown
{
public:
    virtual HRESULT __stdcall SetOneShot(BOOL OneShot) = 0;
    virtual HRESULT __stdcall SetMediaType(const AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT __stdcall GetConnectedMediaType(AM_MEDIA_TYPE *pType) = 0;
    virtual HRESULT __stdcall SetBufferSamples(BOOL BufferThem) = 0;
    virtual HRESULT __stdcall GetCurrentBuffer(long *pBufferSize, long *pBuffer) = 0;
    virtual HRESULT __stdcall GetCurrentSample(IMediaSample **ppSample) = 0;
    virtual HRESULT __stdcall SetCallback(ISampleGrabberCB *pCallback, long WhichMethodToCallback) = 0;

protected:
    ~ISampleGrabber() {}
};

int main(int argc, char* argv[])
{
    if (argc < 2)
        return 1;

    BSTR bstrFilename = SysAllocStringLen(nullptr, static_cast<UINT>(strlen(argv[1])));
    if (!bstrFilename)
    {
        printf("ERROR - Failed to allocate BSTR");
        return 1;
    }

    MultiByteToWideChar(CP_ACP, 0, argv[1], -1, bstrFilename, static_cast<int>(strlen(argv[1])));

    HRESULT hr;

    // Initialize the COM lirary
    hr = CoInitialize(NULL);
    if (FAILED(hr))
    {
        printf("ERROR - Could not initialize COM library");
        return 1;
    }

    // Create the filter graph manager and query for interfaces.
    IMediaDet* pMediaDet = NULL;
    hr = CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC_SERVER, IID_IMediaDet, (void**)&pMediaDet);
    if (FAILED(hr))
    {
        printf("ERROR - Could not create the Filter Graph Manager");
        return 1;
    }
    
    hr = pMediaDet->put_Filename(bstrFilename);
    if (FAILED(hr))
    {
        printf("ERROR - Could not put filename in the Media Detector.");
        return 1;
    }

    double dbFrameRate;
    pMediaDet->get_FrameRate(&dbFrameRate);
    if (FAILED(hr))
    {
        printf("ERROR - get_FrameRate.");
        return 1;
    }
    std::cout << "get_FrameRate - " << dbFrameRate << std::endl;

    long nStreamCount;
    hr = pMediaDet->get_OutputStreams(&nStreamCount);
    if (FAILED(hr))
    {
        printf("ERROR - get_OutputStreams.");
        return 1;
    }
    std::cout << "get_OutputStreams - " << nStreamCount << std::endl;

    for (long n = 0; n < nStreamCount; ++n)
    {
        GUID aMajorType;

        if (SUCCEEDED(pMediaDet->put_CurrentStream(n)) &&
            SUCCEEDED(pMediaDet->get_StreamType(&aMajorType)))
        {
            if (aMajorType == MEDIATYPE_Video)
            {
                std::cout << "MEDIATYPE_Video - Found, index " << n << std::endl;
            }
            else if (aMajorType == MEDIATYPE_Audio)
            {
                std::cout << "MEDIATYPE_Audio - Found, index " << n << std::endl;
            }
        }
    }

    double fLength;
    hr = pMediaDet->get_StreamLength(&fLength);
    if (FAILED(hr))
    {
        printf("ERROR - get_StreamLength.");
        return 1;
    }
    std::cout << "get_StreamLength - " << fLength << std::endl;

    long nCurrentStream = -1;
    pMediaDet->put_CurrentStream(0);
    pMediaDet->get_CurrentStream(&nCurrentStream);
    std::cout << "get_CurrentStream - " << nCurrentStream << std::endl;

    AM_MEDIA_TYPE aMediaType;
    hr = pMediaDet->get_StreamMediaType(&aMediaType);
    if (FAILED(hr))
    {
        printf("ERROR - get_StreamMediaType.");
        return 1;
    }

    LONG nWidth = 0, nHeight = 0;
    if ((aMediaType.formattype == FORMAT_VideoInfo) &&
        (aMediaType.cbFormat >= sizeof(VIDEOINFOHEADER)))
    {
        VIDEOINFOHEADER* pVih = reinterpret_cast<VIDEOINFOHEADER*>(aMediaType.pbFormat);

        nWidth = pVih->bmiHeader.biWidth;
        nHeight = pVih->bmiHeader.biHeight;
    }
    std::cout << "Width - " << nWidth << std::endl;
    std::cout << "Height - " << nHeight << std::endl;

    SysFreeString(bstrFilename);
    pMediaDet->Release();
    CoUninitialize();

    return 0;
}

