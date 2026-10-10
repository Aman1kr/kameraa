#!/usr/bin/env bash
# Configure and build Kameraa on Linux (GCC/Clang) into build/linux.
# Requires: cmake, ninja, git, a C++20 compiler. Qt6/OpenCV come via vcpkg.
#
# Usage:
#   scripts/build-linux.sh [--tests] [--onnx]
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

# Bootstrap vcpkg if VCPKG_ROOT is not set.
VCPKG_ROOT="${VCPKG_ROOT:-$repo/.vcpkg}"
if [[ ! -x "$VCPKG_ROOT/vcpkg" ]]; then
    [[ -d "$VCPKG_ROOT" ]] || git clone https://github.com/microsoft/vcpkg "$VCPKG_ROOT"
    "$VCPKG_ROOT/bootstrap-vcpkg.sh"
fi
toolchain="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
echo "Using vcpkg toolchain: $toolchain"

args=(--preset linux "-DCMAKE_TOOLCHAIN_FILE=$toolchain")
features=()
if [[ $TESTS -eq 1 ]]; then args+=(-DKAMERAA_BUILD_TESTS=ON); features+=(tests); fi
if [[ $ONNX  -eq 1 ]]; then args+=(-DKAMERAA_ENABLE_ONNX=ON); fi
if [[ ${#features[@]} -gt 0 ]]; then
    args+=("-DVCPKG_MANIFEST_FEATURES=$(IFS=';'; echo "${features[*]}")")
fi

echo "Configuring..."
cmake "${args[@]}"
echo "Building..."
cmake --build --preset linux

if [[ $TESTS -eq 1 ]]; then
    echo "Running tests..."
    ctest --test-dir build/linux --output-on-failure
fi

echo "Done. Binary in build/linux/src/app."
echo "Tip: run scripts' Linux helper to enable the virtual camera:"
echo "  sudo packaging/linux/install-v4l2loopback.sh"
