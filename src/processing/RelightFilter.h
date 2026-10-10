#pragma once

#include <atomic>

#include "processing/FilterBase.h"

namespace kameraa {

// Relight + color correction. Optional auto local-contrast (CLAHE) plus manual
// brightness, contrast and saturation. All params normalized to [-1, 1] except
// where noted; 0 means "no change".
class RelightFilter : public FilterBase {
public:
    RelightFilter() : FilterBase("relight") {}

    void setAutoLight(bool on) { autoLight_ = on; }
    void setBrightness(double v) { brightness_ = v; }  // [-1,1]
    void setContrast(double v) { contrast_ = v; }      // [-1,1]
    void setSaturation(double v) { saturation_ = v; }  // [-1,1]

    bool autoLight() const { return autoLight_; }
    double brightness() const { return brightness_; }
    double contrast() const { return contrast_; }
    double saturation() const { return saturation_; }

    void process(Frame& inOut) override;

private:
    std::atomic<bool> autoLight_{true};
    std::atomic<double> brightness_{0.0};
    std::atomic<double> contrast_{0.0};
    std::atomic<double> saturation_{0.0};
};

}  // namespace kameraa
