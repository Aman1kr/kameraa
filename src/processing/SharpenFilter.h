#pragma once

#include <atomic>

#include "processing/FilterBase.h"

namespace kameraa {

// Unsharp-mask sharpening. amount in [0,1].
class SharpenFilter : public FilterBase {
public:
    SharpenFilter() : FilterBase("sharpen") {}

    void setAmount(double a) { amount_ = a; }
    double amount() const { return amount_; }

    void process(Frame& inOut) override;

private:
    std::atomic<double> amount_{0.5};
};

}  // namespace kameraa
