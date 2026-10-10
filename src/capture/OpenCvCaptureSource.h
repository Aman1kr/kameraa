#pragma once

#include <opencv2/videoio.hpp>

#include "core/Interfaces.h"

namespace kameraa {

// Cross-platform capture via OpenCV VideoCapture (Phase 0 MVP).
// Native backends (Media Foundation / AVFoundation / V4L2) are added later.
class OpenCvCaptureSource : public ICaptureSource {
public:
    std::vector<CameraInfo> enumerate() override;
    bool open(const CaptureConfig& cfg) override;
    bool read(Frame& out) override;
    void close() override;
    bool isOpen() const override;

private:
    cv::VideoCapture cap_;
};

}  // namespace kameraa
