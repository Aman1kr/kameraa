#pragma once

#include <cstdint>
#include <string>

#include "core/Frame.h"

namespace kameraa {

// Fixed header at the start of the shared memory region. The DirectShow filter
// (consumer) reads this to pick up the latest frame. Single-producer / single-
// consumer; the consumer polls `sequence` for changes.
struct SharedFrameHeader {
    uint32_t magic;     // 'KMRA'
    uint32_t version;
    uint32_t width;
    uint32_t height;
    uint32_t stride;    // bytes per row
    uint32_t format;    // 0 = BGR24
    uint32_t sequence;  // incremented after each complete write
    uint32_t dataSize;  // bytes of valid frame data
};

// Producer side of a cross-process frame channel backed by shared memory.
// Windows uses a named file mapping; other platforms get a no-op stub (the
// DirectShow sink is Windows-only).
class SharedFrameChannel {
public:
    static constexpr uint32_t kMagic = 0x414D524B;  // 'KMRA'
    static constexpr uint32_t kVersion = 1;

    SharedFrameChannel() = default;
    ~SharedFrameChannel();
    SharedFrameChannel(const SharedFrameChannel&) = delete;
    SharedFrameChannel& operator=(const SharedFrameChannel&) = delete;

    // Creates/opens a named region large enough for maxWidth x maxHeight BGR24.
    bool create(const std::string& name, int maxWidth, int maxHeight);
    bool isOpen() const { return mapping_ != nullptr; }

    // Writes a BGR frame (converted/clamped to the channel's max size). Returns
    // false if closed or the frame is too large.
    bool write(const Frame& frame);

    void close();

private:
    void* mapping_ = nullptr;  // HANDLE on Windows
    void* view_ = nullptr;     // mapped base pointer
    size_t capacity_ = 0;      // total region size in bytes
    int maxWidth_ = 0;
    int maxHeight_ = 0;
    uint32_t sequence_ = 0;
    std::string shmName_;      // POSIX shm name, for unlink on close
};

}  // namespace kameraa
