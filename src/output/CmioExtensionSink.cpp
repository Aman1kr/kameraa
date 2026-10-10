#include "output/CmioExtensionSink.h"

#include "output/VirtualCamShared.h"

#if defined(__APPLE__)

#include <sys/stat.h>

namespace kameraa {

bool CmioExtensionSink::isAvailable() const {
    struct stat st {};
    return ::stat(vcam::kMacExtensionSentinel, &st) == 0;
}

std::string CmioExtensionSink::availabilityHint() const {
    return "Kameraa Camera Extension is not installed/approved. Install the Kameraa "
           "app to /Applications and launch it to activate the system extension, then "
           "approve it in System Settings > Privacy & Security. See macos/README notes "
           "and docs/05-PLATFORM-INTEGRATION.md.";
}

bool CmioExtensionSink::start(const OutputConfig& /*cfg*/) {
    if (!isAvailable()) return false;
    return channel_.create(vcam::kPosixChannelName, vcam::kMaxWidth, vcam::kMaxHeight);
}

bool CmioExtensionSink::pushFrame(const Frame& frame) {
    return channel_.write(frame);
}

void CmioExtensionSink::stop() { channel_.close(); }

}  // namespace kameraa

#else  // Non-macOS: never available.

namespace kameraa {

bool CmioExtensionSink::isAvailable() const { return false; }
std::string CmioExtensionSink::availabilityHint() const {
    return "CMIOExtension virtual camera is only available on macOS.";
}
bool CmioExtensionSink::start(const OutputConfig&) { return false; }
bool CmioExtensionSink::pushFrame(const Frame&) { return false; }
void CmioExtensionSink::stop() {}

}  // namespace kameraa

#endif
