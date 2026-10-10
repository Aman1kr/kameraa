# Changelog

All notable changes to Kameraa are documented here. Format follows
[Keep a Changelog](https://keepachangelog.com/); versioning is [SemVer](https://semver.org/).

## [1.0.0] - 2026-10-10
### Added
- Cross-platform desktop app (Windows, Linux, macOS) with live preview and controls.
- Webcam capture (OpenCV) with camera selection and a synthetic test-pattern source.
- Classical enhancement pipeline: denoise, relight/color, sharpen — with Lite /
  Balanced / Studio presets and persisted settings.
- Adaptive scheduler that downscales work to hold the frame-rate budget.
- Virtual camera output:
  - Linux: v4l2loopback sink (working).
  - Windows: DirectShow sink (app side) + registrable filter DLL skeleton.
  - macOS: CMIOExtension sink (app side) + system-extension skeleton.
- Optional AI stages (ONNX Runtime): super-resolution, background blur/replace,
  face smoothing (with classical skin-mask fallback), and frame interpolation.
- GPU execution providers (CUDA / DirectML / CoreML) with CPU fallback.
- Packaging: CPack, Inno Setup installer, AppImage recipe, DMG script; signing/
  notarization scripts; CI build + release workflows.
- Documentation set (docs/00–13), THIRD_PARTY notices, and an About dialog.

### Known limitations
- Enhancement improves perceived quality but does not reconstruct true 4K detail;
  video-call apps cap transmitted resolution/fps.
- Windows/macOS virtual cameras require their signed native component built and
  installed (DirectShow DLL needs the DirectShow base classes; macOS extension
  needs an Apple Developer ID). The app detects and guides installation.
- AI features require license-cleared model files, which are not bundled.

[1.0.0]: https://github.com/kameraa/kameraa/releases/tag/v1.0.0
