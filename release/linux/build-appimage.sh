#!/usr/bin/env bash
# Packages Kameraa as a portable AppImage. Run on Linux after:
#   scripts/build-linux.sh
# Requires: linuxdeploy + linuxdeploy-plugin-qt (downloaded automatically if absent).
set -euo pipefail

repo="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="$repo/build/linux/src/app/kameraa"
WORK="$repo/release/linux/AppDir"
OUT="$repo/release/linux/dist"
VERSION="${VERSION:-1.0.0}"

[[ -x "$BIN" ]] || { echo "Binary not found: $BIN. Build first (scripts/build-linux.sh)." >&2; exit 1; }

rm -rf "$WORK"; mkdir -p "$WORK/usr/bin" "$WORK/usr/share/applications" \
    "$WORK/usr/share/metainfo" "$WORK/usr/share/icons/hicolor/256x256/apps" \
    "$WORK/usr/share/kameraa/models" "$OUT"

cp "$BIN" "$WORK/usr/bin/kameraa"
cp "$repo/release/linux/kameraa.desktop" "$WORK/usr/share/applications/"
cp "$repo/release/linux/com.kameraa.Kameraa.metainfo.xml" "$WORK/usr/share/metainfo/"
cp "$repo/models/manifest.json" "$WORK/usr/share/kameraa/models/" 2>/dev/null || true
# Provide an icon (replace with a real one before release).
cp "$repo/release/assets/kameraa.png" \
   "$WORK/usr/share/icons/hicolor/256x256/apps/kameraa.png" 2>/dev/null || \
   echo "WARNING: release/assets/kameraa.png missing; add an icon before release." >&2

# Fetch linuxdeploy tools if not present.
tools="$repo/release/linux/.tools"; mkdir -p "$tools"
get() { [[ -x "$tools/$1" ]] || { curl -L -o "$tools/$1" "$2"; chmod +x "$tools/$1"; }; }
get linuxdeploy-x86_64.AppImage \
    https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
get linuxdeploy-plugin-qt-x86_64.AppImage \
    https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage

export OUTPUT="$OUT/Kameraa-$VERSION-x86_64.AppImage"
"$tools/linuxdeploy-x86_64.AppImage" --appdir "$WORK" --plugin qt \
    --desktop-file "$WORK/usr/share/applications/kameraa.desktop" \
    --icon-file "$WORK/usr/share/icons/hicolor/256x256/apps/kameraa.png" \
    --output appimage || {
        echo "AppImage build failed. Ensure Qt is on PATH for the qt plugin." >&2
        exit 1
    }
echo "Created $OUTPUT"
