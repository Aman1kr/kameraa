#include "capture/OpenCvCaptureSource.h"

#include <chrono>

namespace kameraa {

namespace {
int64_t nowUs() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

std::vector<CameraInfo> OpenCvCaptureSource::enumerate() {
    // OpenCV has no portable enumeration API; probe the first few indices.
    std::vector<CameraInfo> devices;
    for (int i = 0; i < 8; ++i) {
        cv::VideoCapture probe(i);
        if (probe.isOpened()) {
            devices.push_back({i, "Camera " + std::to_string(i)});
            probe.release();
        }
    }
    return devices;
}

bool OpenCvCaptureSource::open(const CaptureConfig& cfg) {
    close();
    if (!cap_.open(cfg.index)) return false;
    cap_.set(cv::CAP_PROP_FRAME_WIDTH, cfg.width);
    cap_.set(cv::CAP_PROP_FRAME_HEIGHT, cfg.height);
    cap_.set(cv::CAP_PROP_FPS, cfg.fps);
    return cap_.isOpened();
}

bool OpenCvCaptureSource::read(Frame& out) {
    if (!cap_.isOpened()) return false;
    if (!cap_.read(out.image) || out.image.empty()) return false;
    out.timestampUs = nowUs();
    return true;
}

void OpenCvCaptureSource::close() {
    if (cap_.isOpened()) cap_.release();
}

bool OpenCvCaptureSource::isOpen() const {
    return cap_.isOpened();
}

}  // namespace kameraa
