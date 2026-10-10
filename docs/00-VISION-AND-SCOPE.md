# 00 — Vision & Scope

## 1. Vision
Give anyone with a cheap, ordinary laptop webcam a noticeably better on-camera
presence — cleaner, sharper, well-lit, "studio-like" — in any video application,
without buying new hardware, without a subscription, and without sending a single
frame to the cloud.

## 2. Problem statement
Built-in laptop cameras are typically low resolution, noisy in poor light, flat in
color, and unflattering. Existing fixes are either hardware (expensive), cloud
services (privacy + cost + online-only), or locked into a single app. There is no
free, local, cross-platform tool that enhances the feed and exposes it to *every*
app via a standard virtual camera.

## 3. Goals
- G1 — Capture a real webcam and output an **enhanced virtual camera** selectable in
  Teams, Meet, Zoom, WhatsApp Desktop, OBS, Discord, browsers, etc.
- G2 — Enhancements: denoise, relight + color correction, AI super-resolution
  upscaling, background blur/replace, face restoration/beautify, frame interpolation.
- G3 — Run **locally and offline**; no cloud, no account, no paid tiers.
- G4 — Cross-platform: Windows, Linux, macOS.
- G5 — Usable on **CPU-only basic laptops** at a usable framerate, with **optional
  GPU acceleration** when available.
- G6 — Simple desktop GUI: pick camera, live preview, toggle effects, choose preset.
- G7 — Deliver a **balanced** experience: good quality at a smooth 30–60fps.

## 4. Non-goals (explicitly out of scope)
- NG1 — Manufacturing *true* 4K detail from a low-res sensor. We enhance/upscale; we
  do not fabricate real optical detail.
- NG2 — Forcing call apps to transmit 4K/120fps. Remote quality is limited by the
  call app, not by Kameraa.
- NG3 — Cloud processing, remote model hosting, or telemetry.
- NG4 — Mobile (iOS/Android) in v1.
- NG5 — A full OBS-style scene compositor / streaming suite. We focus on camera
  enhancement, not production switching.
- NG6 — Audio processing (this is a video tool). May be considered later.

## 5. Target users
- Remote workers / students on video calls with weak built-in cameras.
- Streamers / content creators wanting a cheap quality bump.
- Privacy-conscious users who refuse cloud "beauty cam" apps.

## 6. Success criteria
- SC1 — Virtual camera appears and works in at least Teams, Meet, Zoom, OBS on all
  three OSes.
- SC2 — On a mid-range CPU-only laptop, "Balanced" preset sustains ≥30fps at 1080p
  output with denoise + relight + light upscale enabled.
- SC3 — Visible, user-confirmed quality improvement vs raw feed (noise ↓, sharpness
  ↑, exposure/color ↑).
- SC4 — End-to-end latency low enough to feel natural on calls (target < ~150ms
  added latency; see [08-PERFORMANCE.md](08-PERFORMANCE.md)).
- SC5 — Fully functional with no internet connection.

## 7. Key constraints & honest caveats
- C1 — **Sensor limit:** output quality is bounded by what the camera actually
  captures. Enhancement ≠ real new detail.
- C2 — **Call-app caps:** most apps negotiate ≤1080p30. "120Hz / 4K" is a
  local-pipeline capability, surfaced in preview and to apps that accept it.
- C3 — **Compute reality:** enabling *all* AI effects at once on a CPU-only laptop
  will not hold 30–60fps. Mitigated by presets, toggles, dynamic scaling, optional GPU.
- C4 — **macOS distribution:** the virtual camera is a System Extension that must be
  code-signed; public distribution needs an Apple Developer account. Local/self-built
  use can run with developer mode / self-signing.
- C5 — **Linux dependency:** `v4l2loopback` is a GPL-2.0 kernel module the user must
  have installed/loadable.

## 8. Guiding principles
- Local-first & private by default.
- Honest about capabilities; never oversell "4K".
- Degrade gracefully: always keep a smooth passthrough if effects are too heavy.
- Modular: every effect is an optional, independently testable filter.
