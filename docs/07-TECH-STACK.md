# 07 — Tech Stack

## 1. Language & standard
- **C++17** (minimum), targeting **C++20** where toolchains allow.
- Reason: performance (user requirement), direct access to OS camera/driver APIs,
  mature CV/AI C++ ecosystem.

## 2. Build system & dependency management
- **CMake** (≥ 3.21) as the single cross-platform build system.
- **vcpkg** (manifest mode, `vcpkg.json`) for dependencies; Conan as an alternative.
- Out-of-source builds; presets via `CMakePresets.json` per OS/compiler.

## 3. Core libraries
| Concern | Library | Notes / License |
|--------|---------|-----------------|
| GUI | **Qt 6** (Widgets) | LGPL-3.0 (dynamic link) — keep Qt dynamically linked |
| Imaging (classical) | **OpenCV 4.x** | Apache-2.0 |
| AI inference | **ONNX Runtime** | MIT; CPU + DirectML/CUDA/CoreML EPs |
| Capture (native, later) | Media Foundation / AVFoundation / V4L2 | OS SDKs |
| Optional media backend | FFmpeg or GStreamer | LGPL/GPL — **flag** if used |
| JSON config | **nlohmann/json** | MIT |
| Logging | **spdlog** | MIT |
| Testing | **GoogleTest** or **Catch2** | BSD / BSL |
| Hashing (model integrity) | small SHA-256 (e.g. picosha2) | MIT |

> **License watch:** Qt (LGPL → dynamic link), any FFmpeg/GStreamer GPL components,
> and the Linux `v4l2loopback` (GPL-2.0, a runtime dependency not linked into our
> binary). Track all in a top-level `LICENSES/` and `THIRD_PARTY.md` before release.

## 4. Platform SDKs
- **Windows:** Windows SDK (Media Foundation, DirectShow base classes), COM.
- **macOS:** Xcode toolchain, AVFoundation, CoreMediaIO, SystemExtensions; code-signing
  identity for the camera extension.
- **Linux:** kernel headers (for guiding `v4l2loopback`), V4L2 userspace headers.

## 5. GPU / acceleration
- ONNX Runtime execution providers: **CPU** (always), **DirectML** (Windows),
  **CUDA** (NVIDIA/Linux+Windows), **CoreML** (macOS). Auto-detect + manual override.

## 6. Toolchains
| OS | Compiler |
|----|----------|
| Windows | MSVC (VS 2022) |
| Linux | GCC 12+ / Clang 15+ |
| macOS | Apple Clang (Xcode 15+) |

## 7. Repository layout (code)
```
kameraa/
  CMakeLists.txt
  CMakePresets.json
  vcpkg.json
  src/
    app/ core/ capture/ processing/ output/ platform/ gui/ config/
  models/        # *.onnx + manifest.json + LICENSES/
  assets/
  tests/
  cmake/
  docs/
```

## 8. Configuration & presets format
- Settings + presets stored as **JSON** (nlohmann/json), schema-documented.
- Preset file example (shape, not final):
```jsonc
{
  "preset": "Balanced",
  "input":  { "width": 1280, "height": 720, "fps": 30 },
  "output": { "width": 1920, "height": 1080, "fps": 30, "format": "NV12" },
  "filters": [
    { "name": "denoise",   "enabled": true,  "strength": 0.4, "backend": "classical" },
    { "name": "relight",   "enabled": true,  "exposure": 0.2, "wb": "auto" },
    { "name": "sharpen",   "enabled": true,  "amount": 0.5 },
    { "name": "superres",  "enabled": true,  "model": "espcn_x2" },
    { "name": "background", "enabled": true, "mode": "blur", "strength": 0.6 },
    { "name": "face",      "enabled": false },
    { "name": "interp",    "enabled": false }
  ]
}
```

## 9. Coding standards
- Enable warnings-as-errors in CI; `clang-format` + `clang-tidy`.
- RAII everywhere; no raw owning pointers; `std::unique_ptr`/`shared_ptr` for ownership.
- OS-specific code only under `platform/`, `capture/<os>`, `output/<os>` behind interfaces.
- No network calls in the processing path.

## 10. Why these choices (vs alternatives)
- **Qt** over native per-OS UI: one cross-platform GUI codebase, mature, good video widgets.
- **ONNX Runtime** over TensorRT/OpenVINO/libtorch: best portability with optional
  per-vendor acceleration from one model format.
- **OpenCV** for the classical baseline that always runs on CPU.
- **vcpkg** for reproducible cross-platform dependency builds.
