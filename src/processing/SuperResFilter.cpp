#include "processing/SuperResFilter.h"

#include <algorithm>
#include <vector>

#include <opencv2/imgproc.hpp>

namespace kameraa {

bool SuperResFilter::loadModel(const std::string& path, int scale) {
    scale_ = scale > 1 ? scale : 2;
    if (model_.load(path)) {
        status_ = "ready";
        return true;
    }
    status_ = "model not loaded";
    return false;
}

void SuperResFilter::process(Frame& inOut) {
    if (!model_.isLoaded() || inOut.empty()) return;

    const int scale = scale_;
    cv::Mat ycrcb;
    cv::cvtColor(inOut.image, ycrcb, cv::COLOR_BGR2YCrCb);
    std::vector<cv::Mat> ch;
    cv::split(ycrcb, ch);  // ch[0]=Y, ch[1]=Cr, ch[2]=Cb

    const int h = ch[0].rows;
    const int w = ch[0].cols;

    // Y -> float NCHW normalized to [0,1].
    cv::Mat yf;
    ch[0].convertTo(yf, CV_32F, 1.0 / 255.0);
    std::vector<float> input(yf.ptr<float>(0), yf.ptr<float>(0) + static_cast<size_t>(h) * w);
    std::array<int64_t, 4> inShape{1, 1, h, w};

    std::vector<float> output;
    std::array<int64_t, 4> outShape{};
    if (!model_.run(input, inShape, output, outShape)) return;

    const int oh = static_cast<int>(outShape[2]);
    const int ow = static_cast<int>(outShape[3]);
    if (oh <= 0 || ow <= 0 ||
        output.size() < static_cast<size_t>(oh) * ow) {
        return;
    }

    cv::Mat ySr(oh, ow, CV_32F, output.data());
    cv::Mat ySr8;
    ySr.convertTo(ySr8, CV_8U, 255.0, 0.0);  // saturates out-of-range

    // Upscale chroma classically to match the SR luminance size.
    cv::Mat crUp, cbUp;
    cv::resize(ch[1], crUp, cv::Size(ow, oh), 0, 0, cv::INTER_CUBIC);
    cv::resize(ch[2], cbUp, cv::Size(ow, oh), 0, 0, cv::INTER_CUBIC);

    std::vector<cv::Mat> merged{ySr8.clone(), crUp, cbUp};
    cv::Mat out;
    cv::merge(merged, out);
    cv::cvtColor(out, inOut.image, cv::COLOR_YCrCb2BGR);
    (void)scale;
}

}  // namespace kameraa
