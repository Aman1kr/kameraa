# 09 — UI / UX

Qt 6 (Widgets) desktop app. Goal: works instantly with defaults; power users can tune.

## 1. Primary window layout
```
+------------------------------------------------------------+
|  Kameraa                                   [_] [profile v]  |
+------------------------------+-----------------------------+
|                              |  Camera:   [ built-in  v ]  |
|                              |  Preset:   ( Lite | Bal | Studio )
|        LIVE PREVIEW          |  Output:   [1920x1080 @30 v]|
|     (processed output)       |                             |
|   [ before | after toggle ]  |  Effects                    |
|                              |   [x] Denoise      [----o-] |
|                              |   [x] Relight      [---o--] |
|                              |   [x] Sharpen      [--o---] |
|                              |   [x] Super-res    model v  |
|                              |   [x] Background   blur v    |
|                              |   [ ] Face enhance          |
|                              |   [ ] Frame interp          |
+------------------------------+-----------------------------+
|  ● Virtual camera: STOPPED     [ Start Virtual Camera ]     |
|  fps 31 | lat 120ms | CPU 55% | GPU 0%                      |
+------------------------------------------------------------+
```

## 2. Key screens / states
- **First run:** auto-select default camera + Balanced preset; show preview; detect
  virtual-cam backend.
- **Backend missing:** "Start" disabled; banner with exact install steps / button
  (install v4l2loopback / open System Settings / run installer).
- **Running:** "Start" → "Stop"; status dot green; metrics live.
- **Camera error / in use:** preview shows placeholder + message; offer retry / switch
  camera.

## 3. Controls
- **Camera picker** — enumerated devices; refresh button.
- **Preset selector** — Lite / Balanced / Studio (segmented control); changing a slider
  marks preset as "Custom".
- **Output resolution/fps** — dropdown of allowed modes.
- **Per-effect rows** — checkbox (enable) + strength slider + optional dropdown
  (model/mode). Disabled effects are pass-through.
- **Start/Stop Virtual Camera** — primary action; shows backend name.
- **Metrics bar** — fps, added latency, CPU%, GPU%, dropped-frame indicator.

## 4. Before/After preview
- Toggle or split-view to compare raw vs processed; helps users see value and tune.

## 5. Settings / secondary
- GPU acceleration: Auto / CPU only / specific EP.
- Model management: see bundled models; download optional (opt-in) models.
- Config: reset to defaults; import/export preset JSON.
- Start minimized to tray; remember last settings.

## 6. Tray (optional, FR-U7)
- Minimize to tray; quick Start/Stop; quick preset switch; quit.

## 7. UX principles
- **Immediate value:** good-looking defaults before the user touches anything.
- **Honest labels:** call it "Enhance / Upscale", not "Make 4K"; tooltip explains the
  sensor/call-app limits.
- **Non-blocking:** preview and UI never freeze; heavy work stays off the UI thread.
- **Clear recovery:** every error state tells the user exactly what to do.

## 8. Accessibility
- Keyboard navigable; sufficient contrast; scalable layout; tooltips on every control.

## 9. Visual design
- Minimal, dark-first theme; large preview is the hero; controls grouped logically.
- Icons for camera/record/settings; status color-coded (grey/green/amber/red).
