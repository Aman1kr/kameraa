# 01 — Requirements

Priority: **M** = Must, **S** = Should, **C** = Could, **W** = Won't (this release).

## 1. Functional requirements

### 1.1 Capture
- FR-C1 (M) — Enumerate available physical cameras and let the user pick one.
- FR-C2 (M) — Capture frames from the selected camera at its native resolution/fps.
- FR-C3 (M) — Let the user select input resolution/fps from supported modes.
- FR-C4 (S) — Gracefully handle camera disconnect / in-use-by-another-app errors.
- FR-C5 (C) — Allow a static image / test pattern as input source (for testing).

### 1.2 Enhancement pipeline
- FR-E1 (M) — Denoise / degrain (classical + optional AI).
- FR-E2 (M) — Auto relight + exposure + white balance + color correction, with manual
  override sliders (brightness, contrast, saturation, warmth, gamma).
- FR-E3 (M) — Sharpening.
- FR-E4 (S) — AI super-resolution upscaling to a higher output resolution.
- FR-E5 (S) — Background blur (bokeh) and background replacement (image).
- FR-E6 (S) — Face restoration + beautify (smooth skin, enhance eyes), region-gated.
- FR-E7 (C) — Frame interpolation for higher/ smoother output fps.
- FR-E8 (M) — Every effect is independently toggleable and ordered deterministically.
- FR-E9 (M) — Quality presets: Lite / Balanced / Studio, each defining which effects
  run and at what model/resolution tier.
- FR-E10 (S) — Per-effect strength controls.

### 1.3 Virtual camera output
- FR-O1 (M) — Publish the processed frames as a system virtual camera.
- FR-O2 (M) — The virtual camera is selectable in Teams, Meet, Zoom, WhatsApp, OBS,
  Discord, browsers.
- FR-O3 (M) — Output a configurable resolution/fps (subject to performance).
- FR-O4 (S) — Show a friendly "service unavailable" frame when the pipeline stalls.

### 1.4 GUI
- FR-U1 (M) — Live preview of the processed output.
- FR-U2 (S) — Optional side-by-side before/after preview.
- FR-U3 (M) — Camera picker, preset picker, per-effect toggles + sliders.
- FR-U4 (M) — Start/Stop virtual camera button and clear running status.
- FR-U5 (S) — Live FPS / latency / CPU-GPU usage readout.
- FR-U6 (S) — Persist settings between runs (JSON config).
- FR-U7 (C) — System tray minimize + quick toggle.

### 1.5 Platform / setup
- FR-P1 (M) — Detect whether the virtual-camera backend is installed; guide the user
  to install it (v4l2loopback / system extension / DirectShow filter registration).
- FR-P2 (S) — Optional GPU acceleration auto-detected, with manual override.
- FR-P3 (M) — Run fully offline; bundle all models locally.

## 2. Non-functional requirements

### 2.1 Performance
- NFR-P1 (M) — "Balanced" preset ≥30fps at 1080p on a mid-range CPU-only laptop.
- NFR-P2 (S) — "Lite" preset ≥60fps at 720–1080p on low-end CPU-only laptops.
- NFR-P3 (M) — Added end-to-end latency target < ~150ms (Balanced).
- NFR-P4 (M) — Never fully block: if a stage is too slow, drop frames / downscale /
  skip optional stages to preserve realtime output.

### 2.2 Portability
- NFR-PO1 (M) — Single C++ codebase builds on Windows, Linux, macOS via CMake.
- NFR-PO2 (M) — OS-specific code isolated behind interfaces (capture, output, accel).

### 2.3 Privacy & security
- NFR-S1 (M) — No network calls for processing; no telemetry. (Optional, explicit,
  user-initiated model download only; app is fully usable without it.)
- NFR-S2 (M) — No frame data written to disk unless the user explicitly records/snapshots.
- NFR-S3 (M) — Validate all external inputs (camera formats, config files, model files).

### 2.4 Usability
- NFR-U1 (M) — First-run works with sensible defaults (pick default cam, Balanced preset).
- NFR-U2 (S) — Clear, actionable error messages (esp. virtual-cam backend missing).

### 2.5 Reliability
- NFR-R1 (S) — Recover automatically from camera disconnect/reconnect.
- NFR-R2 (S) — No memory growth over long sessions (bounded buffers, no leaks).

### 2.6 Maintainability
- NFR-M1 (M) — Each effect is a self-contained filter implementing a common interface.
- NFR-M2 (S) — Unit-testable processing stages decoupled from capture/output.

## 3. Assumptions
- Users can install a kernel module (Linux) / approve a system extension (macOS) /
  allow driver registration (Windows) — with admin rights where needed.
- Target machines are x86-64 or Apple Silicon; 64-bit OS; ≥4GB RAM.

## 4. Traceability
Requirements map to modules in [03-ARCHITECTURE.md](03-ARCHITECTURE.md) and to
milestones in [11-ROADMAP.md](11-ROADMAP.md).
