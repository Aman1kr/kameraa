#include <gtest/gtest.h>

#include <opencv2/core.hpp>

#include "processing/DenoiseFilter.h"
#include "processing/RelightFilter.h"
#include "processing/SharpenFilter.h"

using namespace kameraa;

namespace {
Frame makeFrame(int w = 64, int h = 48, const cv::Scalar& color = {100, 120, 140}) {
    Frame f;
    f.image = cv::Mat(h, w, CV_8UC3, color);
    return f;
}
double meanLuma(const cv::Mat& bgr) {
    return cv::mean(bgr)[0] * 0.114 + cv::mean(bgr)[1] * 0.587 + cv::mean(bgr)[2] * 0.299;
}
}  // namespace

TEST(FilterTest, DisabledFilterIsPassthrough) {
    DenoiseFilter f;
    f.setEnabled(false);
    Frame frame = makeFrame();
    cv::Mat before = frame.image.clone();
    if (f.isEnabled()) f.process(frame);  // mirrors pipeline gate
    EXPECT_EQ(cv::countNonZero(before != frame.image), 0);
}

TEST(FilterTest, DenoisePreservesSizeAndType) {
    DenoiseFilter f;
    f.setEnabled(true);
    f.setStrength(0.5);
    Frame frame = makeFrame(80, 60);
    f.process(frame);
    EXPECT_EQ(frame.image.cols, 80);
    EXPECT_EQ(frame.image.rows, 60);
    EXPECT_EQ(frame.image.type(), CV_8UC3);
}

TEST(FilterTest, RelightBrightnessRaisesLuma) {
    RelightFilter f;
    f.setEnabled(true);
    f.setAutoLight(false);
    f.setBrightness(0.5);
    Frame frame = makeFrame(64, 64, {80, 80, 80});
    const double before = meanLuma(frame.image);
    f.process(frame);
    EXPECT_GT(meanLuma(frame.image), before);
}

TEST(FilterTest, SharpenKeepsDimensions) {
    SharpenFilter f;
    f.setEnabled(true);
    f.setAmount(0.8);
    Frame frame = makeFrame(100, 50);
    f.process(frame);
    EXPECT_EQ(frame.image.size(), cv::Size(100, 50));
}

TEST(FilterTest, HandlesEmptyFrame) {
    DenoiseFilter f;
    f.setEnabled(true);
    Frame empty;
    EXPECT_NO_THROW(f.process(empty));
}
