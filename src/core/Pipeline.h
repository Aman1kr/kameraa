#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "core/Config.h"
#include "core/Frame.h"
#include "core/Interfaces.h"

namespace kameraa {

// Phase 0 pipeline: capture thread -> (optional) filter chain -> latest-frame slot
// for preview and (later) a virtual-camera sink. Always keeps only the newest frame
// so the stream stays realtime (see docs/04-PIPELINE-DESIGN.md).
class Pipeline {
public:
    Pipeline() = default;
    ~Pipeline();

    void setCapture(std::shared_ptr<ICaptureSource> capture);
    void addFilter(std::shared_ptr<IFilter> filter);
    void setVirtualCamera(std::shared_ptr<IVirtualCamera> sink);
    void setInterpolator(std::shared_ptr<IFrameInterpolator> interp);

    bool start(const CaptureConfig& inCfg, const OutputConfig& outCfg);
    void stop();
    bool isRunning() const { return running_; }

    void setTargetFps(int fps) { targetFps_ = fps > 0 ? fps : 30; }

    // Copy the latest processed frame for preview. Returns false if none yet.
    bool latestFrame(Frame& out) const;

    double currentFps() const { return fps_; }
    double currentWorkScale() const { return workScale_; }

private:
    void run(CaptureConfig inCfg, OutputConfig outCfg);

    std::shared_ptr<ICaptureSource> capture_;
    std::vector<std::shared_ptr<IFilter>> filters_;
    std::shared_ptr<IVirtualCamera> sink_;
    std::shared_ptr<IFrameInterpolator> interpolator_;

    std::thread worker_;
    std::atomic<bool> running_{false};

    mutable std::mutex frameMutex_;
    Frame latest_;
    cv::Mat prevOut_;  // worker-thread only; previous output for interpolation

    std::atomic<double> fps_{0.0};
    std::atomic<int> targetFps_{30};
    std::atomic<double> workScale_{1.0};  // adaptive: shrink work when over budget
};

}  // namespace kameraa
