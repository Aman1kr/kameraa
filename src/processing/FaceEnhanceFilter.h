#pragma once

#include <atomic>
#include <string>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/objdetect.hpp>

#include "processing/FilterBase.h"

namespace kameraa {

// Face smoothing / beautify. Uses YuNet face detection when a model is available
// (smoothing only the face ROIs); otherwise falls back to a skin-color mask applied
// globally. Either way it works without downloads (classical smoothing).
class FaceEnhanceFilter : public FilterBase {
public:
    FaceEnhanceFilter() : FilterBase("face") {}

    bool loadDetector(const std::string& yunetPath);
    bool hasDetector() const { return static_cast<bool>(detector_); }
    void setStrength(double s) { strength_ = s; }  // [0,1]
    std::string statusText() const { return status_; }

    void process(Frame& inOut) override;

private:
    static cv::Mat skinMask(const cv::Mat& bgr);
    void smoothRegion(cv::Mat& img, const cv::Rect& roi, double strength);

    std::atomic<double> strength_{0.5};
    cv::Ptr<cv::FaceDetectorYN> detector_;
    int frameCounter_ = 0;
    std::vector<cv::Rect> lastFaces_;
    std::string status_ = "skin-mask fallback";
};

}  // namespace kameraa
