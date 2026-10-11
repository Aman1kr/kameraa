// Kameraa DirectShow virtual-camera source filter (consumer side).
//
// This is a COM in-proc server (DLL) that exposes a video capture source named
// "Kameraa Virtual Camera". Apps (Zoom/Teams/Meet/OBS/browsers using DirectShow)
// list it like a webcam. Each requested frame is read from the shared-memory
// channel that the Kameraa app writes to (see SharedFrameProtocol.h).
//
// BUILD REQUIREMENT: this filter derives from the DirectShow Base Classes
// (strmbase / "baseclasses" — CSource, CSourceStream), which ship as source with
// the Windows SDK samples and must be compiled into a static lib (strmbase.lib)
// and linked here. That dependency is why this target is OFF by default; see
// virtualcam/CMakeLists.txt and docs/10-BUILD-AND-PACKAGING.md.
//
// The frame plumbing below (FillBuffer / GetMediaType / shared-memory read) is
// complete; the surrounding base-class wiring compiles once strmbase is present.

#include <streams.h>  // DirectShow base classes (strmbase)

#include <windows.h>

#include <cstring>

#include "SharedFrameProtocol.h"

using namespace kameraa_vcam;

// {4B2A7E10-6C3D-4E8F-9A1B-2C5D7E9F0A33} — must match VirtualCamShared.h kFilterClsid.
// clang-format off
static const GUID CLSID_KameraaVCam =
    { 0x4b2a7e10, 0x6c3d, 0x4e8f, { 0x9a, 0x1b, 0x2c, 0x5d, 0x7e, 0x9f, 0x0a, 0x33 } };
// clang-format on

// Maps the shared-memory channel and serves the latest frame.
class SharedChannelReader {
public:
    ~SharedChannelReader() { close(); }

    bool open() {
        if (view_) return true;
        mapping_ = ::OpenFileMappingA(FILE_MAP_READ, FALSE, kChannelName);
        if (!mapping_) return false;
        view_ = ::MapViewOfFile(mapping_, FILE_MAP_READ, 0, 0, 0);
        return view_ != nullptr;
    }

    void close() {
        if (view_) { ::UnmapViewOfFile(view_); view_ = nullptr; }
        if (mapping_) { ::CloseHandle(mapping_); mapping_ = nullptr; }
    }

    // Copies the current frame as BGR24 into dst (expected w*h*3). Returns false
    // if no valid frame is available yet.
    bool readBgr(BYTE* dst, int w, int h) {
        if (!view_ && !open()) return false;
        const auto* hdr = static_cast<const SharedFrameHeader*>(view_);
        if (hdr->magic != kMagic || hdr->sequence == 0) return false;
        if (static_cast<int>(hdr->width) != w || static_cast<int>(hdr->height) != h)
            return false;
        const BYTE* src = static_cast<const BYTE*>(view_) + sizeof(SharedFrameHeader);
        std::memcpy(dst, src, static_cast<size_t>(w) * h * 3);
        return true;
    }

private:
    HANDLE mapping_ = nullptr;
    void* view_ = nullptr;
};

// One output pin producing RGB24 frames at a fixed resolution/fps.
class KameraaStream : public CSourceStream {
public:
    KameraaStream(HRESULT* phr, CSource* pParent)
        : CSourceStream(NAME("Kameraa Stream"), phr, pParent, L"Out") {}

    HRESULT GetMediaType(CMediaType* pmt) override {
        VIDEOINFOHEADER* vih =
            reinterpret_cast<VIDEOINFOHEADER*>(pmt->AllocFormatBuffer(sizeof(VIDEOINFOHEADER)));
        ZeroMemory(vih, sizeof(VIDEOINFOHEADER));
        vih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        vih->bmiHeader.biWidth = kDefaultWidth;
        vih->bmiHeader.biHeight = kDefaultHeight;
        vih->bmiHeader.biPlanes = 1;
        vih->bmiHeader.biBitCount = 24;
        vih->bmiHeader.biCompression = BI_RGB;
        vih->bmiHeader.biSizeImage =
            kDefaultWidth * kDefaultHeight * 3;
        vih->AvgTimePerFrame = 10000000LL / kDefaultFps;
        pmt->SetType(&MEDIATYPE_Video);
        pmt->SetFormatType(&FORMAT_VideoInfo);
        pmt->SetTemporalCompression(FALSE);
        pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
        pmt->SetSampleSize(vih->bmiHeader.biSizeImage);
        return S_OK;
    }

    HRESULT DecideBufferSize(IMemAllocator* pAlloc,
                             ALLOCATOR_PROPERTIES* pProps) override {
        pProps->cBuffers = 2;
        pProps->cbBuffer = kDefaultWidth * kDefaultHeight * 3;
        ALLOCATOR_PROPERTIES actual;
        HRESULT hr = pAlloc->SetProperties(pProps, &actual);
        if (FAILED(hr)) return hr;
        return (actual.cbBuffer < pProps->cbBuffer) ? E_FAIL : S_OK;
    }

    HRESULT FillBuffer(IMediaSample* pSample) override {
        BYTE* data = nullptr;
        pSample->GetPointer(&data);
        const long len = pSample->GetSize();
        const long need = kDefaultWidth * kDefaultHeight * 3;
        if (len < need) return E_FAIL;

        // DirectShow RGB24 is bottom-up; the app writes top-down BGR, which maps
        // to DirectShow's expected byte order after a vertical flip.
        if (!reader_.readBgr(scratch_, kDefaultWidth, kDefaultHeight)) {
            ZeroMemory(data, need);  // no frame yet -> black
        } else {
            const int rowBytes = kDefaultWidth * 3;
            for (int y = 0; y < kDefaultHeight; ++y) {
                std::memcpy(data + y * rowBytes,
                            scratch_ + (kDefaultHeight - 1 - y) * rowBytes, rowBytes);
            }
        }

        // Pace to target fps.
        REFERENCE_TIME now = frame_ * (10000000LL / kDefaultFps);
        REFERENCE_TIME end = now + (10000000LL / kDefaultFps);
        pSample->SetTime(&now, &end);
        pSample->SetSyncPoint(TRUE);
        ++frame_;
        return S_OK;
    }

private:
    SharedChannelReader reader_;
    BYTE scratch_[kMaxWidth * kMaxHeight * 3] = {};
    LONGLONG frame_ = 0;
};

// The filter: a CSource hosting the single output stream.
class KameraaSource : public CSource {
public:
    static CUnknown* WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT* phr) {
        return new KameraaSource(lpunk, phr);
    }

private:
    KameraaSource(LPUNKNOWN lpunk, HRESULT* phr)
        : CSource(NAME("Kameraa Virtual Camera"), lpunk, CLSID_KameraaVCam) {
        m_paStreams = new CSourceStream*[1];
        m_paStreams[0] = new KameraaStream(phr, this);
    }
};

// --- COM registration tables -------------------------------------------------
const AMOVIESETUP_MEDIATYPE kPinTypes = {&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24};
const AMOVIESETUP_PIN kPin = {L"Out",  FALSE, TRUE,  FALSE, FALSE,
                              &CLSID_NULL, nullptr, 1, &kPinTypes};
const AMOVIESETUP_FILTER kFilter = {&CLSID_KameraaVCam, L"Kameraa Virtual Camera",
                                    MERIT_DO_NOT_USE, 1, &kPin};

CFactoryTemplate g_Templates[] = {
    {L"Kameraa Virtual Camera", &CLSID_KameraaVCam, KameraaSource::CreateInstance,
     nullptr, &kFilter}};
int g_cTemplates = sizeof(g_Templates) / sizeof(g_Templates[0]);

// Register/unregister the filter under the video input device category so that
// apps (Zoom/Teams/Meet/OBS/browsers) list it as a selectable camera.
static HRESULT RegisterInVideoCategory(bool reg) {
    HRESULT hr = CoInitialize(nullptr);
    const bool didInit = SUCCEEDED(hr);

    IFilterMapper2* mapper = nullptr;
    hr = CoCreateInstance(CLSID_FilterMapper2, nullptr, CLSCTX_INPROC_SERVER,
                          IID_IFilterMapper2, reinterpret_cast<void**>(&mapper));
    if (SUCCEEDED(hr) && mapper) {
        if (reg) {
            REGPINTYPES pinType = {&MEDIATYPE_Video, &MEDIASUBTYPE_RGB24};
            REGFILTERPINS pin = {};
            pin.strName = const_cast<LPWSTR>(L"Out");
            pin.bRendered = FALSE;
            pin.bOutput = TRUE;
            pin.bZero = FALSE;
            pin.bMany = FALSE;
            pin.clsConnectsToFilter = &CLSID_NULL;
            pin.strConnectsToPin = nullptr;
            pin.nMediaTypes = 1;
            pin.lpMediaType = &pinType;

            REGFILTER2 rf2 = {};
            rf2.dwVersion = 1;
            rf2.dwMerit = MERIT_DO_NOT_USE;
            rf2.cPins = 1;
            rf2.rgPins = &pin;

            IMoniker* moniker = nullptr;
            hr = mapper->RegisterFilter(CLSID_KameraaVCam, L"Kameraa Virtual Camera",
                                        &moniker, &CLSID_VideoInputDeviceCategory,
                                        nullptr, &rf2);
            if (moniker) moniker->Release();
        } else {
            mapper->UnregisterFilter(&CLSID_VideoInputDeviceCategory, nullptr,
                                     CLSID_KameraaVCam);
            hr = S_OK;
        }
        mapper->Release();
    }
    if (didInit) CoUninitialize();
    return hr;
}

STDAPI DllRegisterServer() {
    HRESULT hr = AMovieDllRegisterServer2(TRUE);
    if (FAILED(hr)) return hr;
    return RegisterInVideoCategory(true);
}

STDAPI DllUnregisterServer() {
    RegisterInVideoCategory(false);
    return AMovieDllRegisterServer2(FALSE);
}

extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE, ULONG, LPVOID);
BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved) {
    return DllEntryPoint(static_cast<HINSTANCE>(hModule), dwReason, lpReserved);
}
