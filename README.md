# XIAO nRF52840 Zigbee Button

Working Zigbee baseline for the non-Sense Seeed XIAO nRF52840 using:

- nRF Connect SDK 2.6.0
- Zephyr board `xiao_ble`
- Nordic's Zigbee light-switch application logic, maintained locally

This cleanup does not implement the final two-button remote behavior. It keeps
the known-good commissioning, USB logging, button aliases, and persistent
Zigbee state as a stable starting point.

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

## XIAO buttons

The board overlay maps the Nordic sample's four logical buttons:

| Logical button | XIAO pin | MCU pin |
|---|---|---|
| Button 1 | D1 | P0.03 |
| Button 2 | D2 | P0.28 |
| Button 3 / sleepy enable | D3 | P0.29 |
| Button 4 / factory reset | D4 | P0.04 |

The same overlay selects `timer2` for the Zigbee timer. The extra crypto and
MPSL settings are in `prj.conf`.

See `docs/NEXT_STEPS.md` for the planned low-power two-button evolution.

## Repository layout

This is a standalone Zephyr application. The Nordic sample source needed for
the current baseline is maintained in `src/` and `include/`; no generated
`app/` directory or copy/bootstrap step is required.
