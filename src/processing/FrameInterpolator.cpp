#include "processing/FrameInterpolator.h"

#include <vector>

#include <opencv2/imgproc.hpp>

namespace kameraa {

bool FrameInterpolator::loadModel(const std::string& path) {
    if (model_.load(path)) {
        status_ = "ready";
        return true;
    }
    status_ = "model not loaded";
    return false;
}

bool FrameInterpolator::interpolate(const cv::Mat& a, const cv::Mat& b, cv::Mat& out) {
    if (!model_.isLoaded() || a.empty() || b.empty() || a.size() != b.size()) {
        return false;
    }

    const int h = a.rows;
    const int w = a.cols;

    // Pack both frames as a single 6-channel NCHW float tensor (RGB, [0,1]):
    // channels 0..2 = frame A, 3..5 = frame B.
    std::vector<float> input(static_cast<size_t>(6) * h * w);
    auto fill = [&](const cv::Mat& bgr, int baseChannel) {
        cv::Mat rgb, f;
        cv::cvtColor(bgr, rgb, cv::COLOR_BGR2RGB);
        rgb.convertTo(f, CV_32F, 1.0 / 255.0);
        std::vector<cv::Mat> planes(3);
        for (int c = 0; c < 3; ++c) {
            planes[c] = cv::Mat(h, w, CV_32F,
                                input.data() +
                                    static_cast<size_t>(baseChannel + c) * h * w);
        }
        cv::split(f, planes);
    };
    fill(a, 0);
    fill(b, 3);

    std::array<int64_t, 4> inShape{1, 6, h, w};
    std::vector<float> output;
    std::array<int64_t, 4> outShape{};
    if (!model_.run(input, inShape, output, outShape)) return false;

    const int oc = static_cast<int>(outShape[1]);
    const int oh = static_cast<int>(outShape[2]);
    const int ow = static_cast<int>(outShape[3]);
    if (oc < 3 || oh <= 0 || ow <= 0 ||
        output.size() < static_cast<size_t>(3) * oh * ow) {
        return false;
    }

    std::vector<cv::Mat> planes(3);
    for (int c = 0; c < 3; ++c) {
        planes[c] = cv::Mat(oh, ow, CV_32F,
                            output.data() + static_cast<size_t>(c) * oh * ow);
    }
    cv::Mat rgbF;
    cv::merge(planes, rgbF);
    cv::Mat rgb8;
    rgbF.convertTo(rgb8, CV_8U, 255.0);
    cv::cvtColor(rgb8, out, cv::COLOR_RGB2BGR);
    return true;
}

}  // namespace kameraa
