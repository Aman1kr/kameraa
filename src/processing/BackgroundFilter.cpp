#include "processing/BackgroundFilter.h"

#include <algorithm>
#include <vector>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

namespace kameraa {

bool BackgroundFilter::loadModel(const std::string& path, int inputSize) {
    inputSize_ = inputSize > 0 ? inputSize : 512;
    if (model_.load(path)) {
        status_ = "ready";
        return true;
    }
    status_ = "model not loaded";
    return false;
}

bool BackgroundFilter::setBackgroundImage(const std::string& path) {
    cv::Mat img = cv::imread(path, cv::IMREAD_COLOR);
    if (img.empty()) return false;
    bgImage_ = img;
    return true;
}

void BackgroundFilter::process(Frame& inOut) {
    if (!model_.isLoaded() || inOut.empty()) return;

    const int size = inputSize_;
    const cv::Size frameSize = inOut.image.size();

    // Preprocess: resize to model input, BGR->RGB, normalize to [-1,1], NCHW.
    cv::Mat resized;
    cv::resize(inOut.image, resized, cv::Size(size, size), 0, 0, cv::INTER_AREA);
    cv::Mat rgb;
    cv::cvtColor(resized, rgb, cv::COLOR_BGR2RGB);
    cv::Mat f;
    rgb.convertTo(f, CV_32F, 1.0 / 127.5, -1.0);

    std::vector<float> input(static_cast<size_t>(3) * size * size);
    std::vector<cv::Mat> planes(3);
    for (int c = 0; c < 3; ++c) {
        planes[c] = cv::Mat(size, size, CV_32F,
                            input.data() + static_cast<size_t>(c) * size * size);
    }
    cv::split(f, planes);  // HWC float -> CHW planes (into input buffer)

    std::array<int64_t, 4> inShape{1, 3, size, size};
    std::vector<float> output;
    std::array<int64_t, 4> outShape{};
    if (!model_.run(input, inShape, output, outShape)) return;

    const int oh = static_cast<int>(outShape[2]);
    const int ow = static_cast<int>(outShape[3]);
    if (oh <= 0 || ow <= 0 || output.size() < static_cast<size_t>(oh) * ow) return;

    // Alpha mask -> frame size, 3-channel float in [0,1].
    cv::Mat alphaSmall(oh, ow, CV_32F, output.data());
    cv::Mat alpha;
    cv::resize(alphaSmall, alpha, frameSize, 0, 0, cv::INTER_LINEAR);
    cv::min(alpha, 1.0, alpha);
    cv::max(alpha, 0.0, alpha);
    cv::Mat alpha3;
    cv::cvtColor(alpha, alpha3, cv::COLOR_GRAY2BGR);

    // Build the background layer.
    cv::Mat bg;
    if (mode_ == Mode::Replace && !bgImage_.empty()) {
        cv::resize(bgImage_, bg, frameSize, 0, 0, cv::INTER_LINEAR);
    } else {
        const double s = strength_;
        const int k = std::max(3, static_cast<int>(s * 40) | 1);  // odd kernel
        cv::GaussianBlur(inOut.image, bg, cv::Size(k, k), 0);
    }

    // Composite: fg*alpha + bg*(1-alpha).
    cv::Mat fgF, bgF;
    inOut.image.convertTo(fgF, CV_32FC3);
    bg.convertTo(bgF, CV_32FC3);
    cv::Mat outF = fgF.mul(alpha3) + bgF.mul(cv::Scalar::all(1.0) - alpha3);
    outF.convertTo(inOut.image, CV_8UC3);
}

}  // namespace kameraa
