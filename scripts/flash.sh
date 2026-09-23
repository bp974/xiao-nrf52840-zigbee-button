#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UF2="$ROOT/build/zephyr/zephyr.uf2"

if [[ $# -ne 1 ]]; then
    echo "Usage: $0 <mounted UF2 volume>" >&2
    exit 2
fi

DEST="$1"

if [[ ! -f "$UF2" ]]; then
    echo "UF2 image not found: $UF2" >&2
    echo "Run ./scripts/build.sh first." >&2
    exit 1
fi

if [[ ! -d "$DEST" ]]; then
    echo "UF2 volume is not mounted: $DEST" >&2
    exit 1
fi

if [[ "$(uname -s)" == "Darwin" ]]; then
    # UF2 volumes do not support macOS extended attributes.
    cp -X "$UF2" "$DEST/"
else
    cp "$UF2" "$DEST/"
fi
echo "Copied $UF2 to $DEST"
