#include "output/V4l2LoopbackSink.h"

#if defined(__linux__)

#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>

namespace kameraa {

namespace {

constexpr char kDriverName[] = "v4l2 loopback";

// Clamp to [0,255] without branching surprises.
inline uint8_t clamp8(int v) {
    return static_cast<uint8_t>(v < 0 ? 0 : (v > 255 ? 255 : v));
}

// Pack a BGR frame into YUYV (4:2:2) using BT.601. Width must be even.
void bgrToYuyv(const cv::Mat& bgr, std::vector<uint8_t>& out) {
    const int width = bgr.cols;
    const int height = bgr.rows;
    out.resize(static_cast<size_t>(width) * height * 2);

    uint8_t* dst = out.data();
    for (int y = 0; y < height; ++y) {
        const auto* row = bgr.ptr<cv::Vec3b>(y);
        for (int x = 0; x < width; x += 2) {
            const cv::Vec3b& p0 = row[x];
            const cv::Vec3b& p1 = row[(x + 1 < width) ? x + 1 : x];

            const int b0 = p0[0], g0 = p0[1], r0 = p0[2];
            const int b1 = p1[0], g1 = p1[1], r1 = p1[2];

            const int y0 = (77 * r0 + 150 * g0 + 29 * b0) >> 8;
            const int y1 = (77 * r1 + 150 * g1 + 29 * b1) >> 8;

            const int rAvg = (r0 + r1) / 2;
            const int gAvg = (g0 + g1) / 2;
            const int bAvg = (b0 + b1) / 2;
            const int u = ((-43 * rAvg - 85 * gAvg + 128 * bAvg) >> 8) + 128;
            const int v = ((128 * rAvg - 107 * gAvg - 21 * bAvg) >> 8) + 128;

            *dst++ = clamp8(y0);
            *dst++ = clamp8(u);
            *dst++ = clamp8(y1);
            *dst++ = clamp8(v);
        }
    }
}

bool isLoopbackDevice(int fd) {
    v4l2_capability cap{};
    if (ioctl(fd, VIDIOC_QUERYCAP, &cap) < 0) return false;
    return std::strcmp(reinterpret_cast<const char*>(cap.driver), kDriverName) == 0;
}

}  // namespace

V4l2LoopbackSink::~V4l2LoopbackSink() { stop(); }

std::string V4l2LoopbackSink::findLoopbackDevice() {
    for (int i = 0; i < 64; ++i) {
        std::string path = "/dev/video" + std::to_string(i);
        int fd = ::open(path.c_str(), O_RDWR | O_NONBLOCK);
        if (fd < 0) continue;
        const bool match = isLoopbackDevice(fd);
        ::close(fd);
        if (match) return path;
    }
    return {};
}

bool V4l2LoopbackSink::isAvailable() const {
    // Module loaded?
    struct stat st{};
    if (::stat("/sys/module/v4l2loopback", &st) != 0) return false;
    // At least one loopback device present?
    return !findLoopbackDevice().empty();
}

std::string V4l2LoopbackSink::availabilityHint() const {
    struct stat st{};
    if (::stat("/sys/module/v4l2loopback", &st) != 0) {
        return "v4l2loopback kernel module not loaded. Install it "
               "(e.g. 'sudo apt install v4l2loopback-dkms') then load it:\n"
               "  sudo modprobe v4l2loopback devices=1 card_label=\"Kameraa\" "
               "exclusive_caps=1";
    }
    return "v4l2loopback is loaded but no loopback device was found. Create one:\n"
           "  sudo modprobe v4l2loopback devices=1 card_label=\"Kameraa\" "
           "exclusive_caps=1";
}

bool V4l2LoopbackSink::configureFormat(int width, int height) {
    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    fmt.fmt.pix.width = static_cast<__u32>(width);
    fmt.fmt.pix.height = static_cast<__u32>(height);
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;
    fmt.fmt.pix.bytesperline = static_cast<__u32>(width * 2);
    fmt.fmt.pix.sizeimage = static_cast<__u32>(width * height * 2);
    if (ioctl(fd_, VIDIOC_S_FMT, &fmt) < 0) return false;
    width_ = width;
    height_ = height;
    return true;
}

bool V4l2LoopbackSink::start(const OutputConfig& /*cfg*/) {
    stop();
    devicePath_ = findLoopbackDevice();
    if (devicePath_.empty()) return false;
    fd_ = ::open(devicePath_.c_str(), O_RDWR);
    if (fd_ < 0) return false;
    // Format is (re)negotiated on the first frame to match actual dimensions.
    width_ = height_ = 0;
    return true;
}

bool V4l2LoopbackSink::pushFrame(const Frame& frame) {
    if (fd_ < 0 || frame.empty()) return false;

    const int w = frame.image.cols & ~1;  // YUYV requires even width
    const int h = frame.image.rows;
    if (w <= 0 || h <= 0) return false;

    if (w != width_ || h != height_) {
        if (!configureFormat(w, h)) return false;
    }

    cv::Mat src = (frame.image.cols != w)
                      ? frame.image(cv::Rect(0, 0, w, h)).clone()
                      : frame.image;
    bgrToYuyv(src, yuyv_);

    ssize_t written = ::write(fd_, yuyv_.data(), yuyv_.size());
    return written == static_cast<ssize_t>(yuyv_.size());
}

void V4l2LoopbackSink::stop() {
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
    width_ = height_ = 0;
}

}  // namespace kameraa

#else  // non-Linux: this translation unit is a no-op (factory uses NullVirtualCamera)

namespace kameraa {

V4l2LoopbackSink::~V4l2LoopbackSink() = default;
bool V4l2LoopbackSink::isAvailable() const { return false; }
std::string V4l2LoopbackSink::availabilityHint() const {
    return "v4l2loopback is only available on Linux.";
}
std::string V4l2LoopbackSink::findLoopbackDevice() { return {}; }
bool V4l2LoopbackSink::configureFormat(int, int) { return false; }
bool V4l2LoopbackSink::start(const OutputConfig&) { return false; }
bool V4l2LoopbackSink::pushFrame(const Frame&) { return false; }
void V4l2LoopbackSink::stop() {}

}  // namespace kameraa

#endif
