# 02 — Research & Feasibility

This document records the research behind the architecture: how to publish a virtual
camera on each OS, how to capture, which AI models are viable, and the feasibility of
the headline goals.

## 1. Virtual camera — the critical integration point

The hardest part of this project is **publishing frames as a camera device the whole
OS can see**. Each OS has a different, non-trivial mechanism.

### 1.1 Windows
Two viable paths:
- **DirectShow virtual capture filter** — a COM DLL registered as a video capture
  source. This is the long-established approach used by tools like OBS Virtual Camera
  for maximum app compatibility (works with apps using DirectShow). Requires
  registering the DLL (admin) and implementing a push source filter.
- **Media Foundation Virtual Camera** (Windows 11+) — `MFCreateVirtualCamera` creates
  a system virtual camera visible to Media-Foundation-based apps. Cleaner API but
  newer and Windows-11-only; some apps still prefer DirectShow.

**Decision:** target **DirectShow** first for broad compatibility; consider MF virtual
camera as an additional backend for Windows 11.

### 1.2 Linux
- **`v4l2loopback`** — a GPL-2.0 kernel module that creates virtual `/dev/videoN`
  devices. We write processed frames to the loopback device (V4L2 OUTPUT); any
  V4L2-capable app (browsers/WebRTC, OBS, Zoom) reads it as a normal camera.
- Install via distro package (`v4l2loopback-dkms`) or DKMS build. Needs kernel
  headers; secure-boot may require signing the module.
- Useful options: `exclusive_caps=1` for Chrome/WebRTC compatibility; set label via
  `card_label`; can set caps/fps via `v4l2loopback-ctl`.

**Decision:** use `v4l2loopback`. Detect/guide install; write frames via V4L2.
This is the **easiest backend**, so it is our first virtual-cam target.

### 1.3 macOS
- Legacy **CoreMediaIO DAL plugins** are deprecated.
- Modern approach: **CMIOExtension "Camera Extension"** (a System Extension, macOS
  12.3+). Packaged inside the app bundle; the app activates it via
  `OSSystemExtensionManager`. Requires entitlements
  (`com.apple.developer.system-extension.install`, App Groups), and the extension
  must be **code-signed**; the user must approve it in System Settings.
- Public distribution realistically needs an **Apple Developer account**. Local
  development can use developer mode / self-signing.
- Once approved, the virtual camera is a standard `AVCaptureDevice` and appears in
  FaceTime, Zoom, etc.

**Decision:** implement a CMIOExtension. Document the signing caveat clearly. This is
the **last** backend we tackle due to signing complexity.

### 1.4 Virtual-cam effort ranking
`Linux (v4l2loopback)` < `Windows (DirectShow)` < `macOS (System Extension)`.

## 2. Capture research
- **Windows:** Media Foundation `IMFSourceReader` (modern) or DirectShow. OpenCV's
  `VideoCapture` (MSMF backend) is a quick start.
- **macOS:** AVFoundation `AVCaptureSession`.
- **Linux:** V4L2 (`/dev/videoN`) directly, or via OpenCV/GStreamer.
- **Abstraction:** start with OpenCV `VideoCapture` for a fast cross-platform MVP,
  then add native backends where needed for format/perf control.

## 3. Enhancement techniques — classical vs AI

| Need | Classical (CPU-cheap) | AI (heavier, better) |
|------|----------------------|----------------------|
| Denoise | bilateral / NLM / guided filter | NAFNet, SCUNet, Restormer |
| Relight / low-light | auto WB, CLAHE, tone curves, gamma | Zero-DCE, SCI, RetinexNet |
| Sharpen / detail | unsharp mask | — (handled by super-res) |
| Super-resolution | bicubic/Lanczos (no new detail) | Real-ESRGAN, ESPCN, FSRCNN |
| Background | — | MODNet, RobustVideoMatting (RVM) |
| Face restore | — | GFPGAN, CodeFormer |
| Face beautify | bilateral on skin region | learned skin models |
| Frame interpolation | blending (poor) | RIFE, FILM |

**Strategy:** classical methods as the always-available CPU baseline; AI models as
opt-in upgrades gated by preset and available compute. Lightweight AI (ESPCN/FSRCNN,
Zero-DCE) can run on CPU; heavy AI (Real-ESRGAN, NAFNet, RVM, RIFE) really wants a GPU.

## 4. Inference runtime
- **ONNX Runtime** chosen: single API, multiple **execution providers** — CPU
  (default), **DirectML** (Windows GPUs incl. AMD/Intel/NVIDIA), **CUDA** (NVIDIA),
  **CoreML** (macOS/Apple Silicon). Lets one model format run everywhere with optional
  acceleration.
- Alternatives considered: OpenVINO (great on Intel, less universal), TensorRT
  (NVIDIA-only), ncnn (mobile-focused), direct libtorch (heavy). ONNX Runtime gives
  the best portability/performance balance for our CPU-first, optional-GPU goal.

## 5. Feasibility verdict on headline claims
- "Convert basic cam to ultra-HD / 4K studio look" → **Partially feasible as
  enhancement**, not as literal 4K reconstruction. Deliver as denoise + relight +
  super-res + bokeh + face restore. Market honestly as *enhancement*.
- "Stream 120Hz+, no graininess" → **Local pipeline** can target high fps on capable
  hardware with interpolation; **denoising removes graininess**. But what the *call
  app transmits* is capped by the app (usually ≤1080p30). Document this boundary.
- "Works regardless of how good the laptop camera is" → True in the sense that
  enhancement always applies; but the **better the source, the better the result**.
- "Runs offline, local, free, cross-platform" → **Feasible.** All chosen components
  are free/open-source and run locally. Main caveats: GPL for v4l2loopback and macOS
  signing for distribution.

## 6. Prior art (for reference, not dependencies)
- OBS Virtual Camera (per-OS virtual cam patterns).
- NVIDIA Broadcast / Maxine (closed, GPU-only) — shows the quality ceiling of AI cam
  enhancement, but is proprietary and NVIDIA-only (what we deliberately avoid).
- Snap Camera (discontinued cloud-ish) — example of what NOT to do (privacy/online).

## 7. Open research items (track during build)
- Measure real CPU-only fps for ESPCN/FSRCNN super-res at 720p→1080p.
- Best lightweight low-light model for CPU (Zero-DCE vs SCI).
- RVM vs MODNet quality/perf on CPU for background matting.
- DirectShow vs MF virtual camera compatibility across target apps.
