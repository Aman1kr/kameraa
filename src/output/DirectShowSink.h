#pragma once

#include "core/Interfaces.h"
#include "output/SharedFrameChannel.h"

namespace kameraa {

// Windows virtual camera (app side). Writes processed frames into a shared-memory
// channel consumed by the registered DirectShow source filter DLL. Availability is
// determined by whether that filter's CLSID is registered. See
// docs/05-PLATFORM-INTEGRATION.md.
class DirectShowSink : public IVirtualCamera {
public:
    std::string backendName() const override { return "DirectShow"; }
    bool isAvailable() const override;
    std::string availabilityHint() const override;
    bool start(const OutputConfig& cfg) override;
    bool pushFrame(const Frame& frame) override;
    void stop() override;

private:
    SharedFrameChannel channel_;
};

}  // namespace kameraa
