# Release

This folder holds everything needed to produce publishable builds of Kameraa for
**Windows, Linux, and macOS**, plus the runbook for cutting a release.

## Layout
```
release/
  README.md                     # this file
  RELEASE_CHECKLIST.md          # pre-publish gate
  assets/                       # icons / store imagery (add real art here)
  windows/build-installer.ps1   # windeployqt + Inno Setup installer
  linux/
    kameraa.desktop             # desktop entry
    com.kameraa.Kameraa.metainfo.xml  # AppStream metadata
    build-appimage.sh           # portable AppImage
  macos/
    Info.plist.in               # app bundle metadata (camera usage)
    build-dmg.sh                # macdeployqt + .dmg
```
Related: `packaging/` (filter registration, v4l2loopback, signing scripts),
`.github/workflows/release.yml` (automated multi-OS release).

## How releases are produced
The reliable path is **CI**: pushing a `v*` tag triggers
`.github/workflows/release.yml`, which builds all three OSes, runs tests, packages
installers, and attaches them to a GitHub Release. See `RELEASE_CHECKLIST.md`.

Local one-offs:
- Windows: `scripts\build-windows.ps1` then `release\windows\build-installer.ps1`
- Linux:   `scripts/build-linux.sh`  then `release/linux/build-appimage.sh`
- macOS:   `scripts/build-macos.sh`  then `release/macos/build-dmg.sh` then
           `packaging/macos/sign-and-notarize.sh`

## What you must supply (cannot be automated/committed)
These are the real "market-ready" gates, each requiring your own accounts/keys:
1. **Windows code-signing certificate** (EV/OV) — avoids SmartScreen warnings.
2. **Apple Developer ID + notarization** — required for the macOS app and its
   Camera System Extension to load on users' machines.
3. **License-cleared AI model files** — bundle only permissive models; keep
   restrictive ones opt-in (see `../THIRD_PARTY.md`, `../docs/06-AI-MODELS.md`).
4. **App icons / store artwork** in `assets/`.
5. **DirectShow base classes (strmbase)** to build the Windows virtual-camera DLL.
