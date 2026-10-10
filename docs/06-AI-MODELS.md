# 06 — AI Models

All models are free/open-source, converted to **ONNX**, and bundled locally for
offline use. Inference runs via **ONNX Runtime** (CPU default; optional GPU EPs).

## 1. Selection criteria
- Permissive/compatible license (avoid non-commercial-only where it conflicts with
  our distribution; flag anything restrictive).
- Available or convertible to ONNX.
- Has a **lightweight variant** that can run on CPU, plus a **full variant** for GPU.
- Good quality/perf for realtime video (not just stills).

## 2. Candidate models per task

| Task | Lightweight (CPU) | Full (GPU) | License notes |
|------|-------------------|-----------|---------------|
| Super-resolution | ESPCN, FSRCNN | Real-ESRGAN (x2/x4), SwinIR | Real-ESRGAN BSD-3; ESRGAN/BasicSR Apache-2.0 — verify per weights |
| Denoise | classical (NLM/bilateral) | NAFNet, SCUNet, Restormer | NAFNet MIT; SCUNet — check; verify weights |
| Low-light / relight | Zero-DCE, SCI | RetinexNet, URetinex | Zero-DCE non-commercial in places — **verify**; use classical fallback |
| Background matting | MODNet (small) | RobustVideoMatting (RVM) | MODNet Apache-2.0 (code); RVM GPL-3.0 — **flag**, prefer MODNet |
| Face restore | GPEN (small), RestoreFormer | GFPGAN, CodeFormer | GFPGAN/CodeFormer have **non-commercial** terms — **flag**; may ship opt-in only |
| Face/landmark detect | YuNet (OpenCV Zoo) | — | YuNet MIT-like (OpenCV Zoo) |
| Frame interpolation | — | RIFE, FILM | RIFE MIT (code), weights vary — **verify** |

> **License caution:** several popular face/enhancement models (GFPGAN, CodeFormer,
> parts of Zero-DCE, RVM GPL) carry non-commercial or copyleft terms. Before bundling
> any weights, confirm the license permits our distribution. Where a model is
> restrictive, make it an **optional, user-downloaded** component rather than bundling
> it, and always provide a permissive/classical fallback so the app is fully usable
> out of the box.

## 3. Default model loadout per preset
| Preset | Super-res | Denoise | Relight | Background | Face | Interp |
|--------|-----------|---------|---------|-----------|------|--------|
| Lite | — (Lanczos) | classical | classical | — | — | — |
| Balanced | ESPCN/FSRCNN x2 | classical or small AI | classical (+Zero-DCE if licensed) | MODNet (blur) | — | — |
| Studio | Real-ESRGAN | NAFNet | learned | MODNet/RVM (replace) | GPEN/GFPGAN* | RIFE* |

\* Gated by license clearance + GPU availability; opt-in.

## 4. Inference design (`OnnxModel` wrapper)
- One wrapper class per model: owns an `Ort::Session`, input/output names, and
  pre/post-processing (resize, normalize, NCHW packing, dtype).
- **Execution provider selection:** try preferred GPU EP (DirectML/CUDA/CoreML),
  fall back to CPU automatically; expose manual override in settings.
- **Tiling:** for super-res on high resolutions, process in overlapping tiles to bound
  memory and allow partial/low-latency updates.
- **Reduced-rate inference:** heavy models (background, face) can run every Nth frame
  with result reuse/tracking between runs to save CPU.
- **Warm-up:** run a dummy inference at load to avoid first-frame stalls.
- **Thread control:** cap intra-op threads so inference doesn't starve capture/output.

## 5. Model storage & integrity
- Bundled under `models/` with a `LICENSES/` subfolder containing each model's license.
- A `models/manifest.json` lists: name, task, file, sha256, input size, license, source.
- On load, verify file hash against the manifest (integrity + tamper check).
- Optional models are downloaded on explicit user action to a user-writable models dir;
  the app runs fully offline with only the bundled permissive set.

## 6. Performance expectations (to be measured — see [08-PERFORMANCE.md](08-PERFORMANCE.md))
- CPU-only realtime is realistic for: classical denoise/relight, ESPCN/FSRCNN super-res
  at modest scale, YuNet detection, small MODNet.
- GPU strongly recommended for: Real-ESRGAN, NAFNet, RVM, GFPGAN/CodeFormer, RIFE.
- The adaptive scheduler disables models that blow the latency budget.

## 7. Conversion & tooling (build-time, not shipped in app)
- Convert PyTorch → ONNX with `torch.onnx.export`; simplify with `onnxsim`.
- Validate numerics (compare ONNX vs source on sample frames).
- Optionally quantize (INT8/FP16) CPU models via ONNX Runtime tools for speed.
- Document exact conversion commands per model in `models/CONVERT.md` (added when a
  model is integrated).

## 8. Action items before bundling any weights
- [ ] Confirm license for each chosen weight file; record in `manifest.json`.
- [ ] Replace restrictive models with permissive alternatives or make opt-in.
- [ ] Add classical fallback for every AI stage (guaranteed-available path).
