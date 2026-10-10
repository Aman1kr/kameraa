#pragma once

#include <atomic>
#include <string>

#include "core/Interfaces.h"
#include "processing/OnnxModel.h"

namespace kameraa {

// RIFE-style frame interpolation: given two frames, synthesize the midpoint.
// No-ops (reports unavailable) when no model is loaded.
class FrameInterpolator : public IFrameInterpolator {
public:
    bool loadModel(const std::string& path);
    std::string statusText() const { return status_; }

    void setEnabled(bool e) { enabled_ = e; }
    bool isEnabled() const override { return enabled_; }
    bool isAvailable() const override { return model_.isLoaded(); }
    bool interpolate(const cv::Mat& a, const cv::Mat& b, cv::Mat& out) override;

private:
    OnnxModel model_;
    std::atomic<bool> enabled_{false};
    std::string status_ = "no model";
};

}  // namespace kameraa
