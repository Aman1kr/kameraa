#pragma once

#include "core/Interfaces.h"

namespace kameraa {

// Synthetic capture source: generates an animated test pattern. Used for headless
// tests/benchmarks and as a fallback when no camera is available (FR-C5).
class TestPatternSource : public ICaptureSource {
public:
    std::vector<CameraInfo> enumerate() override;
    bool open(const CaptureConfig& cfg) override;
    bool read(Frame& out) override;
    void close() override;
    bool isOpen() const override { return open_; }

private:
    bool open_ = false;
    int width_ = 1280;
    int height_ = 720;
    int frame_ = 0;
};

}  // namespace kameraa
