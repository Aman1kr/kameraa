#pragma once

#include <atomic>
#include <string>

#include <opencv2/core.hpp>

#include "processing/FilterBase.h"
#include "processing/OnnxModel.h"

namespace kameraa {

// AI background matting (MODNet-style) -> blur or replace the background.
// No-ops when no model is loaded (classical path unaffected).
class BackgroundFilter : public FilterBase {
public:
    enum class Mode { Blur, Replace };

    BackgroundFilter() : FilterBase("background") {}

    bool loadModel(const std::string& path, int inputSize);
    bool available() const { return model_.isLoaded(); }
    std::string statusText() const { return status_; }

    void setMode(Mode m) { mode_ = m; }
    void setStrength(double s) { strength_ = s; }  // blur amount, [0,1]
    bool setBackgroundImage(const std::string& path);

    void process(Frame& inOut) override;

private:
    OnnxModel model_;
    std::atomic<Mode> mode_{Mode::Blur};
    std::atomic<double> strength_{0.6};
    std::atomic<int> inputSize_{512};
    cv::Mat bgImage_;  // replacement background (BGR)
    std::string status_ = "no model";
};

}  // namespace kameraa
