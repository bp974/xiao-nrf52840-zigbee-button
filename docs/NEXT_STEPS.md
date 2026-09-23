# Planned evolution after first successful join

Do these in order. Avoid changing several variables at once.

## Phase 1 — hardware sanity

- Build stock Nordic `light_switch` for `xiao_ble`.
- Flash UF2.
- Confirm boot/logging.
- Confirm Button 1 interrupt works.
- Pair with Zigbee2MQTT.
- Confirm the device remains joined across reset.

## Phase 2 — sleepy operation

Use the stock sample's Button 3 boot behavior first.

Measure current in:
- awake/commissioning
- joined idle
- sleepy idle
- button wake + transmit

Do not add battery reporting until sleepy behavior is stable.

## Phase 3 — replace DK semantics

Convert the application to two logical buttons:

- Button A: D1 / P0.03
- Button B: D2 / P0.28

Desired actions:
- single
- double
- long press

Expose generic Zigbee actions rather than hard-coding a specific target light.

## Phase 4 — automatic sleepy end device

Remove the requirement to hold Button 3 during first join.

The device should:
1. boot,
2. restore its Zigbee network state,
3. remain a sleepy end device,
4. wake on GPIO,
5. transmit the action,
6. return to low-power idle.

The important goal is **not** to factory-reset/rejoin on every wake.

## Phase 5 — battery reporting

Add battery measurement/reporting only after power behavior is known-good.

Potential implementation:
- LiPo voltage measurement through ADC
- Zigbee Power Configuration cluster
- conservative reporting interval
- optional report after button event

## Phase 6 — final hardware

Only D1 and D2 need buttons in the final design.
D3/D4 are scaffolding for the Nordic sample and can be removed once the application no longer depends on the DK interface.
