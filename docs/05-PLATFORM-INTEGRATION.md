# 05 — Platform Integration

How Kameraa captures from the real camera and publishes the virtual camera on each OS.
All OS specifics live behind `ICaptureSource` and `IVirtualCamera`
(see [03-ARCHITECTURE.md](03-ARCHITECTURE.md)).

## 1. Capture backends

| OS | Primary API | MVP shortcut |
|----|-------------|--------------|
| Windows | Media Foundation (`IMFSourceReader`, MSMF) | OpenCV `VideoCapture` (MSMF) |
| macOS | AVFoundation (`AVCaptureSession`) | OpenCV `VideoCapture` (AVFoundation) |
| Linux | V4L2 (`/dev/videoN`) | OpenCV `VideoCapture` (V4L2) |

OpenCV `VideoCapture` gives a working cross-platform capture quickly. Native backends
are added later for precise format/fps control and lower overhead.

## 2. Virtual camera backends

### 2.1 Linux — `v4l2loopback` (FIRST target)
- **Mechanism:** kernel module creates virtual `/dev/videoN`; we open it as a V4L2
  **OUTPUT** device and write processed frames. Any V4L2 app reads it as a webcam.
- **Setup:**
  - Install: `sudo apt install v4l2loopback-dkms` (Debian/Ubuntu) or DKMS build.
  - Load: `sudo modprobe v4l2loopback devices=1 card_label="Kameraa" exclusive_caps=1`
    (`exclusive_caps=1` improves Chrome/WebRTC detection).
  - Persist at boot via `/etc/modules-load.d/` + `/etc/modprobe.d/` if desired.
- **Write path:** set format (`VIDIOC_S_FMT`, e.g. YUYV/NV12), then write frames each
  tick. Can also feed via GStreamer `v4l2sink` as an alternative.
- **Caveats:** GPL-2.0 module; kernel headers required; secure-boot may need module
  signing. Kameraa **detects** the module and guides install; it does not bundle the
  kernel module binary.

### 2.2 Windows — DirectShow filter (SECOND target)
- **Mechanism:** a DirectShow **push source** capture filter packaged as a COM DLL,
  registered system-wide (admin). Apps using DirectShow list it as a camera. The
  Kameraa app feeds frames to the filter via shared memory / a local IPC channel.
- **Why DirectShow first:** widest app compatibility (matches OBS Virtual Camera's
  approach).
- **Windows 11 option:** also expose a **Media Foundation Virtual Camera**
  (`MFCreateVirtualCamera`) for MF-based apps. Secondary backend.
- **Setup:** installer registers/unregisters the DLL (`regsvr32` equivalent via
  installer). Kameraa detects registration and guides the user if missing.
- **Caveats:** registration needs admin once; frame transport (shared memory) must be
  efficient to hit target fps.

### 2.3 macOS — CMIOExtension Camera Extension (LAST target)
- **Mechanism:** a **System Extension** (Camera Extension, macOS 12.3+) bundled inside
  the app. The app activates it via `OSSystemExtensionManager`; the user approves it in
  System Settings. It then appears as a normal `AVCaptureDevice`.
- **Required:** entitlements `com.apple.developer.system-extension.install` and App
  Groups (shared container for app↔extension frame transport); the extension and app
  must be **code-signed**; app must run from `/Applications` to activate.
- **Frame transport:** app writes frames into the shared App Group memory / IOSurface;
  the extension's `CMIOExtensionStream` serves them to clients.
- **Caveats:** public distribution needs an **Apple Developer account ($99/yr)** for
  signing/notarization. Local/self-built use can rely on developer mode + self-signing.
  This is the most involved backend — scheduled last.

## 3. Backend availability detection & guidance (FR-P1)
On startup and before "Start", Kameraa checks:
- Linux: is `v4l2loopback` loaded / a loopback device present?
- Windows: is the DirectShow filter registered (or MF virtual cam available)?
- macOS: is the system extension installed & approved?

If missing, the GUI disables "Start" and shows **actionable instructions** (commands
to run / buttons to launch the installer or open System Settings).

## 4. GPU acceleration per OS (optional, see [06-AI-MODELS.md](06-AI-MODELS.md))
| OS | Preferred ONNX Runtime EP | Fallback |
|----|---------------------------|----------|
| Windows | DirectML (any GPU) / CUDA (NVIDIA) | CPU |
| Linux | CUDA (NVIDIA) | CPU |
| macOS | CoreML (Apple Silicon/Metal) | CPU |

Acceleration is auto-detected with a manual override; the app is fully functional on
**CPU only**.

## 5. Packaging implications
- Linux: ship app + a helper/script to install/load `v4l2loopback`; document DKMS.
- Windows: installer registers the DirectShow DLL and sets up the frame channel.
- macOS: app bundle embeds the signed system extension; installs on first run with
  user approval.
See [10-BUILD-AND-PACKAGING.md](10-BUILD-AND-PACKAGING.md).

## 6. Implementation order (matches roadmap)
1. OpenCV capture (all OSes) + **Linux v4l2loopback** sink → first end-to-end MVP.
2. **Windows DirectShow** sink.
3. **macOS CMIOExtension** sink.
4. Native capture backends where beneficial.
