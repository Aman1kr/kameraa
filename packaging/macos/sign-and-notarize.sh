#!/usr/bin/env bash
# Codesigns and notarizes the Kameraa macOS app bundle (which embeds the Camera
# System Extension). Requires an Apple Developer ID. See docs/10-BUILD-AND-PACKAGING.md.
#
# Usage:
#   APP=/path/to/Kameraa.app \
#   DEV_ID="Developer ID Application: Your Name (TEAMID)" \
#   KEYCHAIN_PROFILE=notary-profile \
#   ./sign-and-notarize.sh
set -euo pipefail

: "${APP:?Set APP=/path/to/Kameraa.app}"
: "${DEV_ID:?Set DEV_ID to your Developer ID Application identity}"
: "${KEYCHAIN_PROFILE:?Set KEYCHAIN_PROFILE (stored via notarytool store-credentials)}"

ENTITLEMENTS="$(dirname "$0")/../../macos/Kameraa.entitlements"

echo "Signing extension and app (hardened runtime)..."
# Sign the embedded system extension first, then the app, inside-out.
find "$APP/Contents/Library/SystemExtensions" -name "*.systemextension" -maxdepth 2 \
    -exec codesign --force --timestamp --options runtime \
        --entitlements "$ENTITLEMENTS" --sign "$DEV_ID" {} \; 2>/dev/null || true

codesign --force --timestamp --options runtime \
    --entitlements "$ENTITLEMENTS" --sign "$DEV_ID" "$APP"

echo "Creating zip for notarization..."
ZIP="${APP%.app}.zip"
ditto -c -k --keepParent "$APP" "$ZIP"

echo "Submitting to Apple notary service..."
xcrun notarytool submit "$ZIP" --keychain-profile "$KEYCHAIN_PROFILE" --wait

echo "Stapling ticket..."
xcrun stapler staple "$APP"
echo "Done. $APP is signed, notarized, and stapled."
