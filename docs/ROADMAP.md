# XIAO Zigbee Button Roadmap

This document records the intended evolution of the nRF52840 application. Keep
completed work checked and add new decisions here so the project does not rely
on conversation history.

## Completed

- [x] Standalone Zephyr application structure.
- [x] NCS 2.6.0 / `xiao_ble` baseline.
- [x] XIAO-safe static flash layout with application start at `0x27000`.
- [x] XIAO Zigbee timer selection using `timer2`.
- [x] Crypto and MPSL configuration required by the XIAO.
- [x] Persistent Zigbee network state and commissioning/rejoin.
- [x] One-button GPIO input on D1 / P0.03.
- [x] Asynchronous debounce and single/double/hold/release detection.
- [x] Zigbee2MQTT actions:
  - single -> On/Off Toggle
  - double -> On
  - hold -> Level Control Move Up
  - release -> Level Control Stop
- [x] Standard Power Configuration cluster.
- [x] XIAO battery ADC measurement using P0.31/AIN7 and P0.14 divider enable.
- [x] Standard battery percentage reporting.
- [x] Broskie Zigbee2MQTT converter with `Broskie Applications` vendor identity.

## Current follow-up

- [ ] Confirm standard `genPowerCfg` reports are visible in Z2M after reconfigure.
- [ ] Add exact battery voltage reporting in millivolts while retaining the
      standard 100 mV Zigbee voltage field.
- [ ] Keep the final nRF converter independent from the ESP-specific converter.

## Sleepy end-device work

- [x] Enable sleepy end-device behavior before Zigbee startup.
- [x] Notify the Zigbee app-utils layer on button activity for wake/rejoin
      handling.
- [ ] Move the first battery sample/report to a wake-aware flow.
- [ ] Disable or minimize the battery-divider current during sleep if safe for
      the XIAO hardware.
- [ ] Confirm button GPIO wake from sleep.
- [ ] Confirm network state persists across sleep and reset.
- [ ] Confirm a button wake does not force a rejoin.
- [ ] Measure awake, joined-idle, sleepy-idle, and transmit current.

## Remote behavior

- [ ] Replace direct coordinator addressing with Zigbee binding or group
      addressing where appropriate.
- [ ] Add the second physical button.
- [ ] Define final single, double, and long-press actions for both buttons.
- [ ] Add exact Z2M/Home Assistant action semantics if standard commands are
      insufficient.

## Battery enhancements

- [ ] Calibrate ADC divider measurements against a multimeter.
- [ ] Confirm battery percentage curve for the selected cell chemistry.
- [ ] Replace the current 15-minute diagnostic interval with a sleepy-device
      reporting policy:
  - report after startup, join, or rejoin;
  - measure on button interaction when the last measurement is older than
    approximately one hour;
  - report periodically every 6–12 hours;
  - report sooner when the battery percentage drops meaningfully.
- [ ] Configure standard Zigbee battery reporting with an appropriate minimum
      interval, maximum interval, and percentage-change threshold.
- [ ] Add exact millivolt telemetry through a clean manufacturer-specific
      attribute or cluster, not the Level Control command.

## Explicitly out of scope for the current milestone

- OTA/FOTA.
- Battery charging control.
- ESP firmware support in the final nRF converter.
- Additional buttons beyond the planned second button.
