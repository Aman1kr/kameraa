#include "core/Pipeline.h"

#include <algorithm>
#include <chrono>

#include <opencv2/imgproc.hpp>

namespace kameraa {

Pipeline::~Pipeline() { stop(); }

void Pipeline::setCapture(std::shared_ptr<ICaptureSource> capture) {
    capture_ = std::move(capture);
}

void Pipeline::addFilter(std::shared_ptr<IFilter> filter) {
    filters_.push_back(std::move(filter));
}

void Pipeline::setVirtualCamera(std::shared_ptr<IVirtualCamera> sink) {
    sink_ = std::move(sink);
}

void Pipeline::setInterpolator(std::shared_ptr<IFrameInterpolator> interp) {
    interpolator_ = std::move(interp);
}

bool Pipeline::start(const CaptureConfig& inCfg, const OutputConfig& outCfg) {
    if (running_ || !capture_) return false;
    if (!capture_->open(inCfg)) return false;
    if (sink_ && !sink_->start(outCfg)) {
        capture_->close();
        return false;
    }
    running_ = true;
    worker_ = std::thread(&Pipeline::run, this, inCfg, outCfg);
    return true;
}

void Pipeline::stop() {
    if (!running_) return;
    running_ = false;
    if (worker_.joinable()) worker_.join();
    if (sink_) sink_->stop();
    if (capture_) capture_->close();
    prevOut_.release();
}

bool Pipeline::latestFrame(Frame& out) const {
    std::lock_guard<std::mutex> lock(frameMutex_);
    if (latest_.empty()) return false;
    out.image = latest_.image.clone();
    out.timestampUs = latest_.timestampUs;
    return true;
}

void Pipeline::run(CaptureConfig inCfg, OutputConfig /*outCfg*/) {
    using clock = std::chrono::steady_clock;
    auto lastTick = clock::now();
    int frameCount = 0;
    int readFailures = 0;

    Frame frame;
    while (running_) {
        if (!capture_->read(frame) || frame.empty()) {
            // Reliability: after sustained read failures, try to reconnect the
            // camera (handles disconnect/resume).
            if (++readFailures >= 30) {
                capture_->close();
                capture_->open(inCfg);
                readFailures = 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            continue;
        }
        readFailures = 0;

        // Adaptive: run filters on a downscaled copy when over budget, then
        // restore to the original resolution for preview/output.
        const double scale = workScale_;
        const cv::Size fullSize = frame.image.size();

        auto procStart = clock::now();
        if (scale < 0.999) {
            cv::Mat small;
            cv::resize(frame.image, small, cv::Size(), scale, scale, cv::INTER_AREA);
            Frame work;
            work.image = small;
            work.timestampUs = frame.timestampUs;
            for (auto& f : filters_) {
                if (f && f->isEnabled()) f->process(work);
            }
            cv::resize(work.image, frame.image, fullSize, 0, 0, cv::INTER_LINEAR);
        } else {
            for (auto& f : filters_) {
                if (f && f->isEnabled()) f->process(frame);
            }
        }
        const double procMs =
            std::chrono::duration<double, std::milli>(clock::now() - procStart).count();

        // Adjust work scale toward the frame budget (hysteresis to avoid flapping).
        const double budgetMs = 1000.0 / static_cast<double>(targetFps_);
        double s = workScale_;
        if (procMs > budgetMs * 0.9 && s > 0.5) {
            s = std::max(0.5, s - 0.25);
        } else if (procMs < budgetMs * 0.5 && s < 1.0) {
            s = std::min(1.0, s + 0.25);
        }
        workScale_ = s;

        {
            std::lock_guard<std::mutex> lock(frameMutex_);
            latest_.image = frame.image;  // shared buffer; cloned on read
            latest_.timestampUs = frame.timestampUs;
        }

        if (sink_) {
            // Frame interpolation: emit a synthesized midpoint before the current
            // frame to raise the output frame rate to the virtual camera.
            if (interpolator_ && interpolator_->isEnabled() &&
                interpolator_->isAvailable() && !prevOut_.empty() &&
                prevOut_.size() == frame.image.size()) {
                cv::Mat mid;
                if (interpolator_->interpolate(prevOut_, frame.image, mid) &&
                    !mid.empty()) {
                    Frame midFrame;
                    midFrame.image = mid;
                    midFrame.timestampUs = frame.timestampUs;
                    sink_->pushFrame(midFrame);
                }
            }
            sink_->pushFrame(frame);
        }
        prevOut_ = frame.image.clone();

        // Rolling FPS over ~1s windows.
        if (++frameCount >= 15) {
            auto now = clock::now();
            double secs = std::chrono::duration<double>(now - lastTick).count();
            if (secs > 0.0) fps_ = frameCount / secs;
            lastTick = now;
            frameCount = 0;
        }
    }
}

}  // namespace kameraa
