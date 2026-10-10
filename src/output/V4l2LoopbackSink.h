#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "core/Interfaces.h"

namespace kameraa {

// Writes processed frames to a v4l2loopback virtual device (/dev/videoN) so any
// V4L2 app (Zoom/Meet/OBS/browsers) sees "Kameraa" as a webcam.
// See docs/05-PLATFORM-INTEGRATION.md.
class V4l2LoopbackSink : public IVirtualCamera {
public:
    ~V4l2LoopbackSink() override;

    std::string backendName() const override { return "v4l2loopback"; }
    bool isAvailable() const override;
    std::string availabilityHint() const override;
    bool start(const OutputConfig& cfg) override;
    bool pushFrame(const Frame& frame) override;
    void stop() override;

private:
    // Returns the first loopback output device path, or empty if none found.
    static std::string findLoopbackDevice();
    bool configureFormat(int width, int height);

    int fd_ = -1;
    int width_ = 0;
    int height_ = 0;
    std::string devicePath_;
    std::vector<uint8_t> yuyv_;  // reusable conversion buffer
};

}  // namespace kameraa
