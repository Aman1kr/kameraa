#pragma once

#include "core/Interfaces.h"
#include "output/SharedFrameChannel.h"

namespace kameraa {

// macOS virtual camera (app side). Writes processed frames into a POSIX shared-
// memory channel consumed by the Kameraa Camera System Extension (CMIOExtension).
// Availability is determined by a sentinel the installer writes once the extension
// is installed & approved. See docs/05-PLATFORM-INTEGRATION.md and macos/.
class CmioExtensionSink : public IVirtualCamera {
public:
    std::string backendName() const override { return "CMIOExtension"; }
    bool isAvailable() const override;
    std::string availabilityHint() const override;
    bool start(const OutputConfig& cfg) override;
    bool pushFrame(const Frame& frame) override;
    void stop() override;

private:
    SharedFrameChannel channel_;
};

}  // namespace kameraa
