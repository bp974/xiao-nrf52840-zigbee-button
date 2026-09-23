#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ ! -d "$ROOT/app" ]]; then
    echo "app/ does not exist. Run:"
    echo "  ./scripts/bootstrap.sh"
    exit 1
fi

if ! command -v west >/dev/null 2>&1; then
    echo "'west' is not available in this terminal."
    echo "Open an nRF Connect SDK 2.9.3 terminal and try again."
    exit 1
fi

echo "Building XIAO nRF52840 Zigbee light_switch..."
echo

west build --no-sysbuild \
    "$ROOT/app" \
    -b xiao_ble \
    -d "$ROOT/build" \
    -p auto \
    -- \
    -DDTC_OVERLAY_FILE="$ROOT/boards/xiao_ble.overlay"

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
