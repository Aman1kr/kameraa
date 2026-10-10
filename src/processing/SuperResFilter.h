#pragma once

#include <atomic>
#include <string>

#include "processing/FilterBase.h"
#include "processing/OnnxModel.h"

namespace kameraa {

// AI super-resolution on the luminance channel (ESPCN/FSRCNN-style); chroma is
// upscaled classically. No-ops when no model is loaded (classical path unaffected).
class SuperResFilter : public FilterBase {
public:
    SuperResFilter() : FilterBase("superres") {}

    bool loadModel(const std::string& path, int scale);
    bool available() const { return model_.isLoaded(); }
    int scale() const { return scale_; }
    std::string statusText() const { return status_; }

    void process(Frame& inOut) override;

private:
    OnnxModel model_;
    std::atomic<int> scale_{2};
    std::string status_ = "no model";
};

}  // namespace kameraa
