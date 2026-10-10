#!/usr/bin/env bash
# Builds a distributable Kameraa.app (with Qt deployed) and packages a .dmg.
# Run on macOS after: scripts/build-macos.sh
# Requires: macdeployqt (from Qt), hdiutil. Signing/notarization is a separate
# step — see packaging/macos/sign-and-notarize.sh.
set -euo pipefail

repo="$(cd "$(dirname "$0")/../.." && pwd)"
APP="$repo/build/macos/src/app/kameraa.app"
OUT="$repo/release/macos/dist"
VERSION="${VERSION:-1.0.0}"

[[ -d "$APP" ]] || { echo "App not found: $APP. Build first (scripts/build-macos.sh)." >&2; exit 1; }

mkdir -p "$OUT"
cp -R "$APP" "$OUT/Kameraa.app"

# Bundle Qt frameworks/plugins into the .app.
if command -v macdeployqt >/dev/null; then
    macdeployqt "$OUT/Kameraa.app" -always-overwrite
else
    echo "WARNING: macdeployqt not found; Qt libraries will not be bundled." >&2
fi

# Copy the model manifest (model binaries are added/downloaded by the user).
mkdir -p "$OUT/Kameraa.app/Contents/Resources/models"
cp "$repo/models/manifest.json" "$OUT/Kameraa.app/Contents/Resources/models/" 2>/dev/null || true

DMG="$OUT/Kameraa-$VERSION.dmg"
rm -f "$DMG"
hdiutil create -volname "Kameraa" -srcfolder "$OUT/Kameraa.app" -ov -format UDZO "$DMG"
echo "Created $DMG"
echo "Next: sign + notarize via packaging/macos/sign-and-notarize.sh"
