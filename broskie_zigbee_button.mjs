import * as m from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from 'zigbee-herdsman-converters/lib/exposes';

const e = exposes.presets;

const buttonActions = {
    exposes: [
        e.action(['single', 'double', 'hold', 'release']),
        e.numeric('battery_voltage', exposes.access.STATE)
            .withUnit('mV')
            .withDescription('Battery voltage'),
    ],

    fromZigbee: [
        // Gesture commands used by both the ESP and nRF52840 firmware.
        {
            cluster: 'genOnOff',
            type: ['commandToggle', 'commandOn'],
            convert: (model, msg) => {
                if (msg.type === 'commandToggle') {
                    return {action: 'single'};
                }

                if (msg.type === 'commandOn') {
                    return {action: 'double'};
                }
            },
        },
        {
            cluster: 'genLevelCtrl',
            type: ['commandMove', 'commandStop', 'commandMoveToLevel'],
            convert: (model, msg) => {
                if (msg.type === 'commandMove') {
                    return {action: 'hold'};
                }

                if (msg.type === 'commandStop') {
                    return {action: 'release'};
                }

                // Compatibility with the ESP firmware's custom voltage
                // telemetry command.
                if (msg.type === 'commandMoveToLevel') {
                    return {
                        battery_voltage: msg.data.transtime,
                    };
                }
            },
        },
        {
            // Standard Zigbee Power Configuration battery voltage, reported
            // by the nRF52840 firmware in units of 100 mV.
            cluster: 'genPowerCfg',
            type: ['attributeReport', 'readResponse'],
            convert: (model, msg) => {
                if (msg.data.batteryVoltage !== undefined) {
                    return {
                        battery_voltage: msg.data.batteryVoltage * 100,
                    };
                }
            },
        },
    ],

    isModernExtend: true,
};

export default {
    zigbeeModel: ['XIAO-Zigbee-Button'],
    model: 'XIAO-Zigbee-Button',
    vendor: 'Broskie Applications',
    description: 'Zigbee single button',
    extend: [
        buttonActions,
        m.battery(),
    ],
};
