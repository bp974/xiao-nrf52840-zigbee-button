# XIAO nRF52840 Zigbee Button

Working Zigbee baseline for the non-Sense Seeed XIAO nRF52840 using:

- nRF Connect SDK 2.6.0
- Zephyr board `xiao_ble`
- a small standalone Zigbee button application

This application currently detects one button's single and double presses. It
keeps the known-good commissioning, USB logging, and persistent Zigbee state as
a stable starting point. The detected actions are logged locally until a
generic Zigbee action representation is selected.

## Build

Use an NCS 2.6.0 shell, then run:

```bash
./scripts/build.sh
```

The output is written to `build/`. The UF2 image is
`build/zephyr/zephyr.uf2`.

Equivalent command:

```bash
west build -p always -b xiao_ble . -d build
```

## Flash

Put the XIAO into its UF2 bootloader and copy `build/zephyr/zephyr.uf2` to the
mounted XIAO volume.

On a host where the UF2 volume is mounted at `/Volumes/XIAO-SENSE`, the helper
can do the copy:

```bash
./scripts/flash.sh /Volumes/XIAO-SENSE
```

Pass the actual mount point for a non-macOS host or a differently named volume.

## Flash layout

`pm_static.yml` is intentionally checked in. It preserves:

- SoftDevice: `0x00000–0x26fff`
- application: starts at `0x27000`
- ZBOSS NVRAM and product configuration: before `0xf4000`
- UF2 bootloader: `0xf4000–0xfffff`

Do not replace this partition map without verifying the generated partitions
and ELF address.

## XIAO button

Connect one momentary button between XIAO D1 and GND. The overlay enables the
nRF52840 internal pull-up, so the button is active low:

| Function | XIAO pin | MCU pin |
|---|---|---|
| Button 1 | D1 | P0.03 |

The same overlay selects `timer2` for the Zigbee timer. The extra crypto and
MPSL settings are in `prj.conf`.

The button actions are sent using the standard Zigbee clusters expected by the
project's Zigbee2MQTT converter:

| Gesture | Zigbee command | Converter action |
|---|---|---|
| Single press | On/Off Toggle | `single` |
| Double press | On/Off On | `double` |
| Long press | Level Control Move Up | `hold` |
| Release after long press | Level Control Stop | `release` |

The button backend is interrupt-driven; debounce, gesture timing, and command
submission are asynchronous so they do not block the Zigbee stack.

See `docs/NEXT_STEPS.md` for the planned low-power two-button evolution.

## Repository layout

This is a standalone Zephyr application. The Nordic sample source needed for
the current baseline is maintained in `src/` and `include/`; no generated
`app/` directory or copy/bootstrap step is required.
