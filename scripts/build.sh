#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if ! command -v west >/dev/null 2>&1; then
    echo "'west' is not available in this terminal."
    echo "Open an nRF Connect SDK 2.6.0 terminal and try again."
    exit 1
fi

echo "Building XIAO nRF52840 Zigbee button..."
echo

# CMake caches the absolute source path. Remove a generated build directory
# left behind after the repository itself has been renamed or moved.
if [[ -f "$ROOT/build/CMakeCache.txt" ]]; then
    CACHED_SOURCE="$(sed -n 's/^CMAKE_HOME_DIRECTORY:INTERNAL=//p' \
        "$ROOT/build/CMakeCache.txt")"
    if [[ -n "$CACHED_SOURCE" && "$CACHED_SOURCE" != "$ROOT" ]]; then
        echo "Removing build cache from a different source path: $CACHED_SOURCE"
        rm -rf "$ROOT/build"
    fi
fi

BUILD_OPTIONS=()
if [[ "${DEBUG_LOGGING:-0}" == "1" ]]; then
    BUILD_OPTIONS+=("-DOVERLAY_CONFIG=prj_debug.conf")
    echo "Debug UART logging enabled."
fi

west build --no-sysbuild \
    "$ROOT" \
    -b xiao_ble \
    -d "$ROOT/build" \
    -p always \
    "${BUILD_OPTIONS[@]}"

echo
echo "Build complete."
echo

if [[ -f "$ROOT/build/zephyr/zephyr.uf2" ]]; then
    file "$ROOT/build/zephyr/zephyr.uf2" || true
    echo
    echo "UF2:"
    echo "  $ROOT/build/zephyr/zephyr.uf2"
else
    echo "WARNING: zephyr.uf2 was not found."
fi
