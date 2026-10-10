#include "processing/FaceEnhanceFilter.h"

#include <algorithm>

#include <opencv2/imgproc.hpp>

namespace kameraa {

namespace {
constexpr int kDetectInterval = 5;  // re-detect every N frames; track in between
}

bool FaceEnhanceFilter::loadDetector(const std::string& yunetPath) {
    try {
        detector_ = cv::FaceDetectorYN::create(yunetPath, "", cv::Size(320, 320),
                                               0.7f, 0.3f, 5000);
        status_ = detector_ ? "YuNet ready" : "skin-mask fallback";
        return static_cast<bool>(detector_);
    } catch (const cv::Exception&) {
        detector_.reset();
        status_ = "skin-mask fallback";
        return false;
    }
}

cv::Mat FaceEnhanceFilter::skinMask(const cv::Mat& bgr) {
    cv::Mat ycrcb;
    cv::cvtColor(bgr, ycrcb, cv::COLOR_BGR2YCrCb);
    cv::Mat mask;
    cv::inRange(ycrcb, cv::Scalar(0, 133, 77), cv::Scalar(255, 173, 127), mask);
    cv::GaussianBlur(mask, mask, cv::Size(7, 7), 0);
    return mask;  // CV_8U, 0..255
}

void FaceEnhanceFilter::smoothRegion(cv::Mat& img, const cv::Rect& roi, double s) {
    const cv::Rect clamped = roi & cv::Rect(0, 0, img.cols, img.rows);
    if (clamped.width < 8 || clamped.height < 8) return;

    cv::Mat region = img(clamped);
    cv::Mat smoothed;
    // Edge-preserving smoothing keeps eyes/mouth while evening skin.
    cv::bilateralFilter(region, smoothed, 7, 40.0, 7.0);

    cv::Mat mask = skinMask(region);
    cv::Mat maskF;
    mask.convertTo(maskF, CV_32F, s / 255.0);
    cv::Mat mask3;
    cv::cvtColor(maskF, mask3, cv::COLOR_GRAY2BGR);

    cv::Mat regF, smF;
    region.convertTo(regF, CV_32FC3);
    smoothed.convertTo(smF, CV_32FC3);
    cv::Mat out = regF.mul(cv::Scalar::all(1.0) - mask3) + smF.mul(mask3);
    out.convertTo(region, CV_8UC3);
}

void FaceEnhanceFilter::process(Frame& inOut) {
    const double s = strength_;
    if (s <= 0.0 || inOut.empty()) return;

    if (detector_) {
        if (frameCounter_ % kDetectInterval == 0) {
            lastFaces_.clear();
            try {
                detector_->setInputSize(inOut.image.size());
                cv::Mat faces;
                detector_->detect(inOut.image, faces);
                for (int i = 0; i < faces.rows; ++i) {
                    const float x = faces.at<float>(i, 0);
                    const float y = faces.at<float>(i, 1);
                    const float w = faces.at<float>(i, 2);
                    const float h = faces.at<float>(i, 3);
                    // Pad the box slightly to include the jaw/forehead.
                    const int px = static_cast<int>(w * 0.15f);
                    const int py = static_cast<int>(h * 0.15f);
                    lastFaces_.emplace_back(static_cast<int>(x) - px,
                                            static_cast<int>(y) - py,
                                            static_cast<int>(w) + 2 * px,
                                            static_cast<int>(h) + 2 * py);
                }
            } catch (const cv::Exception&) {
                lastFaces_.clear();
            }
        }
        ++frameCounter_;

        for (const auto& roi : lastFaces_) {
            smoothRegion(inOut.image, roi, s);
        }
        if (!lastFaces_.empty()) return;
    }

    // Fallback (no detector or no faces found): gentle global skin smoothing.
    smoothRegion(inOut.image, cv::Rect(0, 0, inOut.image.cols, inOut.image.rows),
                 s * 0.6);
}

}  // namespace kameraa
