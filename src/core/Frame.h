#pragma once

#include <cstdint>
#include <opencv2/core.hpp>

namespace kameraa {

// A single video frame flowing through the pipeline.
// Working format is 8-bit BGR by default (see docs/04-PIPELINE-DESIGN.md).
struct Frame {
    cv::Mat image;              // BGR, CV_8UC3
    int64_t timestampUs = 0;    // capture timestamp, microseconds

    bool empty() const { return image.empty(); }
    int width() const { return image.cols; }
    int height() const { return image.rows; }
};

}  // namespace kameraa
