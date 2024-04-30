#include <dshow.h>
#include <atlconv.h>

#include <iomanip>
#include <iostream>
#include <system_error>

// std::cout << std::system_category().message(hr) << '\n';

GUID CLSID_LAVVideoDecoder               = { 0xEE30215D, 0x164F, 0x4A92, {0xA4, 0xEB, 0x9D, 0x4C, 0x13, 0x39, 0x0F, 0x9F} };
GUID CLSID_LAVAudioDecoder               = { 0xE8E73B6B, 0x4CB3, 0x44A4, {0xBE, 0x99, 0x4F, 0x7B, 0xCB, 0x96, 0xE4, 0x91} };

int ShowStreamInfo(IGraphBuilder* pGraph)
{
    HRESULT hr;

    IBasicVideo* pBasicVideo = NULL;
    hr = pGraph->QueryInterface(IID_IBasicVideo, (void**)&pBasicVideo);

    long lVideoWidth = 0, lVideoHeight = 0;
    pBasicVideo->get_VideoWidth(&lVideoWidth);
    pBasicVideo->get_VideoHeight(&lVideoHeight);

    REFTIME dbAvgTimePerFrame = 0.0;
    pBasicVideo->get_AvgTimePerFrame(&dbAvgTimePerFrame);
    double dbFrameRate = 1.0 / dbAvgTimePerFrame;

    std::cout << "Video\n";
    std::cout << "Width\t\t\t" << lVideoWidth << " pixels\n";
    std::cout << "Height\t\t\t" << lVideoHeight << " pixels\n";
    std::cout << "Frame rate\t\t" << dbFrameRate << " FPS\n";

    IMediaSeeking* pMediaSeeking = NULL;
    hr = pGraph->QueryInterface(IID_IMediaSeeking, (void**)&pMediaSeeking);

    LONGLONG llDuration = 0;
    pMediaSeeking->GetDuration(&llDuration);
    std::cout << "Duration\t\t" << llDuration / 10000000.0 << "s\n";

    return 0;
}

int ShowFilters(IGraphBuilder* pGraph)
{
    std::cout << "Filter\n";

    USES_CONVERSION;

    IEnumFilters* pEnumFilters = NULL;
    if (FAILED(pGraph->EnumFilters(&pEnumFilters))) {
        pEnumFilters->Release();
        return -1;
    }
    pEnumFilters->Reset();

    IBaseFilter* pBaseFilter = NULL;
    ULONG ulCount = 0;
    while (SUCCEEDED(pEnumFilters->Next(1, &pBaseFilter, &ulCount)) && ulCount) {
        if (!pBaseFilter)
            continue;
        FILTER_INFO FilterInfo;
        if (FAILED(pBaseFilter->QueryFilterInfo(&FilterInfo)))
            continue;
        std::cout << W2A(FilterInfo.achName) << '\n';
        pBaseFilter->Release();
    }
    pEnumFilters->Release();

    return 0;
}

HRESULT AddFilterByCLSID(IGraphBuilder* pGraph, const GUID& clsid, LPCWCHAR wszName, IBaseFilter** ppF)
{
    if (!pGraph || !ppF)
        return E_POINTER;
    *ppF = NULL;
    IBaseFilter* pF = NULL;
    HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, reinterpret_cast<void**>(&pF));
    if (SUCCEEDED(hr)) {
        hr = pGraph->AddFilter(pF, wszName);
        if (SUCCEEDED(hr))
            *ppF = pF;
        else
            pF->Release();
    }
    return hr;
}

int main()
{
    HRESULT hr;

    // Initialize the COM lirary
    hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printf("ERROR - Could not initialize COM library");
        return 1;
    }

    // Create the filter graph manager and query for interfaces.
    IGraphBuilder* pGraph = NULL;
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void**)&pGraph);
    if (FAILED(hr)) {
        printf("ERROR - Could not create the Filter Graph Manager.");
        return 1;
    }

    IMediaControl* pControl = NULL;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void**)&pControl);

    IMediaEvent* pEvent = NULL;
    hr = pGraph->QueryInterface(IID_IMediaEvent, (void**)&pEvent);

    // Build the graph.
    IBaseFilter* pLAVVideoDecoder = NULL;
    IBaseFilter* pLAVAudioDecoder = NULL;
    AddFilterByCLSID(pGraph, CLSID_LAVVideoDecoder, L"LAV Video Decoder", &pLAVVideoDecoder);
    AddFilterByCLSID(pGraph, CLSID_LAVAudioDecoder, L"LAV Audio Decoder", &pLAVAudioDecoder);
    hr = pGraph->RenderFile(L"D:\\resources\\Forrest_Gump_IMAX.mp4", NULL);

    ShowStreamInfo(pGraph);
    std::cout << '\n';
    ShowFilters(pGraph);

    if (SUCCEEDED(hr))
    {
        // Run the graph.
        hr = pControl->Run();
        if (SUCCEEDED(hr))
        {
            // Wait for completion.
            long evCode;
            pEvent->WaitForCompletion(INFINITE, &evCode);

            // Note: Do not use INFINITE in a real application, because it
            // can block indefinitely.
        }
    }

    pControl->Release();
    pEvent->Release();
    pGraph->Release();
    CoUninitialize();

    return 0;
}
