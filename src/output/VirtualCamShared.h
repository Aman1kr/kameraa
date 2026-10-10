#pragma once

namespace kameraa {

// Contract shared between the Kameraa app (producer) and the DirectShow virtual
// camera filter DLL (consumer). Keep these in sync on both sides.
namespace vcam {

// Named shared-memory region carrying frames (see SharedFrameChannel).
inline constexpr char kChannelName[] = "Local\\KameraaVCam";

// POSIX shared-memory object name (macOS). Must start with '/'.
inline constexpr char kPosixChannelName[] = "/KameraaVCam";

// Sentinel the macOS installer/extension writes once the Camera Extension is
// installed & approved; the app uses it to report availability.
inline constexpr char kMacExtensionSentinel[] =
    "/Library/Application Support/Kameraa/extension_installed";

// Registry/COM identity of the DirectShow source filter. The app checks this
// CLSID under HKCR\CLSID to decide whether the virtual camera is installed.
inline constexpr char kFilterClsid[] = "{4B2A7E10-6C3D-4E8F-9A1B-2C5D7E9F0A33}";
inline constexpr char kFilterName[] = "Kameraa Virtual Camera";

// Maximum frame size the channel reserves (BGR24).
inline constexpr int kMaxWidth = 1920;
inline constexpr int kMaxHeight = 1080;

}  // namespace vcam
}  // namespace kameraa
