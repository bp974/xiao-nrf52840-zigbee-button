#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

find_ncs_root() {
    if [[ -n "${NCS_ROOT:-}" && -d "${NCS_ROOT}/nrf/samples/zigbee/light_switch" ]]; then
        printf '%s\n' "$NCS_ROOT"
        return 0
    fi

    for candidate in \
        /opt/nordic/ncs/v2.9.3 \
        "$HOME/ncs/v2.9.3" \
        "$HOME/nrfconnect/v2.9.3"
    do
        if [[ -d "$candidate/nrf/samples/zigbee/light_switch" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    return 1
}

NCS="$(find_ncs_root || true)"

if [[ -z "$NCS" ]]; then
    echo "Could not find NCS 2.9.3."
    echo
    echo "Run again with:"
    echo "  NCS_ROOT=/path/to/ncs/v2.9.3 ./scripts/bootstrap.sh"
    exit 1
fi

SOURCE="$NCS/nrf/samples/zigbee/light_switch"
DEST="$ROOT/app"

echo "NCS root: $NCS"
echo "Copying Nordic light_switch sample into:"
echo "  $DEST"

rm -rf "$DEST"
cp -R "$SOURCE" "$DEST"

cat > "$ROOT/.ncs-root" <<EOF
$NCS
EOF

echo
echo "Bootstrap complete."
echo "Next:"
echo "  ./scripts/build.sh"
