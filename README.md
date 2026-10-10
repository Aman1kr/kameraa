# Kameraa — AI Virtual Camera Enhancer

Turn a basic laptop webcam into a clean, bright, sharp, "studio-looking" virtual
camera that you can select inside Microsoft Teams, Google Meet, Zoom, WhatsApp
Desktop, OBS, Discord, and any other app that lists system cameras.

Kameraa runs **100% locally** — no cloud, no account, no paid service, works
**offline**. Cross-platform: **Windows, Linux, macOS**.

> ### Read this first — honest expectations
> A cheap camera sensor physically cannot record detail it never captured, so **no
> software can create *true* 4K** out of a low-quality feed. What Kameraa does is
> **enhance** the image: denoise (remove graininess), relight and color-correct,
> AI **super-resolution upscaling**, background blur/replace, face restoration, and
> optional frame interpolation for smoother motion. The result looks dramatically
> better and is published at a high resolution.
>
> Also note: **video-call apps cap what they transmit** (commonly 720p/1080p at
> 30fps). So the "4K / 120Hz" quality applies to your **local pipeline and preview**
> — the remote participant sees whatever the call app allows. That ceiling is set by
> Teams/Zoom/Meet, not by Kameraa.

## What it does
- Captures your real webcam.
- Runs a configurable enhancement pipeline (classical + AI).
- Publishes the enhanced result as a **virtual camera device** the OS and apps can use.
- Live preview + sliders/toggles + quality presets (Lite / Balanced / Studio).

## Status
Early development. See [docs/11-ROADMAP.md](docs/11-ROADMAP.md) for milestones and
[docs/00-VISION-AND-SCOPE.md](docs/00-VISION-AND-SCOPE.md) for goals and non-goals.

## Documentation
| Doc | Purpose |
|-----|---------|
| [00-VISION-AND-SCOPE.md](docs/00-VISION-AND-SCOPE.md) | Goals, non-goals, realistic expectations |
| [01-REQUIREMENTS.md](docs/01-REQUIREMENTS.md) | Functional & non-functional requirements |
| [02-RESEARCH.md](docs/02-RESEARCH.md) | Virtual camera per OS, AI model survey, feasibility |
| [03-ARCHITECTURE.md](docs/03-ARCHITECTURE.md) | System design, modules, data flow |
| [04-PIPELINE-DESIGN.md](docs/04-PIPELINE-DESIGN.md) | Frame pipeline, threading, latency budgets |
| [05-PLATFORM-INTEGRATION.md](docs/05-PLATFORM-INTEGRATION.md) | Capture + virtual cam per OS |
| [06-AI-MODELS.md](docs/06-AI-MODELS.md) | Chosen models, sizes, perf, licenses |
| [07-TECH-STACK.md](docs/07-TECH-STACK.md) | Libraries, versions, build deps |
| [08-PERFORMANCE.md](docs/08-PERFORMANCE.md) | Budgets, benchmarks, preset definitions |
| [09-UI-UX.md](docs/09-UI-UX.md) | Screens, controls, states |
| [10-BUILD-AND-PACKAGING.md](docs/10-BUILD-AND-PACKAGING.md) | Build, installers, signing |
| [11-ROADMAP.md](docs/11-ROADMAP.md) | Phased milestones |
| [12-RISKS.md](docs/12-RISKS.md) | Risk register + mitigations |
| [13-TESTING.md](docs/13-TESTING.md) | Test strategy, quality metrics, device matrix |

## Tech stack (summary)
C++17/20 · CMake · Qt 6 (GUI) · OpenCV (classical imaging) · ONNX Runtime (AI) ·
per-OS virtual camera (DirectShow / v4l2loopback / CMIOExtension).

## License
TBD. Note that some dependencies (e.g. `v4l2loopback`, GPL-2.0) impose obligations —
see [docs/06-AI-MODELS.md](docs/06-AI-MODELS.md) and
[docs/07-TECH-STACK.md](docs/07-TECH-STACK.md).
