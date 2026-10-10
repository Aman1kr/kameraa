# Third-Party Notices

Kameraa uses the third-party components below. Verify and comply with each
license before distributing a build. This file must be reviewed and completed as
a release gate (see docs/10-BUILD-AND-PACKAGING.md §8).

## Libraries

| Component | Purpose | License | Notes |
|-----------|---------|---------|-------|
| Qt 6 | GUI | LGPL-3.0 / commercial | Link dynamically to satisfy LGPL. |
| OpenCV 4.x | Imaging | Apache-2.0 | Permissive. |
| ONNX Runtime | AI inference (optional) | MIT | Optional (`KAMERAA_ENABLE_ONNX`). |
| nlohmann/json | Config JSON | MIT | Permissive. |
| spdlog | Logging | MIT | Permissive. |
| GoogleTest | Tests only | BSD-3-Clause | Not shipped in the app. |

## Runtime / platform dependencies (not linked into the binary)

| Component | Platform | License | Notes |
|-----------|----------|---------|-------|
| v4l2loopback | Linux | GPL-2.0 | Kernel module the user installs; invoked via module/ioctls, not linked. |
| DirectShow Base Classes (strmbase) | Windows | MS sample license | Needed to build the filter DLL (`virtualcam/`). |
| CoreMediaIO / SystemExtensions | macOS | Apple SDK | System frameworks. |

## AI models (NOT bundled by default)

Model weights are downloaded/added by the user. Each must be license-cleared
before bundling. Several popular models carry non-commercial or copyleft terms —
prefer permissive alternatives or ship them as opt-in downloads with a classical
fallback (see docs/06-AI-MODELS.md).

| Model | Task | License status |
|-------|------|----------------|
| ESPCN / FSRCNN | Super-resolution | Verify per weights (often permissive). |
| Real-ESRGAN | Super-resolution | BSD-3 (code); verify weights. |
| MODNet | Background matting | Apache-2.0 (code); verify weights. |
| RobustVideoMatting | Background matting | GPL-3.0 — **avoid bundling**; opt-in only. |
| GFPGAN / CodeFormer | Face restore | **Non-commercial terms** — opt-in only. |
| YuNet (OpenCV Zoo) | Face detection | MIT-style. |
| RIFE | Frame interpolation | MIT (code); verify weights. |

## Release checklist
- [ ] Qt linked dynamically.
- [ ] All bundled model licenses recorded in `models/manifest.json` and here.
- [ ] GPL/copyleft components documented; none statically linked into the app binary.
- [ ] This file shipped with the build.
