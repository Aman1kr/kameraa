#include "output/DirectShowSink.h"

#include "output/VirtualCamShared.h"

#if defined(_WIN32)

#include <windows.h>

namespace kameraa {

namespace {
// True if HKEY_CLASSES_ROOT\CLSID\{clsid} exists (filter DLL registered).
bool clsidRegistered(const char* clsid) {
    std::string key = std::string("CLSID\\") + clsid;
    HKEY h = nullptr;
    LONG r = ::RegOpenKeyExA(HKEY_CLASSES_ROOT, key.c_str(), 0, KEY_READ, &h);
    if (r == ERROR_SUCCESS) {
        ::RegCloseKey(h);
        return true;
    }
    return false;
}
}  // namespace

bool DirectShowSink::isAvailable() const {
    return clsidRegistered(vcam::kFilterClsid);
}

std::string DirectShowSink::availabilityHint() const {
    return "Kameraa virtual camera filter is not registered. Run the Kameraa "
           "installer, or register the filter manually (as Administrator):\n"
           "  regsvr32 kameraa_vcam.dll\n"
           "Then restart your video app so it re-scans cameras.";
}

bool DirectShowSink::start(const OutputConfig& /*cfg*/) {
    if (!isAvailable()) return false;
    return channel_.create(vcam::kChannelName, vcam::kMaxWidth, vcam::kMaxHeight);
}

bool DirectShowSink::pushFrame(const Frame& frame) {
    return channel_.write(frame);
}

void DirectShowSink::stop() { channel_.close(); }

}  // namespace kameraa

#else  // Non-Windows: never available.

namespace kameraa {

bool DirectShowSink::isAvailable() const { return false; }
std::string DirectShowSink::availabilityHint() const {
    return "DirectShow virtual camera is only available on Windows.";
}
bool DirectShowSink::start(const OutputConfig&) { return false; }
bool DirectShowSink::pushFrame(const Frame&) { return false; }
void DirectShowSink::stop() {}

}  // namespace kameraa

#endif
