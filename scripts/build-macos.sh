#!/usr/bin/env bash
# Configure and build Kameraa on macOS (Apple Clang) into build/macos.
# Requires: Xcode command line tools, cmake, ninja, git. Qt6/OpenCV via vcpkg.
#
# Usage:
#   scripts/build-macos.sh [--tests] [--onnx]
#
# Note: the virtual camera on macOS is a signed Camera System Extension built in
# Xcode from macos/ (needs an Apple Developer ID). This script builds the app.
set -euo pipefail

repo="$(cd "$(dirname "$0")/.." && pwd)"
cd "$repo"

TESTS=0; ONNX=0
for arg in "$@"; do
    case "$arg" in
        --tests) TESTS=1 ;;
        --onnx)  ONNX=1 ;;
        *) echo "Unknown option: $arg" >&2; exit 1 ;;
    esac
done

require() { command -v "$1" >/dev/null 2>&1 || { echo "'$1' not found. Install it (see docs/10-BUILD-AND-PACKAGING.md)." >&2; exit 1; }; }
require cmake
require git

VCPKG_ROOT="${VCPKG_ROOT:-$repo/.vcpkg}"
if [[ ! -x "$VCPKG_ROOT/vcpkg" ]]; then
    [[ -d "$VCPKG_ROOT" ]] || git clone https://github.com/microsoft/vcpkg "$VCPKG_ROOT"
    "$VCPKG_ROOT/bootstrap-vcpkg.sh"
fi
toolchain="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
echo "Using vcpkg toolchain: $toolchain"

args=(--preset macos "-DCMAKE_TOOLCHAIN_FILE=$toolchain")
features=()
if [[ $TESTS -eq 1 ]]; then args+=(-DKAMERAA_BUILD_TESTS=ON); features+=(tests); fi
if [[ $ONNX  -eq 1 ]]; then args+=(-DKAMERAA_ENABLE_ONNX=ON -DKAMERAA_ENABLE_COREML=ON); fi
if [[ ${#features[@]} -gt 0 ]]; then
    args+=("-DVCPKG_MANIFEST_FEATURES=$(IFS=';'; echo "${features[*]}")")
fi

echo "Configuring..."
cmake "${args[@]}"
echo "Building..."
cmake --build --preset macos

if [[ $TESTS -eq 1 ]]; then
    echo "Running tests..."
    ctest --test-dir build/macos --output-on-failure
fi

echo "Done. App in build/macos/src/app."
