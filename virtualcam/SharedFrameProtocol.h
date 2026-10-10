#pragma once

#include <cstdint>

// Standalone copy of the shared-memory frame protocol for the DirectShow filter
// DLL (kept dependency-free). MUST stay in sync with:
//   src/output/SharedFrameChannel.h  (SharedFrameHeader)
//   src/output/VirtualCamShared.h    (channel name / CLSID / max size)
namespace kameraa_vcam {

struct SharedFrameHeader {
    uint32_t magic;     // 0x414D524B 'KMRA'
    uint32_t version;   // 1
    uint32_t width;
    uint32_t height;
    uint32_t stride;    // bytes per row
    uint32_t format;    // 0 = BGR24
    uint32_t sequence;  // bumped after each complete write
    uint32_t dataSize;
};

inline constexpr uint32_t kMagic = 0x414D524B;
inline constexpr uint32_t kVersion = 1;
inline constexpr char kChannelName[] = "Local\\KameraaVCam";
inline constexpr int kMaxWidth = 1920;
inline constexpr int kMaxHeight = 1080;
inline constexpr int kDefaultWidth = 1280;
inline constexpr int kDefaultHeight = 720;
inline constexpr int kDefaultFps = 30;

}  // namespace kameraa_vcam
