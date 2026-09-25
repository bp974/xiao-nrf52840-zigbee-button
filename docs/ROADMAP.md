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

- [x] Add exact millivolt battery reporting through `genBasic` attribute
      `0xFF01` while retaining the standard 100 mV Zigbee voltage field.
- [x] Keep the final nRF converter independent from the ESP-specific voltage
      converter.
- [x] Validate sleepy wake responsiveness and single/double/hold/release
      actions in overnight testing.

## Sleepy end-device work

- [x] Enable sleepy end-device behavior before Zigbee startup.
- [x] Notify the Zigbee app-utils layer on button activity for wake/rejoin
      handling.
- [x] Evaluate battery-divider shutdown. Keep P0.14 low because disabling the
      divider can expose P0.31 to VBAT and exceed its 3.6 V input limit.
- [x] Confirm button GPIO wake from sleep.
- [x] Confirm network state persists across sleep and reset.
- [x] Confirm a button wake does not force a rejoin.
- [ ] Measure awake, joined-idle, sleepy-idle, and transmit current.

## Remote behavior (low priority)

- [ ] Replace direct coordinator addressing with Zigbee binding or group
      addressing where appropriate. This is a nice-to-have and should not
      block the current implementation.
- [ ] Add the second physical button.
- [ ] Define final single, double, and long-press actions for both buttons.
- [ ] Add exact Z2M/Home Assistant action semantics if standard commands are
      insufficient.

## Battery enhancements

- [ ] Calibrate ADC divider measurements against a multimeter.
- [ ] Confirm battery percentage curve for the selected cell chemistry.
- [x] Set periodic battery sampling/reporting to every 6 hours.
- [x] Keep six-hour reporting as the final battery policy; button-triggered
      measurements are intentionally not required.
- [ ] Configure standard Zigbee battery reporting with an appropriate minimum
      interval, maximum interval, and percentage-change threshold.
- [x] Add exact millivolt telemetry through a `genBasic` `0xFF01` attribute,
      not the Level Control command.

## Explicitly out of scope for the current milestone

- OTA/FOTA.
- Battery charging control.
- ESP firmware support in the final nRF converter.
- Additional buttons beyond the planned second button.
