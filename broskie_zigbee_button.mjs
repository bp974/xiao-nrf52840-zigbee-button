import * as m from 'zigbee-herdsman-converters/lib/modernExtend';
import * as exposes from 'zigbee-herdsman-converters/lib/exposes';
import * as reporting from 'zigbee-herdsman-converters/lib/reporting';

const e = exposes.presets;

const buttonActions = {
    exposes: [
        e.action(['single', 'double', 'hold', 'release']),
        e.numeric('battery_voltage', exposes.access.STATE)
            .withUnit('mV')
            .withDescription('Battery voltage'),
        e.numeric('voltage', exposes.access.STATE)
            .withUnit('mV')
            .withDescription('Standard Zigbee battery voltage'),
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
            type: ['commandMove', 'commandStop'],
            convert: (model, msg) => {
                if (msg.type === 'commandMove') {
                    return {action: 'hold'};
                }

                if (msg.type === 'commandStop') {
                    return {action: 'release'};
                }

            },
        },
        {
            // Standard Zigbee Power Configuration battery voltage, reported
            // by the nRF52840 firmware in units of 100 mV.
            cluster: 'genPowerCfg',
            type: ['attributeReport', 'readResponse'],
            convert: (model, msg) => {
                const batteryVoltage = msg.data.batteryVoltage ??
                    msg.data.battery_voltage;

                if (batteryVoltage !== undefined && batteryVoltage < 255) {
                    const millivolts = batteryVoltage * 100;

                    return {
                        voltage: millivolts,
                    };
                }
            },
        },
        {
            cluster: 'genBasic',
            type: ['attributeReport', 'readResponse'],
            convert: (model, msg) => {
                const exactVoltage = msg.data['65281'] ?? msg.data[65281];

                if (typeof exactVoltage === 'number' && exactVoltage > 0) {
                    return {battery_voltage: exactVoltage};
                }
            },
        },
    ],

    configure: [
        async (device, coordinatorEndpoint) => {
            const endpoint = device.getEndpoint(10);
            const reportingConfiguration = [{
                // Unknown attributes must include their ZCL data type so
                // zigbee-herdsman can build the configure-reporting request.
                attribute: {ID: 0xFF01, type: 0x21}, // uint16
                minimumReportInterval: 0,
                maximumReportInterval: 21600,
                reportableChange: 10,
            }];

            await reporting.bind(endpoint, coordinatorEndpoint, ['genBasic']);
            await endpoint.configureReporting('genBasic', reportingConfiguration);
            await endpoint.read('genBasic', [0xFF01]);
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
