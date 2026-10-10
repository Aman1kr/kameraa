#include "capture/TestPatternSource.h"

#include <chrono>

#include <opencv2/imgproc.hpp>

namespace kameraa {

namespace {
int64_t nowUs() {
    using namespace std::chrono;
    return duration_cast<microseconds>(steady_clock::now().time_since_epoch()).count();
}
}  // namespace

std::vector<CameraInfo> TestPatternSource::enumerate() {
    return {{0, "Test Pattern"}};
}

bool TestPatternSource::open(const CaptureConfig& cfg) {
    width_ = cfg.width > 0 ? cfg.width : 1280;
    height_ = cfg.height > 0 ? cfg.height : 720;
    frame_ = 0;
    open_ = true;
    return true;
}

bool TestPatternSource::read(Frame& out) {
    if (!open_) return false;

    cv::Mat img(height_, width_, CV_8UC3, cv::Scalar(30, 30, 30));
    // Color bars + a moving box so motion-based stages have something to work on.
    const int bars = 7;
    const cv::Scalar colors[bars] = {
        {255, 255, 255}, {0, 255, 255}, {255, 255, 0}, {0, 255, 0},
        {255, 0, 255},   {0, 0, 255},   {255, 0, 0}};
    const int bw = width_ / bars;
    for (int i = 0; i < bars; ++i) {
        cv::rectangle(img, cv::Rect(i * bw, 0, bw, height_ / 2), colors[i], cv::FILLED);
    }
    const int x = (frame_ * 8) % std::max(1, width_ - 80);
    cv::rectangle(img, cv::Rect(x, height_ / 2 + 20, 80, 80),
                  cv::Scalar(200, 200, 200), cv::FILLED);

    out.image = img;
    out.timestampUs = nowUs();
    ++frame_;
    return true;
}

void TestPatternSource::close() { open_ = false; }

}  // namespace kameraa
