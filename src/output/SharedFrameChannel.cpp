#include "output/SharedFrameChannel.h"

#include <cstring>

namespace kameraa {

namespace {
size_t regionBytes(int w, int h) {
    return sizeof(SharedFrameHeader) + static_cast<size_t>(w) * h * 3;
}
}  // namespace

SharedFrameChannel::~SharedFrameChannel() { close(); }

}  // namespace kameraa

#if defined(_WIN32)

#include <windows.h>

namespace kameraa {

bool SharedFrameChannel::create(const std::string& name, int maxWidth, int maxHeight) {
    close();
    maxWidth_ = maxWidth;
    maxHeight_ = maxHeight;
    capacity_ = regionBytes(maxWidth, maxHeight);

    HANDLE h = ::CreateFileMappingA(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0,
                                    static_cast<DWORD>(capacity_), name.c_str());
    if (h == nullptr) return false;

    void* base = ::MapViewOfFile(h, FILE_MAP_ALL_ACCESS, 0, 0, capacity_);
    if (base == nullptr) {
        ::CloseHandle(h);
        return false;
    }

    mapping_ = h;
    view_ = base;

    auto* hdr = static_cast<SharedFrameHeader*>(view_);
    std::memset(hdr, 0, sizeof(*hdr));
    hdr->magic = kMagic;
    hdr->version = kVersion;
    hdr->format = 0;  // BGR24
    return true;
}

bool SharedFrameChannel::write(const Frame& frame) {
    if (view_ == nullptr || frame.empty()) return false;

    const int w = frame.image.cols;
    const int h = frame.image.rows;
    if (w > maxWidth_ || h > maxHeight_) return false;

    cv::Mat bgr = frame.image;
    if (bgr.type() != CV_8UC3) return false;

    auto* hdr = static_cast<SharedFrameHeader*>(view_);
    auto* data = static_cast<uint8_t*>(view_) + sizeof(SharedFrameHeader);
    const size_t stride = static_cast<size_t>(w) * 3;

    for (int y = 0; y < h; ++y) {
        std::memcpy(data + y * stride, bgr.ptr(y), stride);
    }

    hdr->width = static_cast<uint32_t>(w);
    hdr->height = static_cast<uint32_t>(h);
    hdr->stride = static_cast<uint32_t>(stride);
    hdr->dataSize = static_cast<uint32_t>(stride * h);
    // Publish last: bump sequence after the frame bytes are fully written.
    ++sequence_;
    hdr->sequence = sequence_;
    return true;
}

void SharedFrameChannel::close() {
    if (view_) {
        ::UnmapViewOfFile(view_);
        view_ = nullptr;
    }
    if (mapping_) {
        ::CloseHandle(static_cast<HANDLE>(mapping_));
        mapping_ = nullptr;
    }
    capacity_ = 0;
}

}  // namespace kameraa

#elif defined(__APPLE__)  // POSIX shared memory for the macOS CMIO extension.

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace kameraa {

bool SharedFrameChannel::create(const std::string& name, int maxWidth, int maxHeight) {
    close();
    maxWidth_ = maxWidth;
    maxHeight_ = maxHeight;
    capacity_ = regionBytes(maxWidth, maxHeight);

    int fd = ::shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
    if (fd < 0) return false;
    if (::ftruncate(fd, static_cast<off_t>(capacity_)) != 0) {
        ::close(fd);
        return false;
    }
    void* base = ::mmap(nullptr, capacity_, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    ::close(fd);
    if (base == MAP_FAILED) return false;

    // Store fd handle as non-null marker via view_; keep name for unlink on close.
    view_ = base;
    mapping_ = reinterpret_cast<void*>(1);
    shmName_ = name;

    auto* hdr = static_cast<SharedFrameHeader*>(view_);
    std::memset(hdr, 0, sizeof(*hdr));
    hdr->magic = kMagic;
    hdr->version = kVersion;
    hdr->format = 0;  // BGR24
    return true;
}

bool SharedFrameChannel::write(const Frame& frame) {
    if (view_ == nullptr || frame.empty()) return false;
    const int w = frame.image.cols;
    const int h = frame.image.rows;
    if (w > maxWidth_ || h > maxHeight_ || frame.image.type() != CV_8UC3) return false;

    auto* hdr = static_cast<SharedFrameHeader*>(view_);
    auto* data = static_cast<uint8_t*>(view_) + sizeof(SharedFrameHeader);
    const size_t stride = static_cast<size_t>(w) * 3;
    for (int y = 0; y < h; ++y) {
        std::memcpy(data + y * stride, frame.image.ptr(y), stride);
    }
    hdr->width = static_cast<uint32_t>(w);
    hdr->height = static_cast<uint32_t>(h);
    hdr->stride = static_cast<uint32_t>(stride);
    hdr->dataSize = static_cast<uint32_t>(stride * h);
    ++sequence_;
    hdr->sequence = sequence_;
    return true;
}

void SharedFrameChannel::close() {
    if (view_) {
        ::munmap(view_, capacity_);
        view_ = nullptr;
    }
    if (!shmName_.empty()) {
        ::shm_unlink(shmName_.c_str());
        shmName_.clear();
    }
    mapping_ = nullptr;
    capacity_ = 0;
}

}  // namespace kameraa

#else  // Other platforms (e.g. Linux uses v4l2loopback, not this channel).

namespace kameraa {

bool SharedFrameChannel::create(const std::string&, int, int) { return false; }
bool SharedFrameChannel::write(const Frame&) { return false; }
void SharedFrameChannel::close() {
    view_ = nullptr;
    mapping_ = nullptr;
    capacity_ = 0;
}

}  // namespace kameraa

#endif
