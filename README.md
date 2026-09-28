# XIAO nRF52840 Zigbee Button

Battery-powered Zigbee button for the Seeed XIAO nRF52840 using:

- nRF Connect SDK 2.6.0
- Zephyr board `xiao_ble`
- a standalone Zephyr Zigbee application
- Zigbee2MQTT and Home Assistant integration

The application supports single-button and three-button hardware variants with
single, double, hold, and release gestures. It operates as a sleepy Zigbee end
device, preserves network state, reports battery percentage, and reports exact
battery voltage in millivolts.

## Build

Use an NCS 2.6.0 shell, then run:

```bash
./scripts/build.sh
```

By default this builds the single-button firmware. Build the three-button
firmware with:

```bash
BUTTON_VARIANT=3 ./scripts/build.sh
```

Production builds disable the UART console and application logging. To build a
development image with verbose serial logging enabled, run:

```bash
DEBUG_LOGGING=1 ./scripts/build.sh
```

For a three-button debug image, combine the options:

```bash
BUTTON_VARIANT=3 DEBUG_LOGGING=1 ./scripts/build.sh
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

Connect a momentary button between D1 and GND for the single-button variant.
For the three-button variant, connect buttons between each listed pin and GND.
The overlay enables the nRF52840 internal pull-ups, so the buttons are active
low:

| Function | XIAO pin | MCU pin |
|---|---|---|
| Button 1 | D1 | P0.03 |
| Button 2 | D2 | P0.28 |
| Button 3 | D3 | P0.29 |

The same overlay selects `timer2` for the Zigbee timer. The extra crypto and
MPSL settings are in `prj.conf`.

The button actions are sent using the standard Zigbee clusters expected by the
project's nRF-only Zigbee2MQTT converter
(`zigbee2mqtt/broskie_zigbee_button.mjs`):

| Variant | Gesture | Zigbee endpoint | Converter action |
|---|---:|---|---|
| Single | Single | 10 | `single` |
| Single | Double | 10 | `double` |
| Single | Long press | 10 | `hold` |
| Single | Long-press release | 10 | `release` |
| Three-button | Button 1 single | 10 | `button_1_single` |
| Three-button | Button 2 single | 11 | `button_2_single` |
| Three-button | Button 3 single | 12 | `button_3_single` |
| Three-button | Button N double | 10/11/12 | `button_N_double` |
| Three-button | Button N long press | 10/11/12 | `button_N_hold` |
| Three-button | Button N release | 10/11/12 | `button_N_release` |

The single-button firmware uses model ID `XIAO-Zigbee-Button`. The three-button
firmware uses model ID `XIAO-Zigbee-3Button` and exposes one Zigbee endpoint per
button: 10, 11, and 12. Separate model IDs let Zigbee2MQTT expose only the
actions that exist on each hardware variant.

The button backend is interrupt-driven; debounce, gesture timing, and command
submission are asynchronous so they do not block the Zigbee stack.

## Battery reporting

The XIAO divider is enabled safely through P0.14 and sampled every six hours.
The standard Power Configuration cluster provides battery percentage and coarse
100 mV voltage. Basic cluster attribute `0xFF01` provides the exact measured
voltage in millivolts for Zigbee2MQTT and Home Assistant.

The converter exposes the exact value as the diagnostic `voltage` entity.

Copy the converter to Zigbee2MQTT's `external_converters` directory, or
configure the repository path directly:

```yaml
external_converters:
  - zigbee2mqtt/broskie_zigbee_button.mjs
```

## Home Assistant blueprints

The repository includes separate blueprints for each firmware variant:

- `home-assistant/blueprints/broskie-zigbee-button.yaml` for the single-button firmware
- `home-assistant/blueprints/broskie-zigbee-3button.yaml` for the three-button firmware

Copy the appropriate YAML file into Home Assistant's
`config/blueprints/automation/` directory, reload automations, and create an
automation from the imported blueprint.

## Sleepy operation

The device sleeps when the Zigbee stack is idle and wakes from the button GPIO.
The Zigbee connection and persistent network state remain intact across sleep;
button gestures are processed after wake without requiring a rejoin.

See `docs/ROADMAP.md` for current decisions and future work.

## Repository layout

This is a standalone Zephyr application. Source code is maintained in `src/`
and `include/`; no generated `app/` directory or copy/bootstrap step is
required.
