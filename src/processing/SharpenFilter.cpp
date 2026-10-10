#include "processing/SharpenFilter.h"

#include <opencv2/imgproc.hpp>

namespace kameraa {

void SharpenFilter::process(Frame& inOut) {
    const double amount = amount_;
    if (amount <= 0.0 || inOut.empty()) return;

    cv::Mat blurred;
    cv::GaussianBlur(inOut.image, blurred, cv::Size(0, 0), 3.0);
    cv::addWeighted(inOut.image, 1.0 + amount, blurred, -amount, 0.0, inOut.image);
}

}  // namespace kameraa
