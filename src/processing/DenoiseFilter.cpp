#include "processing/DenoiseFilter.h"

#include <opencv2/imgproc.hpp>

namespace kameraa {

void DenoiseFilter::process(Frame& inOut) {
    const double s = strength_;
    if (s <= 0.0 || inOut.empty()) return;

    // Map strength to bilateral sigmas; keep diameter small for realtime cost.
    const int d = 5;
    const double sigma = 10.0 + s * 60.0;

    cv::Mat out;
    cv::bilateralFilter(inOut.image, out, d, sigma, sigma);
    inOut.image = out;
}

}  // namespace kameraa
