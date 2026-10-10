#include "processing/RelightFilter.h"

#include <vector>

#include <opencv2/imgproc.hpp>

namespace kameraa {

void RelightFilter::process(Frame& inOut) {
    if (inOut.empty()) return;

    const bool autoL = autoLight_;
    const double bright = brightness_;
    const double contrast = contrast_;
    const double sat = saturation_;

    cv::Mat& img = inOut.image;

    // Auto relight: equalize local contrast on the L channel in Lab space.
    if (autoL) {
        cv::Mat lab;
        cv::cvtColor(img, lab, cv::COLOR_BGR2Lab);
        std::vector<cv::Mat> ch;
        cv::split(lab, ch);
        auto clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        clahe->apply(ch[0], ch[0]);
        cv::merge(ch, lab);
        cv::cvtColor(lab, img, cv::COLOR_Lab2BGR);
    }

    // Manual brightness/contrast: out = alpha*in + beta.
    if (bright != 0.0 || contrast != 0.0) {
        const double alpha = 1.0 + contrast;    // contrast gain
        const double beta = bright * 100.0;     // brightness offset
        img.convertTo(img, -1, alpha, beta);
    }

    // Saturation scaling in HSV.
    if (sat != 0.0) {
        cv::Mat hsv;
        cv::cvtColor(img, hsv, cv::COLOR_BGR2HSV);
        std::vector<cv::Mat> ch;
        cv::split(hsv, ch);
        ch[1].convertTo(ch[1], -1, 1.0 + sat, 0.0);
        cv::merge(ch, hsv);
        cv::cvtColor(hsv, img, cv::COLOR_HSV2BGR);
    }
}

}  // namespace kameraa
