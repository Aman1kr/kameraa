#pragma once

#include <string>
#include <utility>

#include "core/Interfaces.h"

namespace kameraa {

// Placeholder virtual-camera backend for platforms whose native sink is not yet
// implemented (Windows DirectShow = Phase 4, macOS CMIOExtension = Phase 5).
// Always reports unavailable and explains why, so the GUI can guide the user.
class NullVirtualCamera : public IVirtualCamera {
public:
    NullVirtualCamera(std::string backendName, std::string hint)
        : backendName_(std::move(backendName)), hint_(std::move(hint)) {}

    std::string backendName() const override { return backendName_; }
    bool isAvailable() const override { return false; }
    std::string availabilityHint() const override { return hint_; }
    bool start(const OutputConfig&) override { return false; }
    bool pushFrame(const Frame&) override { return false; }
    void stop() override {}

private:
    std::string backendName_;
    std::string hint_;
};

}  // namespace kameraa
