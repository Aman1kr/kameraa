#pragma once

#include <atomic>

#include "processing/FilterBase.h"

namespace kameraa {

// Classical edge-preserving denoise (bilateral filter). strength in [0,1].
class DenoiseFilter : public FilterBase {
public:
    DenoiseFilter() : FilterBase("denoise") {}

    void setStrength(double s) { strength_ = s; }
    double strength() const { return strength_; }

    void process(Frame& inOut) override;

private:
    std::atomic<double> strength_{0.4};
};

}  // namespace kameraa
