#include <gtest/gtest.h>

#include <memory>

#include "capture/TestPatternSource.h"
#include "core/Pipeline.h"
#include "processing/DenoiseFilter.h"
#include "processing/RelightFilter.h"
#include "processing/SharpenFilter.h"

using namespace kameraa;

namespace {
// Captures the latest frame from a running pipeline into a counter.
void spinUntilFrame(Pipeline& p, int tries = 200) {
    for (int i = 0; i < tries; ++i) {
        Frame f;
        if (p.latestFrame(f) && !f.empty()) return;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}
}  // namespace

TEST(PipelineTest, ProducesFramesFromTestSource) {
    auto source = std::make_shared<TestPatternSource>();
    Pipeline p;
    p.setCapture(source);

    CaptureConfig in;
    OutputConfig out;
    ASSERT_TRUE(p.start(in, out));
    spinUntilFrame(p);

    Frame f;
    EXPECT_TRUE(p.latestFrame(f));
    EXPECT_FALSE(f.empty());
    p.stop();
    EXPECT_FALSE(p.isRunning());
}

TEST(PipelineTest, RunsFilterChainWithoutCrashing) {
    auto source = std::make_shared<TestPatternSource>();
    auto denoise = std::make_shared<DenoiseFilter>();
    auto relight = std::make_shared<RelightFilter>();
    auto sharpen = std::make_shared<SharpenFilter>();
    denoise->setEnabled(true);
    relight->setEnabled(true);
    sharpen->setEnabled(true);

    Pipeline p;
    p.setCapture(source);
    p.addFilter(denoise);
    p.addFilter(relight);
    p.addFilter(sharpen);

    CaptureConfig in;
    OutputConfig out;
    ASSERT_TRUE(p.start(in, out));
    spinUntilFrame(p);

    Frame f;
    ASSERT_TRUE(p.latestFrame(f));
    EXPECT_EQ(f.image.type(), CV_8UC3);
    p.stop();
}
