# 10 — Build & Packaging

## 1. Build (developer)
Prerequisites: CMake ≥3.21, a C++20 toolchain, vcpkg, Qt 6, and the OS SDK.

```bash
# clone vcpkg and bootstrap (first time)
# then, in repo root:
cmake --preset <os-preset>        # configures with vcpkg toolchain
cmake --build --preset <os-preset> --config Release
```

Dependencies are declared in `vcpkg.json` (OpenCV, ONNX Runtime, nlohmann-json,
spdlog, gtest/catch2). Qt may come from vcpkg or an official Qt install (configurable).

## 2. CMake structure
- Root `CMakeLists.txt` defines the project, options, and adds `src/` subdirectories.
- Each module (`core`, `capture`, `processing`, `output`, `platform`, `gui`, `config`)
  is a static library target; `app` links them into the executable.
- `CMakePresets.json` provides per-OS configure/build presets and toolchain wiring.
- Options: `KAMERAA_ENABLE_CUDA`, `KAMERAA_ENABLE_DIRECTML`, `KAMERAA_ENABLE_COREML`,
  `KAMERAA_BUILD_TESTS`.

## 3. Packaging per OS

### Windows
- Installer (e.g. WiX / Inno Setup / NSIS) that:
  - installs the app + Qt runtime + ONNX Runtime + models,
  - registers the **DirectShow virtual-camera DLL** (admin step),
  - optionally installs the MF virtual camera on Win11.
- Code-sign the binaries/installer (Authenticode) to avoid SmartScreen warnings
  (requires a code-signing certificate).

### Linux
- Package as AppImage / Flatpak / `.deb` + `.rpm`.
- **Does not bundle** the `v4l2loopback` kernel module; instead:
  - declares it as a dependency / documents install (`v4l2loopback-dkms`),
  - ships a helper to load it (`modprobe v4l2loopback ... card_label="Kameraa" exclusive_caps=1`).
- Note GPL-2.0 of the module (runtime dependency, not linked).

### macOS
- `.app` bundle embedding the **Camera System Extension**.
- Requires an **Apple Developer ID**: code-sign app + extension, enable System
  Extension + App Groups entitlements, and **notarize** the app for distribution.
- App must be in `/Applications` to activate the extension; first run prompts the user
  to approve it in System Settings.
- Distribute as a signed/notarized `.dmg`. (Local dev builds can use developer mode +
  self-signing without a paid account.)

## 4. Models in packaging
- Bundle only **permissively licensed** models under `models/` with `LICENSES/` and
  `manifest.json` (sha256 verified at load).
- License-restricted/optional models are **downloaded on explicit user action**
  post-install; the app is fully functional offline with the bundled set.

## 5. Signing & trust summary
| OS | Signing need | Consequence if unsigned |
|----|--------------|-------------------------|
| Windows | Authenticode (recommended) | SmartScreen warnings |
| macOS | Developer ID + notarization (**required** for extension distribution) | Extension won't load for end users |
| Linux | Module signing only if Secure Boot | `modprobe` may be blocked |

## 6. CI (planned)
- Build matrix: Windows/MSVC, Linux/GCC+Clang, macOS/Clang.
- Steps: configure (vcpkg cache) → build → unit tests (headless pipeline) →
  clang-format/clang-tidy checks → package artifacts.
- Keep heavy GPU/integration tests on an optional/manual lane.

## 7. Versioning & releases
- Semantic versioning; changelog per release.
- Release artifacts: Windows installer, Linux AppImage/deb/rpm, macOS dmg, plus
  `THIRD_PARTY.md` and `LICENSES/`.

## 8. Third-party license compliance (release gate)
- [ ] Qt dynamically linked (LGPL).
- [ ] All bundled model licenses cleared and listed.
- [ ] GPL components (v4l2loopback dependency, any FFmpeg/GStreamer GPL) documented.
- [ ] `THIRD_PARTY.md` generated and shipped.
