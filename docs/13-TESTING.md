# 13 — Testing

## 1. Strategy
A layered approach: fast unit tests on pure filters, integration tests on the headless
pipeline, and manual/device tests for the parts that need real cameras, GPUs, and call
apps. CPU-only correctness is automatable; realtime perf and virtual-cam behavior need
hardware.

## 2. Test levels

### 2.1 Unit tests (automated, CI)
- Each `IFilter` is pure (`Frame in → Frame out`) → test with fixture images.
- Assertions: output dimensions/format, parameter effects (e.g. brightness slider
  raises mean luma), no crash on edge sizes, deterministic output.
- Quality where meaningful: **PSNR/SSIM** vs golden outputs for a fixed input + params.
- Config/preset parsing: valid/invalid JSON, schema validation, round-trip save/load.
- `OnnxModel`: loads a tiny test model, correct pre/post-processing shapes, EP fallback
  to CPU when GPU absent.

### 2.2 Integration tests (automated where possible)
- **Headless pipeline** with a file/image or synthetic source → run full filter chain →
  assert output stream properties and that the adaptive scheduler holds target timing
  under an injected "slow stage".
- Backend detection logic (mocked): correct guidance when virtual-cam backend absent.
- Long-run **soak** (headless): N minutes → assert bounded memory, no leaks (valgrind/
  ASan on Linux), stable fps.

### 2.3 Manual / device tests (checklist)
- Virtual camera appears and streams in: **Teams, Google Meet, Zoom, WhatsApp Desktop,
  OBS, Discord, Chrome/Firefox WebRTC** — per OS.
- Camera disconnect/reconnect recovery.
- GPU acceleration on/off parity (same output, faster).
- First-run experience + backend-missing guidance.

## 3. Device / platform matrix
| OS | Virtual-cam backend | CPU-only | GPU |
|----|--------------------|----------|-----|
| Windows 10/11 | DirectShow (+MF on 11) | ✔ | DirectML / CUDA |
| Ubuntu/Fedora | v4l2loopback | ✔ | CUDA |
| macOS 12.3+ (Intel + Apple Silicon) | CMIOExtension | ✔ | CoreML |

## 4. Performance testing
- Fixed input clip through headless pipeline per preset; record per-stage ms, total
  latency, output fps, dropped-frame %, CPU/GPU/RAM (see [08-PERFORMANCE.md](08-PERFORMANCE.md)).
- Compare against acceptance thresholds; flag regressions.
- Run on each device class in the matrix.

## 5. Quality evaluation
- Objective: PSNR/SSIM vs golden on controlled inputs (regression safety, not "beauty").
- Subjective: before/after review on real low-light / noisy / low-res webcam samples;
  track with a small labeled sample set.
- Guard against artifacts: over-sharpening halos, upscale hallucinations, matting edge
  errors, skin-smoothing plastic look.

## 6. Security / privacy tests
- Assert **no network sockets** opened during capture/processing (monitor in test).
- Model files: hash mismatch → load refused.
- Config/model file parsing fuzzed for malformed input (no crash, safe reject).
- No frame data written to disk unless user records/snapshots.

## 7. Tooling
- GoogleTest/Catch2 for C++ tests; CTest to run in CMake/CI.
- Sanitizers (ASan/UBSan/TSan) in dedicated CI jobs.
- clang-format/clang-tidy as gates.
- Golden images + sample clips stored under `tests/data/` (small, representative).

## 8. CI gating (per [10-BUILD-AND-PACKAGING.md](10-BUILD-AND-PACKAGING.md))
- PR must pass: build (all OS) + unit + headless integration + lint/format.
- Nightly/optional: sanitizers, soak, perf benchmarks.
- Manual pre-release: full device/app matrix checklist signed off.

## 9. Definition of done (per feature)
- Unit tests for new filter(s) + config.
- Headless integration still green; scheduler holds timing.
- Manual check in at least one real call app on the relevant OS.
- No new leaks/regressions; docs updated.
