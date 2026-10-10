# Release Checklist

Work top to bottom. Do not publish until every box is checked.

## 1. Code & quality
- [ ] `main`/`master` green on CI for Windows, Linux, macOS (build + tests).
- [ ] Version bumped in `CMakeLists.txt` (`project(... VERSION x.y.z)`).
- [ ] `CHANGELOG.md` updated with the new version + date.
- [ ] No debug/temporary code; logging at sensible levels.
- [ ] Manual smoke test on each OS: capture → enhance → preview.

## 2. Virtual camera verification (per OS)
- [ ] Linux: `v4l2loopback` loads; "Kameraa" appears in Zoom/Meet/OBS/browser.
- [ ] Windows: filter DLL registered; "Kameraa Virtual Camera" selectable in
      Teams/Zoom/Meet/OBS; frames flow.
- [ ] macOS: Camera Extension installs & is approved; appears in FaceTime/Zoom.

## 3. AI features (if shipping models)
- [ ] All bundled model licenses cleared and recorded in `models/manifest.json`
      and `THIRD_PARTY.md`.
- [ ] Restrictive/non-commercial models are opt-in downloads, not bundled.
- [ ] Classical fallback verified for every AI stage.

## 4. Signing & trust
- [ ] Windows binaries + installer signed (Authenticode).
- [ ] macOS app + extension signed (Developer ID) and **notarized + stapled**.
- [ ] Linux AppImage built; checksums generated.

## 5. Packaging
- [ ] Windows installer registers/unregisters the virtual camera correctly
      (install + uninstall tested on a clean VM).
- [ ] macOS `.dmg` opens, app runs from `/Applications`, extension activates.
- [ ] Linux AppImage runs on a clean distro; `.deb` installs and recommends
      `v4l2loopback-dkms`.
- [ ] App icons and store artwork present.

## 6. Legal & docs
- [ ] `LICENSE`, `THIRD_PARTY.md` shipped with each package.
- [ ] Privacy statement accurate (local-only, no telemetry).
- [ ] README/marketing copy avoids overclaiming ("enhance/upscale", not "true 4K").

## 7. Publish
- [ ] Tag the release: `git tag v<x.y.z> && git push origin v<x.y.z>`.
- [ ] CI release job produced artifacts for all three OSes.
- [ ] GitHub Release notes written; artifacts + checksums attached.
- [ ] (Optional) Submit to stores: Microsoft Store / Mac App Store / Flathub.

## 8. Post-release
- [ ] Verify downloads install & run on clean machines.
- [ ] Monitor issues; prepare a point release if needed.
