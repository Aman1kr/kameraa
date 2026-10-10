# 12 — Risks

Scale: Likelihood (L) / Impact (I) = Low / Med / High.

| # | Risk | L | I | Mitigation |
|---|------|---|---|------------|
| R1 | **Over-promised "4K"** disappoints users (sensor can't create real detail) | High | High | Market as *enhancement/upscale*; honest UI labels + tooltips; before/after preview; set expectations in README/UI |
| R2 | **Call apps cap resolution/fps**, so "120Hz/4K" isn't transmitted | High | Med | Document clearly; focus value on clean/bright/sharp 1080p30 which is what apps accept |
| R3 | **All AI effects on CPU can't hit 30–60fps** | High | High | Presets, toggles, adaptive scheduler, reduced-rate inference, lightweight models, optional GPU |
| R4 | **macOS System Extension signing** blocks distribution (needs paid Apple acct) | High | High | Support local self-signed/dev build; document paid requirement; ship macOS last; consider community signing |
| R5 | **Windows DirectShow filter** compatibility/registration issues across apps | Med | High | Follow proven OBS-style push-source pattern; add MF virtual cam on Win11; test app matrix |
| R6 | **Linux v4l2loopback** not installed / secure-boot blocks module | Med | Med | Detect + guide install; document DKMS + module signing; graceful disabled state |
| R7 | **Model licenses** (GFPGAN/CodeFormer/RVM/Zero-DCE) restrict bundling | High | Med | Verify each; ship only permissive by default; restrictive = opt-in download; classical fallback always |
| R8 | **Latency too high** for natural calls | Med | High | Latency budget + measurement; prefer lighter models; GPU offload; drop/interpolate wisely |
| R9 | **Frame transport** (app↔filter/extension) too slow | Med | Med | Shared memory / IOSurface; zero-copy where possible; benchmark early |
| R10 | **Cross-platform maintenance burden** (3 capture + 3 output backends) | Med | Med | Strict interfaces; OpenCV capture baseline; add native backends only when needed |
| R11 | **GPU driver/EP fragmentation** (DirectML/CUDA/CoreML versions) | Med | Med | CPU always works; EP auto-detect + fallback; pin tested runtime versions |
| R12 | **Quality regressions** when toggling/degrading stages | Low | Med | Golden-image tests (PSNR/SSIM); preset snapshot tests |
| R13 | **Memory growth** over long sessions | Med | Med | Bounded buffers, RAII, leak checks, long-run soak tests |
| R14 | **Privacy expectation** broken by accidental network/model download | Low | High | No network in processing path; downloads explicit/opt-in; audit in CI |
| R15 | **Scope creep** (becomes an OBS clone) | Med | Med | Enforce non-goals in [00-VISION-AND-SCOPE.md](00-VISION-AND-SCOPE.md) |

## Top risks to watch early
- R3 (CPU performance) and R4 (macOS signing) are the biggest technical/feasibility
  risks; validate assumptions in P1–P3 before heavy investment in Studio features.
- R7 (model licensing) must be resolved before bundling any weights.
