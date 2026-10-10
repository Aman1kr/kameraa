#pragma once

#include <string>
#include <vector>

#include "core/Config.h"
#include "core/Frame.h"

namespace kameraa {

// Produces frames from a source (physical camera, file, test pattern).
class ICaptureSource {
public:
    virtual ~ICaptureSource() = default;
    virtual std::vector<CameraInfo> enumerate() = 0;
    virtual bool open(const CaptureConfig& cfg) = 0;
    virtual bool read(Frame& out) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
};

// A single enhancement stage. Must be safe to disable (pass-through).
class IFilter {
public:
    virtual ~IFilter() = default;
    virtual std::string name() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    virtual bool isEnabled() const = 0;
    virtual void process(Frame& inOut) = 0;  // no-op when disabled
};

// Publishes processed frames to the OS as a virtual camera.
class IVirtualCamera {
public:
    virtual ~IVirtualCamera() = default;
    virtual std::string backendName() const = 0;
    virtual bool isAvailable() const = 0;   // backend installed/loadable?
    // Human-readable guidance shown when the backend is unavailable.
    virtual std::string availabilityHint() const = 0;
    virtual bool start(const OutputConfig& cfg) = 0;
    virtual bool pushFrame(const Frame& frame) = 0;
    virtual void stop() = 0;
};

// Synthesizes an intermediate frame between two consecutive frames to raise the
// output frame rate (e.g. RIFE). Implemented in processing; used by the pipeline.
class IFrameInterpolator {
public:
    virtual ~IFrameInterpolator() = default;
    virtual bool isEnabled() const = 0;
    virtual bool isAvailable() const = 0;
    // Produces the midpoint frame between a and b. Returns false if unavailable.
    virtual bool interpolate(const cv::Mat& a, const cv::Mat& b, cv::Mat& out) = 0;
};

}  // namespace kameraa
