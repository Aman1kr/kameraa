# 08 — Performance

## 1. Targets (recap from requirements)
- Balanced preset: **≥30fps @ 1080p output** on a mid-range CPU-only laptop.
- Lite preset: **≥60fps** on low-end CPU-only laptops.
- Added end-to-end latency: **< ~150ms** (Balanced).
- Never block: hold a smooth output stream via adaptive degradation.

## 2. What limits performance
- Per-stage compute (AI models dominate).
- Color-space conversions and memory copies between stages.
- Capture/driver latency and output (virtual cam) push cost.
- Single-threaded stalls blocking capture/output.

## 3. Preset definitions (performance-oriented)
| Preset | In → Out | Enabled (default) | Target device |
|--------|----------|-------------------|---------------|
| Lite | 720p → 720/1080p | classical denoise, relight, sharpen | low-end CPU |
| Balanced | 720/1080p → 1080p | + light AI super-res, bg blur | mid CPU / any GPU |
| Studio | 1080p → 1080/4K | + AI denoise, face restore, bg replace, interp | GPU |

## 4. Optimization strategy
1. **Minimize copies/conversions:** agree on a single working format; convert only at
   ingest and before output; process in-place where possible.
2. **Right-size work:** run heavy models at reduced internal resolution, upscale at the
   end; tile super-res.
3. **Reduced-rate inference:** background/face models every Nth frame + tracking.
4. **Threading:** decouple capture, processing, output; keep only latest frames.
5. **Model optimization:** FP16/INT8 quantization for CPU; pick lightweight variants.
6. **Warm-up** models at startup to avoid first-frame spikes.
7. **Thread caps:** bound ONNX intra-op threads so inference doesn't starve I/O threads.
8. **GPU offload** when available via ONNX Runtime EPs.

## 5. Adaptive degradation (enforced by scheduler)
See [04-PIPELINE-DESIGN.md](04-PIPELINE-DESIGN.md) §6. Order when over budget:
lower working resolution → skip lowest-priority optional stage → drop frames. Restore
when headroom returns. The output thread repeats the last frame on underrun so the
stream never freezes.

## 6. Benchmark plan
Measure and record (per stage + end-to-end) on a device matrix:

| Device class | Example | Expectation |
|--------------|---------|-------------|
| Low-end CPU | dual/quad-core, no dGPU | Lite 720p ≥ target |
| Mid CPU | modern 6–8 core, iGPU | Balanced 1080p ≥ 30fps |
| NVIDIA GPU | GTX/RTX | Studio usable |
| Apple Silicon | M1/M2/M3 | Balanced–Studio via CoreML |

Metrics captured: per-stage ms, total latency, output fps, dropped-frame %, CPU%, GPU%,
RAM. Use the headless pipeline (file source) + a fixed clip for repeatable numbers.

## 7. Instrumentation
- Lightweight timing around each stage (steady_clock), rolling averages.
- Expose fps/latency/dropped-frames/CPU-GPU in the GUI (FR-U5).
- Optional CSV export of per-stage timings for tuning (local only).

## 8. Acceptance thresholds (fill with measured data)
| Metric | Lite | Balanced | Studio |
|--------|------|----------|--------|
| Output fps | ≥60 | ≥30 | ≥24 |
| Added latency | <80ms | <150ms | <250ms |
| Dropped frames (steady) | <1% | <5% | <10% |

> These are initial targets; replace with measured results during P2–P5 and track
> regressions in CI where feasible.
